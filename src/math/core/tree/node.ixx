/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native C++ port of the generic Node behavior from
 * .reference/MathJax-src/ts/core/Tree/Node.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.core.tree.node;
export import folia.math.core.tree.node_algorithms;
import folia.math.util.property_host;
import std;

export namespace folia::math::tree {
template<class Product, class Base = Node<Product>>
class AbstractNode : public Base {
    using Receiver=TreeNodeReceiver<Product>;
protected:
    static Receiver nativeReceiver(Product* node) {
        if(!node)throw options::Exception("TypeError",u"Tree algorithm requires a native node");
        Receiver receiver;
        receiver.self=[node]{return ChildValue<Product>(node);};
        receiver.kind=[node]{return Property(node->kind());};
        receiver.kindPrefix=[node]{return node->kind();};
        receiver.properties=[node]{return Property(node->propertyOwner());};
        receiver.assignProperties=[node](Property value){
            auto record=PropertyList::record(value);
            if(!record)throw options::Exception("TypeError",u"Node properties must be a record");
            node->propertyOwner(*record);
        };
        // Bound PropertyList owners dispatch to Context::ownKeys/live Get/Set,
        // preserving enumerable Symbols and the target's __proto__ setter.
        // The unbound native owner exposes only its actual typed key domain.
        receiver.copyProperties=[node]{return Property(node->propertyOwner().copyOwnEnumerableProperties());};
        receiver.setProperty=[node](Property name,Property value){
            const auto text=property_host::debugText(property_host::string(name));
            node->propertyOwner().set(text,std::move(value));
        };
        receiver.getProperty=[node](Property name){
            const auto text=property_host::debugText(property_host::string(name));
            return node->propertyOwner().get(text);
        };
        receiver.getPropertyNames=[node]{return node->propertyOwner().keys();};
        receiver.deleteProperty=[node](Property name){
            const auto text=property_host::debugText(property_host::string(name));
            node->propertyOwner().erase(text);
        };
        receiver.children=[node]{return node->childArrayOwner();};
        receiver.copyChildren=[node]{
            auto children=node->childArrayOwner();
            return children?children:std::make_shared<ChildArray<Product>>();
        };
        receiver.assignChildren=[node](FactoryChildren<Product> value){node->childArrayOwner(std::move(value));};
        receiver.newChildren=[] {return std::make_shared<ChildArray<Product>>();};
        receiver.captureCreator=[node]{
            auto& factory=node->factoryOwner();
            return std::function<ChildValue<Product>(Property)>([&factory](Property kind){
                const auto name=property_host::debugText(property_host::string(kind));
                return ChildValue<Product>(factory.create(name,FactoryArgs{}));
            });
        };
        receiver.assignPropertiesOn=[](ChildValue<Product> target,Property value){
            auto* product=target.requireNodeForWrite("properties");
            auto record=PropertyList::record(value);
            if(!record)throw options::Exception("TypeError",u"Node properties must be a record");
            product->propertyOwner(*record);
        };
        receiver.appendOn=[](ChildValue<Product> target,ChildValue<Product> child){
            return target.requireNode("appendChild")->appendChild(std::move(child));
        };
        receiver.childIndex=[node](ChildValue<Product> child){
            // Preserve virtual `this.childIndex(child)` dispatch for derived
            // native nodes while the shared algorithm owns the operation.
            const auto index=node->childIndex(std::move(child));
            return index?Property(static_cast<double>(*index)):Property(PropertyNull{});
        };
        receiver.writeChildAt=[](FactoryChildren<Product> children,Property index,ChildValue<Product> value){
            const auto numeric=property_host::number(index);
            if(!std::isfinite(numeric)||numeric<0||std::trunc(numeric)!=numeric)
                throw options::Exception("TypeError",u"Native child index is not an integer");
            children->set(static_cast<std::size_t>(numeric),std::move(value));
        };
        receiver.spliceChildAt=[](FactoryChildren<Product> children,Property index){
            const auto numeric=property_host::number(index);
            if(!std::isfinite(numeric)||numeric<0||std::trunc(numeric)!=numeric)
                throw options::Exception("TypeError",u"Native child index is not an integer");
            children->splice(static_cast<std::int64_t>(numeric),std::size_t{1});
        };
        receiver.factoryNodeIsKind=[node](Property kind){
            return node->factoryOwner().nodeIsKind(node,property_host::debugText(property_host::string(kind)));
        };
        receiver.isKindOn=[](ChildValue<Product> value,Property kind){
            const auto name=property_host::debugText(property_host::string(kind));
            if(const auto source=value.sourceAccess())return source->isKind(name);
            return value.requireNode("isKind")->isKind(name);
        };
        receiver.walkOn=[](ChildValue<Product> value,const std::function<void(ChildValue<Product>,const std::any&)>& visit,
                           const std::any& data,const std::any& visitorIdentity){
            if(const auto source=value.sourceAccess()){
                source->walkTreeRaw(visit,data,visitorIdentity);
                return;
            }
            auto* const node=value.requireNode("walkTree");
            const TreeVisitor<Product> childVisit=[&visit](Product* child,const std::any& childData){
                visit(ChildValue<Product>(child),childData);
            };
            (void)node->walkTree(childVisit,data);
        };
        return receiver;
    }
public:
    explicit AbstractNode(FactoryServices<Product>& factory) : Base(factory) {}
    // Reuse the native receiver already consumed by TreeNodeAlgorithms.
    // This is an operation view, not a second owner or Context identity.
    Receiver treeReceiver() {
        return nativeReceiver(static_cast<Product*>(this));
    }
    AbstractNode(const AbstractNode&) = delete;
    AbstractNode& operator=(const AbstractNode&) = delete;
    AbstractNode(AbstractNode&&) = delete;
    AbstractNode& operator=(AbstractNode&&) = delete;
    virtual ~AbstractNode() = default;

