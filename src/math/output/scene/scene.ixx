export module folia.math.output.scene;
export import folia.math.output.scene.types;
import std;

export namespace folia::math::output::scene {
struct Group {
    NodeId id{};
    Transform transform{};
    std::optional<Clip> clip;
    std::optional<Link> link;
    std::optional<Tooltip> tooltip;
    SemanticMetadata metadata;
    std::vector<std::variant<Primitive, std::shared_ptr<Group const>>> entries;
    ElementRole role{ElementRole::math_wrapper};
    Paint paint{};
};

// A Scene is a completed immutable value. All content is owned by value;
// glyph/font data is reachable only through shared const resources.
class Scene {
    Group root_;
    Metrics metrics_;
    FontMetrics font_metrics_;
    DisplayLayout display_layout_;

  public:
    // Construction snapshots the full tree and glyph resources, so callers
    // may safely retain and mutate their source values after this returns.
    Scene(Group root, Metrics metrics, FontMetrics font_metrics, DisplayLayout display_layout = {});
    Scene(Scene const&) = default;
    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene const&) = default;
    Scene& operator=(Scene&&) noexcept = default;
    [[nodiscard]] Group const& root() const noexcept;
    [[nodiscard]] Metrics const& metrics() const noexcept;
    [[nodiscard]] FontMetrics const& font_metrics() const noexcept;
    [[nodiscard]] DisplayLayout const &display_layout() const noexcept;
};
}
