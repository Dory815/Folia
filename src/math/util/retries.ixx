// MathJax 4.1.3 ts/util/Retries.ts typed promise adapter.
// It models retry errors and the retry loop over the native job queue; it does
// not provide JavaScript Promise reflection or legacy MathJax.Callback.
export module folia.math.util.retries;
export import folia.math.util.promise;
import std;
export namespace folia::math::retries {
class RetryError final:public std::exception {
    async::PromiseRef retry_;
public:
    explicit RetryError(async::PromiseRef);
    const char* what()const noexcept override;
    async::PromiseRef retry()const{return retry_;}
};
[[noreturn]] void retryAfter(const async::QueueRef&);
[[noreturn]] void retryAfter(async::PromiseRef);
async::PromiseRef handleRetriesFor(const async::QueueRef&,std::function<std::any()>);
}
