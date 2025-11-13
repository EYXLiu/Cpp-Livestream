#include "ml_model.h"
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include "video_capture.h"

Model::Model(const std::string& model_path) {
    if (model_path.empty()) return;
    classifier_.load(model_path);
    model_ = true;
}

Model::~Model() {
    stop();
}

void Model::start(VideoCapture& input) {
    if (running_) return;
    running_ = true;
    thread_ = std::thread(&Model::detection_loop, this, std::ref(input));
}

void Model::stop() {
    if (!running_) return;
    if (thread_.joinable()) {
        thread_.join();
    }
    running_ = false;
}

cv::Mat Model::get_annotated() {
    std::lock_guard<std::mutex> lock(mtx_);
    return annotated_frame_.clone();
}

void Model::detection_loop(VideoCapture& input) {
    cv::Mat frame, gray;
    int frame_count = 0;
    auto last_time = std::chrono::steady_clock::now();
    
    while (running_) {
        frame = input.get_latest_frame();
        if (frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);

        cv::Mat annotated = frame.clone();

        if (model_) {
            std::vector<cv::Rect> detections;
            classifier_.detectMultiScale(gray, detections);

            for (const auto &d : detections) {
                cv::rectangle(annotated, d, cv::Scalar(0, 255, 0), 2);
            }
        }

        frame_count++;
        auto now = std::chrono::steady_clock::now();
        double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(now - last_time).count();
        double fps = frame_count / elapsed_seconds;
        
        std::string s = std::to_string(int(fps)) + "FPS";
        cv::putText(annotated, s, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        if (elapsed_seconds >= 2.0) {
            frame_count = 0;
            last_time = now;
        }

        {
            std::lock_guard<std::mutex> lock(mtx_);
            annotated_frame_= annotated;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}
