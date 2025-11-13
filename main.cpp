#include "video_capture.h"
#include "ml_model.h"
#include "http_server.h"
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    VideoCapture cap(0);
    cap.start();

    Model mdl("");
    mdl.start(cap);

    HttpServer server(8080);
    server.start(mdl);

    std::cout << "Press to quit" << std::endl;
    std::cin.get();

    server.stop();
    mdl.stop();
    cap.stop();
    
    return 0;
}