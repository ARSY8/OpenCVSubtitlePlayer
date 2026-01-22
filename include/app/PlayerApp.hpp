#pragma once

#include "render/SubtitleRenderer.hpp"
#include "subs/SubtitleTrack.hpp"
#include "video/VideoSource.hpp"
#include "subs/SubtitleTimingController.hpp"


#include <cstdint>
#include <optional>

class PlayerApp {
public:
    PlayerApp(VideoSource video,
              std::optional<SubtitleTrack> subs,
              SubtitleRenderer renderer);

    int run();

private:
    VideoSource video_;
    std::optional<SubtitleTrack> subs_;
    SubtitleTimingController timing_;
    SubtitleRenderer renderer_;


    bool paused_ = false;
    int64_t subsOffsetMs_ = 0;

    int frameDelayMs_ = 40;

    void handleKey(int key);
    void drawHud(cv::Mat& frame, int64_t t_ms) const;

    bool shouldExit_ = false;

    void drawProgressBar(cv::Mat& frame, int64_t t_ms) const;

    struct ProgressBarGeom {
    bool valid = false;
    int x0 = 0;
    int x1 = 0;
    int y0 = 0;
    int y1 = 0;

    int frameW = 0;
    int frameH = 0;


    int64_t durMs = 0;
};

    mutable ProgressBarGeom bar_;
    bool draggingBar_ = false;

    static void onMouseThunk(int event, int x, int y, int flags, void* userdata);
    void onMouse(int event, int x, int y, int flags);
    void seekByMouseX(int x);
    bool needRefreshFrame_ = false;

};  