    void initializeNode(FactoryArgs args = {}) {
        TreeConstructorReceiver receiver;
        receiver.writeOwnFields=[this]{
            this->setParent(PropertyNull{});
            propertyOwner(PropertyList{});
            this->childArrayOwner(std::make_shared<ChildArray<Product>>());
        };
        receiver.propertyKeys=[args]{
            if(args.empty()||!args[0].has_value())return std::vector<std::u16string>{};
            const auto* input=std::any_cast<PropertyList>(&args[0]);
            if(!input)type_error("Node properties must be a PropertyList");
            std::vector<std::u16string> result;
            for(const auto& key:input->keys())result.push_back(property_host::text(key));
            return result;
        };
        receiver.propertyValueNative=[args](std::u16string_view key){
            if(args.empty()||!args[0].has_value())return Property(Missing{});
            const auto* input=std::any_cast<PropertyList>(&args[0]);
            if(!input)type_error("Node properties must be a PropertyList");
            return input->get(property_host::debugText(key));
        };
        receiver.callSetPropertyNative=[this](std::u16string_view key,Property value){
            this->setProperty(property_host::debugText(key),std::move(value));
        };
        receiver.childLengthTruthy=[args]{
            if(args.size()<2||!args[1].has_value())return false;
            const auto* children=std::any_cast<FactoryChildren<Product>>(&args[1]);
            if(!children)type_error("Node children must be a child-array handle");
            if(!*children)type_error("Cannot read length of null children");
            return (*children)->lengthTruthy();
        };
        receiver.callSetChildren=[this,args]{
            const auto* children=std::any_cast<FactoryChildren<Product>>(&args[1]);
            if(!children)type_error("Node children must be a child-array handle");
            this->setChildren(*children);
        };
        TreeNodeAlgorithms<Product>::initialize(receiver);
    }

    std::string kind() const override { return "unknown"; }

