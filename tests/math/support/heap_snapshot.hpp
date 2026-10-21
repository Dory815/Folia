#pragma once

#include <cstddef>

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

namespace math_heap_test {

struct Snapshot {
    std::size_t normal_blocks{};
    std::size_t client_blocks{};
    std::size_t normal_bytes{};
    std::size_t client_bytes{};
};

inline constexpr bool has_crt_heap_snapshot =
#if defined(_WIN32) && defined(_DEBUG)
    true;
#else
    false;
#endif

inline Snapshot snapshot() noexcept {
#if defined(_WIN32) && defined(_DEBUG)
    _CrtMemState state{};
    _CrtMemCheckpoint(&state);
    return {state.lCounts[_NORMAL_BLOCK], state.lCounts[_CLIENT_BLOCK], state.lSizes[_NORMAL_BLOCK],
            state.lSizes[_CLIENT_BLOCK]};
#else
    return {};
#endif
}

inline bool same_heap(Snapshot const &left, Snapshot const &right) noexcept {
    return left.normal_blocks == right.normal_blocks && left.client_blocks == right.client_blocks &&
           left.normal_bytes == right.normal_bytes && left.client_bytes == right.client_bytes;
}

} // namespace math_heap_test
