#pragma once

#include <cstdint>

class SubtitleTimingController {
public:
    void increase(int64_t deltaMs) { offsetMs_ += deltaMs; }
    void reset() { offsetMs_ = 0; }

    int64_t apply(int64_t t_ms) const {
        int64_t r = t_ms + offsetMs_;
        return (r < 0) ? 0 : r;
    }

    int64_t offsetMs() const { return offsetMs_; }

private:
    int64_t offsetMs_ = 0;
};
