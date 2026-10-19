/*************************************************************
 * MathJax 4.1.3 ts/output/common/Direction.ts
 * Copyright (c) 2021-2026 The MathJax Consortium; Apache-2.0.
 *************************************************************/
export module folia.math.output.common.direction;
import std;

export namespace folia::math::output::common {
struct DirectionValues {
    std::string_view None;
    std::string_view Vertical;
    std::string_view Horizontal;
};
inline constexpr DirectionValues DIRECTION{"", "v", "h"};
inline constexpr std::string_view V = DIRECTION.Vertical;
inline constexpr std::string_view H = DIRECTION.Horizontal;
inline constexpr std::string_view NONE = DIRECTION.None;
inline constexpr std::string_view VERTICAL = DIRECTION.Vertical;
inline constexpr std::string_view HORIZONTAL = DIRECTION.Horizontal;
}
