/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 util/FunctionList.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA256: d4106c42268e0d7cb3549f8ffdbeb5bab87eae6daffae7314d5e724045a6918f
 *************************************************************/
module folia.math.util.function_list;
import std;

namespace folia::math {
namespace {

struct AsyncFrame {
    PrioritizedList<FunctionValue>::StorageRef items;
    std::vector<std::any> arguments;
    double index = -1;
    std::weak_ptr<async::JobQueue> queue;
    async::PromiseRef output;
};

void resumeAsync(const std::shared_ptr<AsyncFrame>& frame) {
    // The session owns promise state. A resume holds it only for this
    // synchronous turn; continuations retain the frame's weak queue handle.
    const auto queue = frame->queue.lock();
    if (!queue) functions::typeError(u"FunctionList async session has expired");

    for (;;) {
        ++frame->index;
        if (!frame->items)
            functions::typeError(u"FunctionList captured a null item array");
        if (frame->index >= static_cast<double>(frame->items->size())) {
            frame->output->resolve(std::any(true));
            return;
        }
        if (frame->index < 0 || std::trunc(frame->index) != frame->index)
            functions::typeError(u"Invalid FunctionList item index");

        const auto itemIndex = static_cast<std::size_t>(frame->index);
        const auto record = (*frame->items)[itemIndex];
        if (!record) functions::typeError(u"FunctionList item record is null");
        std::any result = functions::invoke(record, frame->arguments);

        if (auto input = functions::promise(result, queue)) {
            // Keep the source's two links: input.then(execute).catch(fail).
            // The intermediate promise propagates input rejection first.
            auto intermediate = input->then(
                [frame](const async::Value&) -> async::Value {
                    resumeAsync(frame);
                    return {};
                });
            intermediate->catchError(
                [output = frame->output](const async::Value& reason) -> async::Value {
                    output->reject(reason);
                    return {};
                });
            return;
        }

        if (functions::isFalse(result)) {
            frame->output->resolve(std::any(false));
            return;
        }
    }
}

}

void FunctionList::initializeFunctions(FunctionDefinitionsRef list) {
    // makeFunctionList invokes this after allocation, so virtual addList sees
    // the completed derived object just as the JS constructor does.
    if (list) addList(std::move(list));
}

void FunctionList::addList(FunctionDefinitionsRef list) {
    if (!list) functions::typeError(u"FunctionList definition list is null");
    class NativeDefinitions final:public FunctionDefinitionsSource {
        FunctionDefinitionsRef list_;
    public:
        explicit NativeDefinitions(FunctionDefinitionsRef list):list_(std::move(list)){}
        std::size_t length()override{return list_->size();}
        FunctionDefinition read(std::size_t index)override{return (*list_)[index];}
    } source(std::move(list));
    appendDefinitions(source);
}
void FunctionList::addList(FunctionDefinitionsSource& source){appendDefinitions(source);}
void FunctionList::appendDefinitions(FunctionDefinitionsSource&){
    throw tree::FactoryError("Unimplemented",u"FunctionList::appendDefinitions");
}

bool FunctionList::execute(FunctionArgs args) {
    // Rest arguments are snapshotted once; storage is separately captured so
    // rebinds do not redirect this execution, while mutations stay live.
    const std::vector<std::any> arguments(args.begin(), args.end());
    if (!itemStorage()) functions::typeError(u"FunctionList item array is null");
    for (auto current = begin(), finish = end(); current != finish; ++current) {
        const auto record = current.record();
        if (!record) functions::typeError(u"FunctionList item record is null");
        const std::any result = functions::invoke(record, arguments);
        if (functions::isFalse(result)) return false;
    }
    return true;
}

async::PromiseRef FunctionList::asyncExecute(const async::QueueRef& queue,
                                             FunctionArgs args) {
    // Match source setup order: rest slots and items are captured before the
    // output promise is created; frame ownership is independent of this list.
    auto frame = std::make_shared<AsyncFrame>();
    frame->arguments.assign(args.begin(), args.end());
    frame->items = itemStorage();
    frame->index = -1;
    frame->queue = queue;
    if (!queue) functions::typeError(u"FunctionList async session is missing");

    frame->output = async::Promise::create(queue);
    try {
        // Promise executors run synchronously; callback failures reject this
        // already-created output promise rather than escaping the method.
        resumeAsync(frame);
    } catch (...) {
        frame->output->reject(async::currentReason());
    }
    return frame->output;
}

}
