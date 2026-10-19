// Candidate shared implementation of MathJax 4.1.3 core/Tree/Node.js.
// Installed by both AbstractNode<Product> and the Context prototype binder.
export module folia.math.core.tree.node_algorithms;
export import folia.math.core.tree.node_types;
import folia.math.util.property_host;
import std;

export namespace folia::math::tree {
template<class Product> struct TreeNodeReceiver {
    using Child=ChildValue<Product>;
    using Children=FactoryChildren<Product>;
    std::function<Child()> self;
    std::function<Property()> kind;
    std::function<std::string()> kindPrefix; // JS + string, default ToPrimitive hint
    std::function<Property()> properties;
    std::function<void(Property)> assignProperties;
    std::function<Property()> copyProperties; // Object.assign({}, current properties)
    // Optional raw-key operations. Context supplies them to retain Symbol keys;
    // native PropertyList receivers use the string-key fallback below.
    std::function<void(Property,Property)> setProperty;
    std::function<Property(Property)> getProperty;
    std::function<std::vector<std::string>()> getPropertyNames;
    std::function<void(Property)> deleteProperty;
    std::function<Children()> children;
    std::function<Children()> copyChildren; // Source `this.childNodes || []`
    std::function<void(Children)> assignChildren;
    std::function<Children()> newChildren;
    // Context uses the live current object's methods, preserving arbitrary
    // overridden return values. Native ChildArray is the typed fallback.
    std::function<void(Children,Child)> pushChild;
    std::function<Property(Children,Child)> indexOfRaw;
    std::function<std::function<Child(Property)>()> captureCreator;
    std::function<void(Child,Property)> assignPropertiesOn;
    std::function<Child(Child,Child)> appendOn;
    std::function<Property(Child)> childIndex; // exact Source result; only null skips
    std::function<void(Children,Property,Child)> writeChildAt;
    std::function<void(Children,Property)> spliceChildAt;
    std::function<bool(Property)> factoryNodeIsKind;
    std::function<bool(Child,Property)> isKindOn;
    std::function<void(Child,const std::function<void(Child,const std::any&)>&,
                       const std::any&,const std::any&)> walkOn;
};

// The constructor's receiver may be an existing arbitrary Context object.
// All callbacks operate on that exact receiver; neither class allocation nor
// typed argument conversion is hidden in this algorithm.
struct TreeConstructorReceiver {
    std::function<void()> writeOwnFields; // factory, parent, properties, childNodes, in this order
    std::function<std::vector<std::u16string>()> propertyKeys; // Object.keys(argument)
    // Exactly one value/call pair is supplied. Native initialization retains
    // typed Property directly; Context initialization retains exact Value.
    std::function<options::Value(std::u16string_view)> propertyValue;
    std::function<void(std::u16string_view,options::Value)> callSetProperty;
    std::function<Property(std::u16string_view)> propertyValueNative;
    std::function<void(std::u16string_view,Property)> callSetPropertyNative;
    std::function<bool()> childLengthTruthy;
    std::function<void()> callSetChildren;
};

template<class Product> struct TreeNodeAlgorithms {
    using Receiver=TreeNodeReceiver<Product>;
    using Child=ChildValue<Product>;
    static void initialize(const TreeConstructorReceiver& receiver) {
        receiver.writeOwnFields();
        const auto names=receiver.propertyKeys();
        for(const auto& name:names){
            if(receiver.propertyValueNative&&receiver.callSetPropertyNative)
                receiver.callSetPropertyNative(name,receiver.propertyValueNative(name));
            else
                receiver.callSetProperty(name,receiver.propertyValue(name));
        }
        if(receiver.childLengthTruthy())receiver.callSetChildren();
    }
    static void setProperty(const Receiver& receiver,std::string_view name,Property value) {
        if(receiver.setProperty){receiver.setProperty(Property(std::string(name)),std::move(value));return;}
        auto record=PropertyList::record(receiver.properties());
        if(!record)throw FactoryError("TypeError",u"Cannot set properties on a non-object record");
        record->set(name,std::move(value));
    }
    static void setProperty(const Receiver& receiver,Property name,Property value) {
        if(receiver.setProperty){receiver.setProperty(std::move(name),std::move(value));return;}
        const auto text=property_host::debugText(property_host::string(name));
        setProperty(receiver,std::string_view(text),std::move(value));
    }
    static Property getProperty(const Receiver& receiver,std::string_view name) {
        if(receiver.getProperty)return receiver.getProperty(Property(std::string(name)));
        const auto record=PropertyList::record(receiver.properties());
        if(!record)throw FactoryError("TypeError",u"Cannot read properties from a non-object record");
        return record->get(name);
    }
    static Property getProperty(const Receiver& receiver,Property name) {
        if(receiver.getProperty)return receiver.getProperty(std::move(name));
        const auto text=property_host::debugText(property_host::string(name));
        return getProperty(receiver,std::string_view(text));
    }
    static std::vector<std::string> getPropertyNames(const Receiver& receiver) {
        if(receiver.getPropertyNames)return receiver.getPropertyNames();
        const auto record=PropertyList::record(receiver.properties());
        if(!record)throw FactoryError("TypeError",u"Cannot enumerate a non-object record");
        return record->keys();
    }
    static Property getAllProperties(const Receiver& receiver) {return receiver.properties();}
    static void removeProperty(const Receiver& receiver,std::span<const std::string> names) {
        if(receiver.deleteProperty){for(const auto& name:names)receiver.deleteProperty(Property(name));return;}
        auto record=PropertyList::record(receiver.properties());
        if(!record)throw FactoryError("TypeError",u"Cannot delete properties from a non-object record");
        for(const auto& name:names)record->erase(name);
    }
    static void removeProperty(const Receiver& receiver,std::span<const Property> names) {
        if(receiver.deleteProperty){for(const auto& name:names)receiver.deleteProperty(name);return;}
        for(const auto& name:names){const auto text=property_host::debugText(property_host::string(name));
            const std::array<std::string,1> one{text};removeProperty(receiver,one);}
    }
    static bool isKind(const Receiver& receiver,Property kind) {
        return receiver.factoryNodeIsKind(std::move(kind));
    }
    static void setChildren(const Receiver& receiver,typename Receiver::Children input) {
        receiver.assignChildren(receiver.newChildren());
        if(!input)throw FactoryError("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        input->forOf([&](Child child){(void)receiver.appendOn(receiver.self(),std::move(child));});
    }
    static Child appendChild(const Receiver& receiver,Child child) {
        const auto children=receiver.children();
        if(!children)throw FactoryError("TypeError",u"Cannot read properties of null (reading 'push')");
        if(receiver.pushChild)receiver.pushChild(children,child);
        else children->push_back(child);
        child.setParent(receiver.self());
        return child;
    }
    static Child replaceChild(const Receiver& receiver,Child next,Child previous) {
        const auto index=receiver.childIndex(previous);
        if(!std::holds_alternative<PropertyNull>(index.value)){
            const auto children=receiver.children();
            receiver.writeChildAt(children,index,next);
            next.setParent(receiver.self());
            if(previous.getParent()==receiver.self())previous.setParent(PropertyNull{});
        }
        return next;
    }
    static Child removeChild(const Receiver& receiver,Child child) {
        const auto index=receiver.childIndex(child);
        if(!std::holds_alternative<PropertyNull>(index.value)){
            const auto children=receiver.children();
            receiver.spliceChildAt(children,index);
            child.setParent(PropertyNull{});
        }
        return child;
    }
    static Property childIndex(const Receiver& receiver,Child child) {
        const auto children=receiver.children();
        if(!children)throw FactoryError("TypeError",u"Cannot read properties of null (reading 'indexOf')");
        if(receiver.indexOfRaw){
            const auto result=receiver.indexOfRaw(children,child);
            if(const auto* number=std::get_if<double>(&result.value);number&&*number==-1.0)
                return PropertyNull{};
            return result;
        }
        const auto result=children->indexOf(std::move(child));
        return result?Property(static_cast<double>(*result)):Property(PropertyNull{});
    }
    static Child copy(const Receiver& receiver) {
        TreeCallGuard<Product> guard;
        const auto create=receiver.captureCreator();
        const Child result=create(receiver.kind());
        receiver.assignPropertiesOn(result,receiver.copyProperties());
        const auto children=receiver.copyChildren?receiver.copyChildren():receiver.children();
        if(children)children->forOf([&](Child child){
            if(child.truthy())(void)receiver.appendOn(result,child.copy());
        });
        return result;
    }
    static void walkTreeFiltered(const Receiver& receiver,const std::function<void(Child,const std::any&)>& visit,
                         const std::any& data,const std::any& visitorIdentity,
                         const std::function<bool(Child)>& descend) {
        TreeCallGuard<Product> guard;
        visit(receiver.self(),data);
        const auto children=receiver.children();
        if(!children)throw FactoryError("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        children->forOf([&](Child child){if(descend(child))receiver.walkOn(child,visit,data,visitorIdentity);});
    }
    static void walkTree(const Receiver& receiver,const std::function<void(Child,const std::any&)>& visit,
                         const std::any& data={},const std::any& visitorIdentity={}) {
        walkTreeFiltered(receiver,visit,data,visitorIdentity,[](Child child){return child.truthy();});
    }
    static std::vector<Child> findNodes(const Receiver& receiver,Property kind) {
        auto found=std::make_shared<std::vector<Child>>();
        const auto accepts=receiver.isKindOn;
        receiver.walkOn(receiver.self(),[found,accepts,kind](Child node,const std::any&){
            if(accepts(node,kind))found->push_back(node);
        },{},{});
        return *found;
    }
    static std::string toString(const Receiver& receiver) {
        TreeCallGuard<Product> guard;
        const auto kind=receiver.kindPrefix?receiver.kindPrefix():property_host::debugText(property_host::string(receiver.kind()));
        const auto children=receiver.children();
        if(!children)throw FactoryError("TypeError",u"Cannot read properties of null (reading 'join')");
        return kind+"("+children->joinedString()+")";
    }
};

template<class Product> struct TreeEmptyNodeAlgorithms {
    using Receiver=TreeNodeReceiver<Product>;
    using Child=ChildValue<Product>;
    static void setChildren(const Receiver&,typename Receiver::Children) {}
    static Child appendChild(const Receiver&,Child child) {return child;}
    static Child replaceChild(const Receiver&,Child,Child previous) {return previous;}
    static Property childIndex(const Receiver&,Child) {return PropertyNull{};}
    static void walkTree(const Receiver& receiver,
        const std::function<void(Child,const std::any&)>& visit,const std::any& data={}) {
        TreeCallGuard<Product> guard;
        visit(receiver.self(),data);
    }
    static Property toString(const Receiver& receiver) {
        TreeCallGuard<Product> guard;
        return receiver.kind();
    }
};
}
