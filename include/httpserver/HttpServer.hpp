#include <HttpRequest.hpp>
#include <HttpResponse.hpp>
#include <common/StringUtils.hpp>
#include <common/thread.hpp>
#include <iostream>
#include <sstream>
#include <vector>

#ifdef __linux__

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>  // for memset
#include <sys/socket.h>
#include <unistd.h>  // for close

#elif _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#endif

const size_t BUFFER_SIZE = 8192;

std::string valueOf(const HttpRequestHeader &header, const std::string &key) {
  auto headers = header.headers;
  auto it = headers.find(key);
  if (it == headers.end()) return "";
  return it->second;
}

HttpRequest parseRequest(int client, const std::string &recvData) {
  if (recvData.empty()) return HttpRequest{};
  std::vector<std::string> headerbody = split(recvData, "\r\n\r\n");
  auto header = headerbody[0];
  auto lines = split(header, "\r\n");
  // parse header message
  auto message = split(lines[0], " ");
  auto requestHeader = HttpRequestHeader{};
  requestHeader.method = message[0];
  requestHeader.path = message[1];
  requestHeader.version = message[2];
  // parse header attrs
  for (size_t i = 1; i < lines.size(); ++i) {
    auto pair = split(lines[i], ": ");
    requestHeader.headers[pair[0]] = pair[1];
  }
  // parse body
  auto body = headerbody[1];
  // parse remaining body
  const auto contentLength = valueOf(requestHeader, "Content-Length");
  if (contentLength != "") {
    const auto len = std::stoi(contentLength);
    char buffer[BUFFER_SIZE];
    while (body.size() < len) {
      int n = recv(client, buffer, BUFFER_SIZE, 0);
      body.append(buffer, n);
    }
  }
  return HttpRequest{requestHeader, body};
}

struct HttpServer {
  HttpServer(unsigned short port) : m_socket{} {
#ifdef _WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2, 0), &data);
#endif
    sockaddr_in srcAddr;
    memset(&srcAddr, 0, sizeof(srcAddr));
    srcAddr.sin_port = htons(port);
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    const char yes = 1;
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    bind(m_socket, (sockaddr *)&srcAddr, sizeof(srcAddr));
    listen(m_socket, 1);
  }

  ~HttpServer() {
#ifdef _WIN32
    WSACleanup();
#endif
  }

  template <class RequestHandler>
  void run(RequestHandler &&handler) {
    sockaddr_in dstAddr;
    socklen_t dstAddrSize = sizeof(dstAddr);
    while (1) {
      std::vector<std::future<void>> results = {};
      for (int i = 0; i < 1000; ++i) {
        auto future = std::async(
            [&](RequestHandler &&handler) {
              int client = accept(m_socket, (sockaddr *)&dstAddr, &dstAddrSize);
            reuseaddr:
              auto recvData = std::string{};
              char buffer[BUFFER_SIZE];
              int recvCount = recv(client, buffer, BUFFER_SIZE, 0);
              if (recvCount > 0) recvData.append(buffer, recvCount);

              // parse request
              auto request = parseRequest(client, recvData);

              bool keepAlive =
                  valueOf(request.header, "Connection") == "keep-alive";

              // handle request
              HttpResponse resp = handler(request);

              // send response
              std::stringstream ss;
              ss << resp.message << "\r\n";
              ss << "Content-Length: " << resp.body.size() << "\r\n";
              ss << "Content-Type: " << resp.mimetype << "\r\n";
              ss << "\r\n";
              ss << resp.body;
              auto response = ss.str();
              send(client, response.data(), response.size(), 0);

              if (keepAlive) goto reuseaddr;
#ifdef __linux__
              close(client);
#elif _WIN32
              closesocket(client);
#endif
              return;
            },
            std::move(handler));
        results.emplace_back(std::move(future));
      }
      for (auto &&f : results) f.get();
    }
  }

  int m_socket;
  std::mutex m_iomutex;
};
