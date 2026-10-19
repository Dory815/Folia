export module folia.math.adaptors.lite.document;
export import folia.math.adaptors.lite.element;
import std;
export namespace folia::math::adaptors::lite {
class LiteDocument {
    LiteNodeRef ref_;
public:
    explicit LiteDocument(LiteNodeValue defaultView = PropertyNull{}, std::shared_ptr<LiteNodeStore> store = {});
    explicit LiteDocument(LiteNodeRef ref, bool existing);
    LiteNodeRef ref() const { return ref_; }
    std::u16string kind() const { return u"#document"; }
    LiteNodeValue root() const;
    LiteNodeValue head() const;
    LiteNodeValue body() const;
    LiteNodeValue defaultView() const;
    void defaultView(LiteNodeValue window);
    std::u16string type() const;
    void type(std::u16string value);
};
}
