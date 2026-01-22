#pragma once

#include "render/RenderStyle.hpp"
#include "subs/SubtitleCue.hpp"

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class SubtitleRenderer {
public:
    explicit SubtitleRenderer(RenderStyle style = {});

    void draw(cv::Mat& frame, const SubtitleCue& cue) const;

private:
    RenderStyle style_;

    static void drawOutlinedText(cv::Mat& frame,
                                 const std::string& text,
                                 cv::Point org,
                                 double scale,
                                 int thickness);

    std::vector<std::string> wrapLines(const std::vector<std::string>& lines,
                                       int maxWidthPx) const;
};