    // The Context field aliases this actual PropertyList handle.
    PropertyList propertyOwner()const override{
        if(propertyReader_)return propertyReader_();
        if(sourcePropertiesRaw_){
            if(auto record=PropertyList::record(*sourcePropertiesRaw_))return *record;
            type_error("Node properties is not an object");
        }
        return properties;
    }
    void propertyOwner(PropertyList value)override{
        if(propertyWriter_){propertyWriter_(std::move(value));return;}
        sourcePropertiesRaw_.reset();
        properties=std::move(value);
    }
    void sourcePropertyAccessors(std::function<PropertyList()> read,
                                 std::function<void(PropertyList)> write){
        propertyReader_=std::move(read);propertyWriter_=std::move(write);
    }
    // Raw value for the actual Source data cell; bypass propertyOwner hooks.
    Property sourcePropertiesCell()const{
        if(sourcePropertiesRaw_)return *sourcePropertiesRaw_;
        return Property(properties);
    }
    PropertyList sourcePropertiesField() const { return propertyOwner(); }
    std::optional<Property> sourcePropertiesRawField()const{return sourcePropertiesRaw_;}
    void setSourcePropertiesField(PropertyList value) { propertyOwner(std::move(value)); }
    void setSourcePropertiesRawField(Property value){
        if(const auto record=PropertyList::record(value)){
            properties=*record;
            sourcePropertiesRaw_.reset();
        }else sourcePropertiesRaw_=std::move(value);
    }

    void setProperty(std::string_view name, Property value) override {
        TreeNodeAlgorithms<Product>::setProperty(nativeReceiver(static_cast<Product*>(this)),name,std::move(value));
    }

    Property getProperty(std::string_view name) const override {
        return TreeNodeAlgorithms<Product>::getProperty(nativeReceiver(const_cast<Product*>(static_cast<const Product*>(this))),name);
    }

    std::vector<std::string> getPropertyNames() const override {
        return TreeNodeAlgorithms<Product>::getPropertyNames(nativeReceiver(static_cast<Product*>(const_cast<AbstractNode*>(this))));
    }

    PropertyList getAllProperties() const override {
        return PropertyList::record(TreeNodeAlgorithms<Product>::getAllProperties(nativeReceiver(static_cast<Product*>(const_cast<AbstractNode*>(this))))).value();
    }

    void removeProperty(std::span<const std::string> names) override {
        TreeNodeAlgorithms<Product>::removeProperty(nativeReceiver(static_cast<Product*>(this)),names);
    }

    bool isKind(std::string_view name) const override {
        return TreeNodeAlgorithms<Product>::isKind(nativeReceiver(static_cast<Product*>(const_cast<AbstractNode*>(this))),Property(std::string(name)));
    }

    void setChildren(FactoryChildren<Product> children) override {
        // The replacement happens before the source starts iterating its input.
        TreeNodeAlgorithms<Product>::setChildren(nativeReceiver(static_cast<Product*>(this)),std::move(children));
    }

    ChildValue<Product> appendChild(ChildValue<Product> child) override {
        return TreeNodeAlgorithms<Product>::appendChild(nativeReceiver(static_cast<Product*>(this)),std::move(child));
    }

    ChildValue<Product> replaceChild(ChildValue<Product> newChild, ChildValue<Product> oldChild) override {
        return TreeNodeAlgorithms<Product>::replaceChild(nativeReceiver(static_cast<Product*>(this)),std::move(newChild),std::move(oldChild));
    }

    ChildValue<Product> removeChild(ChildValue<Product> child) override {
        return TreeNodeAlgorithms<Product>::removeChild(nativeReceiver(static_cast<Product*>(this)),std::move(child));
    }

    std::optional<std::size_t> childIndex(ChildValue<Product> child) const override {
        const auto result=TreeNodeAlgorithms<Product>::childIndex(nativeReceiver(static_cast<Product*>(const_cast<AbstractNode*>(this))),std::move(child));
        if(std::holds_alternative<PropertyNull>(result.value))return {};
        const auto numeric=property_host::number(result);
        return static_cast<std::size_t>(numeric);
    }

    Product* copy(bool keepIds = false) override {
        (void)keepIds;
        return TreeNodeAlgorithms<Product>::copy(nativeReceiver(static_cast<Product*>(this))).requireNodeForWrite("copy");
    }

    std::vector<Product*> findNodes(std::string_view name) override {
        std::vector<Product*> result;
        for(auto child:TreeNodeAlgorithms<Product>::findNodes(nativeReceiver(static_cast<Product*>(this)),Property(std::string(name))))
            result.push_back(child.requireNode("findNodes"));
        return result;
    }

