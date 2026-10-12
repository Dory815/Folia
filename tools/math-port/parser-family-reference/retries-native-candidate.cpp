// Typed candidate for the pinned RetryError/retryAfter/handleRetriesFor path.
// Uses the actual native Promise/JobQueue; it does not claim JS Promise or
// Error-object reflection.
#include "../../../tests/support/folia_test.hpp"
import folia.math.util.retries;

namespace {
using namespace boost::ut;
using namespace folia::math;

suite retries_native_candidate=[] {
    "retry_after_throws_with_the_same_retry_promise_and_source_message"_test=[] {
        const auto queue=std::make_shared<async::JobQueue>();
        const auto retry=async::Promise::create(queue);
        bool caught=false;
        try{retries::retryAfter(retry);}
        catch(const retries::RetryError& error){
            caught=true;
            expect(error.retry()==retry);
            expect(eq(std::string(error.what()),std::string(
                "MathJax retry -- an asynchronous action is required; try using one of the promise-based functions and await its resolution.")));
        }
        expect(caught);
    };

    "handle_retries_resolves_the_original_promise_with_a_synchronous_value"_test=[] {
        const auto queue=std::make_shared<async::JobQueue>();
        const auto result=retries::handleRetriesFor(queue,[]()->std::any{return 17;});
        expect(result->state()==async::State::fulfilled);
        const auto* value=std::any_cast<int>(&result->value());
        expect(value&&*value==17);
    };

    "retry_after_replays_only_after_its_real_promise_resolves"_test=[] {
        const auto queue=std::make_shared<async::JobQueue>();
        const auto retry=async::Promise::create(queue);
        std::size_t calls=0;
        const auto result=retries::handleRetriesFor(queue,[&]()->std::any{
            if(++calls==1)retries::retryAfter(retry);
            return std::any(42);
        });
        expect(eq(calls,std::size_t{1}));
        expect(result->state()==async::State::pending);
        retry->resolve();
        while(queue->runOne()){}
        expect(eq(calls,std::size_t{2}));
        expect(result->state()==async::State::fulfilled);
        const auto* value=std::any_cast<int>(&result->value());
        expect(value&&*value==42);
    };

    "ordinary_failure_rejects_the_original_output_promise"_test=[] {
        const auto queue=std::make_shared<async::JobQueue>();
        const auto result=retries::handleRetriesFor(queue,[]()->std::any{
            throw std::runtime_error("source failure");
        });
        expect(result->state()==async::State::rejected);
        const auto* error=std::any_cast<std::exception_ptr>(&result->value());
        expect(error&&static_cast<bool>(*error));
        if(error&&*error){
            try{std::rethrow_exception(*error);}
            catch(const std::runtime_error& failure){expect(eq(std::string(failure.what()),std::string("source failure")));}
            catch(...){expect(false);}
        }
    };

    "rejected_retry_promise_rejects_without_replaying_code"_test=[] {
        const auto queue=std::make_shared<async::JobQueue>();
        const auto retry=async::Promise::create(queue);
        std::size_t calls=0;
        const auto result=retries::handleRetriesFor(queue,[&]()->std::any{
            ++calls;
            retries::retryAfter(retry);
        });
        expect(eq(calls,std::size_t{1}));
        retry->reject(std::any(std::string("retry failure")));
        while(queue->runOne()){}
        expect(eq(calls,std::size_t{1}));
        expect(result->state()==async::State::rejected);
        const auto* reason=std::any_cast<std::string>(&result->value());
        expect(reason&&*reason=="retry failure");
    };

    "retry_error_from_a_returned_promise_retains_and_replays_the_run_closure"_test=[] {
        const auto queue=std::make_shared<async::JobQueue>();
        const auto returned=async::Promise::create(queue);
        const auto retry=async::Promise::create(queue);
        std::size_t calls=0;
        const auto result=retries::handleRetriesFor(queue,[&]()->std::any{
            if(++calls==1)return std::any(returned);
            return std::any(29);
        });
        expect(eq(calls,std::size_t{1}));
        expect(result->state()==async::State::pending);
        returned->reject(std::any(std::make_exception_ptr(retries::RetryError(retry))));
        while(queue->runOne()){}
        expect(eq(calls,std::size_t{1}));
        expect(result->state()==async::State::pending);
        retry->resolve();
        while(queue->runOne()){}
        expect(eq(calls,std::size_t{2}));
        expect(result->state()==async::State::fulfilled);
        const auto* value=std::any_cast<int>(&result->value());
        expect(value&&*value==29);
    };
};
}
