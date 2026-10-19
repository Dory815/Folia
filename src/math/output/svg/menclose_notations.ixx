export module folia.math.output.svg.menclose_notations;
export import folia.math.output.common;
import std;

export namespace folia::math::output::svg {
// Immutable pinned SVG-profile notation metadata. Common consumes this math
// catalog; the renderer only consumes its geometry and paints it.
std::span<common::NotationSpec const> mencloseNotations() noexcept;
} // namespace folia::math::output::svg
