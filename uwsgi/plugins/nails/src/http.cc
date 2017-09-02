#include <unordered_map>
#include <string>

#include "common.h"
#include "http.h"
#include "uwsgi-includer.h"


bool parseCookieHeader(std::string const& cookie_str, ParamMap& cookies) {
  auto begin = cookie_str.cbegin();
  size_t start = 0, eq = 0, end = 0;  
  do {
    puts("loop");
    start = cookie_str.find_first_not_of(' ', end);
    printf("start: %lu\n", start);
    if (start == std::string::npos) {
      return true;
    }
    end = cookie_str.find_first_of(';', start);
    printf("end: %lu\n", end);
    if (end == std::string::npos) {
      end = cookie_str.size();
    }

    eq = cookie_str.find_first_of('=', start);
    printf("eq: %lu\n", eq);
    if (eq == std::string::npos) {
      return false;
    }
    if (eq > end) {
      return false;
    }

    puts(decodeURIComponentPlus(begin+start, begin+eq).c_str());
    puts(decodeURIComponentPlus(begin+eq+1, begin+end).c_str());

    cookies.emplace(
      std::make_pair(
        decodeURIComponentPlus(begin+start, begin+eq),
        decodeURIComponentPlus(begin+eq+1, begin+end)
      )
    );
    end += 1;
  } while (end < cookie_str.size());


  return true;
}

std::tuple<std::string,std::string,std::string>
parseContentDisposition(std::string const& header) {
  //Content-Disposition: form-data; name="build"; filename="tea"
  //Content-Disposition: file; filename="file1.txt"

  auto start = header.cbegin()+sizeof("Content-Disposition: ")-1;
  size_t semicolon = header.find(";", start-header.cbegin());
  if (semicolon == std::string::npos) {
    return std::make_tuple("", "", "");
  }
  std::string dispo{&*start, &*(header.cbegin()+semicolon)};
  if (dispo == "form-data") {
    size_t name_eq = header.find("; name=\"", semicolon);
    if (name_eq == std::string::npos) {
      return std::make_tuple("", "", "");
    }
    name_eq += sizeof("; name=\"")-1;
    size_t name_end = header.find("\"", name_eq);
    if (name_end == std::string::npos) {
      return std::make_tuple("", "", "");
    }
    std::string name{header, name_eq, name_end-name_eq};
    size_t filename_eq = header.find("; filename=\"", name_end);
    if (filename_eq == std::string::npos) {
      return std::make_tuple(std::move(dispo), std::move(name), "");
    }
    filename_eq += sizeof("; filename=\"")-1;
    size_t filename_end = header.find("\"", filename_eq);
    if (filename_end == std::string::npos) {
      return std::make_tuple(std::move(dispo), std::move(name), "");
    }
    return std::make_tuple(std::move(dispo), std::move(name),
                           std::string{header, filename_eq,
                                       filename_end-filename_eq});
  } else if (dispo == "file") {
    size_t filename_eq = header.find("filename=\"", semicolon) + sizeof("filename=\"")-1;
    if (filename_eq == std::string::npos) {
      return std::make_tuple("", "", "");
    }
    size_t filename_end = header.find("\"", filename_eq);
    return std::make_tuple(std::move(dispo), "",
                           std::string{header, filename_eq,
                                       filename_end-filename_eq});
  } else {
    return std::make_tuple("","","");
  }
  return std::make_tuple("", "", "");
}




enum class MultipartParserState {
  PreBoundary,
  Boundary,
  BoundaryHaveCR,
  Header,
  HeaderHaveCR,
  Data
};

enum class MultipartParserHeader {
  Disposition,
  Type,
  TransferEncoding,
  CRLF
};

enum class MultipartParserMode {
  None,
  Form,
  File,
  Mixed
};


enum class MultipartParserFileState {
  NotCreated,
  Created
};

enum class MultipartParserFormState {
  New,
  Existing
};

