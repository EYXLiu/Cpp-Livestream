#include "video_capture.h"
#include "ml_model.h"
#include "http_server.h"
#include "websocket_server.h"
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    VideoCapture cap(0);
    cap.start();

    Model mdl("");
    mdl.start(cap);

    HttpServer http(8080);
    WebsocketServer ws(9090);

    http.start(mdl);
    ws.start(mdl);

    std::cin.get();

    http.stop();
    ws.stop();
    mdl.stop();
    cap.stop();

    return 0;
}