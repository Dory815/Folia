module folia.math.adaptors.lite.document;
import folia.math.adaptors.lite.element;
import folia.math.util.property_host;
import std;
namespace folia::math::adaptors::lite {
LiteDocument::LiteDocument(LiteNodeValue view, std::shared_ptr<LiteNodeStore> store) {
    LiteNodeRef viewNode;
    if(const auto* node=std::get_if<LiteNodeRef>(&view))viewNode=*node;
    if(!store)store=viewNode?liteStore(viewNode):LiteNodeStore::create();
    ref_ = store->create(NodeKind::Document, u"#document");
    auto headNode = LiteElement(std::u16string(u"head"), PropertyList{}, PropertyArray{}, store).ref();
    auto bodyNode = LiteElement(std::u16string(u"body"), PropertyList{}, PropertyArray{}, store).ref();
    PropertyArray rootChildren{folia::math::nodeProperty(headNode),folia::math::nodeProperty(bodyNode)};
    auto rootNode = LiteElement(std::u16string(u"html"), PropertyList{}, std::move(rootChildren), store).ref();
    auto& doc = store->record(ref_);
    doc.root = folia::math::nodeProperty(rootNode);
    doc.head = folia::math::nodeProperty(headNode);
    doc.body = folia::math::nodeProperty(bodyNode);
    if(std::holds_alternative<LiteNodeRef>(view))doc.defaultView=nodeProperty(viewNode);
    // The emitted constructor uses a default parameter `window = null`, so an
    // explicit undefined argument receives the same null default.
    else if(std::holds_alternative<Missing>(view))doc.defaultView=Property(PropertyNull{});
    else if(std::holds_alternative<PropertyNull>(view))doc.defaultView=Property(PropertyNull{});
    else doc.defaultView=std::get<Property>(view);
    doc.documentType=Property(property_host::debugText(u""));
}
LiteDocument::LiteDocument(LiteNodeRef ref, bool) : ref_(std::move(ref)) {
    if (!ref_ || recordConst(ref_).nodeKind != NodeKind::Document) throw std::invalid_argument("not a LiteDocument");
}
LiteNodeValue LiteDocument::root() const { return propertyNode(recordConst(ref_).root); }
LiteNodeValue LiteDocument::head() const { return propertyNode(recordConst(ref_).head); }
LiteNodeValue LiteDocument::body() const { return propertyNode(recordConst(ref_).body); }
LiteNodeValue LiteDocument::defaultView() const { return propertyNode(recordConst(ref_).defaultView); }
void LiteDocument::defaultView(LiteNodeValue window) {
    auto& raw=record(ref_).defaultView;
    if(std::holds_alternative<LiteNodeRef>(window))raw=nodeProperty(std::get<LiteNodeRef>(window));
    else if(std::holds_alternative<Missing>(window))raw=Property(Missing{});
    else if(std::holds_alternative<PropertyNull>(window))raw=Property(PropertyNull{});
    else raw=std::get<Property>(std::move(window));
}
std::u16string LiteDocument::type() const { return property_host::string(recordConst(ref_).documentType); }
void LiteDocument::type(std::u16string value) { record(ref_).documentType = Property(property_host::debugText(value)); }
}
