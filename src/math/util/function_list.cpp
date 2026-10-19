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
    FunctionListItemsAccessRef access;
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
        if (!frame->access)functions::typeError(u"FunctionList captured a null item owner");
        ++frame->index;
        if (!frame->access->indexBeforeLength(frame->index)) {
            frame->output->resolve(std::any(true));
            return;
        }
        if (frame->index < 0 || std::trunc(frame->index) != frame->index)
            functions::typeError(u"Invalid FunctionList item index");

        const auto itemIndex = static_cast<std::size_t>(frame->index);
        const auto record=frame->access->read(itemIndex);
        std::any result;
        if(record.native()&&!record.contextBacked())result=functions::invoke(record.nativeRecord(),frame->arguments);
        else{
            std::vector<options::Value> args;args.reserve(frame->arguments.size());
            for(const auto& arg:frame->arguments)args.push_back(frame->access->convertArgument(arg));
            const auto returned=record.invoke(args);
            if(const auto* object=std::get_if<options::ObjectRef>(&returned.data)){
                const auto& payload=record.context()->payload(*object);
                if(const auto* promise=std::any_cast<async::PromiseRef>(&payload);promise&&*promise)result=*promise;
                else result=returned;
            }else result=returned;
        }

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
void FunctionList::addListWithDefault(FunctionDefinitionsRef list,double defaultPriority){
    if(!list)functions::typeError(u"FunctionList definition list is null");
    class NativeDefinitions final:public FunctionDefinitionsSource{
        FunctionDefinitionsRef list_;
    public:
        explicit NativeDefinitions(FunctionDefinitionsRef list):list_(std::move(list)){}
        std::size_t length()override{return list_->size();}
        FunctionDefinition read(std::size_t index)override{return (*list_)[index];}
    } source(std::move(list));
    appendDefinitions(source,defaultPriority);
}
void FunctionList::addListWithDefault(FunctionDefinitionsSource& source,double defaultPriority){
    appendDefinitions(source,defaultPriority);
}
void FunctionList::rebindStorage(StorageRef storage){
    Base::rebindStorage(std::move(storage));
}
void FunctionList::appendDefinitions(FunctionDefinitionsSource& source,std::optional<double> defaultPriority){
    // Match `for (const item of list)`: the source identity is retained and
    // its current length/value are observed on each step.
    for(std::size_t index=0;index<source.length();++index){
        const FunctionDefinition definition=source.read(index);
        if(const auto* direct=std::get_if<FunctionValue>(&definition)){
            if(defaultPriority)this->add(*direct,*defaultPriority);
            else this->add(*direct);
            continue;
        }

        const FunctionTupleRef tuple=std::get<FunctionTupleRef>(definition);
        if(!tuple)functions::typeError(u"FunctionList tuple is null");
        // Source reads both tuple slots before invoking the virtual add method.
        const FunctionValue item=tuple->item;
        const std::optional<double> priority=tuple->priority;
        if(priority)this->add(item,*priority);
        else if(defaultPriority)this->add(item,*defaultPriority);
        else this->add(item);
    }
}

bool FunctionList::execute(FunctionArgs args) {
    // Rest arguments are snapshotted once; storage is separately captured so
    // rebinds do not redirect this execution, while mutations stay live.
    const std::vector<std::any> arguments(args.begin(), args.end());
    return executeUsing([&](const ItemRef& record){
        if(record.native()&&!record.contextBacked())return !functions::isFalse(functions::invoke(record.nativeRecord(),arguments));
        std::vector<options::Value> values;values.reserve(arguments.size());
        for(const auto& arg:arguments)values.push_back(record.convertArgument(arg));
        const auto result=record.invoke(values);return !(std::holds_alternative<bool>(result.data)&&!std::get<bool>(result.data));
    });
}

bool FunctionList::executeUsing(const std::function<bool(const ItemRef&)>& callback){
    if(!callback)functions::typeError(u"FunctionList iterator callback is not callable");
    if(!itemsAccess())functions::typeError(u"FunctionList item owner is null");
    // Use the inherited iterator: it captures the current StorageRef, retains
    // the yielded record across callback mutations, and observes live length.
    auto current=begin();
    for(;;){
        auto step=current.next();
        if(step.done)return true;
        if(!callback(step.value))return false;
    }
}

async::PromiseRef FunctionList::asyncExecute(const async::QueueRef& queue,
                                             FunctionArgs args) {
    // Match source setup order: rest slots and items are captured before the
    // output promise is created; frame ownership is independent of this list.
    auto frame = std::make_shared<AsyncFrame>();
    frame->arguments.assign(args.begin(), args.end());
    frame->access=itemsAccess();
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
