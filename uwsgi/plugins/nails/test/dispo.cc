#include <tuple>
#include <string>
#include <stdio.h>

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


int main() {
  auto t = parseContentDisposition(
    "Content-Disposition: form-data; name=\"gg\""
  );

  puts(std::get<0>(t).c_str());
  puts(std::get<1>(t).c_str());
  puts(std::get<2>(t).c_str());
  return 0;
}