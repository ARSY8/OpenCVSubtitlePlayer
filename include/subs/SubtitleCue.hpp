#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct SubtitleCue {
    int64_t start_ms = 0;
    int64_t end_ms = 0;
    std::vector<std::string> lines;
};
