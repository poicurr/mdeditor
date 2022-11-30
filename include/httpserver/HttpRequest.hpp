#pragma once

#include <common/StringUtils.hpp>
#include <map>
#include <string>
#include <vector>

struct HttpRequestHeader {
  std::string method;
  std::string path;
  std::string version;
  std::map<std::string, std::string> headers;
};

struct HttpRequest {
  HttpRequestHeader header;
  std::string body;
};
