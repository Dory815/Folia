module folia.math.adaptors.lite.node_store;
import folia.math.util.property_host;
import std;

namespace folia::math::adaptors::lite {
std::shared_ptr<LiteNodeStore> LiteNodeStore::create() {
    return std::shared_ptr<LiteNodeStore>(new LiteNodeStore());
}
void LiteNodeStore::retireRecordsSince(RequestMark mark){
    if(mark.owner!=this||mark.generation!=retirementGeneration_||mark.first==0||mark.first>next_)
        throw std::invalid_argument("Lite request mark does not belong to the current store state");
    for(auto it=records_.begin();it!=records_.end();){
        if(it->first>=mark.first){
            ownerViews_.erase(it->first);
            contextProjections_.erase(it->first);
            it=records_.erase(it);
        }else ++it;
    }
    ++retirementGeneration_;
}
LiteNodeRef LiteNodeStore::create(NodeKind kind, std::u16string name, std::u16string value) {
    const auto id = next_++;
    LiteNodeRecord node;
    node.id = id;
    node.nodeKind = kind;
    node.kind = Property(property_host::debugText(name));
    node.value = Property(property_host::debugText(value));
    records_.emplace(id, std::move(node));
    return {std::static_pointer_cast<DOMNodeStore>(shared_from_this()), id};
}
LiteNodeRecord& LiteNodeStore::record(LiteNodeRef node) {
    if (node.store.get() != this || !node.id) throw std::invalid_argument("LiteAdaptor node belongs to another store");
    auto found = records_.find(node.id);
    if (found == records_.end()) throw std::invalid_argument("unknown LiteAdaptor node");
    return found->second;
}
const LiteNodeRecord& LiteNodeStore::record(LiteNodeRef node) const {
    if (node.store.get() != this || !node.id) throw std::invalid_argument("LiteAdaptor node belongs to another store");
    auto found = records_.find(node.id);
    if (found == records_.end()) throw std::invalid_argument("unknown LiteAdaptor node");
    return found->second;
}
bool LiteNodeStore::owns(LiteNodeRef node) const noexcept { return node.store.get() == this && records_.contains(node.id); }
Property LiteNodeStore::nodeProperty(std::uint64_t id) { return Property(object(ref(id))); }
std::optional<std::uint64_t> LiteNodeStore::nodeId(const Property& value) const {
    const auto storage = std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);
    if (!storage || !*storage || !(*storage)->binding) return {};
    const auto binding = std::dynamic_pointer_cast<DOMNodePropertyBinding>((*storage)->binding);
    if (!binding) return {};
    const auto ref = binding->nodeRef();
    if (ref.store.get() != this || !records_.contains(ref.id)) return {};
    return ref.id;
}
LiteNodeRef LiteNodeStore::ref(NodeId id) { return {std::static_pointer_cast<DOMNodeStore>(shared_from_this()), id}; }
LiteNodeRef LiteNodeStore::ref(NodeId id) const {
    return {std::static_pointer_cast<DOMNodeStore>(std::const_pointer_cast<LiteNodeStore>(shared_from_this())), id};
}
void LiteNodeStore::append(LiteNodeRef parentNode, LiteNodeRef child) {
    auto children = childArray(parentNode);
    children.push(folia::math::nodeProperty(child));
    setField(child, "parent", folia::math::nodeProperty(parentNode));
}
void LiteNodeStore::setParent(LiteNodeRef child, Property parent) { setField(child, "parent", std::move(parent)); }
void LiteNodeStore::bindOwnerView(LiteNodeRef node, std::weak_ptr<LiteNodeOwnerView> view) {
    record(node);
    auto incoming = view.lock();
    if (!incoming) throw std::invalid_argument("Lite owner view must be live when bound");
    if (auto found = ownerViews_.find(node.id); found != ownerViews_.end()) {
        if (auto current = found->second.lock()) {
            if (current == incoming) return;
            throw std::logic_error("Lite node already has a different owner view");
        }
    }
    ownerViews_.insert_or_assign(node.id, std::move(view));
}
OwnerViewLookup LiteNodeStore::ownerView(LiteNodeRef node) const {
    record(node);
    auto found = ownerViews_.find(node.id);
    if (found == ownerViews_.end()) return {};
    if (auto view = found->second.lock()) return {OwnerViewLookup::Status::live, std::move(view)};
    return {OwnerViewLookup::Status::expired, {}};
}
void LiteNodeStore::bindContextProjection(LiteNodeRef node,std::weak_ptr<LiteNodeContextProjection> projection){
    record(node);
    auto incoming=projection.lock();
    if(!incoming)throw std::invalid_argument("Lite Context projection must be live when bound");
    if(auto found=contextProjections_.find(node.id);found!=contextProjections_.end()){
        if(auto current=found->second.lock()){
            if(current==incoming)return;
            throw std::logic_error("Lite node already has a different Context projection");
        }
    }
    contextProjections_.insert_or_assign(node.id,std::move(projection));
}
ContextProjectionLookup LiteNodeStore::contextProjection(LiteNodeRef node)const{
    record(node);
    const auto found=contextProjections_.find(node.id);
    if(found==contextProjections_.end())return{};
    if(auto projection=found->second.lock())return{ContextProjectionLookup::Status::live,std::move(projection)};
    return{ContextProjectionLookup::Status::expired,{}};
}
Property LiteNodeStore::field(LiteNodeRef node,std::string_view name)const{
    const auto projection=contextProjection(node);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    if(projection.status==ContextProjectionLookup::Status::live)
        return projection.projection->getProperty(name);
    return backingField(node,name);
}
Property LiteNodeStore::backingField(LiteNodeRef node,std::string_view name)const{
    const auto lookup = ownerView(node);
    if (lookup.status == OwnerViewLookup::Status::expired)
        throw options::Exception("TypeError", u"Lite node owner view has expired");
    if (lookup.status == OwnerViewLookup::Status::live) return lookup.view->getProperty(name);
    const auto& r = record(node);
    if (name == "kind" && r.nodeKind == NodeKind::Element) return r.kind;
    if (name == "attributes" && r.nodeKind == NodeKind::Element) return Property(r.attributes);
    if (name == "children" && r.nodeKind == NodeKind::Element) return Property(r.children);
    if (name == "value" && (r.nodeKind == NodeKind::Text || r.nodeKind == NodeKind::Comment)) return r.value;
    if (name == "parent") return r.parent;
    if (name == "styles" && r.nodeKind == NodeKind::Element) return r.properties.getOwn("styles");
    throw std::invalid_argument("unsupported Lite owner field");
}
void LiteNodeStore::setField(LiteNodeRef node, std::string_view name, Property value) {
    const auto projection=contextProjection(node);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    if(projection.status==ContextProjectionLookup::Status::live){
        projection.projection->setProperty(name,std::move(value));
        return;
    }
    backingSetField(node,name,std::move(value));
}
void LiteNodeStore::backingSetFieldFromContext(LiteNodeRef node,std::string_view name,Property value){
    const auto lookup=ownerView(node);
    if(lookup.status==OwnerViewLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node owner view has expired");
    if(lookup.status==OwnerViewLookup::Status::live){
        lookup.view->setPropertyFromContext(name,std::move(value));
        return;
    }
    backingSetField(node,name,std::move(value));
}
void LiteNodeStore::backingSetField(LiteNodeRef node,std::string_view name,Property value){
    const auto lookup = ownerView(node);
    if (lookup.status == OwnerViewLookup::Status::expired)
        throw options::Exception("TypeError", u"Lite node owner view has expired");
    if (lookup.status == OwnerViewLookup::Status::live) {
        lookup.view->setProperty(name, std::move(value));
        return;
    }
    auto& r = record(node);
    if (name == "kind" && r.nodeKind == NodeKind::Element) { r.kind = std::move(value); return; }
    if (name == "attributes" && r.nodeKind == NodeKind::Element) {
        auto list = PropertyList::record(value);
        if (!list) property_host::typeError("attributes must be an object");
        r.attributes = *list; return;
    }
    if (name == "children" && r.nodeKind == NodeKind::Element) {
        auto array = PropertyArray::array(value);
        if (!array) property_host::typeError("children must be an array");
        r.children = *array; return;
    }
    if (name == "value" && (r.nodeKind == NodeKind::Text || r.nodeKind == NodeKind::Comment)) { r.value=std::move(value); return; }
    if (name == "parent") { r.parentOwn=!value.missing();r.parent = std::move(value);return; }
    if (name == "styles" && r.nodeKind == NodeKind::Element) { r.properties.setOwn("styles", std::move(value)); return; }
    throw std::invalid_argument("unsupported Lite owner field");
}
void LiteNodeStore::backingSetFieldRemovedFromContext(LiteNodeRef node,std::string_view name){
    const auto lookup=ownerView(node);
    if(lookup.status==OwnerViewLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node owner view has expired");
    if(lookup.status==OwnerViewLookup::Status::live){
        lookup.view->removeProperty(name);
        return;
    }
    auto& r=record(node);
    if(name=="parent"){r.parent=Missing{};r.parentOwn=false;return;}
    if(name=="kind"&&r.nodeKind==NodeKind::Element){r.kind=Missing{};return;}
    if(name=="attributes"&&r.nodeKind==NodeKind::Element){r.attributes=PropertyList{};return;}
    if(name=="children"&&r.nodeKind==NodeKind::Element){r.children=PropertyArray{};return;}
    if(name=="value"&&(r.nodeKind==NodeKind::Text||r.nodeKind==NodeKind::Comment)){r.value=Missing{};return;}
    if(name=="styles"&&r.nodeKind==NodeKind::Element){r.properties.erase("styles");r.styles.reset();return;}
    throw std::invalid_argument("unsupported Lite owner field");
}
std::shared_ptr<styles::Styles> LiteNodeStore::nodeStyles(LiteNodeRef node) const {
    const auto projection=contextProjection(node);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    if(projection.status==ContextProjectionLookup::Status::live)return projection.projection->getStyles();
    return backingNodeStyles(node);
}
std::shared_ptr<styles::Styles> LiteNodeStore::backingNodeStyles(LiteNodeRef node)const{
    const auto lookup = ownerView(node);
    if (lookup.status == OwnerViewLookup::Status::expired)
        throw options::Exception("TypeError", u"Lite node owner view has expired");
    return lookup.status == OwnerViewLookup::Status::live ? lookup.view->getStyles() : record(node).styles;
}
void LiteNodeStore::setNodeStyles(LiteNodeRef node, std::shared_ptr<styles::Styles> value) {
    const auto projection=contextProjection(node);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    if(projection.status==ContextProjectionLookup::Status::live){projection.projection->setStyles(std::move(value));return;}
    backingSetNodeStyles(node,std::move(value));
}
void LiteNodeStore::backingSetNodeStyles(LiteNodeRef node,std::shared_ptr<styles::Styles> value){
    const auto lookup = ownerView(node);
    if (lookup.status == OwnerViewLookup::Status::expired)
        throw options::Exception("TypeError", u"Lite node owner view has expired");
    if (lookup.status == OwnerViewLookup::Status::live) lookup.view->setStyles(std::move(value));
    else record(node).styles = std::move(value);
}
PropertyList LiteNodeStore::object(LiteNodeRef node) {
    auto& r=record(node);
    if(!r.objectBound){
        r.object=PropertyList::live(std::make_shared<LiteNodePropertyBinding>(shared_from_this(),node.id));
        r.objectBound=true;
    }
    return r.object;
}
void LiteNodeStore::bindObject(LiteNodeRef node, PropertyList object) {
    auto& r = record(node);
    if (r.objectBound) throw std::logic_error("Lite node already has a canonical object binding");
    r.object = std::move(object);
    r.objectBound = true;
}
std::optional<LiteNodeRef> LiteNodeStore::fromProperty(const Property& value){
    auto object=PropertyList::object(value);if(!object)return{};
    auto storage=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);
    if(!storage||!*storage||!(*storage)->binding)return{};
    auto binding=std::dynamic_pointer_cast<DOMNodePropertyBinding>((*storage)->binding);
    if(!binding)return{};
    return binding->nodeRef();
}

LiteNodeRef LiteNodePropertyBinding::ref()const{
    auto store=store_.lock();if(!store)throw std::invalid_argument("expired LiteAdaptor node store");
    return store->ref(id_);
}
Property LiteNodePropertyBinding::get(std::string_view name){
    auto n=ref();auto& r=record(n);
    const auto store=store_.lock();
    if(!store)throw options::Exception("TypeError",u"Lite node store has expired");
    const auto projection=store->contextProjection(n);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    if(projection.status==ContextProjectionLookup::Status::live)
        return projection.projection->getProperty(name);
    if(name=="kind"&&r.nodeKind==NodeKind::Element)return nodeField(n,"kind");
    if(name=="kind"&&r.nodeKind==NodeKind::Text)return Property(property_host::debugText(u"#text"));
    if(name=="kind"&&r.nodeKind==NodeKind::Comment)return Property(property_host::debugText(u"#comment"));
    if(name=="kind"&&r.nodeKind==NodeKind::Document)return Property(property_host::debugText(u"#document"));
    if(name=="value"&&(r.nodeKind==NodeKind::Text||r.nodeKind==NodeKind::Comment))return nodeField(n,"value");
    if(name=="attributes"&&r.nodeKind==NodeKind::Element)return nodeField(n,"attributes");
    if(name=="children"&&r.nodeKind==NodeKind::Element)return nodeField(n,"children");
    if(name=="parent")return nodeField(n,"parent");
    if(name=="styles"&&r.nodeKind==NodeKind::Element)return nodeField(n,"styles");
    if(name=="root"&&r.nodeKind==NodeKind::Document)return r.root;
    if(name=="head"&&r.nodeKind==NodeKind::Document)return r.head;
    if(name=="body"&&r.nodeKind==NodeKind::Document)return r.body;
    if(name=="type"&&r.nodeKind==NodeKind::Document)return r.documentType;
    if(name=="defaultView"&&r.nodeKind==NodeKind::Document)return r.defaultView;
    if(name=="document"&&r.nodeKind==NodeKind::Window)return r.document;
    if(name=="nodes"&&r.nodeKind==NodeKind::List)return Property(r.listNodes);
    return r.properties.get(name);
}
Property LiteNodePropertyBinding::getOwn(std::string_view name){
    auto n=ref();auto& r=record(n);
    const auto store=store_.lock();
    if(!store)throw options::Exception("TypeError",u"Lite node store has expired");
    const auto projection=store->contextProjection(n);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    if(projection.status==ContextProjectionLookup::Status::live)
        return projection.projection->hasOwnProperty(name)
            ?projection.projection->getProperty(name):Property(Missing{});
    const bool own=(r.nodeKind==NodeKind::Element&&(name=="kind"||name=="attributes"||name=="children"||name=="parent"||name=="styles"))||
        ((r.nodeKind==NodeKind::Text||r.nodeKind==NodeKind::Comment)&&(name=="value"||name=="parent"))||
        (r.nodeKind==NodeKind::Document&&(name=="defaultView"||name=="root"||name=="head"||name=="body"||name=="type"))||
        (r.nodeKind==NodeKind::Window&&name=="document")||(r.nodeKind==NodeKind::List&&name=="nodes");
    if(!own)return r.properties.getOwn(name);
    return hasOwn(name)?get(name):Property(Missing{});
}
bool LiteNodePropertyBinding::has(std::string_view name)const{
    const auto node=ref();const auto store=store_.lock();
    if(!store)throw options::Exception("TypeError",u"Lite node store has expired");
    const auto projection=store->contextProjection(node);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    return projection.status==ContextProjectionLookup::Status::live
        ?projection.projection->hasProperty(name):hasOwn(name);
}
bool LiteNodePropertyBinding::hasOwn(std::string_view name)const{
    auto n=ref();const auto& r=recordConst(n);
    const auto store=store_.lock();
    if(!store)throw options::Exception("TypeError",u"Lite node store has expired");
    const auto projection=store->contextProjection(n);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    if(projection.status==ContextProjectionLookup::Status::live)
        return projection.projection->hasOwnProperty(name);
    if(name=="parent"){
        const auto lookup=store->ownerView(n);
        if(lookup.status==OwnerViewLookup::Status::expired)
            throw options::Exception("TypeError",u"Lite node owner view has expired");
        return lookup.status==OwnerViewLookup::Status::live
            ?lookup.view->hasOwnProperty(name):r.parentOwn;
    }
    const bool own=(r.nodeKind==NodeKind::Element&&(name=="kind"||name=="attributes"||name=="children"||name=="styles"))||
        ((r.nodeKind==NodeKind::Text||r.nodeKind==NodeKind::Comment)&&name=="value")||
        (r.nodeKind==NodeKind::Document&&(name=="defaultView"||name=="root"||name=="head"||name=="body"||name=="type"))||
        (r.nodeKind==NodeKind::Window&&name=="document")||(r.nodeKind==NodeKind::List&&name=="nodes");
    return own||r.properties.hasOwn(name);
}
void LiteNodePropertyBinding::set(std::string_view name,Property value){
    const auto node=ref();const auto store=store_.lock();
    if(!store)throw options::Exception("TypeError",u"Lite node store has expired");
    const auto projection=store->contextProjection(node);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    if(projection.status==ContextProjectionLookup::Status::live){
        projection.projection->setProperty(name,std::move(value));
        return;
    }
    setOwn(name,std::move(value));
}
void LiteNodePropertyBinding::setOwn(std::string_view name,Property value){
    auto n=ref();auto& r=record(n);
    const auto store=store_.lock();
    if(!store)throw options::Exception("TypeError",u"Lite node store has expired");
    const auto projection=store->contextProjection(n);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    if(projection.status==ContextProjectionLookup::Status::live){
        projection.projection->defineOwnProperty(name,std::move(value));
        return;
    }
    if(name=="kind"&&r.nodeKind==NodeKind::Element){setNodeField(n,"kind",std::move(value));return;}
    if(name=="value"&&(r.nodeKind==NodeKind::Text||r.nodeKind==NodeKind::Comment)){setNodeField(n,"value",std::move(value));return;}
    if(name=="attributes"&&r.nodeKind==NodeKind::Element){setNodeField(n,"attributes",std::move(value));return;}
    if(name=="children"&&r.nodeKind==NodeKind::Element){setNodeField(n,"children",std::move(value));return;}
    if(name=="parent"){setNodeField(n,"parent",std::move(value));return;}
    if(name=="styles"&&r.nodeKind==NodeKind::Element){setNodeField(n,"styles",std::move(value));return;}
    if(name=="type"&&r.nodeKind==NodeKind::Document){r.documentType=std::move(value);return;}
    if(name=="defaultView"&&r.nodeKind==NodeKind::Document){r.defaultView=std::move(value);return;}
    if(name=="root"&&r.nodeKind==NodeKind::Document){r.root=std::move(value);return;}
    if(name=="head"&&r.nodeKind==NodeKind::Document){r.head=std::move(value);return;}
    if(name=="body"&&r.nodeKind==NodeKind::Document){r.body=std::move(value);return;}
    if(name=="document"&&r.nodeKind==NodeKind::Window){r.document=std::move(value);return;}
    if(name=="nodes"&&r.nodeKind==NodeKind::List){auto p=PropertyArray::array(value);if(!p)property_host::typeError("nodes must be an array");r.listNodes=*p;return;}
    r.properties.setOwn(name,std::move(value));
}
void LiteNodePropertyBinding::erase(std::string_view name){
    const auto node=ref();
    const auto store=store_.lock();
    if(!store)throw options::Exception("TypeError",u"Lite node store has expired");
    const auto projection=store->contextProjection(node);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    const auto& r=recordConst(node);
    if(projection.status==ContextProjectionLookup::Status::live){
        projection.projection->eraseProperty(name);
        return;
    }
    const auto lookup=store->ownerView(node);
    if(lookup.status==OwnerViewLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node owner view has expired");
    if(name=="parent"&&lookup.status==OwnerViewLookup::Status::live){
        lookup.view->removeProperty(name);
        return;
    }
    record(node).properties.erase(name);
}
std::vector<std::string> LiteNodePropertyBinding::keys()const{
    auto n=ref();const auto&r=recordConst(n);std::vector<std::string> out;
    const auto store=store_.lock();
    if(!store)throw options::Exception("TypeError",u"Lite node store has expired");
    const auto projection=store->contextProjection(n);
    if(projection.status==ContextProjectionLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node Context projection has expired");
    if(projection.status==ContextProjectionLookup::Status::live)
        return projection.projection->keys();
    std::vector<std::string> fields;
    if(r.nodeKind==NodeKind::Element)fields={"kind","attributes","children","parent","styles"};
    else if(r.nodeKind==NodeKind::Text||r.nodeKind==NodeKind::Comment)fields={"value","parent"};
    else if(r.nodeKind==NodeKind::Document)fields={"defaultView","root","head","body","type"};
    else if(r.nodeKind==NodeKind::Window)fields={"document","DOMParser","NodeList","HTMLCollection","HTMLElement","DocumentFragment","Document"};
    else if(r.nodeKind==NodeKind::List)fields={"nodes"};
    for(const auto& field:fields)if(hasOwn(field))out.push_back(field);
    for(auto&k:r.properties.keys())if(std::find(out.begin(),out.end(),k)==out.end())out.push_back(k);
    return out;
}
Property LiteNodePropertyBinding::prototype()const{return Property(PropertyNull{});}
Property LiteNodePropertyBinding::primitive(options::PrimitiveHint){return Property(property_host::debugText(u"[object Object]"));}
}
