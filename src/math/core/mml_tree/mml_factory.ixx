// Root-frozen portable declarations for complete MmlFactory.ts; Apache-2.0.
export module folia.math.core.mml_tree.mml_factory;
export import folia.math.core.mml_tree.mml;
export import folia.math.core.tree.node_factory;
import std;
export namespace folia::math::mml {
class MmlFactory:public tree::AbstractNodeFactory<MmlNode>,public std::enable_shared_from_this<MmlFactory> {
    using Base=tree::AbstractNodeFactory<MmlNode>;
    std::function<void(MmlNode&,std::shared_ptr<MmlFactory>)> identityBinder_;
    const void* identityBinderOwner_=nullptr;
    std::function<std::shared_ptr<MmlClassInfo>(const tree::FactoryClassRef<MmlNode>&)> sourceClassInfoResolver_;
    const void* sourceClassInfoResolverOwner_=nullptr;
    std::vector<options::ObjectRef> adoptIdentityTargets_;
public:
    using Base::create;
    inline static std::shared_ptr<tree::FactoryClassMap<MmlNode>> defaultNodes=folia::math::mml::MML();
    explicit MmlFactory(std::optional<tree::FactoryClassMap<MmlNode>> nodes=std::nullopt,
                       const tree::FactoryClassMap<MmlNode>* classDefaults=nullptr);
    tree::FactoryCreatorView<MmlNode> MML()const;
    // Bound MmlFactory calls use the same live Source field readers as the
    // Context method. Unbound calls retain the native creator path.
    template<class BuildArguments>
    MmlNode* createWithArguments(std::string_view kind,BuildArguments&& buildArguments){
        return Base::createWithArguments(
            kind,std::forward<BuildArguments>(buildArguments));
    }
    MmlNode* create(std::string_view kind,PropertyList properties={},
                    tree::FactoryChildren<MmlNode> children={});
    MmlNode* adopt(std::unique_ptr<MmlNode>)override;
    // Optional Context identity bridge. A shared factory is the actual owner
    // retained by each bound native node; borrowed factories do not bind.
    void setIdentityBinder(const void* owner,
                           std::function<void(MmlNode&,std::shared_ptr<MmlFactory>)> binder){
        if(identityBinderOwner_&&identityBinderOwner_!=owner)
            throw tree::FactoryError("Unimplemented",u"One MmlFactory cannot publish nodes into two Context identities");
        identityBinderOwner_=owner;identityBinder_=std::move(binder);
    }
    void clearIdentityBinder(const void* owner){
        if(identityBinderOwner_==owner){identityBinder_={};identityBinderOwner_=nullptr;}
    }
    // A bound runtime class object is the source of static fields for nodes
    // created by this factory. The resolver is per actual factory instance;
    // the global FactoryClass/MmlClassInfo remains an immutable typed template.
    void setSourceClassInfoResolver(const void* owner,
        std::function<std::shared_ptr<MmlClassInfo>(const tree::FactoryClassRef<MmlNode>&)> resolver){
        if(sourceClassInfoResolverOwner_&&sourceClassInfoResolverOwner_!=owner)
            throw tree::FactoryError("Unimplemented",u"One MmlFactory cannot publish class statics into two runtime modules");
        sourceClassInfoResolverOwner_=owner;sourceClassInfoResolver_=std::move(resolver);
    }
    void clearSourceClassInfoResolver(const void* owner){
        if(sourceClassInfoResolverOwner_==owner){sourceClassInfoResolver_={};sourceClassInfoResolverOwner_=nullptr;}
    }
    std::shared_ptr<MmlClassInfo> sourceClassInfo(const tree::FactoryClassRef<MmlNode>& cls)const{
        if(!cls)throw tree::FactoryError("TypeError",u"MmlFactory class metadata requires a class descriptor");
        if(sourceClassInfoResolver_){
            auto value=sourceClassInfoResolver_(cls);
            if(!value)throw tree::FactoryError("TypeError",u"MmlFactory runtime class metadata is unavailable");
            return value;
        }
        const auto* info=std::any_cast<std::shared_ptr<MmlClassInfo>>(&cls->staticData);
        if(!info||!*info)throw tree::FactoryError("TypeError",u"MmlFactory class has no native metadata");
        return *info;
    }
    // A Source constructor called with an ordinary receiver still allocates
    // one real native product. Its adopt hook binds that product to the exact
    // supplied Context object instead of manufacturing a second identity.
    template<class Construct>
    MmlNode* constructInto(options::ObjectRef receiver,Construct&& construct){
        const auto depth=adoptIdentityTargets_.size();
        adoptIdentityTargets_.push_back(receiver);
        try{
            auto* result=std::invoke(std::forward<Construct>(construct));
            adoptIdentityTargets_.resize(depth);
            return result;
        }catch(...){
            adoptIdentityTargets_.resize(depth);
            throw;
        }
    }
    std::optional<options::ObjectRef> takeAdoptIdentityTarget(){
        if(adoptIdentityTargets_.empty())return std::nullopt;
        auto target=adoptIdentityTargets_.back();
        adoptIdentityTargets_.pop_back();
        return target;
    }
    bool hasAdoptIdentityTarget()const noexcept{return !adoptIdentityTargets_.empty();}
    // Keep the FactoryArgs overload explicit.  The inherited typed overload
    // supplies Source's one-argument/default-properties call without making
    // create(kind) ambiguous.
    MmlNode* create(std::string_view kind,tree::FactoryArgs args)override;
};
}
