// Native identity binding to the one Context graph; no math algorithms.
export module folia.math.util.native_identities;
export import folia.math.util.options;
import std;
export namespace folia::math {
class NativeIdentityResolutionFailure final:public options::Exception {
public:
    std::string requestedType;
    std::string payloadType;
    NativeIdentityResolutionFailure(std::string requested,std::string payload)
        :options::Exception("Unimplemented",u"Context object has no binding for this native reference type"),
         requestedType(std::move(requested)),payloadType(std::move(payload)){}
};
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
    bool requestCaptureActive_=false;
    std::vector<Identity> requestCaptured_;
    void forgetRequestCapture(const Identity& identity){
        if(!requestCaptureActive_)return;
        std::erase(requestCaptured_,identity);
    }
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
    // Return whether this registry is the canonical owner of the Context
    // handle. This is used to avoid retiring a record both through this typed
    // owner and through a containing request-local Context record.
    bool ownsHandle(options::ObjectRef object)const noexcept{
        for(const auto& [identity,handle]:objects_){(void)identity;if(handle==object)return true;}
        return false;
    }
    template<class Pointee>std::optional<options::ObjectRef> findPointee(const Pointee* pointer)const{
        if(!pointer)return std::nullopt;
        for(const auto& [identity,object]:objects_){
            if constexpr(requires(const Identity& value){value.get();}){
                if(identity.get()==pointer)return object;
            }else if constexpr(std::is_pointer_v<Identity>){
                if(identity==pointer)return object;
            }
        }
        return std::nullopt;
    }
    options::ObjectRef bind(Identity identity,options::ObjectRef object,std::shared_ptr<void> lifetime={}){
        context_.kind(object); // Reject foreign-context handles before aliasing.
        if(!identity)throw options::Exception("Unimplemented",u"Native identity binding requires a non-null actual owner");
        if(auto existing=find(identity))return *existing;
        if constexpr(std::is_pointer_v<Identity>)if(!lifetime)
            throw options::Exception("Unimplemented",u"Raw native identity requires its actual owning graph lifetime");
        const auto& payload=std::as_const(context_).payload(object);
        if(payload.has_value())throw options::Exception("Unimplemented",u"Context object already has a different native payload binding");
        if(requestCaptureActive_)requestCaptured_.push_back(identity);
        try{
            context_.setPayload(object,NativeBindingPayload<Identity>{identity,std::move(lifetime)});
            const auto [inserted,added]=objects_.emplace(identity,object);
            (void)inserted;
            if(!added)throw options::Exception("Unimplemented",u"Native identity binding changed during insertion");
        }catch(...){
            objects_.erase(identity);
            context_.setPayload(object,{});
            forgetRequestCapture(identity);
            throw;
        }
        return object;
    }
    options::ObjectRef instance(Identity identity,options::ObjectRef actualConstructor,std::shared_ptr<void> lifetime={}){
        context_.kind(actualConstructor);
        if(auto existing=find(identity))return *existing;
        return bind(std::move(identity),context_.instance(actualConstructor),std::move(lifetime));
    }
    // The registry is a strong owner of every canonical native identity and
    // its Context handle. Enumerate explicitly so a future scoped owner can
    // classify roots; this does not make all entries persistent roots by
    // default, because registries may also contain per-conversion identities.
    template<class Visitor>void visitCanonicalHandles(Visitor&& visitor)const{
        for(const auto& [identity,object]:objects_)visitor(identity,object);
    }
    std::size_t canonicalHandleCount()const noexcept{return objects_.size();}
    Identity resolve(options::ObjectRef object)const{
        const auto& payload=std::as_const(context_).payload(object);
        const auto* native=std::any_cast<NativeBindingPayload<Identity>>(&payload);
        if(!native||!native->identity)
            throw NativeIdentityResolutionFailure(typeid(Identity).name(),payload.has_value()?payload.type().name():"<empty>");
        if constexpr(std::is_pointer_v<Identity>)if(!native->lifetime)
            throw options::Exception("Unimplemented",u"Raw native payload has lost its owning graph lifetime");
        const auto canonical=find(native->identity);
        if(!canonical||*canonical!=object)throw options::Exception("Unimplemented",u"Context object is not the canonical native reference binding");
        return native->identity;
    }
    // Explicit retirement of one identity after its owner domain proves no
    // Source handle can escape. Context records remain allocated; clearing the
    // matching payload makes typed resolution fail and releases its owner lease.
    bool erase(const Identity& identity){
        const auto found=objects_.find(identity);
        if(found==objects_.end())return false;
        const auto object=found->second;
        const auto& payload=std::as_const(context_).payload(object);
        const auto* bound=std::any_cast<NativeBindingPayload<Identity>>(&payload);
        if(!bound||bound->identity!=identity)return false;
        const Identity retainedIdentity=identity;
        context_.setPayload(object,{});
        objects_.erase(found);
        forgetRequestCapture(retainedIdentity);
        return true;
    }
    // Retire the complete Context record for an identity after its owner has
    // detached every live root. Unlike erase(), this releases native cells,
    // views, callbacks, descriptors and payload retained by the stale handle.
    // Context keeps the slot as a tombstone, so the ObjectRef can never alias
    // a later binding. Public Source handles must not use this operation.
    bool retire(const Identity& identity){
        const auto found=objects_.find(identity);
        if(found==objects_.end())return false;
        const auto object=found->second;
        const auto& payload=std::as_const(context_).payload(object);
        const auto* bound=std::any_cast<NativeBindingPayload<Identity>>(&payload);
        if(!bound||bound->identity!=identity)return false;
        const Identity retainedIdentity=identity;
        context_.retireObjectRecord(object);
        objects_.erase(found);
        forgetRequestCapture(retainedIdentity);
        return true;
    }
    // Capture only bindings created after an explicit owner-scope boundary.
    // Callers classify the selected registry as request-only and must promote
    // any binding that becomes reachable from durable Source state.
    void beginRequestCapture(){
        if(requestCaptureActive_||!requestCaptured_.empty())throw options::Exception("TypeError",u"Identity request capture is already active or has unfinished retirement");
        requestCaptured_.clear();requestCaptureActive_=true;
    }
    void promoteFromRequestCapture(const Identity& identity){forgetRequestCapture(identity);}
    // This is explicit retirement, not tracing or general collection. A
    // request owner must detach its persistent roots before calling it.
    void retireCapturedRequestBindings(){
        if(!requestCaptureActive_&&requestCaptured_.empty())throw options::Exception("TypeError",u"Identity request capture is not active");
        // Stop doing arbitrary-entry removal while retiring the full suffix;
        // otherwise every erase would scan the remaining vector and make a
        // large formula's teardown quadratic in the number of identities.
        requestCaptureActive_=false;
        while(!requestCaptured_.empty()){
            const Identity identity=requestCaptured_.back();
            if(objects_.contains(identity)&&!retire(identity))
                throw options::Exception("TypeError",u"Captured request identity no longer matches its Context owner");
            requestCaptured_.pop_back();
        }
    }
    // Runtime/domain teardown only, after Source consumers have released these
    // references. Stale Context handles subsequently fail typed resolution.
    void clear(){
        for(const auto&[identity,object]:objects_){
            const auto& payload=std::as_const(context_).payload(object);
            const auto* bound=std::any_cast<NativeBindingPayload<Identity>>(&payload);
            if(bound&&bound->identity==identity)context_.setPayload(object,{});
        }
        objects_.clear();
    }
};
}
