#include "video_capture.h"
#include <iostream>

VideoCapture::VideoCapture(int camera_id) : camera_id_(camera_id) {}

VideoCapture::~VideoCapture() {
    stop();
}

void VideoCapture::start() {
    if (running_) return;

    cap_.open(camera_id_);
    if (!cap_.isOpened()) {
        std::cerr << "Failed to open camera " << camera_id_ << std::endl;
        return;
    }

    cap_.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap_.set(cv::CAP_PROP_FPS, 30);

    thread_ = std::thread(&VideoCapture::capture_loop, this);
    running_ = true;
}

void VideoCapture::stop() {
    if (!running_) return;
    if (thread_.joinable()) {
        thread_.join();
    }
    cap_.release();
    running_ = false;
}

cv::Mat VideoCapture::get_latest_frame() {
    std::lock_guard<std::mutex> lock(mtx_);
    return latest_frame_.clone();
}

void VideoCapture::capture_loop() {
    cv::Mat frame;

    while (running_) {
        if (!cap_.read(frame)) continue;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            latest_frame_ = frame.clone();
        }
    }
}
