#include <HttpRequest.hpp>
#include <HttpResponse.hpp>
#include <common/StringUtils.hpp>
#include <csignal>
#include <cstdlib>
#include <future>
#include <iostream>
#include <sstream>
#include <thread>
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
const size_t MAX_CONNECTIONS = 5;

std::string valueOf(const HttpRequestHeader &header, const std::string &key) {
  auto headers = header.headers;
  auto it = headers.find(key);
  if (it == headers.end()) return "";
  return it->second;
}

HttpRequest parseRequest(int client, const std::string &readData) {
  if (readData.empty()) return HttpRequest{};
  std::vector<std::string> headerbody = split(readData, "\r\n\r\n");
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
      int n = read(client, buffer, BUFFER_SIZE);
      if (n > 0) body.append(buffer, n);
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
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_port = htons(port);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(m_socket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
      std::cerr << "[error] failed to bind" << std::endl;
      closeConnection(m_socket);
      exit(1);
    }

    if (listen(m_socket, MAX_CONNECTIONS) < 0) {
      std::cerr << "[error] failed to listen" << std::endl;
      closeConnection(m_socket);
      exit(1);
    }
  }

  ~HttpServer() {
#ifdef _WIN32
    WSACleanup();
#endif
  }

  template <class RequestHandler>
  void run(RequestHandler &&handler) {
    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    while (true) {
      int client = accept(m_socket, (sockaddr *)&clientAddr, &clientAddrSize);
      auto readData = std::string{};
      char buffer[BUFFER_SIZE];
      int bytesRead = read(client, buffer, BUFFER_SIZE);

      if (bytesRead < 0) {
        closeConnection(client);
        continue;
      }
      readData.append(buffer, bytesRead);

      // parse request
      const auto request = parseRequest(client, readData);

      // handle request
      const HttpResponse resp = handler(request);

      // send response
      std::stringstream ss;
      ss << resp.message << "\r\n";
      ss << "Content-Length: " << resp.body.size() << "\r\n";
      ss << "Content-Type: " << resp.mimetype << "\r\n";
      ss << "\r\n";
      ss << resp.body;
      const auto response = ss.str();
      write(client, response.c_str(), response.size());

      closeConnection(client);
    }
  }

  void closeConnection(int socket) {
#ifdef __linux__
    close(socket);
#elif _WIN32
    closesocket(socket);
#endif
  }

  void shutdown() { closeConnection(m_socket); }

  bool m_shutdown;
  int m_socket;
};
