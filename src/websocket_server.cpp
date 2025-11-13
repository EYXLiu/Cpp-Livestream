#include "websocket_server.h"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <sstream>
#include <vector>
#include <cstring>

#include "base64.h"

WebsocketServer::WebsocketServer(int port) : port_(port), server_fd_(-1) {}

WebsocketServer::~WebsocketServer() {
    stop();
}

void WebsocketServer::start(Model& model) {
    if (running_) return;

    thread_ = std::thread(&WebsocketServer::server_loop, this, std::ref(model));
    running_ = true;
}

void WebsocketServer::stop() {
    if (!running_) return;

    if (thread_.joinable()) {
        thread_.join();
    }

    if (server_fd_ >= 0) {
        close(server_fd_);
    }

    running_ = false;
}

std::string WebsocketServer::generate_accept_key(const std::string &client_key) {
    std::string key = client_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    unsigned char sha1_hash[20];
    SHA1(reinterpret_cast<const unsigned char*>(key.c_str()), key.size(), sha1_hash);

    return base64_encode(sha1_hash, 20);
}

void WebsocketServer::server_loop(Model& model) {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        perror("Socket failed");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return;
    }

    if (listen(server_fd_, 5) < 0) {
        perror("Listen failed");
        return;
    }

    std::cout << "Websocket server running at ws://localhost:" << port_ << std::endl;

    while (running_) {
        int client_fd = accept(server_fd_, nullptr, nullptr);
        if (client_fd < 0) {
            if (running_) perror ("Accept failed");
            continue;
        }
        std::thread(&WebsocketServer::handle_client, this, client_fd, std::ref(model)).detach();
    }
}

void WebsocketServer::handle_client(int client_fd, Model& model) {
    char buffer[2048];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) -1, 0);
    if (n <= 0) {
        close(client_fd);
        return;
    }
    buffer[n] = '\0';
    std::string req(buffer);

    size_t pos = req.find("Sec-WebSocket-Key:");
    if (pos == std::string::npos) {
        close(client_fd);
        return;
    }
    size_t end = req.find("\r\n", pos);
    std::string client_key = req.substr(pos + 18, end - pos - 18);
    client_key.erase(remove(client_key.begin(), client_key.end(), ' '), client_key.end());

    std::string accept_key = generate_accept_key(client_key);
    std::string response = "HTTP/1.1 101 Switching Protocols\r\n" "Upgrade: websocket\r\n" "Connection: Upgrade\r\n" "Sec-WebSocket-Accept: " + accept_key + "\r\n\r\n";
    send(client_fd, response.c_str(), response.size(), 0);

    while (running_) {
        cv::Mat frame = model.get_annotated();
        if (frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        std::vector<uchar> encoded;
        cv::imencode(".jpg", frame, encoded);

        std::vector<uint8_t> frame_hdr = {0x82};
        if (encoded.size() < 126) {
            frame_hdr.push_back((uint8_t)encoded.size());
        } else if (encoded.size() < 65536) {
            frame_hdr.push_back(126);
            frame_hdr.push_back((encoded.size() >> 8) & 0xFF);
            frame_hdr.push_back(encoded.size() & 0xFF);
        } else {
            frame_hdr.push_back(127);
            for (int i = 7; i >= 0; --i)
                frame_hdr.push_back((encoded.size() >> (i*8)) & 0xFF);
        }

        send(client_fd, frame_hdr.data(), frame_hdr.size(), 0);
        send(client_fd, encoded.data(), encoded.size(), 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    close(client_fd);
}
