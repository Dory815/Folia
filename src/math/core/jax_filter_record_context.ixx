// Per-call Source-owned InputJax/OutputJax filter record Context identity.
// The Context object is the only property store; the NativeDataField callbacks
// below are raw-cell projections and never perform a Context lookup.
export module folia.math.core.jax_filter_record_context;
export import folia.math.util.options;
export import folia.math.util.object_types;
import std;

export namespace folia::math::core {
struct JaxFilterRecordField {
    options::Key key;
    std::function<options::Value()> readRaw;
    std::function<void(options::Value)> writeRaw;
    std::function<void()> removeRaw;
};

class JaxFilterRecordContext {
    options::Context* context_{};
    options::ObjectRef identity_{};
    std::weak_ptr<std::atomic_bool> lease_;
    bool guarded_{};
public:
    JaxFilterRecordContext(options::Context& context,options::ObjectRef identity,
                           std::weak_ptr<std::atomic_bool> lease={})
        :context_(&context),identity_(identity),lease_(std::move(lease)){
        const std::weak_ptr<std::atomic_bool> empty;
        guarded_=lease_.owner_before(empty)||empty.owner_before(lease_);
    }
    options::ObjectRef identity()const noexcept{return identity_;}
    options::Context& context()const{ensureActive();return *context_;}
    void ensureActive()const{
        if(!guarded_)return; // Only non-runtime reference fixtures omit a lease.
        const auto lease=lease_.lock();
        if(!lease||!lease->load(std::memory_order_acquire))
            throw options::Exception("TypeError",u"Jax filter record Context owner has expired");
    }
    options::Value get(const options::Key& key)const{
        ensureActive();return context_->get(options::Value(identity_),key);
    }
    void set(const options::Key& key,options::Value value)const{
        ensureActive();context_->set(options::Value(identity_),key,std::move(value));
    }
};

// Publish the three real enumerable own properties in Source literal order.
// Payload and NativeDataField owners are the exact existing native record.
inline std::shared_ptr<JaxFilterRecordContext> bindJaxFilterRecord(
    options::Context& context,const std::shared_ptr<NativeObject>& owner,
    std::weak_ptr<std::atomic_bool> lease,std::span<const JaxFilterRecordField> fields,
    std::optional<options::ObjectRef> existingIdentity={}){
    if(!owner||fields.size()!=3)
        throw options::Exception("TypeError",u"Jax filter record requires its actual owner and three fields");
    const auto identity=existingIdentity?*existingIdentity:context.object();
    context.payload(identity)=owner;
    auto result=std::make_shared<JaxFilterRecordContext>(context,identity,std::move(lease));
    for(const auto& field:fields){
        if(!field.readRaw||!field.writeRaw||!field.removeRaw)
            throw options::Exception("TypeError",u"Jax filter record field requires raw owner accessors and deletion");
        auto rawRead=field.readRaw;auto rawWrite=field.writeRaw;auto rawRemove=field.removeRaw;auto record=result;
        auto data=std::make_shared<options::NativeDataField>(owner,
            [record,rawRead=std::move(rawRead)](){record->ensureActive();return rawRead();},
            [record,rawWrite=std::move(rawWrite)](options::Value value){record->ensureActive();rawWrite(std::move(value));},
            [record,rawRemove=std::move(rawRemove)](){record->ensureActive();rawRemove();});
        context.bindData(identity,field.key,std::move(data));
    }
    return result;
}
}
