#include "app/PlayerApp.hpp"
#include "render/SubtitleRenderer.hpp"
#include "subs/SrtParser.hpp"
#include "video/VideoSource.hpp"

#include <iostream>
#include <optional>

#include <filesystem>
#include <optional>
#include <vector>

static std::optional<std::filesystem::path>
autoFindSubtitles(const std::filesystem::path& videoPath) {
    using namespace std::filesystem;

    path dir  = videoPath.parent_path();
    path base = videoPath.stem();

    std::vector<path> candidates = {
        dir / (base.string() + ".srt"),
        dir / (base.string() + ".en.srt"),
        dir / (base.string() + ".ru.srt"),
    };

    for (const auto& p : candidates) {
        if (exists(p) && is_regular_file(p)) {
            return p;
        }
    }
    return std::nullopt;
}


int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <video.mp4> [subs.srt]\n";
            return 1;
        }

        const std::filesystem::path videoPath = argv[1];

        std::optional<SubtitleTrack> subs;
        if (argc >= 3) {
            const std::filesystem::path srtPath = argv[2];
            SrtParser parser;
            subs = parser.parseFile(srtPath);
            std::cout << "Loaded subtitles: " << srtPath.string() << "\n";
        } else {
            std::cout << "No subtitles provided\n";
        }

        VideoSource video(videoPath);
        
        RenderStyle st;
        st.reservedBottomPx = 40;
        SubtitleRenderer renderer(st);

        PlayerApp app(std::move(video), std::move(subs), std::move(renderer));

        std::cerr << "MAIN: before run\n";
        int rc = app.run();
        std::cerr << "MAIN: after run rc=" << rc << "\n";
        return rc;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 2;
    }
}

