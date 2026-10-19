export module folia.math.output.scene.builder;
export import folia.math.output.scene;
import std;

export namespace folia::math::output::scene {
// Builder owns a mutable tree. `freeze() &&` moves all completed values into
// a Scene; the returned Scene has no connection to the Builder.
class Builder {
    struct State;
    std::unique_ptr<State> state_;
public:
    Builder();
    ~Builder();
    Builder(Builder const&) = delete;
    Builder& operator=(Builder const&) = delete;
    Builder(Builder&&) noexcept;
    Builder& operator=(Builder&&) noexcept;

    [[nodiscard]] NodeId add_group(NodeId parent = 0, Transform transform = {},
        std::optional<Clip> clip = {}, std::optional<Link> link = {},
        std::optional<Tooltip> tooltip = {}, SemanticMetadata metadata = {});
    void add_primitive(NodeId group, Primitive primitive);
    void set_metrics(Metrics metrics);
    void set_font_metrics(FontMetrics metrics);
    [[nodiscard]] Scene freeze() &&;
};
}
