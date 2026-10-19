#include "support/folia_test.hpp"
import folia.math.util.promise_context_bindings;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o = folia::math::options;
namespace a = folia::math::async;

o::Value encode(const a::Value& input)
{
    if (const auto* value = std::any_cast<o::Value>(&input)) return *value;
    if (const auto* thrown = std::any_cast<a::ThrownValue>(&input))
        if (const auto* value = std::any_cast<o::Value>(&thrown->value)) return *value;
    return o::Value(o::Undefined{});
}

a::Value decode(o::Value value)
{
    return a::Value(std::move(value));
}

struct PromiseOwner
{
    o::Context context;
    OptionFunctionBindings provider{context};
    std::shared_ptr<std::atomic_bool> lease = std::make_shared<std::atomic_bool>(true);
    a::QueueRef queue = std::make_shared<a::JobQueue>();
    std::shared_ptr<PromiseContextBindings> bindings;

    PromiseOwner()
    {
        bindings = std::make_shared<PromiseContextBindings>(context, queue, lease, encode, decode);
        bindings->initialize();
    }
};

a::PromiseRef payloadPromise(o::Context& context, o::Value const& wrapper)
{
    const auto* object = std::get_if<o::ObjectRef>(&wrapper.data);
    if (!object) return {};
    const auto* promise = std::any_cast<a::PromiseRef>(&context.payload(*object));
    return promise ? *promise : a::PromiseRef{};
}

