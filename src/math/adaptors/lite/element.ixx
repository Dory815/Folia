export module folia.math.adaptors.lite.element;
export import folia.math.adaptors.lite.node_store;
export import folia.math.util.styles;
import std;
export namespace folia::math::adaptors::lite {
class LiteElement {
    LiteNodeRef ref_;
    static PropertyList spread(const PropertyList& source);
public:
    LiteElement(std::u16string kind, PropertyList attributes = {}, PropertyArray children = {}, std::shared_ptr<LiteNodeStore> store = {});
    LiteElement(Property kind, PropertyList attributes, PropertyArray children, std::shared_ptr<LiteNodeStore> store = {});
    explicit LiteElement(LiteNodeRef ref);
    LiteNodeRef ref() const { return ref_; }
    std::u16string kind() const;
    PropertyList attributes() const;
    PropertyArray children() const;
    LiteNodeValue parent() const;
    PropertyList properties() const;
    std::shared_ptr<styles::Styles> styles() const;
};
}
