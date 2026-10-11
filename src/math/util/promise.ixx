// Native asynchronous host service. No JS interpreter, platform API or thread creation.
export module folia.math.util.promise;
export import folia.math.core.tree.factory_types;
import std;
export namespace folia::math::async {
using Value=std::any;
struct ThrownValue {Value value;};
Value currentReason();
struct PromiseState;
class JobQueue {
    std::deque<std::function<void()>> jobs_;
    std::vector<std::unique_ptr<PromiseState>> promises_;
    std::shared_ptr<const int> realm_=std::make_shared<const int>(0);
    friend class Promise;
public:
    JobQueue();
    ~JobQueue();
    JobQueue(const JobQueue&)=delete;
    JobQueue& operator=(const JobQueue&)=delete;
    void post(std::function<void()>);
    bool runOne();
    std::size_t pending()const;
    const std::shared_ptr<const int>& realm()const{return realm_;}
};
using QueueRef=std::shared_ptr<JobQueue>;
class Promise;
using PromiseRef=std::shared_ptr<Promise>;
using Handler=std::function<Value(const Value&)>;
enum class State {pending,fulfilled,rejected};
class Promise:public std::enable_shared_from_this<Promise> {
    struct Reaction {Handler fulfilled,rejected;PromiseRef next;};
    std::weak_ptr<JobQueue> queue_;
    std::shared_ptr<const int> realm_;
    std::size_t index_=0;
    friend struct PromiseState;
    explicit Promise(const QueueRef&);
    PromiseState& record()const;
    void settle(State,Value);
    void schedule(Reaction);
public:
    static PromiseRef create(const QueueRef&);
    Promise(const Promise&)=delete;
    Promise& operator=(const Promise&)=delete;
    void resolve(Value={});
    void reject(Value);
    PromiseRef then(Handler fulfilled={},Handler rejected={});
    PromiseRef catchError(Handler rejected);
    State state()const;
    const Value& value()const;
    bool belongsTo(const QueueRef&)const;
};
}
