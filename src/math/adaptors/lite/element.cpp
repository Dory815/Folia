module folia.math.adaptors.lite.element;
import folia.math.util.property_host;
import std;
namespace folia::math::adaptors::lite {
PropertyList LiteElement::spread(const PropertyList& source) {
    PropertyList result;
    for (const auto& key : source.keys()) result.set(key, source.get(key));
    return result;
}
namespace {
Property valueForNode(LiteNodeRef node){return nodeProperty(node);}
void assignParent(const Property& child, Property parent){
    auto projected=propertyNode(child);
    if(auto* node=std::get_if<LiteNodeRef>(&projected)){liteStore(*node)->object(*node).setOwn("parent",std::move(parent));return;}
    if(auto object=PropertyList::record(child)){object->setOwn("parent",std::move(parent));return;}
    if(child.missing()||std::holds_alternative<PropertyNull>(child.value))
        property_host::typeError("Cannot set properties of " + std::string(child.missing()?"undefined":"null") + " (setting 'parent')");
    property_host::typeError("Cannot create property 'parent' on primitive value");
}
}
LiteElement::LiteElement(std::u16string kind, PropertyList attributes, PropertyArray children, std::shared_ptr<LiteNodeStore> store) {
    if(!store)store=LiteNodeStore::create();
    ref_ = store->create(NodeKind::Element, std::move(kind));
    auto& node = store->record(ref_);
    node.attributes = spread(attributes);
    node.children = PropertyArray{};
    for(std::size_t i=0;i<children.size();++i)node.children.push(children.get(i));
    for(std::size_t i=0;i<node.children.size();++i)assignParent(node.children.get(i),valueForNode(ref_));
}
LiteElement::LiteElement(Property kind, PropertyList attributes, PropertyArray children, std::shared_ptr<LiteNodeStore> store) {
    if(!store)store=LiteNodeStore::create();
    ref_=store->create(NodeKind::Element);
    auto& node=store->record(ref_);node.kind=std::move(kind);node.attributes=spread(attributes);
    node.children=PropertyArray{};for(std::size_t i=0;i<children.size();++i)node.children.push(children.get(i));
    for(std::size_t i=0;i<node.children.size();++i)assignParent(node.children.get(i),valueForNode(ref_));
}
LiteElement::LiteElement(LiteNodeRef ref) : ref_(std::move(ref)) {
    if (!ref_ || recordConst(ref_).nodeKind != NodeKind::Element) throw std::invalid_argument("LiteNode is not a LiteElement");
}
std::u16string LiteElement::kind() const { return property_host::string(nodeField(ref_,"kind")); }
PropertyList LiteElement::attributes() const { return attributeList(ref_); }
PropertyArray LiteElement::children() const { return childArray(ref_); }
LiteNodeValue LiteElement::parent() const { return parentValue(ref_); }
PropertyList LiteElement::properties() const { return recordConst(ref_).properties; }
std::shared_ptr<styles::Styles> LiteElement::styles() const { return liteStore(ref_)->nodeStyles(ref_); }
}