bool parseMultipartBody(struct wsgi_request *wsgi_req, std::string& _ctboundary,
                        ParamMap& forms, FileMap& files) {

  size_t bytes_read = 0;
  size_t bytes_left = wsgi_req->post_cl;
  char const* body = nullptr;
  std::string chunk{};
  size_t form_total = 0;

  char const* current = nullptr;
  char const* cr = nullptr;
  char const* lf = nullptr;
  uint64_t colon_off = 0;
  char const* end = nullptr;

  MultipartParserState state = MultipartParserState::Boundary;
  MultipartParserHeader header_type = MultipartParserHeader::Disposition;
  MultipartParserMode mode = MultipartParserMode::None;
  MultipartParserFileState file_state = MultipartParserFileState::NotCreated;
  MultipartParserFormState form_state = MultipartParserFormState::New;

  std::string firstctboundary = "--" + _ctboundary;
  std::string ctboundary_end = firstctboundary + "--";

  std::string ctboundary = "\r\n--" + _ctboundary;
  uint64_t const firstctboundary_len = firstctboundary.size();
  uint64_t const ctboundary_len = ctboundary.size();
  uint64_t const ctboundary_end_len = ctboundary_end.size();

  std::tuple<std::string, std::string, std::string> header_data{"", "", ""};

  std::string boundary, header_holder, header, header_name,
              dispo, formname, filename, content_type, encoding,
              filepath,
              data, mixed_boundary;

  FILE* fp = nullptr;
  char preboundary_byte = '\0';
  uint8_t preboundary_off = 0;

  do {
    bytes_read = 0;
    body = uwsgi_request_body_read(wsgi_req,
                                   CHUNK_SIZE < bytes_left ?
                                     CHUNK_SIZE : bytes_left,
                                   (ssize_t*)&bytes_read);
    if (bytes_read == 0) {
      return false;
    }

    bytes_left -= bytes_read;
    current = body;
    end = &(body[bytes_read]);

    while (current < end) {
      debug_puts("outer");
      switch (state) {

        case MultipartParserState::PreBoundary: {
          debug_puts("MultipartParserState::PreBoundary");
          switch (preboundary_off) {
            case 0: {
              debug_puts("0");
              if (bytes_read > 1) {
                if (current[0] == '-' && current[1] == '-') {
                  if (bytes_left == 0) {
                    return true;
                  } else {
                    return false;
                  }
                } else if (current[0] == '\r' && current[1] == '\n') {
                  current += 2;
                  state = MultipartParserState::Boundary;
                }
              } else {
                preboundary_byte = *current;
                preboundary_off = 1;
                current += 1;
              }
              break;
            }
            case 1: {
              debug_puts("1");
              if (preboundary_byte == '-' && *current == '-') {
                if (bytes_left == 0) {
                    return true;
                  } else {
                    return false;
                  }
                } else if (preboundary_byte == '\r' && *current == '\n') {
                  current += 1;
                  state = MultipartParserState::Boundary;
                }
              break;
            }
          }
          break;
        }

        case MultipartParserState::Boundary: {
          debug_puts("MultipartParserState::Boundary");
          cr = (char const*)memchr(current, '\r', end-current);
          if (cr == nullptr) {
            boundary.append(current, end);
            current = end;
          } else {
            boundary.append(current, cr);
            current = cr+1;
            state = MultipartParserState::BoundaryHaveCR;
          }
          break;
        }

        case MultipartParserState::BoundaryHaveCR: {
          debug_puts("MultipartParserState::BoundaryHaveCR");
          debug_printf("boundary       : %s\n", boundary.c_str());
          debug_printf("firstctboundary: %s\n", firstctboundary.c_str());
          if (*current != '\n') {
            debug_puts("*current != '\n'");
            return false;
          }
          if (boundary.size() == firstctboundary_len) {
            debug_puts("boundary.size() == ctboundary_len");
            if (boundary != firstctboundary) {
              return false;
            }
            //
            current += 1;
            state = MultipartParserState::Header;
          } else if (boundary.size() == ctboundary_end_len) {
            debug_puts("boundary.size() == ctboundary_end_len");
            if (boundary != ctboundary_end) {
              return false;
            } else {
              return true;
            }
          } else {
            debug_printf("boundary.size(): %lu\n", boundary.size());
            debug_printf("ctboundary_len: %lu\n", ctboundary_len);            
            debug_printf("ctboundary_end_len: %lu\n", ctboundary_end_len);

            debug_puts("else");
            return false;
          }
          boundary = "";
          break;
        }

        case MultipartParserState::Header: {
          debug_puts("MultipartParserState::Header");
          cr = (char const*)memchr(current, '\r', end-current);
          if (cr == nullptr) {
            header_holder.append(current, end);
            current = end;
          } else {
            header = std::move(header_holder);
            header.append(current, cr);
            header_holder = "";
            current = cr+1;
            state = MultipartParserState::HeaderHaveCR;
          }
          break;
        }

        case MultipartParserState::HeaderHaveCR: {
          debug_puts("MultipartParserState::HeaderHaveCR");
          if (*current != '\n') {
            debug_puts("*current != '\\n'");
            return false;
          }
          debug_puts(header.c_str());
          if (header == "") {
            debug_puts("CRLF");
            if (header_type == MultipartParserHeader::Disposition) {
              debug_puts("incorrect state");
              return false;
            }
            debug_puts("pass");
            //setup state
            header_type = MultipartParserHeader::Disposition;
            state = MultipartParserState::Data;
          } else {
            colon_off = header.find(':');
            if (colon_off == std::string::npos) {
              return false;
            }
            header_name.assign(header, 0, colon_off);
            std::transform(header_name.begin(), header_name.end(),
                           header_name.begin(), std::tolower);
            if (header_name == "content-disposition") {
              debug_puts("Disposition");
              if (header_type != MultipartParserHeader::Disposition) {
                debug_puts("invalid state");
                return false;
              }
              std::tie(dispo, formname, filename) =
                  parseContentDisposition(header);
              if (dispo == "") {
                debug_puts("blank");
                return false;
              }
              if (filename != "") {
                debug_printf("filename: %s\n", filename.c_str());
                mode = MultipartParserMode::File;
              } else {
                mode = MultipartParserMode::Form;
              }
              header_type = MultipartParserHeader::Type;

            } else if (header_name == "content-type") {
              debug_puts("Type");
              if (header_type != MultipartParserHeader::Type) {
                return false;
              }

              content_type.assign(header, colon_off+2,
                                  std::string::npos);
              if (startsWith(content_type, "multipart/mixed; boundary=")) {
                mixed_boundary.assign(content_type,
                                      sizeof("multipart/mixed; boundary=")-1,
                                      std::string::npos);
                if (mixed_boundary.empty()) {
                  return false;
                }
                mode = MultipartParserMode::Mixed;
              }
              header_type = MultipartParserHeader::TransferEncoding;

            } else if (header_name == "content-transfer-encoding") {
              debug_puts("TransferEncoding");
              if (header_type != MultipartParserHeader::TransferEncoding) {
                return false;
              }
              encoding.assign(header, colon_off+1, header.size() - colon_off);
              header_type = MultipartParserHeader::CRLF;
            } else {
              debug_puts("else");
              return false;
            }
            state = MultipartParserState::Header;
          }
          current += 1;
          break;
        }
        case MultipartParserState::Data: {
          debug_puts("Data");

          switch (mode) {
            case MultipartParserMode::None: {
              debug_puts("MultipartParserMode::None");
              return false;
              break;
            }
            case MultipartParserMode::Form: {
              debug_puts("MultipartParserMode::Form");
              switch (form_state) {
                case MultipartParserFormState::New: {
                  debug_puts("MultipartParserFormState::New");
                  forms[formname] = "";
                  form_state = MultipartParserFormState::Existing;
                  break;
                }
                case MultipartParserFormState::Existing: {
                  debug_puts("MultipartParserFormState::Existing");
                  std::string& form = forms[formname];
                  size_t oldsz = chunk.size();
                  chunk.append(current, end-current);
                  size_t bound_off = chunk.find(ctboundary);
                  if (bound_off == std::string::npos) {
                    if (chunk.size() > ctboundary.size()) {
                      form.append(chunk, 0, chunk.size()-ctboundary.size());
                      form_total += chunk.size()-ctboundary.size();
                      chunk = std::string{chunk,
                                          chunk.size() - ctboundary.size(),
                                          std::string::npos};
                    }
                    current = end;
                  } else {
                    form.append(chunk, 0, bound_off);
                    form_total += form.size();
                    char const* nul = (char const*)memchr(form.data(), '\0',
                                                          form.size()
                    );
                    if (nul != nullptr) {
                      form.resize(nul - form.data());
                    }
                    
                    size_t remaining = chunk.size() - bound_off;
                    if ( (size_t)(end - current) >= remaining) {
                      current = end - remaining + 2;
                      debug_printf("%s\n", current);
                    } else {
                      if (remaining > firstctboundary_len) {
                        boundary.assign(chunk, bound_off+2, firstctboundary_len);
                        current += firstctboundary_len + bound_off+2 - oldsz;
                      } else {
                        boundary.assign(chunk, bound_off+2, std::string::npos);
                        current = end;
                      }
                    }
                    chunk = "";
                    form_state = MultipartParserFormState::New;
                    mode = MultipartParserMode::None;
                    state = MultipartParserState::Boundary;
                  }
                  break;
                }
              }
              break;
            }
            case MultipartParserMode::File: {
              debug_puts("MultipartParserMode::File");
              switch (file_state) {
                case MultipartParserFileState::NotCreated: {
                  debug_puts("MultipartParserFileState::NotCreated");
                  debug_puts(filename.c_str());

                  auto search = files.find(formname);
                  if(search != files.end()) {
                    return false; //nice try
                  }

                  char tmp[] = "/tmp/nails_upload_XXXXXX";
                  int fd = mkstemp(tmp);
                  if (fd == -1) {
                    return false;
                  }

                  filepath.assign(tmp);

                  debug_puts(filepath.c_str());
                  fp = fdopen(fd, "ab");

                  files[formname] = {{filename, content_type}, filepath};


                  file_state = MultipartParserFileState::Created;
                  break;
                }
                case MultipartParserFileState::Created: {
                  debug_puts("MultipartParserFileState::Created");
                  size_t oldsz = chunk.size();
                  chunk.append(current, end-current);
                  size_t bound_off = chunk.find(ctboundary);
                  if (bound_off == std::string::npos) {
                    if (chunk.size() > ctboundary.size()) {
                      fwrite(chunk.data(), 1,
                             chunk.size() - ctboundary.size(),
                             fp);
                      chunk = std::string{chunk,
                                          chunk.size() - ctboundary.size(),
                                          std::string::npos};
                    }
                    current = end;
                  } else {
                    fwrite(chunk.data(), 1,
                           bound_off,
                           fp);
                    fclose(fp);
                    fp = nullptr;


                    size_t remaining = chunk.size() - bound_off;
                    if ( (size_t)(end - current) >= remaining) {
                      current = end - remaining + 2;
                      debug_printf("%s\n", current);
                    } else {
                      if (remaining > firstctboundary_len) {
                        boundary.assign(chunk, bound_off+2, firstctboundary_len);
                        current += firstctboundary_len + bound_off+2 - oldsz;
                      } else {
                        boundary.assign(chunk, bound_off+2, std::string::npos);
                        current = end;
                      }
                    }

                    chunk = "";
                    file_state = MultipartParserFileState::NotCreated;
                    mode = MultipartParserMode::None;
                    state = MultipartParserState::Boundary;
                  }
                  break;
                }
              }
              break;
            }
            case MultipartParserMode::Mixed: {
              debug_puts("MultipartParserMode::Mixed");
              //"Note: In particular, this means that multiple files submitted as part of a single <input type=file multiple> element will result in each file having its own field; the 'sets of files' feature ('multipart/mixed') of RFC 2388 is not used."
              //https://www.w3.org/TR/html5/forms.html#multipart-form-data
              return false;
              break;
            }

          }
          //hmm, but we are going until we hit the boundary;
          //pobably best to use the boundary state
          break;
        }
      }
    }
  

  } while (bytes_left > 0);
  return false;
}











