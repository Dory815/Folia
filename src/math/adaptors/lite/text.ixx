export module folia.math.adaptors.lite.text;
export import folia.math.adaptors.lite.node_store;
import std;
export namespace folia::math::adaptors::lite {
class LiteText {
    LiteNodeRef ref_;
public:
    explicit LiteText(std::u16string text = {}, bool comment = false, std::shared_ptr<LiteNodeStore> store = {});
    explicit LiteText(Property text, bool comment = false, std::shared_ptr<LiteNodeStore> store = {});
    explicit LiteText(LiteNodeRef ref);
    LiteNodeRef ref() const { return ref_; }
    std::u16string value() const;
    void value(std::u16string text);
    std::u16string kind() const;
    LiteNodeValue parent() const;
    PropertyList properties() const;
};
}
