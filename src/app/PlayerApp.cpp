#include "app/PlayerApp.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>


#include <iomanip>

#include <algorithm>
#include <sstream>


void PlayerApp::onMouseThunk(int event, int x, int y, int flags, void* userdata) {
    auto* self = static_cast<PlayerApp*>(userdata);
    if (self) self->onMouse(event, x, y, flags);
}

void PlayerApp::seekByMouseX(int x) {
    if (!bar_.valid || bar_.durMs <= 0) return;

    if (x < bar_.x0) x = bar_.x0;
    if (x > bar_.x1) x = bar_.x1;

    double ratio = (bar_.x1 == bar_.x0) ? 0.0
        : static_cast<double>(x - bar_.x0) / static_cast<double>(bar_.x1 - bar_.x0);

    int64_t t = static_cast<int64_t>(ratio * static_cast<double>(bar_.durMs));

    video_.seekMs(t);
    needRefreshFrame_ = true;

}


void PlayerApp::onMouse(int event, int x, int y, int flags) {
    if (!bar_.valid) return;

    cv::Rect imgRect = cv::getWindowImageRect("player");
    if (imgRect.width <= 0 || imgRect.height <= 0) return;

    int xi = x - imgRect.x;
    int yi = y - imgRect.y;

    if (xi < 0 || yi < 0 || xi >= imgRect.width || yi >= imgRect.height) {
        if (event == cv::EVENT_LBUTTONUP) draggingBar_ = false;
        return;
    }

    if (bar_.frameW <= 0 || bar_.frameH <= 0) return;

    int fx = static_cast<int>( (static_cast<double>(xi) / imgRect.width)  * bar_.frameW );
    int fy = static_cast<int>( (static_cast<double>(yi) / imgRect.height) * bar_.frameH );

    const bool inside = (fx >= bar_.x0 && fx <= bar_.x1 && fy >= bar_.y0 && fy <= bar_.y1);

    if (event == cv::EVENT_LBUTTONDOWN && inside) {
        draggingBar_ = true;
        seekByMouseX(fx);
    } else if (event == cv::EVENT_MOUSEMOVE && draggingBar_) {
        seekByMouseX(fx);
    } else if (event == cv::EVENT_LBUTTONUP) {
        draggingBar_ = false;
    }
}



PlayerApp::PlayerApp(VideoSource video,
                     std::optional<SubtitleTrack> subs,
                     SubtitleRenderer renderer)
    : video_(std::move(video))
    , subs_(std::move(subs))
    , renderer_(std::move(renderer))
{
    double fps = video_.fps();
    frameDelayMs_ = static_cast<int>(1000.0 / std::max(1.0, fps));
}

void PlayerApp::handleKey(int key) {
    if (key == 27 || key == 'q' || key == 'Q') {
        paused_ = true;
        subsOffsetMs_ = (std::numeric_limits<int64_t>::min)();
        return;
    }

    if (key == ' ') paused_ = !paused_;

    if (key == 'a' || key == 'A') video_.seekMs(video_.timeMs() - 5000);
    if (key == 'd' || key == 'D') video_.seekMs(video_.timeMs() + 5000);

    if (key == 'j' || key == 'J') subsOffsetMs_ -= 100;
    if (key == 'k' || key == 'K') subsOffsetMs_ += 100;
    if (key == '0') subsOffsetMs_ = 0;
}

