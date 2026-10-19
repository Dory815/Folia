#include "support/folia_test.hpp"

import folia.platform.editor_frame_pacing;

using namespace boost::ut;
using folia::platform::editor::FramePacingDeadline;

suite editor_frame_pacing_tests = [] {

"frame pacing preserves its interval across clock uptimes and refresh rates"_test = [] {
    using Clock = std::chrono::steady_clock;
    for (auto uptimeHours : {0, 1, 24, 90, 96, 720, 8760})
    {
        auto previous = Clock::time_point(std::chrono::hours(uptimeHours) + std::chrono::seconds(1));
        for (auto refreshRate : {30, 60, 144, 180, 240, 500})
        {
            auto intervalSeconds = 1.0f / static_cast<float>(refreshRate);
            auto deadline = FramePacingDeadline(previous, intervalSeconds);
            static_assert(std::is_same_v<decltype(deadline), Clock::time_point>);
            auto interval = std::chrono::duration<double>(deadline - previous).count();
            expect(std::abs(interval - static_cast<double>(intervalSeconds)) < 1.0e-7);
            expect(previous + std::chrono::microseconds(100) < deadline);
            expect(previous + std::chrono::milliseconds(34) > deadline);
        }
    }
};

};
