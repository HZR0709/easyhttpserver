#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <winSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

int handleClient(SOCKET client_socket);
std::string handleHTTPRequest(const std::string& request, SOCKET client_socket);
bool loadFileContent(const std::string& path, std::string& content, std::string& content_type);

// 处理客户端连接
int handleClient(SOCKET client_socket) {
    char buffer[10240] = { 0 };
    int valread = 0;
    std::string request;

    while ((valread = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[valread] = '\0';
        request.append(buffer);

        // 检查是否接收到完整的HTTP请求（以\r\n\r\n结尾）
        if (request.find("\r\n\r\n") != std::string::npos) {
            std::string response = handleHTTPRequest(request, client_socket);
            send(client_socket, response.c_str(), response.size(), 0);
            request.clear(); // 清空已处理的请求
        }
    }

    if (valread <= 0) {
        printf("客户端<Socket=%d>已退出，任务结束。\n", client_socket);
        return -1;
    }

    return 0;
}

// 处理HTTP请求并生成响应
std::string handleHTTPRequest(const std::string& request, SOCKET client_socket) {
    std::istringstream request_stream(request);
    std::string method, path, version;
    request_stream >> method >> path >> version;

    std::ostringstream response_stream;
    if (method == "GET") {
        std::string response_body;
        std::string content_type;

        // 根据请求路径查找文件并生成响应内容
        if (path == "/") {
            path = "/index.html"; // 默认首页
        }

        if (loadFileContent("./dist" + path, response_body, content_type)) { // 将dist目录作为根目录
            response_stream << "HTTP/1.1 200 OK\r\n";
            response_stream << "Content-Type: " << content_type << "\r\n";
            response_stream << "Content-Length: " << response_body.size() << "\r\n";
            response_stream << "Connection: close\r\n";
            response_stream << "\r\n";
            response_stream << response_body;
        }
        else {
            response_stream << "HTTP/1.1 404 Not Found\r\n";
            response_stream << "Content-Type: text/html\r\n";
            response_stream << "Connection: close\r\n";
            response_stream << "\r\n";
            response_stream << "<html><body><h1>404 Not Found</h1></body></html>";
        }
    }
    else if (method == "POST") {
        std::string headers;
        std::string line;
        size_t content_length = 0;

        // 读取头部字段
        while (std::getline(request_stream, line) && line != "\r") {
            headers += line + "\n";
            if (line.find("Content-Length:") != std::string::npos) {
                content_length = std::stoul(line.substr(16));
            }
        }

        // 读取消息体
        std::string body;
        if (content_length > 0) {
            body.resize(content_length);
            request_stream.read(&body[0], content_length);
        }

        // 生成响应
        response_stream << "HTTP/1.1 200 OK\r\n";
        response_stream << "Content-Type: text/html\r\n";
        response_stream << "Connection: close\r\n";
        response_stream << "\r\n";
        response_stream << "<html><body><h1>POST data received</h1><pre>" << body << "</pre></body></html>";
    }
    else {
        response_stream << "HTTP/1.1 405 Method Not Allowed\r\n";
        response_stream << "Content-Type: text/html\r\n";
        response_stream << "Connection: close\r\n";
        response_stream << "\r\n";
        response_stream << "<html><body><h1>Method Not Allowed</h1></body></html>";
    }

    return response_stream.str();
}

// 加载文件内容并确定内容类型
bool loadFileContent(const std::string& path, std::string& content, std::string& content_type) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::ostringstream content_stream;
    content_stream << file.rdbuf();
    content = content_stream.str();

    // 确定内容类型
    if (path.ends_with(".html")) {
        content_type = "text/html";
    }
    else if (path.ends_with(".css")) {
        content_type = "text/css";
    }
    else if (path.ends_with(".js")) {
        content_type = "application/javascript";
    }
    else if (path.ends_with(".jpg") || path.ends_with(".jpeg")) {
        content_type = "image/jpeg";
    }
    else if (path.ends_with(".png")) {
        content_type = "image/png";
    }
    else {
        content_type = "application/octet-stream";
    }

    return true;
}

std::vector<SOCKET> g_clients;

int main() {
#ifdef _WIN32
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
#endif
    // 1.建立socket套接字
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // 2.bind绑定用于接收客户端连接的网络端口
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
    _sin.sin_addr.s_addr = INADDR_ANY;
#endif
    if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
        printf("绑定网络端口失败...\n");
    }
    else {
        printf("绑定网络端口成功...\n");
    }
    // 3.listen监听网络端口
    if (SOCKET_ERROR == listen(_sock, 5)) {
        printf("监听网络端口失败...\n");
    }
    else {
        printf("监听网络端口成功...\n");
    }

    while (true) {
        fd_set fdRead; // 描述符集合
        fd_set fdWrite;
        fd_set fdExp;

        FD_ZERO(&fdRead); // 清理集合
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExp);

        FD_SET(_sock, &fdRead);// 将描述符加入集合中
        FD_SET(_sock, &fdWrite);
        FD_SET(_sock, &fdExp);
        SOCKET maxSock = _sock;
        for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
            FD_SET(g_clients[n], &fdRead);
            if (maxSock < g_clients[n]) {
                maxSock = g_clients[n];
            }
        }

        timeval t = { 0, 0 };
        int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
        if (ret < 0) {
            printf("select任务结束。\n");
            break;
        }
        // 判断描述符是否在集合中
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);

            // accept等待接受客户端连接
            sockaddr_in clientAddr = {};
            int nAddrLen = sizeof(sockaddr_in);
            SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
            _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
            _cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
            if (INVALID_SOCKET == _cSock) {
                printf("接收到无效客户端SOCKET...\n");
            }
            else {
                g_clients.push_back(_cSock);
                printf("新客户端加入：socket = %d, IP = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
            }
        }

        for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
            if (FD_ISSET(g_clients[n], &fdRead)) {
                if (-1 == handleClient(g_clients[n])) {
                    auto iter = g_clients.begin() + n;
                    if (iter != g_clients.end()) {
                        g_clients.erase(iter);
                    }
                }
            }
        }
    }
    for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
        closesocket(g_clients[n]);
    }
    // 7.关闭套接字closesocket
    closesocket(_sock);
#ifdef _WIN32
    // 清除Windows socket环境
    WSACleanup();
#endif
    printf("已退出。\n");
    return 0;
}
