#pragma once

#include <string>

struct HttpResponse {
  std::string message;
  std::string mimetype;
  std::string body;
};
