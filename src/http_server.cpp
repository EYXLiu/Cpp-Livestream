#include "http_server.h"
#include <iostream>

HttpServer::HttpServer(int port): port_(port), server_fd_(-1) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start(Model& model) {
    if (running_) return;

    thread_ = std::thread(&HttpServer::stream_loop, this, std::ref(model));
    running_ = true;
}

void HttpServer::stop() {
    if (!running_) return;

    if (thread_.joinable()) {
        thread_.join();
    }

    if (server_fd_ >= 0) {
        close(server_fd_);
    }

    running_ = false;
}

void HttpServer::stream_loop(Model& model) {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        perror("Socket failed");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return;
    }

    if (listen(server_fd_, 5) < 0) {
        perror("Listen failed");
        return;
    }

    std::cout << "HTTP MJPEG server running on " 
              << "\033]8;;http://localhost:" << port_ << "\033\\" 
              << "\033[24m"
              << "port " << port_ 
              << "\033[0m"
              << "\033]8;;\033\\\n";

    while (running_) {
        int client_fd = accept(server_fd_, nullptr, nullptr);
        if (client_fd < 0) {
            if (running_) perror ("Accept failed");
            continue;
        }

        std::thread(&HttpServer::handle_client, this, client_fd, std::ref(model)).detach();
    }

    close(server_fd_);
    server_fd_ = -1;
}

void HttpServer::handle_client(int client_fd, Model& model) {
    std::string header = "HTTP/1.1 200 OK\r\n" "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";

    send(client_fd, header.c_str(), header.size(), 0);

    while (running_) {
         cv::Mat frame = model.get_annotated();
         if (frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
         }

         std::vector<uchar> buffer;
         cv::imencode(".jpg", frame, buffer);

         std::string boundary = "--frame\r\n" "Content-Type: image/jpeg\r\n" "Content-Length: " + std::to_string(buffer.size()) + "\r\n\r\n";

         send(client_fd, boundary.c_str(), boundary.size(), 0);
         send(client_fd, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0);
         send(client_fd, "\r\n", 2, 0);

         std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    close(client_fd);
}
