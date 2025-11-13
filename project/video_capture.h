#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <mutex>


class VideoCapture {
public:
    explicit VideoCapture(int camera_id = 0);
    ~VideoCapture();
    void start();
    void stop();
    cv::Mat get_latest_frame();

private:
    void capture_loop();
    int camera_id_;
    cv::VideoCapture cap_;
    std::thread thread_;
    std::atomic<bool> running_{false};
    std::mutex mtx_;
    cv::Mat latest_frame_;
};

#endif