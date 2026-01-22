#include "video/VideoSource.hpp"

#include <stdexcept>

VideoSource::VideoSource(const std::filesystem::path& videoPath) {
    cap_.open(videoPath.string());
    if (!cap_.isOpened()) {
        throw std::runtime_error("Cannot open video: " + videoPath.string());
    }
    cv::Mat probe;
    if (!cap_.read(probe) || probe.empty()) {
        throw std::runtime_error("Video opened but cannot read first frame (missing codec/backend?)");
    }
    cap_.set(cv::CAP_PROP_POS_FRAMES, 0);

}

bool VideoSource::read(cv::Mat& frame) {
    return cap_.read(frame);
}

int64_t VideoSource::timeMs() const {
    double ms = cap_.get(cv::CAP_PROP_POS_MSEC);
    return static_cast<int64_t>(ms);
}

void VideoSource::seekMs(int64_t t_ms) {
    if (t_ms < 0) t_ms = 0;
    cap_.set(cv::CAP_PROP_POS_MSEC, static_cast<double>(t_ms));
}

double VideoSource::fps() const {
    double f = cap_.get(cv::CAP_PROP_FPS);
    return (f > 1e-6) ? f : 25.0;
}

cv::Size VideoSource::frameSize() const {
    int w = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_WIDTH));
    int h = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_HEIGHT));
    return {w, h};
}

int64_t VideoSource::durationMs() const {
    double fps = cap_.get(cv::CAP_PROP_FPS);
    double frames = cap_.get(cv::CAP_PROP_FRAME_COUNT);

    if (fps > 1e-6 && frames > 0.5) {
        double ms = (frames / fps) * 1000.0;
        return static_cast<int64_t>(ms);
    }

    return 0;
}
