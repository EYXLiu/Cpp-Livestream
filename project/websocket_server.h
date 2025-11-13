#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "ml_model.h"

class WebsocketServer {
public:
    explicit WebsocketServer(int port = 9090);
    ~WebsocketServer();
    void start(Model& model);
    void stop();

private:
    void server_loop(Model& model);
    void handle_client(int client_fd, Model& model);
    std::string generate_accept_key(const std::string& client_key);
    std::atomic<bool> running_{false};
    int port_;
    int server_fd_;
    std::thread thread_;
};

#endif