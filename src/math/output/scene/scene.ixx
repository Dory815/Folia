export module folia.math.output.scene;
export import folia.math.output.scene.types;
import std;

export namespace folia::math::output::scene {
class Scene;
struct ActionRegion {
    Rule hitbox;
    std::shared_ptr<Scene const> tooltip;
    std::uint64_t resource_order{};
};
struct ToggleAction {
    double selection{};
    std::size_t child_count{};
    bool enriched{};
};
struct MathTooltipAction {
    Unit dx{}, dy{};
    std::uint32_t post_delay_ms{600}, clear_delay_ms{100};
};
struct StatuslineAction {
    std::string text;
};
using Action = std::variant<ToggleAction, MathTooltipAction, StatuslineAction>;

struct Group {
    NodeId id{};
    Transform transform{};
    std::optional<Clip> clip;
    std::optional<Link> link;
    std::optional<Tooltip> tooltip;
    SemanticMetadata metadata;
    std::vector<std::variant<Primitive, std::shared_ptr<Group const>, ActionRegion>> entries;
    ElementRole role{ElementRole::math_wrapper};
    Paint paint{};
    std::optional<Viewport> viewport;
    std::optional<ForeignContent> foreign_content;
    std::optional<Action> action;
};

// A Scene is a completed immutable value. All content is owned by value;
// glyph/font data is reachable only through shared const resources.
class Scene {
    Group root_;
    Metrics metrics_;
    FontMetrics font_metrics_;
    DisplayLayout display_layout_;
    FrameLayout frame_layout_;

  public:
    // Construction snapshots the full tree and glyph resources, so callers
    // may safely retain and mutate their source values after this returns.
    Scene(Group root, Metrics metrics, FontMetrics font_metrics, DisplayLayout display_layout = {},
          FrameLayout frame_layout = {});
    Scene(Scene const &) = default;
    Scene(Scene &&) noexcept = default;
    Scene &operator=(Scene const &) = default;
    Scene &operator=(Scene &&) noexcept = default;
    [[nodiscard]] Group const &root() const noexcept;
    [[nodiscard]] Metrics const &metrics() const noexcept;
    [[nodiscard]] FontMetrics const &font_metrics() const noexcept;
    [[nodiscard]] DisplayLayout const &display_layout() const noexcept;
    [[nodiscard]] FrameLayout const &frame_layout() const noexcept;
};
} // namespace folia::math::output::scene
