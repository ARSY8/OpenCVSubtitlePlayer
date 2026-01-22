#include "subs/SrtParser.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

static void drop_cr(std::string& line) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
}

std::string SrtParser::trim(std::string s) {
    auto is_space = [](unsigned char c) { return c == ' ' || c == '\t'; };

    size_t l = 0;
    while (l < s.size() && is_space(static_cast<unsigned char>(s[l]))) ++l;

    size_t r = s.size();
    while (r > l && is_space(static_cast<unsigned char>(s[r - 1]))) --r;

    return s.substr(l, r - l);
}

int64_t SrtParser::parseTimeMs(std::string_view s) {
    if (s.size() < 12) throw std::runtime_error("Bad time format");

    auto to2 = [&](size_t pos) -> int {
        if (pos + 1 >= s.size()) throw std::runtime_error("Bad time format");
        if (s[pos] < '0' || s[pos] > '9' || s[pos + 1] < '0' || s[pos + 1] > '9')
            throw std::runtime_error("Bad time format");
        return (s[pos] - '0') * 10 + (s[pos + 1] - '0');
    };

    auto to3 = [&](size_t pos) -> int {
        if (pos + 2 >= s.size()) throw std::runtime_error("Bad time format");
        for (size_t i = 0; i < 3; ++i) {
            char c = s[pos + i];
            if (c < '0' || c > '9') throw std::runtime_error("Bad time format");
        }
        return (s[pos] - '0') * 100 + (s[pos + 1] - '0') * 10 + (s[pos + 2] - '0');
    };

    int hh = to2(0);
    if (s[2] != ':') throw std::runtime_error("Bad time format");
    int mm = to2(3);
    if (s[5] != ':') throw std::runtime_error("Bad time format");
    int ss = to2(6);
    if (s[8] != ',') throw std::runtime_error("Bad time format");
    int mmm = to3(9);

    return (static_cast<int64_t>(hh) * 3600 +
            static_cast<int64_t>(mm) * 60 +
            static_cast<int64_t>(ss)) * 1000 +
           mmm;
}

SubtitleTrack SrtParser::parseFile(const std::filesystem::path& path) const {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open SRT: " + path.string());

    std::vector<SubtitleCue> cues;
    std::string line;

    while (true) {
        std::string s;
        while (std::getline(in, s)) {
            drop_cr(s);
            if (!trim(s).empty()) break;
        }
        if (!in) break; 

        std::string timeLine;
        if (!std::getline(in, timeLine))
            throw std::runtime_error("Unexpected EOF after cue index");
        drop_cr(timeLine);

        bool s_is_timeline = (s.find("-->") != std::string::npos);
        if (s_is_timeline) {
            timeLine = s;
        }

        auto arrow = timeLine.find("-->");
        if (arrow == std::string::npos)
            throw std::runtime_error("Bad timeline line: " + timeLine);

        std::string left = trim(timeLine.substr(0, arrow));
        std::string right = trim(timeLine.substr(arrow + 3));

        int64_t start = parseTimeMs(left);
        int64_t end = parseTimeMs(right);
        if (end < start) throw std::runtime_error("Cue end < start");

        std::vector<std::string> lines;

        if (!s_is_timeline) {
            std::string firstText = trim(timeLine);
            if (!firstText.empty() && firstText.find("-->") == std::string::npos)
                lines.push_back(firstText);
        }

        while (std::getline(in, line)) {
            drop_cr(line);
            std::string t = trim(line);
            if (t.empty()) break;
            lines.push_back(t);
        }

        if (!lines.empty()) cues.push_back(SubtitleCue{start, end, std::move(lines)});
    }

    return SubtitleTrack(std::move(cues));
}
