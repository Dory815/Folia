// Root generic native promise/job host; worker session owns all state records.
module folia.math.util.promise;
import std;
namespace folia::math::async {
struct PromiseState {
    State state=State::pending;
    Value value;
    bool resolved=false;
    std::vector<Promise::Reaction> reactions;
};
namespace {
[[noreturn]] void fail(std::u16string message){throw tree::FactoryError("TypeError",std::move(message));}
}
Value currentReason(){
    const auto exception=std::current_exception();
    if(!exception)fail(u"No current native exception");
    try{std::rethrow_exception(exception);}catch(const ThrownValue& error){return error.value;}catch(...){return exception;}
}
JobQueue::JobQueue()=default;
JobQueue::~JobQueue()=default;
void JobQueue::post(std::function<void()> job){
    if(!job)fail(u"Native job is not callable");
    jobs_.push_back(std::move(job));
}
bool JobQueue::runOne(){
    if(jobs_.empty())return false;
    auto job=std::move(jobs_.front());
    jobs_.pop_front();
    job();
    return true;
}
std::size_t JobQueue::pending()const{return jobs_.size();}
Promise::Promise(const QueueRef&queue):queue_(queue),realm_(queue?queue->realm():nullptr){
    if(!queue)fail(u"A native promise requires a live session queue");
    index_=queue->promises_.size();
    queue->promises_.push_back(std::make_unique<PromiseState>());
}
PromiseRef Promise::create(const QueueRef&queue){return PromiseRef(new Promise(queue));}
PromiseState& Promise::record()const{
    const auto queue=queue_.lock();
    if(!queue||index_>=queue->promises_.size())fail(u"Native promise session has expired");
    return *queue->promises_[index_];
}
State Promise::state()const{return record().state;}
const Value& Promise::value()const{return record().value;}
bool Promise::belongsTo(const QueueRef&queue)const{return queue&&realm_==queue->realm();}
void Promise::schedule(Reaction reaction){
    const auto queue=queue_.lock();
    if(!queue)fail(u"Native promise session has expired");
    const auto& current=record();
    const auto state=current.state;
    const Value value=current.value;
    queue->post([state,value,reaction=std::move(reaction)]()mutable{
        const auto& handler=state==State::fulfilled?reaction.fulfilled:reaction.rejected;
        if(!handler){
            if(state==State::fulfilled)reaction.next->resolve(value);
            else reaction.next->reject(value);
            return;
        }
        try{reaction.next->resolve(handler(value));}
        catch(...){reaction.next->reject(currentReason());}
    });
}
void Promise::settle(State state,Value value){
    auto& current=record();
    if(current.state!=State::pending)return;
    current.state=state;
    current.value=std::move(value);
    auto reactions=std::move(current.reactions);
    current.reactions.clear();
    for(auto& reaction:reactions)schedule(std::move(reaction));
}
void Promise::resolve(Value value){
    auto& current=record();
    if(current.resolved)return;
    current.resolved=true;
    const auto* promise=std::any_cast<PromiseRef>(&value);
    if(promise&&*promise){
        const auto adopted=*promise;
        const auto self=shared_from_this();
        if(adopted==self){
            settle(State::rejected,std::make_exception_ptr(tree::FactoryError("TypeError",u"Native promise cannot resolve itself")));
            return;
        }
        // Source resolution of another promise enters a queued thenable job.
        const auto queue=queue_.lock();
        if(!queue)fail(u"Native promise session has expired");
        queue->post([self,adopted]{
            try{
                adopted->then([self](const Value&value){self->settle(State::fulfilled,value);return Value{};},
                              [self](const Value&reason){self->settle(State::rejected,reason);return Value{};});
            }catch(...){self->settle(State::rejected,currentReason());}
        });
        return;
    }
    settle(State::fulfilled,std::move(value));
}
void Promise::reject(Value reason){
    auto& current=record();
    if(current.resolved)return;
    current.resolved=true;
    settle(State::rejected,std::move(reason));
}
PromiseRef Promise::then(Handler fulfilled,Handler rejected){
    const auto queue=queue_.lock();
    if(!queue)fail(u"Native promise session has expired");
    auto next=create(queue);
    Reaction reaction{std::move(fulfilled),std::move(rejected),next};
    auto& current=record();
    if(current.state==State::pending)current.reactions.push_back(std::move(reaction));
    else schedule(std::move(reaction));
    return next;
}
PromiseRef Promise::catchError(Handler rejected){return then({},std::move(rejected));}
}
