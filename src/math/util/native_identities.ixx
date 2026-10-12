// Native identity binding to the one Context graph; no math algorithms.
export module folia.math.util.native_identities;
export import folia.math.util.options;
import std;
export namespace folia::math {
template<class Identity>struct NativeBindingPayload {
    Identity identity;
    // Raw pointers require their actual owning graph here; shared identities
    // already retain their actual owner in identity itself.
    std::shared_ptr<void> lifetime;
};
// Identity is an actual non-null native pointer/shared owner, never a copied
// node, class descriptor or dictionary. Payload type remains visible to callers.
template<class Identity>class NativeIdentityBindings {
    options::Context& context_;
    std::unordered_map<Identity,options::ObjectRef> objects_;
public:
    explicit NativeIdentityBindings(options::Context& context):context_(context){}
    NativeIdentityBindings(const NativeIdentityBindings&)=delete;
    NativeIdentityBindings& operator=(const NativeIdentityBindings&)=delete;
    options::Context& context()const{return context_;}
    std::optional<options::ObjectRef> find(const Identity& identity)const{
        const auto found=objects_.find(identity);
        if(found==objects_.end())return std::nullopt;
        return found->second;
    }
    options::ObjectRef bind(Identity identity,options::ObjectRef object,std::shared_ptr<void> lifetime={}){
        context_.kind(object); // Reject foreign-context handles before aliasing.
        if(!identity)throw options::Exception("Unimplemented",u"Native identity binding requires a non-null actual owner");
        if(auto existing=find(identity))return *existing;
        if constexpr(std::is_pointer_v<Identity>)if(!lifetime)
            throw options::Exception("Unimplemented",u"Raw native identity requires its actual owning graph lifetime");
        auto& payload=context_.payload(object);
        if(payload.has_value())throw options::Exception("Unimplemented",u"Context object already has a different native payload binding");
        payload=NativeBindingPayload<Identity>{identity,std::move(lifetime)};
        objects_.emplace(std::move(identity),object);
        return object;
    }
    options::ObjectRef instance(Identity identity,options::ObjectRef actualConstructor,std::shared_ptr<void> lifetime={}){
        context_.kind(actualConstructor);
        if(auto existing=find(identity))return *existing;
        return bind(std::move(identity),context_.instance(actualConstructor),std::move(lifetime));
    }
    Identity resolve(options::ObjectRef object)const{
        const auto* native=std::any_cast<NativeBindingPayload<Identity>>(&context_.payload(object));
        if(!native||!native->identity)throw options::Exception("Unimplemented",u"Context object has no binding for this native reference type");
        if constexpr(std::is_pointer_v<Identity>)if(!native->lifetime)
            throw options::Exception("Unimplemented",u"Raw native payload has lost its owning graph lifetime");
        const auto canonical=find(native->identity);
        if(!canonical||*canonical!=object)throw options::Exception("Unimplemented",u"Context object is not the canonical native reference binding");
        return native->identity;
    }
    // Runtime/domain teardown only, after Source consumers have released these
    // references. Stale Context handles subsequently fail typed resolution.
    void clear(){
        for(const auto&[identity,object]:objects_){
            auto& payload=context_.payload(object);
            const auto* bound=std::any_cast<NativeBindingPayload<Identity>>(&payload);
            if(bound&&bound->identity==identity)payload.reset();
        }
        objects_.clear();
    }
};
}