enum class URLDecodeParserState {
  Normal,
  PartialValue, //implies we have the full name
  PartialName
};

void parseURLEncodedBody(struct wsgi_request *wsgi_req,
                         ParamMap& forms) {

  size_t bytes_read = 0;
  size_t bytes_left = wsgi_req->post_cl;
  char const* body = nullptr;

  char const* current = nullptr;
  char const* amp = nullptr;
  char const* eq = nullptr;
  char const* end = nullptr;

  URLDecodeParserState state = URLDecodeParserState::Normal;
  std::string name_holder, name, value_holder, value;

  do { //the only thing that should be "weird" about this implementation is
       //that it doesn't validate things like `foo=hello=bar&yo=dawg` and
       //will yield `{"foo": "hello=bar", "yo": "dawg"}`. It will also truncate
       //at %00 or any other invalid % encoded value (e.g. `%ZZ`) rather than
       //error out.
    bytes_read = 0; //uwsgi_request_body_read just increments it

    //note: this reuses/reallocs the same buffer internally,
    //      e.g. wsgi_req->post_read_buf
    body = uwsgi_request_body_read(wsgi_req,
                                   CHUNK_SIZE < bytes_left ?
                                     CHUNK_SIZE : bytes_left,
                                   (ssize_t*)&bytes_read);
    if (bytes_read == 0) {
      return;
    }

    bytes_left -= bytes_read;
    current = body;
    end = &(body[bytes_read]);

    //note: this could be optimized by doing multiple bytes searches at once
    //for all tokens of the state
    while (current < end) {
      if (memchr(current, '\0', end-current) != nullptr) {
        return;
      }
      switch (state) {
        case URLDecodeParserState::Normal: {
          //if there is an & in there, we can safely parse the stuff before it
          amp = (char const*)memchr(current, '&', end-current);
          if (amp == nullptr) {
            //we can try to look for an = though
            eq = (char const*)memchr(current, '=', end-current);
            if (eq == nullptr) {
              //they are toying with us now
              name_holder.assign(current, end);
              state = URLDecodeParserState::PartialName;
            } else {
              name = decodeURIComponentPlus(current, eq);
              value_holder.assign(eq+1, end);
              state = URLDecodeParserState::PartialValue;
            }
            current = end;
          } else {
            eq = (char const*)memchr(current, '=', amp-current);
            if (eq == nullptr) {
              forms.emplace(std::make_pair(decodeURIComponentPlus(current, amp), ""));
            } else {
              forms.emplace(std::make_pair(decodeURIComponentPlus(current, eq),
                                           decodeURIComponentPlus(eq+1, amp)));
            }
            current = amp+1;
            state = URLDecodeParserState::Normal;
          }
          break;
        }
        case URLDecodeParserState::PartialValue: {
          amp = (char const*)memchr(current, '&', end-current);
          if (amp == nullptr) {
            value_holder += std::string{current, end};
            current = end;
          } else {
            if (!value_holder.empty()) {
              value_holder += std::string{current, amp};
              value = decodeURIComponentPlus(value_holder.begin(),
                                      value_holder.end());
              forms.emplace(std::make_pair(std::move(name),
                                           std::move(value)));
            } else {
              forms.emplace(std::make_pair(std::move(name),
                                           decodeURIComponentPlus(current, amp))
              );
            }
            state = URLDecodeParserState::Normal;
            current = amp+1;
          }
          break;
        }
        case URLDecodeParserState::PartialName: {
          amp = (char const*)memchr(current, '&', end-current);
          if (amp == nullptr) {
            eq = (char const*)memchr(current, '=', end-current);
            if (eq == nullptr) {
              name_holder += std::string{current, end};
            } else {
              name_holder += std::string{current, eq};
              name = decodeURIComponentPlus(name_holder.begin(), name_holder.end());
              value_holder.assign(eq+1, end);
              state = URLDecodeParserState::PartialValue;
            }
            current = end;
          } else {
            eq = (char const*)memchr(current, '=', amp-current);
            if (eq == nullptr) {
              name_holder += std::string{current, end};
              forms.emplace(
                std::make_pair(decodeURIComponentPlus(name_holder.begin(),
                                               name_holder.end()),
                               "")
              );
              state = URLDecodeParserState::Normal;
              current  = end;
            } else {
              name_holder += std::string{current, eq};
              name = decodeURIComponentPlus(name_holder.begin(), name_holder.end());
              forms.emplace(std::make_pair(std::move(name),
                                           decodeURIComponentPlus(current, amp))
              );
              state = URLDecodeParserState::Normal;
              current = amp+1;
            }
          }
          break;
        }
      }
    }
  } while (bytes_left > 0); //(total_read < wsgi_req->post_cl);

  if (state == URLDecodeParserState::PartialName) {
    forms.emplace(std::make_pair(
        decodeURIComponentPlus(name_holder.begin(),name_holder.end()),
        ""
    ));
  } else if (state == URLDecodeParserState::PartialValue) {
    forms.emplace(std::make_pair(
        name,
        decodeURIComponentPlus(value_holder.begin(),value_holder.end())
    ));
  }
  //return ret;
}
