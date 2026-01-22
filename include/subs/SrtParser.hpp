#pragma once

#include "subs/SubtitleTrack.hpp"

#include <filesystem>
#include <string_view>

class SrtParser {
public:
    SubtitleTrack parseFile(const std::filesystem::path& path) const;

private:
    static int64_t parseTimeMs(std::string_view s);
    static std::string trim(std::string s);
};
