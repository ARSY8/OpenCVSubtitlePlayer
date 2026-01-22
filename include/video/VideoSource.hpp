#pragma once

#include <opencv2/opencv.hpp>

#include <cstdint>
#include <filesystem>

class VideoSource {
public:
    explicit VideoSource(const std::filesystem::path& videoPath);

    bool read(cv::Mat& frame);    
    int64_t timeMs() const;         
    void seekMs(int64_t t_ms);    

    double fps() const;
    cv::Size frameSize() const;

    int64_t durationMs() const;


private:
    cv::VideoCapture cap_;
};