void PlayerApp::drawHud(cv::Mat& frame, int64_t t_ms) const {
    std::ostringstream oss;
    oss << "t=" << t_ms << "ms"
        << "  paused=" << (paused_ ? "yes" : "no")
        << "  offset=" << subsOffsetMs_ << "ms";

    cv::putText(frame, oss.str(), {20, 40},
                cv::FONT_HERSHEY_SIMPLEX, 0.8,
                cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
}

static std::string formatTime(int64_t ms) {
    if (ms < 0) ms = 0;
    int64_t totalSec = ms / 1000;
    int64_t m = totalSec / 60;
    int64_t s = totalSec % 60;

    std::ostringstream oss;
    oss << m << ":" << std::setw(2) << std::setfill('0') << s;
    return oss.str();
}

void PlayerApp::drawProgressBar(cv::Mat& frame, int64_t t_ms) const {
    bar_.valid = false;
    bar_.durMs = 0;
    bar_.frameW = frame.cols;
    bar_.frameH = frame.rows;

    const int64_t dur = video_.durationMs();
    if (dur <= 0) return;

    bar_.durMs = dur;

    if (t_ms < 0) t_ms = 0;
    if (t_ms > dur) t_ms = dur;

    const int W = frame.cols;
    const int H = frame.rows;

    bar_.frameW = W;
    bar_.frameH = H;


    const int padX = std::max(30, W / 20);

    const int panelH = std::max(26, H / 28); 
    const int barH   = std::max(4,  H / 180); 
    const int yPanelTop = H - panelH;

    {
        cv::Rect panel(0, yPanelTop, W, panelH);
        cv::Mat roi = frame(panel);
        cv::Mat overlay;
        roi.copyTo(overlay);
        overlay.setTo(cv::Scalar(0, 0, 0));
        cv::addWeighted(overlay, 0.35, roi, 0.65, 0, roi);
    }

    const std::string left  = formatTime(t_ms);
    const std::string right = formatTime(dur);

    const double textScale = std::max(0.6, H / 900.0);
    const int textTh = 2;

    int base = 0;
    cv::Size szL = cv::getTextSize(left,  cv::FONT_HERSHEY_SIMPLEX, textScale, textTh, &base);
    cv::Size szR = cv::getTextSize(right, cv::FONT_HERSHEY_SIMPLEX, textScale, textTh, &base);

    const int textY = yPanelTop + std::max(28, panelH / 2);

    cv::putText(frame, left,  {padX, textY},
                cv::FONT_HERSHEY_SIMPLEX, textScale,
                cv::Scalar(255, 255, 255), textTh, cv::LINE_AA);

    cv::putText(frame, right, {W - padX - szR.width, textY},
                cv::FONT_HERSHEY_SIMPLEX, textScale,
                cv::Scalar(255, 255, 255), textTh, cv::LINE_AA);

    const int x0 = padX + szL.width + 20;
    const int x1 = (W - padX - szR.width - 20);
    if (x1 <= x0 + 10) return;

    const int yBarCenter = yPanelTop + panelH / 2;
    const int yBarTop = yBarCenter - barH / 2;

    bar_.valid = true;
    bar_.x0 = x0;
    bar_.x1 = x1;

    bar_.y0 = std::max(0, yBarTop - 8);
    bar_.y1 = std::min(H - 1, yBarTop + barH + 8);



    double ratio = static_cast<double>(t_ms) / static_cast<double>(dur);
    ratio = std::clamp(ratio, 0.0, 1.0);

    const int filled = x0 + static_cast<int>((x1 - x0) * ratio);

    cv::rectangle(frame,
                  cv::Rect(x0, yBarTop, x1 - x0, barH),
                  cv::Scalar(90, 90, 90),
                  cv::FILLED);

    cv::rectangle(frame,
                  cv::Rect(x0, yBarTop, std::max(0, filled - x0), barH),
                  cv::Scalar(230, 230, 230),
                  cv::FILLED);

    const int knobR = std::max(4, barH + 2);


    const int kx = std::clamp(filled, x0, x1);
    const int ky = yBarCenter;

    cv::circle(frame, {kx, ky}, knobR, cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
    cv::circle(frame, {kx, ky}, knobR, cv::Scalar(0, 0, 0), 1, cv::LINE_AA);
}


int PlayerApp::run() {
    cv::namedWindow("player", cv::WINDOW_NORMAL);

    cv::setMouseCallback("player", &PlayerApp::onMouseThunk, this);


    cv::Mat frame;

    while (true) {

        if (paused_ && needRefreshFrame_) {
        video_.read(frame);
        needRefreshFrame_ = false;
    }
        if (!paused_) {
            if (!video_.read(frame)) break; 
        }

        if (frame.empty()) continue;

        int64_t t = video_.timeMs();

        if (subsOffsetMs_ == (std::numeric_limits<int64_t>::min)()) break;

        if (subs_) {
            int64_t ts = t + subsOffsetMs_;
            if (ts < 0) ts = 0;

            if (const SubtitleCue* cue = subs_->activeAt(ts)) {
                renderer_.draw(frame, *cue);
            }
        }

        drawHud(frame, t);
        drawProgressBar(frame, t);


        cv::imshow("player", frame);
        int key = cv::waitKey(paused_ ? 30 : frameDelayMs_);
        if (key != -1) handleKey(key);
    }

    return 0;
}
