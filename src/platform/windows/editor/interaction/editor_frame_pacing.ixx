export module folia.platform.editor_frame_pacing;
import std;

export namespace folia::platform::editor
{
    constexpr std::chrono::steady_clock::time_point FramePacingDeadline(
        std::chrono::steady_clock::time_point previousSubmission,
        float intervalSeconds) noexcept
    {
        // Convert the relative interval before adding it to the absolute clock.
        // A floating-point time_point loses frame-sized increments as uptime grows.
        return previousSubmission + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<float>(intervalSeconds));
    }
}
