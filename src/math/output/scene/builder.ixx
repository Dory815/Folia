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
                                   std::optional<Tooltip> tooltip = {},
                                   SemanticMetadata metadata = {},
                                   ElementRole role = ElementRole::math_wrapper, Paint paint = {});
    void add_primitive(NodeId group, Primitive primitive);
    [[nodiscard]] Transform transform(NodeId id) const;
    void set_transform(NodeId id, Transform value);
    [[nodiscard]] SemanticMetadata metadata(NodeId id) const;
    void set_metadata(NodeId id, SemanticMetadata value);
    [[nodiscard]] Paint paint(NodeId id) const;
    void set_paint(NodeId id, Paint value);
    [[nodiscard]] std::size_t entry_count(NodeId id) const;
    void insert_primitive(NodeId id, std::size_t index, Primitive primitive);
    [[nodiscard]] NodeId wrap_contents(NodeId id, Transform transform = {},
                                       SemanticMetadata metadata = {},
                                       ElementRole role = ElementRole::anonymous_group);
    void set_display_layout(DisplayLayout value);
    [[nodiscard]] DisplayLayout display_layout() const;
    void set_metrics(Metrics metrics);
    void set_font_metrics(FontMetrics metrics);
    [[nodiscard]] Scene freeze() &&;
};
}
