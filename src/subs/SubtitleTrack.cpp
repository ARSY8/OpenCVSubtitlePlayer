#include "subs/SubtitleTrack.hpp"

#include <algorithm>

SubtitleTrack::SubtitleTrack(std::vector<SubtitleCue> cues)
    : cues_(std::move(cues))
{
    std::sort(cues_.begin(), cues_.end(),
              [](const SubtitleCue& a, const SubtitleCue& b) {
                  return a.start_ms < b.start_ms;
              });
}

const SubtitleCue* SubtitleTrack::activeAt(int64_t t_ms) const {
    if (cues_.empty()) return nullptr;

    if (hint_ < cues_.size()) {
        const auto& c = cues_[hint_];
        if (c.start_ms <= t_ms && t_ms < c.end_ms) return &c;
    }

    while (hint_ + 1 < cues_.size() && cues_[hint_].end_ms <= t_ms) {
        ++hint_;
        const auto& c = cues_[hint_];
        if (c.start_ms <= t_ms && t_ms < c.end_ms) return &c;
    }

    auto it = std::upper_bound(
        cues_.begin(), cues_.end(), t_ms,
        [](int64_t t, const SubtitleCue& c) { return t < c.start_ms; });

    if (it == cues_.begin()) {
        hint_ = 0;
        return nullptr;
    }

    --it;
    hint_ = static_cast<size_t>(std::distance(cues_.begin(), it));

    const auto& c = *it;
    if (c.start_ms <= t_ms && t_ms < c.end_ms) return &c;
    return nullptr;
}
