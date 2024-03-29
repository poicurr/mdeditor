#include <HttpRequest.hpp>
#include <HttpResponse.hpp>
#include <HttpServer.hpp>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include "parser/Parser.hpp"
#include "tokenizer/Tokenizer.hpp"

std::string loadfile(std::ifstream& ifs) {
  static char buffer[1024];
  auto ret = std::string{};
  while (!ifs.eof()) {
    ifs.read(buffer, 1024);
    ret.append(buffer, ifs.gcount());
  }
  return ret;
}

std::string mimetype(const std::string& target) {
  auto mimetype = std::string{"text/html"};
  if (endsWith(target, ".ico")) {
    mimetype = "image/x-icon";
  } else if (endsWith(target, ".js")) {
    mimetype = "application/javascript";
  } else if (endsWith(target, ".css")) {
    mimetype = "text/css";
  }
  return mimetype;
}

HttpResponse get(const HttpRequest& request) {
  const auto path = request.header.path;
  if (path.empty())
    return HttpResponse{"HTTP/1.1 404 Not Found", "text/html", "404 Not Found"};

  if (contains(path, ".."))
    return HttpResponse{"HTTP/1.1 404 Not Found", "text/html", "404 Not Found"};

  auto target = std::string{};
  if (path == "/") {
    target = "./editor/index.html";
  } else {
    target = "./editor" + path;
  }

  std::ifstream ifs(target, std::ios::binary);
  if (!ifs.is_open())
    return HttpResponse{"HTTP/1.1 404 Not Found", "text/html", "404 Not Found"};

  return HttpResponse{"HTTP/1.1 200 OK", mimetype(target), loadfile(ifs)};
}

HttpResponse post(const HttpRequest& request) {
  const auto path = request.header.path;
  if (path != "/update")
    return HttpResponse{"HTTP/1.1 404 Not Found", "text/html", "404 Not Found"};

  if (request.body.empty())
    return HttpResponse{"HTTP/1.1 200 OK", "text/html", ""};

  auto body = request.body + "\n";
  m2h::Tokenizer tokenizer;
  const auto& tokens = tokenizer.tokenize(body.c_str());
  m2h::Parser parser;
  const auto& nodes = parser.parse(tokens);
  std::stringstream ss;
  for (auto&& node : nodes) {
    node->print(ss, "");
  }
  return HttpResponse{"HTTP/1.1 200 OK", "text/html", ss.str()};
}

int port = 8000;
HttpServer server(port);

void sigintHandler(int sig) {
  std::cout << "\nReceived SIGINT signal. Cleaning up and exiting."
            << std::endl;
  server.shutdown();
  std::exit(0);
}

int main(int argc, char const* argv[]) {
  std::cout << "Server is running at http://127.0.0.1:" << port << std::endl;

  if (std::signal(SIGINT, sigintHandler) == SIG_ERR) {
    perror("Error registering SIGINT handler");
    return 1;
  }

  server.run([](const HttpRequest& request) {
    const auto method = toUpper(request.header.method);
    if (method == "GET") {
      return get(request);
    } else if (method == "POST") {
      return post(request);
    }
    return HttpResponse{"HTTP/1.1 404 Not Found", "text/html", "404 Not Found"};
  });
}
