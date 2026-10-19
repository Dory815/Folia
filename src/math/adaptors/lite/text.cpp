module folia.math.adaptors.lite.text;
import folia.math.util.property_host;
import std;
namespace folia::math::adaptors::lite {
LiteText::LiteText(std::u16string text, bool comment, std::shared_ptr<LiteNodeStore> store) {
    if(!store)store=LiteNodeStore::create();
    ref_ = store->create(comment ? NodeKind::Comment : NodeKind::Text,
        comment ? u"#comment" : u"#text", std::move(text));
}
LiteText::LiteText(Property text, bool comment, std::shared_ptr<LiteNodeStore> store) {
    if(!store)store=LiteNodeStore::create();
    ref_=store->create(comment?NodeKind::Comment:NodeKind::Text,comment?u"#comment":u"#text");
    record(ref_).value=std::move(text);
}
LiteText::LiteText(LiteNodeRef ref) : ref_(std::move(ref)) {
    if (!ref_ || (recordConst(ref_).nodeKind != NodeKind::Text && recordConst(ref_).nodeKind != NodeKind::Comment))
        throw std::invalid_argument("LiteNode is not a LiteText");
}
std::u16string LiteText::value() const { return property_host::string(recordConst(ref_).value); }
void LiteText::value(std::u16string text) { record(ref_).value = Property(property_host::debugText(text)); }
std::u16string LiteText::kind() const { return recordConst(ref_).nodeKind == NodeKind::Comment ? u"#comment" : u"#text"; }
LiteNodeValue LiteText::parent() const { return parentValue(ref_); }
PropertyList LiteText::properties() const { return recordConst(ref_).properties; }
}
