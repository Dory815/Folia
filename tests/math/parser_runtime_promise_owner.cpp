#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.util.option_functions;
import folia.math.util.regexp_host;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o = folia::math::options;
namespace a = folia::math::async;
namespace t = folia::math::tex;

struct RuntimeOwner {
    o::Context context;
    OptionFunctionBindings functions{context};
    regexp_host::RegExpBindings regexp{context};
    Unicode16Normalization unicode;
    core::InputJaxModule input{context};
    core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context, findMath, regexp};
    std::unique_ptr<t::ParserRuntime> runtime;

    RuntimeOwner() {
        runtime = std::make_unique<t::ParserRuntime>(context, functions, regexp, unicode, input, findTeX);
        runtime->initializeParserRuntime();
    }
};

a::PromiseRef payloadPromise(o::Context& context, const o::Value& wrapper) {
    const auto* object = std::get_if<o::ObjectRef>(&wrapper.data);
    if (!object) return {};
    const auto* promise = std::any_cast<a::PromiseRef>(&context.payload(*object));
    return promise ? *promise : a::PromiseRef{};
}

suite parser_runtime_promise_owner = [] {
    "ParserRuntime FunctionList async execution uses the runtime queue and expires with its Context"_test = [] {
        std::weak_ptr<a::JobQueue> weakQueue;
        a::PromiseRef inputPromise;
        a::PromiseRef outputPromise;
        a::PromiseRef chainedPromise;
        o::ObjectRef listObject{};
        o::ObjectRef savedAsyncExecute{};
        o::ObjectRef savedThen{};
        o::ObjectRef outputWrapperObject{};
        {
            auto owner = std::make_unique<RuntimeOwner>();
            auto& context = owner->context;
            auto& runtime = *owner->runtime;
            const auto queue = runtime.queue();
            weakQueue = queue;
            inputPromise = a::Promise::create(queue);

            // This Context object is the real OptionFunctionBindings result. Its
            // payload is the native Promise returned by the callback, as in the
            // FunctionListContextBindings async value conversion path.
            const auto inputWrapper = context.object();
            context.setPayload(inputWrapper, inputPromise);
            auto native = std::make_shared<Function>();
            native->call = [inputWrapper](const FunctionReceiver&, FunctionArgs) -> std::any {
                return std::any(o::Value(inputWrapper));
            };
            OptionCallBinding binding{
                [](o::Context&, o::Value receiver) { return FunctionReceiver(std::move(receiver)); },
                [](o::Context&, std::span<const o::Value>) { return std::vector<std::any>{}; },
                [inputWrapper](o::Context&, const std::any& result) -> o::Value {
                    const auto* returned = std::any_cast<o::Value>(&result);
                    const auto* returnedObject = returned ? std::get_if<o::ObjectRef>(&returned->data) : nullptr;
                    if (!returnedObject || *returnedObject != inputWrapper)
                        throw std::logic_error("async callback result lost its Context Promise object");
                    return *returned;
                }};
            const auto callback = owner->functions.bind(native, std::move(binding), u"pending Promise callback");
            const auto definitions = context.array();
            context.set(definitions, o::Key(u"0"), o::Value(callback));
            auto list = runtime.functionLists.make(o::Value(definitions));
            listObject = runtime.functionLists.bind(list);
            savedAsyncExecute = std::get<o::ObjectRef>(context.get(o::Value(listObject), o::Key(u"asyncExecute")).data);
            const auto outputWrapper = context.callMember(savedAsyncExecute, o::Value(listObject), std::span<const o::Value>{});
            outputWrapperObject = std::get<o::ObjectRef>(outputWrapper.data);
            outputPromise = payloadPromise(context, outputWrapper);
            expect(outputPromise && outputPromise->belongsTo(queue) && outputPromise->state() == a::State::pending)
                << "the actual FunctionList asyncExecute returns a pending Promise owned by ParserRuntime.queue";

            const auto continuation = context.function(u"continue", [](o::Context&, o::Value, std::span<const o::Value> args) {
                return args.empty() ? o::Value(o::Undefined{}) : args.front();
            });
            savedThen = std::get<o::ObjectRef>(context.get(outputWrapper, o::Key(u"then")).data);
            const auto chainedWrapper = context.callMember(savedThen, outputWrapper, std::array<o::Value, 1>{o::Value(continuation)});
            chainedPromise = payloadPromise(context, chainedWrapper);
            expect(chainedPromise && chainedPromise->belongsTo(queue) && chainedPromise->state() == a::State::pending)
                << "then continues the real async execution on the same runtime queue";

            expect(inputPromise->state() == a::State::pending && outputPromise->state() == a::State::pending &&
                   chainedPromise->state() == a::State::pending)
                << "the real async list and its Context then chain remain pending on the unresolved callback Promise";

            // The still-live Context owns the installed FunctionList service
            // closures. ParserRuntime shutdown invalidates them, but does not
            // forcibly detach roots that the Context still owns.
            owner->runtime.reset();
            expect(!weakQueue.expired()) << "the live Context remains the intentional owner of installed FunctionList services";
            bool asyncExpired = false;
            try {
                (void)context.callMember(savedAsyncExecute, o::Value(listObject), std::span<const o::Value>{});
            } catch (const o::Exception& error) {
                asyncExpired = error.name == "TypeError" && error.message == u"FunctionList runtime services have expired";
            }
            expect(asyncExpired) << "an old FunctionList method reports an explicit error after ParserRuntime shutdown";
            bool promiseExpired = false;
            try {
                (void)context.callMember(savedThen, o::Value(outputWrapperObject), std::span<const o::Value>{});
            } catch (const o::Exception& error) {
                promiseExpired = error.name == "TypeError" && error.message == u"Promise Context services have expired";
            }
            expect(promiseExpired) << "an old Promise method reports an explicit error after ParserRuntime shutdown";
            owner.reset();
        }
        expect(weakQueue.expired()) << "after Context and ParserRuntime owners leave scope, pending native handles do not retain the queue";
        expect(inputPromise && outputPromise && chainedPromise)
            << "external PromiseRef handles are non-owning with respect to the queue";
    };
};
} // namespace