suite promise_context_owner = [] {
    "pending real promise reaction does not retain its queue or context bindings"_test = [] {
        std::weak_ptr<a::JobQueue> weakQueue;
        std::weak_ptr<PromiseContextBindings> weakBindings;
        a::PromiseRef pending;
        a::PromiseRef chained;
        {
            auto owner = std::make_shared<PromiseOwner>();
            weakQueue = owner->queue;
            weakBindings = owner->bindings;
            pending = a::Promise::create(owner->queue);
            const o::Value wrapped(owner->bindings->wrap(pending));
            const auto callback = owner->context.function(u"pending", [](o::Context&, o::Value, std::span<const o::Value>) {
                return o::Value(u"finished");
            });
            const auto then = owner->context.get(wrapped, o::Key(u"then"));
            const auto result = owner->context.callMember(then, wrapped, std::array<o::Value, 1>{o::Value(callback)});
            chained = payloadPromise(owner->context, result);
            expect(chained && chained->state() == a::State::pending)
                << "the actual context projection has installed a pending native reaction";
            owner.reset();
        }
        expect(weakQueue.expired()) << "JobQueue -> PromiseState -> pending reaction does not keep the queue alive";
        expect(weakBindings.expired()) << "pending reaction keeps only a weak PromiseContextBindings owner";
        expect(pending && chained) << "native promise handles themselves do not own their queue";
    };

    "live canonical queue preserves rejection then catch chaining"_test = [] {
        PromiseOwner owner;
        auto input = a::Promise::create(owner.queue);
        const o::Value wrapped(owner.bindings->wrap(input));
        const auto wrapperAgain = owner.bindings->wrap(input);
        expect(wrapperAgain == std::get<o::ObjectRef>(wrapped.data)) << "one native promise has one canonical context wrapper";

        std::vector<o::Value> caught;
        const auto fulfilled = owner.context.function(u"fulfilled", [](o::Context&, o::Value, std::span<const o::Value>) {
            return o::Value(u"unexpected fulfillment");
        });
        const auto then = owner.context.get(wrapped, o::Key(u"then"));
        const auto chainedWrapper = owner.context.callMember(
            then, wrapped, std::array<o::Value, 1>{o::Value(fulfilled)});
        const auto chained = payloadPromise(owner.context, chainedWrapper);
        expect(chained && chained->belongsTo(owner.queue)) << "then returns a promise in the original queue";

        const auto catchMethod = owner.context.get(chainedWrapper, o::Key(u"catch"));
        const auto catchCallback = owner.context.function(u"catch", [&](o::Context&, o::Value, std::span<const o::Value> args) {
            caught.assign(args.begin(), args.end());
            return o::Value(u"caught after rejection");
        });
        const auto outputWrapper = owner.context.callMember(
            catchMethod, chainedWrapper, std::array<o::Value, 1>{o::Value(catchCallback)});
        const auto output = payloadPromise(owner.context, outputWrapper);
        expect(output && output != chained && output->belongsTo(owner.queue))
            << "catch creates a distinct promise on the same canonical queue";

        input->reject(a::ThrownValue{a::Value(o::Value(u"original rejection"))});
        while (owner.queue->runOne()) {}
        expect(caught == std::vector<o::Value>{o::Value(u"original rejection")});
        expect(output->state() == a::State::fulfilled);
        expect(std::any_cast<o::Value>(output->value()) == o::Value(u"caught after rejection"));

        auto recoveredInput = a::Promise::create(owner.queue);
        const o::Value recoveredWrapped(owner.bindings->wrap(recoveredInput));
        const auto recovery = owner.context.function(u"recovery", [](o::Context&, o::Value, std::span<const o::Value>) {
            return o::Value(u"recovered");
        });
        const auto recoveryThen = owner.context.get(recoveredWrapped, o::Key(u"then"));
        const auto recoveredChain = owner.context.callMember(
            recoveryThen, recoveredWrapped, std::array<o::Value, 2>{o::Value(fulfilled), o::Value(recovery)});
        const auto recoveryCatch = owner.context.get(recoveredChain, o::Key(u"catch"));
        bool catchAfterRecoveryCalled = false;
        const auto recoveryCatchCallback = owner.context.function(u"catch after recovery", [&](o::Context&, o::Value, std::span<const o::Value>) {
            catchAfterRecoveryCalled = true;
            return o::Value(u"unexpected catch");
        });
        const auto recoveredOutputWrapper = owner.context.callMember(
            recoveryCatch, recoveredChain, std::array<o::Value, 1>{o::Value(recoveryCatchCallback)});
        const auto recoveredOutput = payloadPromise(owner.context, recoveredOutputWrapper);
        recoveredInput->reject(a::ThrownValue{a::Value(o::Value(u"handled rejection"))});
        while (owner.queue->runOne()) {}
        expect(!catchAfterRecoveryCalled) << "catch does not run after then's rejection handler fulfills its chain";
        expect(recoveredOutput->state() == a::State::fulfilled);
        expect(std::any_cast<o::Value>(recoveredOutput->value()) == o::Value(u"recovered"));
    };

    "expired bindings reject late invocation and pending reactions explicitly"_test = [] {
        auto owner = std::make_shared<PromiseOwner>();
        auto queue = owner->queue;
        auto input = a::Promise::create(queue);
        const o::Value wrapped(owner->bindings->wrap(input));
        const auto then = owner->context.get(wrapped, o::Key(u"then"));
        const auto callback = owner->context.function(u"late", [](o::Context&, o::Value, std::span<const o::Value>) {
            return o::Value(u"must not run");
        });
        const auto result = owner->context.callMember(then, wrapped, std::array<o::Value, 1>{o::Value(callback)});
        const auto chained = payloadPromise(owner->context, result);
        std::weak_ptr<PromiseContextBindings> weakBindings = owner->bindings;

        owner->bindings.reset();
        expect(weakBindings.expired()) << "context method callbacks do not retain the bindings provider";
        bool directExpiredError = false;
        try {
            (void)owner->context.callMember(then, wrapped, std::array<o::Value, 1>{o::Value(callback)});
        } catch (const o::Exception& error) {
            directExpiredError = error.name == "TypeError" && error.message == u"Promise Context services have expired";
        }
        expect(directExpiredError) << "calling a saved then method after owner expiry returns the explicit service error";

        input->resolve(a::Value(o::Value(u"late value")));
        while (queue->runOne()) {}
        expect(chained && chained->state() == a::State::rejected)
            << "a queued reaction whose bindings expired rejects its canonical next promise";
        const auto* failure = std::any_cast<std::exception_ptr>(&chained->value());
        bool reactionExpiredError = false;
        if (failure) {
            try {
                std::rethrow_exception(*failure);
            } catch (const o::Exception& error) {
                reactionExpiredError = error.name == "TypeError" && error.message == u"Promise Context services have expired";
            }
        }
        expect(reactionExpiredError) << "late pending reaction propagates the explicit service-expired error";
    };
};
}
