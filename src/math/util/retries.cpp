// MathJax 4.1.3 ts/util/Retries.ts over the native promise queue.
module folia.math.util.retries;
import std;
namespace folia::math::retries {
RetryError::RetryError(async::PromiseRef retry):retry_(std::move(retry)){}
const char* RetryError::what()const noexcept{return "MathJax retry -- an asynchronous action is required; try using one of the promise-based functions and await its resolution.";}
[[noreturn]] void retryAfter(const async::QueueRef& queue){
    if(!queue)throw std::invalid_argument("retryAfter requires the loaded session promise queue");
    auto promise=async::Promise::create(queue);promise->resolve();retryAfter(std::move(promise));
}
[[noreturn]] void retryAfter(async::PromiseRef promise){
    throw RetryError(std::move(promise));
}
async::PromiseRef handleRetriesFor(const async::QueueRef& queue,std::function<std::any()> code){
    if(!queue)throw std::invalid_argument("handleRetriesFor requires the loaded session promise queue");
    if(!code)throw std::invalid_argument("handleRetriesFor requires a callable");
    const auto output=async::Promise::create(queue);
    auto run=std::make_shared<std::function<void()>>();
    const std::weak_ptr<std::function<void()>> weakRun=run;
    auto handleError=std::make_shared<std::function<void(std::any)>>();
    *handleError=[queue,output,weakRun](std::any reason){
        try{
            if(const auto* exception=std::any_cast<std::exception_ptr>(&reason);exception&&*exception)std::rethrow_exception(*exception);
            output->reject(std::move(reason));
        }catch(const RetryError& error){
            const auto retry=error.retry();
            if(!retry||!retry->belongsTo(queue)){output->reject(std::make_exception_ptr(error));return;}
            retry->then([run=weakRun.lock()](const std::any&){if(run)(*run)();return std::any{};},
                        [output](const std::any& retryError){output->reject(retryError);return std::any{};});
        }catch(...){output->reject(std::move(reason));}
    };
    *run=[queue,code=std::move(code),output,weakRun,handleError]()mutable{
        try{
            std::any result=code();
            if(const auto* pending=std::any_cast<async::PromiseRef>(&result);pending&&*pending){
                if(!(*pending)->belongsTo(queue))throw std::invalid_argument("retry promise belongs to another session");
                const auto retainedRun=weakRun.lock();
                (*pending)->then([output](const std::any& value){output->resolve(value);return std::any{};},
                                 [handleError,retainedRun](const std::any& reason){
                                     // Keep the recursive Source `run` closure
                                     // alive while a returned Promise rejects;
                                     // handleError may attach a retry continuation
                                     // through its weak reference.
                                     (void)retainedRun;
                                     (*handleError)(reason);
                                     return std::any{};
                                 });
                return;
            }
            output->resolve(std::move(result));
        }catch(const RetryError& error){
            const auto retry=error.retry();
            if(!retry||!retry->belongsTo(queue)){output->reject(std::make_exception_ptr(error));return;}
            retry->then([run=weakRun.lock()](const std::any&){if(run)(*run)();return std::any{};},
                        [output](const std::any& reason){output->reject(reason);return std::any{};});
        }catch(...){(*handleError)(async::currentReason());}
    };
    (*run)();
    return output;
}
}
