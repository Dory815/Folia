export module folia.math.adaptors.lite.list;
export import folia.math.adaptors.lite.node_store;
import folia.math.util.option_types;
import std;
export namespace folia::math::adaptors::lite {
class LiteList {
    LiteNodeRef ref_;
public:
    explicit LiteList(PropertyArray nodes = {}, std::shared_ptr<LiteNodeStore> store = {});
    explicit LiteList(LiteNodeRef ref);
    LiteNodeRef ref() const { return ref_; }
    PropertyArray nodes() const;
    void nodes(PropertyArray value);
    void append(Property node);
    std::size_t size() const;
    LiteNodeValue at(std::size_t index) const;
    [[noreturn]] void next() const;
};
}
