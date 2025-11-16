#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>

#include "ml_model.h"

class HttpServer {
public:
    explicit HttpServer(int port = 8080);
    ~HttpServer();
    void start(Model& model);
    void stop();

private:
    void stream_loop(Model& model);
    void encoder_loop(Model& model);
    void handle_client(int client_fd, Model& model);
    std::thread thread_;
    std::atomic<bool> running_{false};
    int port_;
    int server_fd_;

    std::vector<uchar> latest_jpeg_;
    std::mutex jpeg_mutex_;
    std::thread encoder_thread_;
    std::atomic<bool> encoder_running_{false};
};

#endif