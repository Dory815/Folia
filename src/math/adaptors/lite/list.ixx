export module folia.math.adaptors.lite_adaptor:list;
export import :element;
import std;

export namespace folia::math::adaptors::lite {
class LiteList final {
    std::vector<LiteNodeRef> nodes_;

public:
    using const_iterator = std::vector<LiteNodeRef>::const_iterator;

    LiteList() = default;
    explicit LiteList(std::span<LiteNodeRef const> nodes)
        : nodes_(nodes.begin(), nodes.end()) {}
    void append(LiteNodeRef node) { nodes_.push_back(std::move(node)); }
    std::size_t size() const noexcept { return nodes_.size(); }
    LiteNodeRef const& at(std::size_t index) const { return nodes_.at(index); }
    std::vector<LiteNodeRef> const& nodes() const noexcept { return nodes_; }
    const_iterator begin() const noexcept { return nodes_.begin(); }
    const_iterator end() const noexcept { return nodes_.end(); }
};
}
