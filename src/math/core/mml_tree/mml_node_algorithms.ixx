// Shared Source-order MML operations over live native or Context receivers.
// This module deliberately depends on Tree interfaces, not complete MmlNode.
export module folia.math.core.mml_tree.mml_node_algorithms;
export import folia.math.core.tree.node_algorithms;
export import folia.math.core.tree.factory_types;
export import folia.math.core.mml_tree.attributes;
import std;

export namespace folia::math::mml {
template<class Product> class MmlNativeDynamic;
template<class Product> struct MmlSourceValue {
    options::Value value;
    tree::TreeNodeAccessRef<Product> provider;
};

template<class Product> struct MmlNativeCallable {
    std::function<MmlNativeDynamic<Product>(MmlNativeDynamic<Product>,
        std::span<const MmlNativeDynamic<Product>>)> invoke;
};

// Finite operation-time values for the native adapter. ChildValue carries a
// node without manufacturing a Context identity; the remaining alternatives
// are actual native owners or callable tokens, not a second field store.
template<class Product> class MmlNativeDynamic {
public:
    using Child=tree::ChildValue<Product>;
    using Children=tree::FactoryChildren<Product>;
    using Callable=std::shared_ptr<MmlNativeCallable<Product>>;
    using Data=std::variant<Property,Child,tree::FactoryServices<Product>*,
        tree::FactoryClassRef<Product>,PropertyList,std::shared_ptr<Attributes>,Children,Callable,options::Value,
        MmlSourceValue<Product>>;
private:
    Data data_;
public:
    explicit MmlNativeDynamic(Property value):data_(std::move(value)){}
    explicit MmlNativeDynamic(Child value):data_(std::move(value)){}
    explicit MmlNativeDynamic(tree::FactoryServices<Product>* value):data_(value){}
    explicit MmlNativeDynamic(tree::FactoryClassRef<Product> value):data_(std::move(value)){}
    explicit MmlNativeDynamic(PropertyList value):data_(std::move(value)){}
    explicit MmlNativeDynamic(std::shared_ptr<Attributes> value):data_(std::move(value)){}
    explicit MmlNativeDynamic(Children value):data_(std::move(value)){}
    explicit MmlNativeDynamic(Callable value):data_(std::move(value)){}
    explicit MmlNativeDynamic(options::Value value):data_(std::move(value)){}
    explicit MmlNativeDynamic(MmlSourceValue<Product> value):data_(std::move(value)){}
    const Data& data()const noexcept{return data_;}
    Data& data()noexcept{return data_;}
};

// Primitive operations are intentionally small. They preserve Source Get,
// Set, Call, key enumeration and iteration points; algorithm bodies below are
// the single implementation used by native and Context method entry points.
template<class Product,class Dynamic> struct MmlNodeReceiver {
    using Child=tree::ChildValue<Product>;
    using Children=tree::FactoryChildren<Product>;
    tree::TreeNodeReceiver<Product> tree;
    std::function<void()> ensureActive;
    std::function<Child()> self;
    std::function<Dynamic(Child)> toDynamic;
    std::function<Child(Dynamic)> toChild;
    std::function<Dynamic(Dynamic,std::u16string_view)> get;
    std::function<Dynamic(Dynamic,std::size_t)> getIndex;
    std::function<Dynamic(Children)> childrenValue;
    std::function<void(Dynamic,std::u16string_view,Dynamic)> set;
    std::function<Dynamic(Dynamic,Dynamic,std::span<const Dynamic>)> invoke;
    std::function<Dynamic(std::u16string_view,Dynamic,std::span<const Dynamic>)> invokeCoreTreeSuper;
    std::function<Dynamic(std::u16string_view)> string;
    std::function<Dynamic(double)> numberValue;
    std::function<Dynamic()> undefined;
    std::function<Dynamic()> nullValue;
    std::function<Dynamic(Dynamic)> arrayOfOne;
    std::function<Dynamic(Dynamic,Dynamic)> constructAttributes;
    std::function<bool(Dynamic)> truthy;
    std::function<double(Dynamic)> number;
    std::function<bool(Dynamic,Dynamic)> strictEqual;
    std::function<Children(Dynamic)> childrenOf;
    std::function<Dynamic(Dynamic)> objectAssign;
    std::function<std::vector<std::u16string>(Dynamic)> ownEnumerableStringKeys;
    std::function<void(Dynamic,const std::function<void(Dynamic)>&)> forOf;
    std::function<Dynamic(const std::function<Dynamic(std::span<const Dynamic>)>&)> callback;
};

template<class Product,class Dynamic> class MmlNodeAlgorithms {
    using Receiver=MmlNodeReceiver<Product,Dynamic>;
    using Child=typename Receiver::Child;
    using Children=typename Receiver::Children;
    static Dynamic method(Receiver& r,Dynamic object,std::u16string_view name,
                          std::span<const Dynamic> args={}) {
        // Keep member Get before the argument list is constructed by callers.
        auto callable=r.get(object,name);
        return r.invoke(std::move(callable),std::move(object),args);
    }
    static Dynamic property(Receiver& r,Child child){return r.toDynamic(std::move(child));}
    static Children children(Receiver& r,Dynamic raw){return r.childrenOf(std::move(raw));}
public:
    static Dynamic setChildren(Receiver& r,Dynamic rawChildren){
        const Child self=r.self();
        auto rawSelf=property(r,self);
        const auto firstArity=r.number(r.get(rawSelf,u"arity"));
        if(firstArity<0){
            auto currentChildNodes=r.get(rawSelf,u"childNodes");
            auto first=r.toChild(r.getIndex(currentChildNodes,0));
            auto rawFirst=property(r,first);
            const auto callable=r.get(rawFirst,u"setChildren");
            const std::array<Dynamic,1> args{rawChildren};
            return r.invoke(callable,rawFirst,args);
        }
        const std::array<Dynamic,1> args{rawChildren};
        return r.invokeCoreTreeSuper(u"setChildren",rawSelf,args);
    }

    static Dynamic appendChild(Receiver& r,Child child){
        const Child self=r.self();
        auto rawSelf=property(r,self);
        if(r.number(r.get(rawSelf,u"arity"))<0){
            auto rawChildren=r.get(rawSelf,u"childNodes");
            auto first=r.toChild(r.getIndex(rawChildren,0));
            auto rawFirst=property(r,first);
            const auto callable=r.get(rawFirst,u"appendChild");
            const std::array<Dynamic,1> args{property(r,child)};
            (void)r.invoke(callable,rawFirst,args);
            return property(r,child);
        }
        auto rawChild=property(r,child);
        if(!r.truthy(r.get(rawChild,u"isInferred"))){
            const std::array<Dynamic,1> args{rawChild};
            return r.invokeCoreTreeSuper(u"appendChild",rawSelf,args);
        }

        if(r.strictEqual(r.get(rawSelf,u"arity"),r.numberValue(std::numeric_limits<double>::infinity()))){
            auto rawChildNodes=r.get(rawChild,u"childNodes");
            const auto callable=r.get(rawChildNodes,u"forEach");
            auto visitor=r.callback([receiver=r,rawSelf](std::span<const Dynamic> args){
                if(receiver.ensureActive)receiver.ensureActive();
                const auto value=args.empty()?receiver.undefined():args.front();
                const std::array<Dynamic,1> appendArgs{value};
                (void)receiver.invokeCoreTreeSuper(u"appendChild",rawSelf,appendArgs);
                return receiver.undefined();
            });
            const std::array<Dynamic,1> args{visitor};
            (void)r.invoke(callable,rawChildNodes,args);
            return r.toDynamic(child);
        }

        auto rawFactory=r.get(rawSelf,u"factory");
        const auto create=r.get(rawFactory,u"create");
        const std::array<Dynamic,1> createArgs{r.string(u"mrow")};
        const auto replacement=r.invoke(create,rawFactory,createArgs);
        const auto setChildren=r.get(replacement,u"setChildren");
        const std::array<Dynamic,1> childrenArgs{r.get(rawChild,u"childNodes")};
        (void)r.invoke(setChildren,replacement,childrenArgs);
        r.set(replacement,u"attributes",r.get(rawChild,u"attributes"));
        const auto propertyNames=method(r,rawChild,u"getPropertyNames");
        r.forOf(propertyNames,[&](Dynamic name){
            const auto setProperty=r.get(replacement,u"setProperty");
            const auto value=method(r,rawChild,u"getProperty",std::span<const Dynamic>(&name,1));
            const std::array<Dynamic,2> setArgs{name,value};
            (void)r.invoke(setProperty,replacement,setArgs);
        });
        const std::array<Dynamic,1> appendArgs{replacement};
        return r.invokeCoreTreeSuper(u"appendChild",rawSelf,appendArgs);
    }

    static Dynamic replaceChild(Receiver& r,Child next,Child old){
        const Child self=r.self();
        auto rawSelf=property(r,self);
        if(r.number(r.get(rawSelf,u"arity"))<0){
            tree::TreeCallGuard<Product> recursiveDispatch;
            auto rawChildren=r.get(rawSelf,u"childNodes");
            auto first=r.toChild(r.getIndex(rawChildren,0));
            auto rawFirst=property(r,first);
            const auto callable=r.get(rawFirst,u"replaceChild");
            const std::array<Dynamic,2> args{property(r,next),property(r,old)};
            (void)r.invoke(callable,rawFirst,args);
            return property(r,next);
        }
        const std::array<Dynamic,2> args{property(r,next),property(r,old)};
        return r.invokeCoreTreeSuper(u"replaceChild",rawSelf,args);
    }

    static Dynamic copy(Receiver& r,bool keepIds=false){
        const Child self=r.self();
        const Dynamic rawSelf=property(r,self);
        const Dynamic rawFactory=r.get(rawSelf,u"factory");
        const Dynamic create=r.get(rawFactory,u"create");
        const Dynamic kind=r.get(rawSelf,u"kind");
        const std::array<Dynamic,1> createArgs{kind};
        Dynamic result=r.invoke(create,rawFactory,createArgs);

        const Dynamic rawProperties=r.get(rawSelf,u"properties");
        r.set(result,u"properties",r.objectAssign(rawProperties));
        const Dynamic attributesCondition=r.get(rawSelf,u"attributes");
        if(r.truthy(attributesCondition)){
            const Dynamic rawAttributes=r.get(rawSelf,u"attributes");
            const Dynamic getAll=r.get(rawAttributes,u"getAllAttributes");
            const auto attributesArgs=std::span<const Dynamic>{};
            const Dynamic attributes=r.invoke(getAll,rawAttributes,attributesArgs);
            const auto names=r.ownEnumerableStringKeys(attributes);
            for(const auto& name:names){
                if(name==u"id"&&!keepIds)continue;
                const Dynamic targetAttributes=r.get(result,u"attributes");
                const Dynamic setter=r.get(targetAttributes,u"set");
                const Dynamic value=r.get(attributes,name);
                const std::array<Dynamic,2> args{r.string(name),value};
                (void)r.invoke(setter,targetAttributes,args);
            }
        }

        const Dynamic firstChildNodesRead=r.get(rawSelf,u"childNodes");
        if(r.truthy(firstChildNodesRead)){
            const Dynamic lengthReceiver=r.get(rawSelf,u"childNodes");
            const Dynamic rawLength=r.get(lengthReceiver,u"length");
            if(!r.truthy(rawLength))return result;
            Dynamic iteration=r.get(rawSelf,u"childNodes");
            const Dynamic iterationLength=r.get(iteration,u"length");
            const auto one=r.numberValue(1);
            if(r.strictEqual(iterationLength,one)){
                const Child conditionChild=r.toChild(r.getIndex(iteration,0));
                const Dynamic conditionValue=property(r,conditionChild);
                if(r.truthy(r.get(conditionValue,u"isInferred"))){
                    const Child branchChild=r.toChild(r.getIndex(iteration,0));
                    iteration=r.get(property(r,branchChild),u"childNodes");
                }
            }
            r.forOf(iteration,[&](Dynamic rawChild){
                if(r.truthy(rawChild)){
                    const auto append=r.get(result,u"appendChild");
                    const auto copyFunction=r.get(rawChild,u"copy");
                    const auto childCopy=r.invoke(copyFunction,rawChild,{});
                    const std::array<Dynamic,1> args{childCopy};
                    (void)r.invoke(append,result,args);
                }else{
                    const auto resultChildren=r.get(result,u"childNodes");
                    const auto push=r.get(resultChildren,u"push");
                    const std::array<Dynamic,1> args{r.nullValue()};
                    (void)r.invoke(push,resultChildren,args);
                }
            });
        }
        return result;
    }
};
}
