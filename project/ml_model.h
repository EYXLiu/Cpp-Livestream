#ifndef ML_MODEL_H
#define ML_MODEL_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>

#include "video_capture.h"

class Model {
public:
    explicit Model(const std::string& model_path);
    ~Model();
    void start(VideoCapture& input);
    void stop();
    cv::Mat get_annotated();

private:
    void detection_loop(VideoCapture& input);

    std::atomic<bool> model_{false};
    cv::CascadeClassifier classifier_;
    std::thread thread_;
    std::atomic<bool> running_{false};
    std::mutex mtx_;
    cv::Mat annotated_frame_;
};

#endif