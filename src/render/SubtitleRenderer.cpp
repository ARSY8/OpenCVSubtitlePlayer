#include "render/SubtitleRenderer.hpp"

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

static cv::Scalar white() { return {255, 255, 255}; }
static cv::Scalar black() { return {0, 0, 0}; }

SubtitleRenderer::SubtitleRenderer(RenderStyle style)
    : style_(style) {}

void SubtitleRenderer::drawOutlinedText(cv::Mat& frame,
                                       const std::string& text,
                                       cv::Point org,
                                       double scale,
                                       int thickness) {
    cv::putText(frame, text, org,
                cv::FONT_HERSHEY_SIMPLEX,
                scale, black(),
                thickness + 2, cv::LINE_AA);

    cv::putText(frame, text, org,
                cv::FONT_HERSHEY_SIMPLEX,
                scale, white(),
                thickness, cv::LINE_AA);
}

std::vector<std::string>
SubtitleRenderer::wrapLines(const std::vector<std::string>& lines,
                            int maxWidthPx) const {
    std::vector<std::string> result;

    for (const std::string& line : lines) {
        std::istringstream iss(line);
        std::string word;
        std::string current;

        while (iss >> word) {
            std::string test = current.empty() ? word : (current + " " + word);

            int baseline = 0;
            cv::Size sz = cv::getTextSize(
                test,
                cv::FONT_HERSHEY_SIMPLEX,
                style_.fontScale,
                style_.thickness,
                &baseline
            );

            if (sz.width <= maxWidthPx) {
                current = std::move(test);
            } else {
                if (!current.empty()) result.push_back(current);
                current = word;
            }
        }

        if (!current.empty()) result.push_back(current);
    }

    return result;
}

void SubtitleRenderer::draw(cv::Mat& frame, const SubtitleCue& cue) const {
    if (frame.empty()) return;

    const int maxWidth = frame.cols - 2 * style_.marginPx;
    if (maxWidth <= 0) return;

    std::vector<std::string> lines = wrapLines(cue.lines, maxWidth);
    if (lines.empty()) return;

    int baseline = 0;
    std::vector<cv::Size> sizes;
    sizes.reserve(lines.size());

    int totalHeight = 0;
    for (const auto& l : lines) {
        cv::Size sz = cv::getTextSize(
            l,
            cv::FONT_HERSHEY_SIMPLEX,
            style_.fontScale,
            style_.thickness,
            &baseline
        );
        sizes.push_back(sz);
        totalHeight += sz.height + style_.lineSpacingPx;
    }
    totalHeight -= style_.lineSpacingPx;

    const int safeGap = 6; 
    int yStart = frame.rows - style_.marginPx - style_.reservedBottomPx - safeGap - totalHeight;

    if (yStart < style_.marginPx) yStart = style_.marginPx;

    const int padX = 18;
    const int padY = 4;
    const int gap = style_.lineSpacingPx;

    int y = yStart; 

    for (size_t i = 0; i < lines.size(); ++i) {
        const cv::Size sz = sizes[i];

        const int boxH = sz.height + 2 * padY;

        const int yBase = y + padY + sz.height;

        int xText = (frame.cols - sz.width) / 2;

        if (style_.drawBox) {
            int boxX = xText - padX;
            int boxW = sz.width + 2 * padX;

            int boxTop = y;

            if (boxX < 0) { boxW += boxX; boxX = 0; }
            if (boxX + boxW > frame.cols) boxW = frame.cols - boxX;

            int boxHClamped = boxH;
            int boxTopClamped = boxTop;
            if (boxTopClamped < 0) { boxHClamped += boxTopClamped; boxTopClamped = 0; }
            if (boxTopClamped + boxHClamped > frame.rows) boxHClamped = frame.rows - boxTopClamped;

            if (boxW > 0 && boxHClamped > 0) {
                cv::Rect box(boxX, boxTopClamped, boxW, boxHClamped);

                cv::Mat roi = frame(box);
                cv::Mat overlay = roi.clone();
                overlay.setTo(cv::Scalar(0, 0, 0));

                cv::addWeighted(overlay, style_.boxAlpha,
                                roi, 1.0 - style_.boxAlpha,
                                0.0, roi);
            }
        }

        drawOutlinedText(frame, lines[i], {xText, yBase},
                        style_.fontScale, style_.thickness);

        y += boxH + gap;

        if (y >= frame.rows) break;
    }

}