    std::any walkTree(const TreeVisitor<Product>& visit, const std::any& data = {}) override {
        TreeNodeAlgorithms<Product>::walkTree(nativeReceiver(static_cast<Product*>(this)),[&](ChildValue<Product> child,const std::any& value){
            visit(child.requireNode("walkTree"),value);
        },data);
        return data;
    }

    std::any walkTreeRaw(const TreeRawVisitor<Product>& visit,const std::any& data={},
                         const std::any& visitorIdentity={}) override {
        TreeNodeAlgorithms<Product>::walkTree(nativeReceiver(static_cast<Product*>(this)),visit,data,visitorIdentity);
        return data;
    }

    std::string toString() const override {
        return TreeNodeAlgorithms<Product>::toString(nativeReceiver(static_cast<Product*>(const_cast<AbstractNode*>(this))));
    }

protected:
    PropertyList properties;
    // A Source assignment may store any Value in `properties`, including
    // null. This discriminant is the active field owner until an object-valued
    // assignment installs a PropertyList projection.
    std::optional<Property> sourcePropertiesRaw_;
    std::function<PropertyList()> propertyReader_;
    std::function<void(PropertyList)> propertyWriter_;

    [[noreturn]] static void type_error(std::string_view message) {
        throw FactoryError("TypeError", std::u16string(message.begin(), message.end()));
    }
};

template<class Product, class Base = Node<Product>>
class AbstractEmptyNode : public AbstractNode<Product, Base> {
    using Parent = AbstractNode<Product, Base>;
public:
    using Parent::Parent;
    AbstractEmptyNode(const AbstractEmptyNode&) = delete;
    AbstractEmptyNode& operator=(const AbstractEmptyNode&) = delete;
    AbstractEmptyNode(AbstractEmptyNode&&) = delete;
    AbstractEmptyNode& operator=(AbstractEmptyNode&&) = delete;

    void setChildren(FactoryChildren<Product> children) override {TreeEmptyNodeAlgorithms<Product>::setChildren(Parent::nativeReceiver(static_cast<Product*>(this)),std::move(children));}
    ChildValue<Product> appendChild(ChildValue<Product> child) override {return TreeEmptyNodeAlgorithms<Product>::appendChild(Parent::nativeReceiver(static_cast<Product*>(this)),std::move(child));}
    ChildValue<Product> replaceChild(ChildValue<Product>, ChildValue<Product> oldChild) override {return TreeEmptyNodeAlgorithms<Product>::replaceChild(Parent::nativeReceiver(static_cast<Product*>(this)),{},std::move(oldChild));}
    ChildValue<Product> removeChild(ChildValue<Product> child) override {return TreeEmptyNodeAlgorithms<Product>::replaceChild(Parent::nativeReceiver(static_cast<Product*>(this)),{},std::move(child));}
    std::optional<std::size_t> childIndex(ChildValue<Product> child) const override {
        const auto result=TreeEmptyNodeAlgorithms<Product>::childIndex(Parent::nativeReceiver(static_cast<Product*>(const_cast<AbstractEmptyNode*>(this))),std::move(child));
        return std::holds_alternative<PropertyNull>(result.value)?std::optional<std::size_t>{}:std::optional<std::size_t>{static_cast<std::size_t>(property_host::number(result))};
    }

    std::any walkTree(const TreeVisitor<Product>& visit, const std::any& data = {}) override {
        TreeEmptyNodeAlgorithms<Product>::walkTree(Parent::nativeReceiver(static_cast<Product*>(this)),[&](ChildValue<Product> child,const std::any& value){visit(child.requireNode("walkTree"),value);},data);
        return data;
    }
    std::any walkTreeRaw(const TreeRawVisitor<Product>& visit,const std::any& data={},
                         const std::any& visitorIdentity={}) override {
        (void)visitorIdentity;
        TreeEmptyNodeAlgorithms<Product>::walkTree(Parent::nativeReceiver(static_cast<Product*>(this)),visit,data);
        return data;
    }

    std::string toString() const override {
        return property_host::debugText(property_host::string(TreeEmptyNodeAlgorithms<Product>::toString(Parent::nativeReceiver(static_cast<Product*>(const_cast<AbstractEmptyNode*>(this))))));
    }
};
}
