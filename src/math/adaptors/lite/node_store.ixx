// Shared identity-bearing record graph for the MathJax LiteAdaptor.
// Source files remain split into their original node/adaptor responsibilities;
// this module owns only stable node identity and graph storage.
export module folia.math.adaptors.lite.node_store;
export import folia.math.core.dom_node;
import folia.math.core.tree.property;
import folia.math.util.styles;
import folia.math.util.property_host;
import std;

export namespace folia::math::adaptors::lite {
using NodeId = std::uint64_t;
enum class NodeKind { Element, Text, Comment, Document, Window, List };
using LiteNodeRef = DOMNodeRef;
using LiteNodeValue = DOMNodeValue;
class LiteNodeStore;
// A Scene-backed owner view lets the original Lite DOM identity expose fields
// stored by its actual downstream owner without retaining a second Lite copy.
// Implementations are owned by the Scene/session; LiteNodeStore holds weak
// references and fails closed if a bound owner has expired.
class LiteNodeOwnerView {
public:
    virtual ~LiteNodeOwnerView() = default;
    // Property protocol covers kind/attributes/children/parent/styles, where
    // the latter may be a Context-backed Property view. Typed style access is
    // also exposed for C++ LiteAdaptor operations.
    virtual Property getProperty(std::string_view field) const = 0;
    virtual void setProperty(std::string_view field, Property value) = 0;
    virtual void setPropertyFromContext(std::string_view field,Property value){setProperty(field,std::move(value));}
    virtual void removeProperty(std::string_view field){setProperty(field,Property(Missing{}));}
    virtual bool hasOwnProperty(std::string_view field) const = 0;
    virtual std::vector<Property> childrenSnapshot()const{return {};}
    virtual std::shared_ptr<styles::Styles> getStyles() const = 0;
    virtual void setStyles(std::shared_ptr<styles::Styles> value) = 0;
};
// A canonical Context identity may project native Lite field operations
// through its current own descriptor. This is deliberately separate from the
// backing owner view used by NativeDataField callbacks.
class LiteNodeContextProjection {
public:
    virtual ~LiteNodeContextProjection() = default;
    virtual Property getProperty(std::string_view field) const = 0;
    virtual void setProperty(std::string_view field,Property value) = 0;
    virtual void defineOwnProperty(std::string_view field,Property value) = 0;
    virtual bool hasProperty(std::string_view field) const = 0;
    virtual bool hasOwnProperty(std::string_view field) const = 0;
    virtual std::vector<std::string> keys() const = 0;
    virtual void eraseProperty(std::string_view field) = 0;
    virtual std::shared_ptr<styles::Styles> getStyles()const=0;
    virtual void setStyles(std::shared_ptr<styles::Styles> value)=0;
};
struct OwnerViewLookup {
    enum class Status { none, live, expired } status = Status::none;
    std::shared_ptr<LiteNodeOwnerView> view;
};
struct ContextProjectionLookup {
    enum class Status { none, live, expired } status = Status::none;
    std::shared_ptr<LiteNodeContextProjection> projection;
};
struct LiteNodeRecord {
    NodeId id = 0;
    NodeKind nodeKind = NodeKind::Element;
    Property kind = Missing{};
    Property value = Missing{};
    PropertyList attributes;
    PropertyList properties;
    // These are the Source arrays themselves.  Copies of PropertyArray are
    // handles, so retained `children`/`List.nodes` references keep the same
    // array identity until Source replaces the field.
    PropertyArray children;
    Property parent = Missing{};
    bool parentOwn = false;
    std::shared_ptr<styles::Styles> styles;
    Property documentType = Missing{};
    Property root = Missing{};
    Property head = Missing{};
    Property body = Missing{};
    Property defaultView = Property(PropertyNull{});
    Property document = Missing{};
    PropertyArray listNodes;
    PropertyList object;
    bool objectBound = false;
};

class LiteNodePropertyBinding final : public DOMNodePropertyBinding {
    std::weak_ptr<LiteNodeStore> store_;
    NodeId id_ = 0;
public:
    LiteNodePropertyBinding(std::weak_ptr<LiteNodeStore> store, NodeId id)
        : store_(std::move(store)), id_(id) {}
    LiteNodeRef ref() const;
    DOMNodeRef nodeRef() const override { return ref(); }
    Property get(std::string_view name) override;
    Property getOwn(std::string_view name) override;
    bool has(std::string_view name) const override;
    bool hasOwn(std::string_view name) const override;
    void set(std::string_view name, Property value) override;
    void setOwn(std::string_view name, Property value) override;
    void erase(std::string_view name) override;
    std::vector<std::string> keys() const override;
    Property prototype() const override;
    Property primitive(options::PrimitiveHint hint) override;
};

class LiteNodeStore : public DOMNodeStore, public std::enable_shared_from_this<LiteNodeStore> {
    NodeId next_ = 1;
    std::uint64_t retirementGeneration_=0;
    std::unordered_map<NodeId, LiteNodeRecord> records_;
    std::unordered_map<NodeId, std::weak_ptr<LiteNodeOwnerView>> ownerViews_;
    std::unordered_map<NodeId, std::weak_ptr<LiteNodeContextProjection>> contextProjections_;
    LiteNodeStore() = default;
public:
    struct RequestMark { const LiteNodeStore* owner=nullptr;NodeId first=0;std::uint64_t generation=0; };
    static std::shared_ptr<LiteNodeStore> create();
    RequestMark requestMark()const noexcept{return {this,next_,retirementGeneration_};}
    void retireRecordsSince(RequestMark);
    std::size_t recordCountForDiagnostics()const noexcept{return records_.size();}
    LiteNodeRef create(NodeKind kind, std::u16string name = {}, std::u16string value = {});
    LiteNodeRecord& record(LiteNodeRef node);
    const LiteNodeRecord& record(LiteNodeRef node) const;
    bool owns(LiteNodeRef node) const noexcept;
    Property nodeProperty(std::uint64_t id) override;
    std::optional<std::uint64_t> nodeId(const Property& value) const override;
    LiteNodeRef ref(NodeId id);
    LiteNodeRef ref(NodeId id) const;
    PropertyList object(LiteNodeRef node);
    void bindObject(LiteNodeRef node, PropertyList object);
    static std::optional<LiteNodeRef> fromProperty(const Property& value);
    void append(LiteNodeRef parent, LiteNodeRef child);
    void setParent(LiteNodeRef child, Property parent);
    void bindOwnerView(LiteNodeRef node, std::weak_ptr<LiteNodeOwnerView> view);
    OwnerViewLookup ownerView(LiteNodeRef node) const;
    void bindContextProjection(LiteNodeRef node,std::weak_ptr<LiteNodeContextProjection> projection);
    ContextProjectionLookup contextProjection(LiteNodeRef node) const;
    Property field(LiteNodeRef node, std::string_view name) const;
    void setField(LiteNodeRef node, std::string_view name, Property value);
    Property backingField(LiteNodeRef node,std::string_view name)const;
    void backingSetField(LiteNodeRef node,std::string_view name,Property value);
    void backingSetFieldFromContext(LiteNodeRef node,std::string_view name,Property value);
    void backingSetFieldRemovedFromContext(LiteNodeRef node,std::string_view name);
    std::shared_ptr<styles::Styles> backingNodeStyles(LiteNodeRef node)const;
    void backingSetNodeStyles(LiteNodeRef node,std::shared_ptr<styles::Styles> value);
    std::shared_ptr<styles::Styles> nodeStyles(LiteNodeRef node) const;
    void setNodeStyles(LiteNodeRef node, std::shared_ptr<styles::Styles> value);
};

inline std::shared_ptr<LiteNodeStore> liteStore(LiteNodeRef node) {
    auto store = std::dynamic_pointer_cast<LiteNodeStore>(node.store);
    if (!store || !node.id) throw std::invalid_argument("invalid LiteAdaptor node handle");
    return store;
}

inline LiteNodeRecord& record(LiteNodeRef node) {
    if (!node) throw std::invalid_argument("invalid LiteAdaptor node handle");
    return liteStore(node)->record(node);
}
inline const LiteNodeRecord& recordConst(LiteNodeRef node) {
    if (!node) throw std::invalid_argument("invalid LiteAdaptor node handle");
    return std::as_const(*liteStore(node)).record(node);
}
inline LiteNodeValue parentValue(LiteNodeRef node) {
    return propertyNode(liteStore(node)->field(node, "parent"));
}
inline Property nodeField(LiteNodeRef node, std::string_view name) {
    return liteStore(node)->field(node, name);
}
inline void setNodeField(LiteNodeRef node, std::string_view name, Property value) {
    liteStore(node)->setField(node, name, std::move(value));
}
inline PropertyArray childArray(LiteNodeRef node) {
    auto value = nodeField(node, "children");
    auto array = PropertyArray::array(value);
    if (!array) throw options::Exception("TypeError", u"Lite node children is not an array");
    return *array;
}
// LiteAdaptor.childNodes is Source's `[...node.children]` operation. It has
// different read semantics from mutators such as append, which fetch `push`.
inline PropertyArray childNodesArray(LiteNodeRef node) {
    const auto lookup=liteStore(node)->ownerView(node);
    if(lookup.status==OwnerViewLookup::Status::expired)
        throw options::Exception("TypeError",u"Lite node owner view has expired");
    if(lookup.status==OwnerViewLookup::Status::live){
        PropertyArray result;
        for(auto& value:lookup.view->childrenSnapshot())result.push(std::move(value));
        return result;
    }
    auto value=nodeField(node,"children");
    if(auto array=PropertyArray::array(value))return *array;
    if(const auto* text=std::get_if<std::string>(&value.value)){
        const auto source=property_host::string(value);
        PropertyArray result;
        for(std::size_t index=0;index<source.size();++index){
            std::u16string unit(1,source[index]);
            if(source[index]>=0xD800&&source[index]<=0xDBFF&&index+1<source.size()&&
               source[index+1]>=0xDC00&&source[index+1]<=0xDFFF)unit.push_back(source[++index]);
            result.push(Property(property_host::debugText(unit)));
        }
        (void)text;
        return result;
    }
    if(std::holds_alternative<double>(value.value)||std::holds_alternative<bool>(value.value))return {};
    if(value.missing()||std::holds_alternative<PropertyNull>(value.value))
        throw options::Exception("TypeError",u"Source children value is not iterable");
    throw options::Exception("TypeError",u"Source children value is not iterable");
}
inline PropertyList attributeList(LiteNodeRef node) {
    auto value = nodeField(node, "attributes");
    auto list = PropertyList::record(value);
    if (!list) throw options::Exception("TypeError", u"Lite node attributes is not an object");
    return *list;
}
inline LiteNodeRef nodeFromProperty(const Property& value) {
    auto node=LiteNodeStore::fromProperty(value);
    if(!node)throw options::Exception("TypeError",u"Expected a LiteAdaptor node object");
    return *node;
}
inline std::vector<LiteNodeRef> nodeArrayValues(const PropertyArray& values) {
    std::vector<LiteNodeRef> result;result.reserve(values.size());
    for(std::size_t i=0;i<values.size();++i){
        if(!values.has(i))throw options::Exception("TypeError",u"LiteAdaptor node array contains a hole");
        result.push_back(nodeFromProperty(values.get(i)));
    }
    return result;
}
inline void appendNode(PropertyArray& values,LiteNodeRef node){values.push(folia::math::nodeProperty(node));}
}
