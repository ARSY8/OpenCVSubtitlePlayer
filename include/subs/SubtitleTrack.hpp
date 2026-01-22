#pragma once

#include "subs/SubtitleCue.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

class SubtitleTrack {
public:
    explicit SubtitleTrack(std::vector<SubtitleCue> cues);

    const SubtitleCue* activeAt(int64_t t_ms) const;

    size_t size() const noexcept { return cues_.size(); }

private:
    std::vector<SubtitleCue> cues_;
    mutable size_t hint_ = 0;
};
