// Per-call Source-visible InputJax/OutputJax filter record Context identity.
// The Context object is the only property store; the NativeDataField callbacks
// below are raw-cell projections and never perform a Context lookup.
export module folia.math.core.jax_filter_record_context;
export import folia.math.util.options;
export import folia.math.util.object_types;
import std;

export namespace folia::math::core {
// Exact, document-routed ownership of the temporary outer filter argument.
// A missing document scope leaves ordinary Source-visible calls untouched.
class JaxFilterRecordOwnerScope {
public:
    class Token {
        friend class JaxFilterRecordOwnerScope;
        const JaxFilterRecordOwnerScope* owner_{};
        std::uint64_t nonce_{},serial_{};
        options::ObjectRef document_{};
        Token(const JaxFilterRecordOwnerScope* owner,std::uint64_t nonce,
              std::uint64_t serial,options::ObjectRef document) noexcept
            :owner_(owner),nonce_(nonce),serial_(serial),document_(document){}
    public:
        Token()=default;
        Token(const Token&)=delete;
        Token& operator=(const Token&)=delete;
        Token(Token&& other) noexcept
            :owner_(std::exchange(other.owner_,nullptr)),nonce_(std::exchange(other.nonce_,0)),
             serial_(std::exchange(other.serial_,0)),document_(other.document_){}
        Token& operator=(Token&&)=delete; // An open destination must not lose its batch.
    };
private:
    struct Batch {
        options::ObjectRef document;
        std::uint64_t serial;
        bool closing=false;
        std::vector<options::ObjectRef> records;
    };
    options::Context* context_;
    std::uint64_t nonce_,nextSerial_=1;
    std::vector<Batch> batches_;
    inline static std::atomic<std::uint64_t> nextNonce_{1};
public:
    explicit JaxFilterRecordOwnerScope(options::Context& context)
        :context_(&context),nonce_(nextNonce_.fetch_add(1,std::memory_order_relaxed)){}
    JaxFilterRecordOwnerScope(const JaxFilterRecordOwnerScope&)=delete;
    JaxFilterRecordOwnerScope& operator=(const JaxFilterRecordOwnerScope&)=delete;
    options::Context& context()const noexcept{return *context_;}
    Token begin(options::ObjectRef document){
        (void)context_->kind(document); // The token names a live identity in this Context.
        if(std::ranges::any_of(batches_,[](const Batch& batch){return batch.closing;}))
            throw options::Exception("TypeError",u"Jax filter record retirement is pending");
        if(std::ranges::any_of(batches_,[&](const Batch& batch){return batch.document==document;}))
            throw options::Exception("TypeError",u"Document already owns a Jax filter record scope");
        if(nextSerial_==0)throw options::Exception("RangeError",u"Jax filter record token serial exhausted");
        const auto serial=nextSerial_++;
        batches_.push_back(Batch{document,serial});
        return Token(this,nonce_,serial,document);
    }
    void capture(options::ObjectRef document,options::ObjectRef record){
        const auto found=std::ranges::find_if(batches_,[&](const Batch& batch){return batch.document==document;});
        if(found==batches_.end())return;
        if(found->closing)throw options::Exception("TypeError",u"Jax filter record scope is closing");
        found->records.push_back(record);
    }
    void close(Token& token){
        if(token.owner_!=this||token.nonce_!=nonce_||token.serial_==0)
            throw options::Exception("TypeError",u"Jax filter record token belongs to another owner or is stale");
        const auto found=std::ranges::find_if(batches_,[&](const Batch& batch){
            return batch.serial==token.serial_&&batch.document==token.document_;});
        if(found==batches_.end())
            throw options::Exception("TypeError",u"Jax filter record token is stale");
        found->closing=true;
        while(!found->records.empty()){
            context_->retireObjectRecord(found->records.back());
            found->records.pop_back();
        }
        batches_.erase(found);
        token.owner_=nullptr;token.serial_=0;token.nonce_=0;
    }
};

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
    try{
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
    }catch(...){
        // The caller that supplied an existing identity owns its cleanup.
        if(!existingIdentity)context.retireObjectRecord(identity);
        throw;
    }
}
}
