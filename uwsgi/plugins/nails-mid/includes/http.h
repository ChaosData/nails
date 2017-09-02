#ifndef _HTTP_H_
#define _HTTP_H_



#include <unordered_map>
#include <string>

constexpr uint64_t MAX_NORMAL_CONTENT_LENGTH = 2*1024*1024;
constexpr uint64_t MAX_FILE_CONTENT_LENGTH = 20*1024*1024;
constexpr uint64_t MAX_FORM_LENGTH = 2*1024*1024;

constexpr size_t CHUNK_SIZE = //1;
//sizeof("\r\n--------------------------190ff68b309a6884")-1;
1024*16;


std::tuple<std::string,std::string,std::string>
parseContentDisposition(std::string const& header);


struct FileStorage {
  struct {
    std::string filename;
    std::string content_type;
  } external;
    std::string path;
};

using ParamMap = std::unordered_map<std::string,std::string>;
using FileMap = std::unordered_map<std::string, //param name
                                   FileStorage>;


struct wsgi_request;

bool parseMultipartBody(struct wsgi_request *wsgi_req, std::string& _ctboundary,
                        ParamMap& forms, FileMap& files);


void parseURLEncodedBody(struct wsgi_request *wsgi_req,
                         ParamMap& forms);


#endif
