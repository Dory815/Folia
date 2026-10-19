module folia.math.core.mml_tree.xml_dom_adaptor;
import folia.math.util.utf16;
import folia.math.util.option_types;
import std;

namespace folia::math::mml {
namespace {
struct NodeKey {
    const folia::math::DOMNodeStore* store=nullptr;
    std::uint64_t id=0;
    friend bool operator==(const NodeKey&,const NodeKey&)=default;
};
struct NodeKeyHash {
    std::size_t operator()(const NodeKey& key)const noexcept {
        const auto a=std::hash<const void*>{}(key.store);
        const auto b=std::hash<std::uint64_t>{}(key.id);
        return a^(b+0x9e3779b97f4a7c15ull+(a<<6)+(a>>2));
    }
};

class DomXmlObject final : public XmlObject {
public:
    folia::math::DOMNodeRef node;
    std::shared_ptr<XmlDomAdaptor> owner;
    DomXmlObject(folia::math::DOMNodeRef value,std::shared_ptr<XmlDomAdaptor> bridge)
        :node(std::move(value)),owner(std::move(bridge)){}
};

[[noreturn]] void typeError(std::u16string message){
    throw folia::math::options::Exception("TypeError",std::move(message));
}
std::u16string decode(std::string_view value){return folia::math::utf16::decode(value).text;}
}

struct XmlDomAdaptor::State {
    std::shared_ptr<folia::math::AbstractDOMAdaptor> dom;
    std::unordered_map<NodeKey,std::weak_ptr<XmlObject>,NodeKeyHash> identities;
    explicit State(std::shared_ptr<folia::math::AbstractDOMAdaptor> value):dom(std::move(value)){}
};

XmlDomAdaptor::XmlDomAdaptor(std::shared_ptr<folia::math::AbstractDOMAdaptor> dom)
    :state_(std::make_shared<State>(std::move(dom))){
    if(!state_->dom)typeError(u"XmlDomAdaptor requires an actual DOM adaptor");
}
std::shared_ptr<XmlDomAdaptor> XmlDomAdaptor::create(
    std::shared_ptr<folia::math::AbstractDOMAdaptor> dom){
    return std::shared_ptr<XmlDomAdaptor>(new XmlDomAdaptor(std::move(dom)));
}
XmlDomAdaptor::~XmlDomAdaptor()=default;

std::shared_ptr<folia::math::AbstractDOMAdaptor> XmlDomAdaptor::domAdaptor()const{
    return state_->dom;
}
XmlRef XmlDomAdaptor::wrap(folia::math::DOMNodeRef node){
    if(!node||!node.store)typeError(u"Cannot wrap an invalid DOM node");
    const NodeKey key{node.store.get(),node.id};
    if(const auto found=state_->identities.find(key);found!=state_->identities.end()){
        if(auto existing=found->second.lock())return existing;
        state_->identities.erase(found);
    }
    auto result=std::make_shared<DomXmlObject>(std::move(node),shared_from_this());
    state_->identities.emplace(key,result);
    return result;
}
folia::math::DOMNodeRef XmlDomAdaptor::resolve(XmlRef object)const{
    if(!object)typeError(u"Cannot read properties of null (reading 'node')");
    auto actual=std::dynamic_pointer_cast<DomXmlObject>(object);
    if(!actual)typeError(u"XML object is not backed by the output DOM store");
    return actual->node;
}
XmlRef XmlDomAdaptor::fromDOM(folia::math::DOMNodeRef node){return wrap(std::move(node));}
Property XmlDomAdaptor::nodeProperty(XmlRef object)const{
    return folia::math::nodeProperty(resolve(std::move(object)));
}

XmlRef XmlDomAdaptor::clone(XmlRef object){
    return wrap(state_->dom->clone(resolve(std::move(object)),true));
}
std::string XmlDomAdaptor::serializeXML(XmlRef object){
    return folia::math::utf16::encode(state_->dom->serializeXML(resolve(std::move(object))));
}
Property XmlDomAdaptor::getAttribute(XmlRef object,std::string_view name){
    return state_->dom->getAttribute(resolve(std::move(object)),decode(name));
}
XmlRef XmlDomAdaptor::node(std::string_view kind,PropertyList definition,
                           std::vector<XmlRef> children){
    PropertyArray childSlots;
    for(auto& child:children)
        childSlots.push(Property(folia::math::nodeProperty(resolve(std::move(child)))));
    return wrap(state_->dom->node(decode(kind),std::move(definition),std::move(childSlots)));
}
std::string XmlDomAdaptor::outerHTML(XmlRef object){
    return folia::math::utf16::encode(state_->dom->outerHTML(resolve(std::move(object))));
}
std::u16string XmlDomAdaptor::textContent(XmlRef object){
    return state_->dom->textContent(resolve(std::move(object)));
}
std::string XmlDomAdaptor::kind(XmlRef object){
    return folia::math::utf16::encode(state_->dom->kind(resolve(std::move(object))));
}
}
