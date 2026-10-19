// Context projection for the one native Promise/JobQueue service.
export module folia.math.util.promise_context_bindings;
export import folia.math.util.promise;
export import folia.math.util.option_functions;
import std;
export namespace folia::math {
class PromiseContextBindings:public std::enable_shared_from_this<PromiseContextBindings> {
    options::Context& context_;
    std::weak_ptr<async::JobQueue> queue_;
    std::weak_ptr<std::atomic_bool> lease_;
    std::function<options::Value(const async::Value&)> encode_;
    std::function<async::Value(options::Value)> decode_;
    std::vector<std::pair<std::weak_ptr<async::Promise>,options::ObjectRef>> wrappers_;
    options::ObjectRef prototype_{};
    async::QueueRef queueLease()const {
        const auto active=lease_.lock();
        if(!active||!active->load(std::memory_order_acquire))
            throw options::Exception("TypeError",u"Promise Context services have expired");
        auto queue=queue_.lock();
        if(!queue)throw options::Exception("TypeError",u"Promise Context services have expired");
        return queue;
    }
    void ensureActive()const{(void)queueLease();}
public:
    PromiseContextBindings(options::Context& context,async::QueueRef queue,
        std::weak_ptr<std::atomic_bool> lease,
        std::function<options::Value(const async::Value&)> encode,
        std::function<async::Value(options::Value)> decode)
        :context_(context),queue_(queue),lease_(std::move(lease)),encode_(std::move(encode)),decode_(std::move(decode)){}
    void initialize(){
        ensureActive();
        prototype_=context_.object();
        const std::weak_ptr<PromiseContextBindings> weak=shared_from_this();
        auto install=[weak](std::u16string name,bool catchOnly){
            const auto self=weak.lock();if(!self)throw options::Exception("TypeError",u"Promise Context services have expired");
            const auto method=self->context_.function(name,[weak,catchOnly](options::Context&,options::Value receiver,std::span<const options::Value> args)->options::Value{
                const auto state=weak.lock();if(!state)throw options::Exception("TypeError",u"Promise Context services have expired");
                const auto queue=state->queueLease();
                (void)queue;
                const auto promise=state->unwrap(receiver);if(!promise)throw options::Exception("TypeError",u"Promise method called on incompatible receiver");
                const options::Value fulfilled=args.empty()?options::Value(options::Undefined{}):args[0];
                const options::Value rejected=!catchOnly&&args.size()>1?args[1]:options::Value(options::Undefined{});
                auto adapt=[weak](options::Value callback)->async::Handler{
                    const auto state=weak.lock();if(!state)throw options::Exception("TypeError",u"Promise Context services have expired");state->ensureActive();
                    const auto* object=std::get_if<options::ObjectRef>(&callback.data);
                    if(!object||state->context_.kind(*object)!=options::ObjectKind::function)return {};
                    return [weak,callback=std::move(callback)](const async::Value& input)->async::Value{
                        const auto state=weak.lock();if(!state)throw options::Exception("TypeError",u"Promise Context services have expired");
                        const auto queue=state->queueLease();
                        (void)queue;
                        const std::array<options::Value,1> argv{state->encode_(input)};
                        return state->decode_(state->context_.callMember(callback,options::Value(options::Undefined{}),argv,u"Promise reaction"));
                    };
                };
                async::Handler onFulfilled=adapt(fulfilled),onRejected=adapt(rejected);
                const auto next=catchOnly?promise->catchError(std::move(onFulfilled)):
                    promise->then(std::move(onFulfilled),std::move(onRejected));
                return options::Value(state->wrap(next));
            });
            self->context_.define(method,options::Key(u"length"),options::Descriptor::data(options::Value(catchOnly?1.0:2.0),false,false,true));
            self->context_.define(self->prototype_,options::Key(name),options::Descriptor::data(options::Value(method),true,false,true));
        };
        install(u"then",false);install(u"catch",true);
    }
    async::QueueRef queue()const{return queueLease();}
    async::PromiseRef unwrap(options::Value value)const {
        const auto queue=queueLease();
        const auto* object=std::get_if<options::ObjectRef>(&value.data);
        if(!object)return {};
        if(const auto* promise=std::any_cast<async::PromiseRef>(&context_.payload(*object));promise&&*promise&&(*promise)->belongsTo(queue))return *promise;
        return {};
    }
    options::ObjectRef wrap(async::PromiseRef promise) {
        const auto queue=queueLease();
        if(!promise||!promise->belongsTo(queue))throw options::Exception("TypeError",u"Promise belongs to a different Context queue");
        for(auto& [known,object]:wrappers_)if(known.lock()==promise)return object;
        if(prototype_.index==0)throw options::Exception("TypeError",u"Promise Context methods are not initialized");
        const auto object=context_.object();context_.setPayload(object,promise);context_.setPrototype(object,prototype_);wrappers_.push_back({promise,object});
        return object;
    }
};
}
