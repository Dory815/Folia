#include "support/folia_test.hpp"
import folia.math.util.function_list_bindings;
import folia.math.input.tex.map_handler;
import folia.math.util.option_functions;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o=folia::math::options;

suite function_list_context_array_owner=[] {
  "context_to_object_preserves_object_identity_and_primitive_delete_semantics"_test=[] {
    o::Context context;
    const auto array=context.array();
    expect(context.toObject(o::Value(array))==array);
    const auto boxed=context.toObject(o::Value(u"x"));
    expect(context.get(o::Value(boxed),o::Key(u"0"))==o::Value(u"x"));
    bool typeError=false;
    try{context.erase(boxed,o::Key(u"0"));}catch(const o::Exception& error){typeError=error.name=="TypeError";}
    expect(typeError);
    typeError=false;
    try{(void)context.toObject(o::Value(o::Null{}));}catch(const o::Exception& error){typeError=error.name=="TypeError";}
    expect(typeError);
  };

  "generic_prioritized_array_checks_provider_lease_before_all_context_entrypoints"_test=[] {
    o::Context context;auto lease=std::make_shared<std::atomic_bool>(true);
    using Cell=PrioritizedListItem<FunctionValue>;
    using Array=PrioritizedListContextArray<FunctionValue>;
    auto storage=std::make_shared<std::vector<std::shared_ptr<Cell>>>();
    storage->push_back({});
    auto cell=std::make_shared<Cell>(Cell{3,FunctionValue{Property("live")}});storage->push_back(cell);
    std::size_t recordReads=0,rawWrites=0;
    Array::Callbacks callbacks;
    callbacks.ensureActive=[weak=std::weak_ptr<std::atomic_bool>(lease)]{
      const auto active=weak.lock();if(!active||!active->load())throw o::Exception("TypeError",u"array owner expired");
    };
    callbacks.recordObject=[&](const std::shared_ptr<Cell>&){++recordReads;return context.object();};
    callbacks.cellFromValue=[](const o::Value&)->std::shared_ptr<Cell>{return {};};
    callbacks.rawCell=[&](o::Value value){++rawWrites;auto result=std::make_shared<Cell>(Cell{5,FunctionValue{Property(Missing{})}});result->rawArraySlot=true;result->rawSlotValue=std::move(value);return result;};
    const auto [array,view]=makePrioritizedListContextArray<FunctionValue>(context,storage,std::move(callbacks));
    expect(context.get(o::Value(array),o::Key(u"length"))==o::Value(2));
    expect(!context.hasOwn(o::Value(array),o::Key(u"0")));
    const auto liveRecord=context.get(o::Value(array),o::Key(u"1"));
    expect(std::holds_alternative<o::ObjectRef>(liveRecord.data));
    const auto recordCallsBefore=recordReads,rawCallsBefore=rawWrites;
    lease->store(false);
    const auto rejected=[&](auto&& operation){bool typeError=false;try{operation();}catch(const o::Exception& error){typeError=error.name=="TypeError";}return typeError;};
    expect(rejected([&]{(void)context.get(o::Value(array),o::Key(u"length"));}));
    expect(rejected([&]{(void)context.get(o::Value(array),o::Key(u"0"));}));
    expect(rejected([&]{(void)context.get(o::Value(array),o::Key(u"1"));}));
    expect(rejected([&]{context.set(o::Value(array),o::Key(u"0"),o::Value(17));}));
    expect(rejected([&]{context.set(o::Value(array),o::Key(u"length"),o::Value(0));}));
    expect(rejected([&]{context.erase(array,o::Key(u"1"));}));
    expect(rejected([&]{(void)context.ownKeys(o::Value(array));}));
    expect(recordReads==recordCallsBefore&&rawWrites==rawCallsBefore);
    expect(storage->size()==2&&(*storage)[0]==nullptr&&(*storage)[1]==cell);
  };

  "function_list_native_alias_survives_foreign_owner_rebind"_test=[] {
    o::Context context;
    OptionFunctionBindings functions(context);
    FunctionListContextBindings bindings(context,functions);
    auto list=std::make_shared<FunctionList>();
    list->add(FunctionValue{Property("a")},0);list->add(FunctionValue{Property("b")},1);
    auto old=list->storageRef();
    auto iterator=list->begin();auto oldEnd=list->end();
    const auto instance=o::Value(bindings.bind(list));
    const auto replacement=context.array();
    context.set(instance,o::Key(u"items"),o::Value(replacement));
    expect(!list->storageRef());
    list->add(FunctionValue{Property("new")},2);
    expect(context.toNumber(context.get(o::Value(replacement),o::Key(u"length")))==1);
    old->push_back(std::make_shared<FunctionListItem>(FunctionListItem{3,FunctionValue{Property("old-tail")}}));
    expect(std::get<Property>(iterator.record().nativeRecord()->item)==Property("a"));
    ++iterator;expect(std::get<Property>(iterator.record().nativeRecord()->item)==Property("b"));
    ++iterator;expect(std::get<Property>(iterator.record().nativeRecord()->item)==Property("old-tail"));
    ++iterator;expect(iterator==oldEnd);
  };

  "function_list_native_seed_context_operations_keep_raw_records"_test=[] {
    o::Context context;OptionFunctionBindings functions(context);FunctionListContextBindings bindings(context,functions);
    auto list=std::make_shared<FunctionList>();list->add(FunctionValue{Property("seed")},3);
    const auto oldStorage=list->storageRef();const auto seedRecord=(*oldStorage)[0];auto captured=list->begin();const auto capturedRecord=captured.record();
    const auto instance=o::Value(bindings.bind(list));
    expect(list->storageRef()==oldStorage);
    const auto items=context.get(instance,o::Key(u"items"));
    const auto seedObject=context.get(items,o::Key(u"0"));
    expect(std::any_cast<NativeObjectRef>(context.payload(std::get<o::ObjectRef>(seedObject.data)))==std::static_pointer_cast<NativeObject>(seedRecord));
    const auto callback=context.function(u"raw",[](o::Context&,o::Value,std::span<const o::Value>){return o::Value(true);});
    const auto add=context.get(instance,o::Key(u"add"));
    (void)context.callMember(add,instance,std::array<o::Value,2>{o::Value(callback),o::Value(u"12")});
    ++captured;const auto capturedAdded=captured.record();
    const auto record=context.get(items,o::Key(u"1"));
    expect(context.get(record,o::Key(u"item"))==o::Value(callback));
    expect(context.get(record,o::Key(u"priority"))==o::Value(u"12"));
    context.set(record,o::Key(u"priority"),o::Value(false));
    expect(context.get(record,o::Key(u"priority"))==o::Value(false));
    expect(capturedRecord.readPriorityValue()==o::Value(3));
    const auto newCallback=context.function(u"newRaw",[](o::Context&,o::Value,std::span<const o::Value>){return o::Value(true);});
    context.set(record,o::Key(u"item"),o::Value(newCallback));
    expect(std::get<o::ObjectRef>(std::get<o::Value>(capturedAdded.recordIdentity()).data)==std::get<o::ObjectRef>(record.data));
    expect(std::get<o::Value>(capturedAdded.readItem())==o::Value(newCallback));
    expect(capturedAdded.readPriorityValue()==o::Value(false));
    expect(oldStorage->size()==2&&(*oldStorage)[0]==seedRecord);
    list->remove(FunctionValue{Property("seed")});
    const auto execute=context.get(instance,o::Key(u"execute"));
    expect(context.callMember(execute,instance,std::span<const o::Value>{})==o::Value(true));
    expect(oldStorage->size()==1&&(*oldStorage)[0]!=seedRecord);
    // Removing the seed from the array detaches its record; it must not
    // rewrite that old record with the value assigned to the other entry.
    expect(!seedRecord->hasRawItem);
    expect(std::get<Property>(seedRecord->item)==Property("seed"));
    expect((*oldStorage)[0]->hasRawItem&&(*oldStorage)[0]->rawItem==o::Value(newCallback));
  };

  "function_list_replaceable_context_array_is_single_live_owner"_test=[] {
    o::Context context;
    OptionFunctionBindings functions(context);
    FunctionListContextBindings bindings(context,functions);
    auto list=std::make_shared<FunctionList>();
    const auto instance=o::Value(bindings.bind(list));
    const auto old=context.array(2);
    context.set(instance,o::Key(u"items"),o::Value(old));
    expect(!list->storageRef());
    std::vector<std::string> calls;
    const auto replacement=context.array();
    const auto callbackC=context.function(u"c",[&](o::Context&,o::Value,std::span<const o::Value>){calls.push_back("c");return o::Value(true);});
    const auto callbackB=context.function(u"b",[&](o::Context&,o::Value,std::span<const o::Value>){calls.push_back("b");return o::Value(true);});
    const auto callbackA=context.function(u"a",[&](o::Context& cx,o::Value,std::span<const o::Value>){
      calls.push_back("a");
      const auto splice=cx.get(o::Value(old),o::Key(u"splice"));
      (void)cx.callMember(splice,o::Value(old),std::array<o::Value,2>{o::Value(1),o::Value(1)});
      const auto record=cx.object();cx.set(o::Value(record),o::Key(u"item"),o::Value(callbackC));cx.set(o::Value(record),o::Key(u"priority"),o::Value(3));
      const auto push=cx.get(o::Value(old),o::Key(u"push"));
      (void)cx.callMember(push,o::Value(old),std::array<o::Value,1>{o::Value(record)});
      cx.set(instance,o::Key(u"items"),o::Value(replacement));
      return o::Value(true);
    });
    const auto recordA=context.object(),recordB=context.object();
    context.set(o::Value(recordA),o::Key(u"item"),o::Value(callbackA));context.set(o::Value(recordA),o::Key(u"priority"),o::Value(0));
    context.set(o::Value(recordB),o::Key(u"item"),o::Value(callbackB));context.set(o::Value(recordB),o::Key(u"priority"),o::Value(1));
    context.set(o::Value(old),o::Key(u"0"),o::Value(recordA));context.set(o::Value(old),o::Key(u"1"),o::Value(recordB));
    auto it=list->begin();
    const auto identity=std::get<o::Value>(it.record().recordIdentity());
    expect(identity==o::Value(recordA));
    expect(list->execute());
    expect(calls==std::vector<std::string>{"a","c"});
    expect(context.get(instance,o::Key(u"items"))==o::Value(replacement));
    expect(context.toNumber(context.get(o::Value(old),o::Key(u"length")))==2);
    ++it;
    expect(std::get<o::Value>(it.record().recordIdentity())==o::Value(context.get(o::Value(old),o::Key(u"1"))));
  };

  "map_handler_invokes_context_fallback_with_record_receiver"_test=[] {
    o::Context context;OptionFunctionBindings functions(context);FunctionListContextBindings bindings(context,functions);
    const auto parserObject=context.object();context.set(o::Value(parserObject),o::Key(u"currentCS"),o::Value(u"\\old"));
    struct Parser final:tex::ParserContext{o::Context* cx; o::ObjectRef object; Parser(o::Context& c,o::ObjectRef o):cx(&c),object(o){} tex::ParserContextIdentity contextIdentity()const override{return{cx,object};} Property currentCS()const override{return Property("\\old");}void setCurrentCS(Property)override{}} parser(context,parserObject);
    tex::MapHandler registry;tex::SubHandler handler(registry);const auto list=handler.fallbackOwner();
    const o::Value instance(bindings.bind(list));const auto array=context.array();context.set(instance,o::Key(u"items"),o::Value(array));
    const auto record=context.object();const auto callback=context.function(u"fallback",[&](o::Context& cx,o::Value receiver,std::span<const o::Value> args){
      expect(receiver==o::Value(record));expect(args.size()==2);expect(args[1]==o::Value(u"x"));
      expect(args[0]==o::Value(parserObject));
      expect(cx.get(args[0],o::Key(u"currentCS"))==o::Value(u"\\old"));
      cx.set(args[0],o::Key(u"currentCS"),o::Value(u"\\changed"));
      expect(cx.get(o::Value(parserObject),o::Key(u"currentCS"))==o::Value(u"\\changed"));
      return o::Value(true);
    });
    context.set(o::Value(record),o::Key(u"item"),o::Value(callback));context.set(o::Value(record),o::Key(u"priority"),o::Value(1));
    context.set(o::Value(array),o::Key(u"0"),o::Value(record));
    const auto result=handler.parse(tex::ParseInput{parser,Property("x")});
    expect(std::holds_alternative<Property>(result));
  };

  "function_list_context_array_hole_fails_when_native_execute_reads_it"_test=[] {
    o::Context context;
    OptionFunctionBindings functions(context);
    FunctionListContextBindings bindings(context,functions);
    auto list=std::make_shared<FunctionList>();
    const auto instance=o::Value(bindings.bind(list));
    const auto array=context.array();
    context.set(instance,o::Key(u"items"),o::Value(array));
    const auto callback=context.function(u"foreign",[](o::Context&,o::Value,std::span<const o::Value>){return o::Value(true);});
    const auto add=context.get(instance,o::Key(u"add"));
    (void)context.callMember(add,instance,std::array<o::Value,2>{o::Value(callback),o::Value(2)});
    expect(context.get(instance,o::Key(u"items"))==o::Value(array));
    expect(context.toNumber(context.get(o::Value(array),o::Key(u"length")))==1);
    const auto remove=context.get(instance,o::Key(u"remove"));
    (void)context.callMember(remove,instance,std::array<o::Value,1>{o::Value(callback)});
    expect(context.toNumber(context.get(o::Value(array),o::Key(u"length")))==0);
    const FunctionValue nativeItem{Property("native-item")};
    list->add(nativeItem,3);
    const auto nativeRecord=context.get(o::Value(array),o::Key(u"0"));
    expect(context.get(nativeRecord,o::Key(u"item"))==o::Value(u"native-item"));
    list->remove(nativeItem);
    expect(context.toNumber(context.get(o::Value(array),o::Key(u"length")))==0);
    context.set(o::Value(array),o::Key(u"length"),o::Value(1));
    bool threw=false;
    try{(void)list->execute();}catch(const o::Exception& error){threw=error.name=="TypeError";}
    expect(threw);
  };

  "prioritized_iterator_reads_slot_then_live_raw_length"_test=[] {
    o::Context context;OptionFunctionBindings functions(context);FunctionListContextBindings bindings(context,functions);
    auto list=std::make_shared<FunctionList>();const o::Value instance(bindings.bind(list));
    const auto items=context.object();context.define(items,o::Key(u"length"),o::Descriptor::data(o::Value(1)));
    std::vector<std::string> trace;const auto callback=context.function(u"entry",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back("callback");return o::Value(true);});
    const auto record=context.object();context.set(o::Value(record),o::Key(u"item"),o::Value(callback));context.set(o::Value(record),o::Key(u"priority"),o::Value(1));
    const auto getter=context.function(u"slot",[&](o::Context& cx,o::Value,std::span<const o::Value>){trace.push_back("slot");cx.set(o::Value(items),o::Key(u"length"),o::Value(0));return o::Value(record);});
    context.define(items,o::Key(u"0"),o::Descriptor::access(getter,{}));context.set(instance,o::Key(u"items"),o::Value(items));
    auto iterator=list->begin();const auto step=iterator.next();
    expect(step.done);expect(trace==std::vector<std::string>{"slot"});
    expect(std::get<o::Value>(step.value.recordIdentity())==o::Value(record));
    trace.clear();expect(list->execute());expect(trace==std::vector<std::string>{"slot"});
  };

  "prioritized_context_owner_distinguishes_holes_from_own_undefined"_test=[] {
    o::Context context;OptionFunctionBindings functions(context);FunctionListContextBindings bindings(context,functions);
    auto list=std::make_shared<FunctionList>();const o::Value instance(bindings.bind(list));const auto array=context.array(2);
    context.set(o::Value(array),o::Key(u"1"),o::Value(o::Undefined{}));context.set(instance,o::Key(u"items"),o::Value(array));
    const auto access=list->itemsAccess();expect(!access->read(0).present());expect(access->read(1).present());
  };

  "prioritized_iterator_uses_raw_fractional_and_nan_lengths"_test=[] {
    o::Context context;OptionFunctionBindings functions(context);FunctionListContextBindings bindings(context,functions);
    auto list=std::make_shared<FunctionList>();const o::Value instance(bindings.bind(list));std::vector<std::string> trace;
    const auto makeItems=[&](double length){const auto items=context.object();context.define(items,o::Key(u"length"),o::Descriptor::data(o::Value(length)));const auto getter=context.function(u"slot",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back("slot");return o::Value(o::Undefined{});});context.define(items,o::Key(u"0"),o::Descriptor::access(getter,{}));context.set(instance,o::Key(u"items"),o::Value(items));return items;};
    makeItems(0.5);auto fractional=list->begin().next();expect(fractional.done);expect(trace==std::vector<std::string>{"slot"});
    trace.clear();makeItems(std::numeric_limits<double>::quiet_NaN());auto nan=list->begin().next();expect(!nan.done);expect(trace==std::vector<std::string>{"slot"});
  };

  "prioritized_iterator_advances_index_before_reentrant_getter"_test=[] {
    o::Context context;OptionFunctionBindings functions(context);FunctionListContextBindings bindings(context,functions);
    auto list=std::make_shared<FunctionList>();const o::Value instance(bindings.bind(list));const auto items=context.object();
    context.define(items,o::Key(u"length"),o::Descriptor::data(o::Value(2)));
    const auto record0=context.object(),record1=context.object();std::vector<std::string> trace;FunctionList::iterator iterator;std::optional<FunctionList::iterator::Step> nested;
    const auto getter1=context.function(u"slot1",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back("slot1");return o::Value(record1);});
    const auto getter0=context.function(u"slot0",[&](o::Context&,o::Value,std::span<const o::Value>){trace.push_back("slot0");nested=iterator.next();return o::Value(record0);});
    context.define(items,o::Key(u"0"),o::Descriptor::access(getter0,{}));context.define(items,o::Key(u"1"),o::Descriptor::access(getter1,{}));context.set(instance,o::Key(u"items"),o::Value(items));
    iterator=list->begin();const auto outer=iterator.next();
    expect(!outer.done);expect(std::get<o::Value>(outer.value.recordIdentity())==o::Value(record0));
    expect(nested.has_value()&&!nested->done);expect(std::get<o::Value>(nested->value.recordIdentity())==o::Value(record1));
    expect(trace==std::vector<std::string>{"slot0","slot1"});
  };

  "function_list_source_async_execute_returns_context_promise"_test=[] {
    o::Context context;OptionFunctionBindings functions(context);auto lease=std::make_shared<std::atomic_bool>(true);auto queue=std::make_shared<async::JobQueue>();FunctionListContextBindings bindings(context,functions,lease,queue);
    auto list=std::make_shared<FunctionList>();const auto instance=o::Value(bindings.bind(list));
    const auto old=context.array();context.set(instance,o::Key(u"items"),o::Value(old));
    auto input=async::Promise::create(queue);
    const auto promiseObject=context.object();context.payload(promiseObject)=input;
    std::vector<std::string> calls;std::vector<o::Value> outcomes;
    const auto first=context.function(u"first",[&](o::Context&,o::Value,std::span<const o::Value>){calls.push_back("first");return o::Value(promiseObject);});
    const auto second=context.function(u"second",[&](o::Context&,o::Value,std::span<const o::Value>){calls.push_back("second");return o::Value(false);});
    auto record=[&](o::ObjectRef callback,double priority){const auto r=context.object();context.set(o::Value(r),o::Key(u"item"),o::Value(callback));context.set(o::Value(r),o::Key(u"priority"),o::Value(priority));return r;};
    context.set(o::Value(old),o::Key(u"0"),o::Value(record(first,0)));context.set(o::Value(old),o::Key(u"1"),o::Value(record(second,1)));
    const auto run=context.get(instance,o::Key(u"asyncExecute"));
    const auto wrapped=context.callMember(run,instance,std::array<o::Value,1>{o::Value(7)});
    const auto wrapper=std::get<o::ObjectRef>(wrapped.data);
    const auto output=std::any_cast<async::PromiseRef>(context.payload(wrapper));
    const auto then=context.get(wrapped,o::Key(u"then"));
    const auto observer=context.function(u"observer",[&](o::Context&,o::Value,std::span<const o::Value> args){outcomes.assign(args.begin(),args.end());return o::Value(o::Undefined{});});
    (void)context.callMember(then,wrapped,std::array<o::Value,1>{o::Value(observer)});
    context.set(instance,o::Key(u"items"),o::Value(context.array()));
    input->resolve(std::any(true));
    while(queue->runOne()){}
    expect(calls==std::vector<std::string>{"first","second"});
    expect(output->state()==async::State::fulfilled);
    expect(outcomes==std::vector<o::Value>{o::Value(false)});
  };

  "generic_context_promise_preserves_chain_rejection_and_identity"_test=[] {
    o::Context context;auto queue=std::make_shared<async::JobQueue>();auto lease=std::make_shared<std::atomic_bool>(true);
    auto bindings=std::make_shared<PromiseContextBindings>(context,queue,lease,
      [](const async::Value& value)->o::Value{if(const auto* thrown=std::any_cast<async::ThrownValue>(&value)){if(const auto* raw=std::any_cast<o::Value>(&thrown->value))return *raw;}if(const auto* raw=std::any_cast<o::Value>(&value))return *raw;return o::Value(o::Undefined{});},
      [](o::Value value)->async::Value{return async::Value(std::move(value));});
    bindings->initialize();
    auto rejected=async::Promise::create(queue);rejected->reject(async::ThrownValue{async::Value(o::Value(u"reason"))});
    const auto wrapped=o::Value(bindings->wrap(rejected));expect(bindings->wrap(rejected)==std::get<o::ObjectRef>(wrapped.data));
    std::vector<o::Value> reasons;
    const auto fulfilled=context.function(u"fulfilled",[](o::Context&,o::Value receiver,std::span<const o::Value>){expect(std::holds_alternative<o::Undefined>(receiver.data));return o::Value(u"wrong");});
    const auto onRejected=context.function(u"rejected",[&](o::Context&,o::Value receiver,std::span<const o::Value> args){expect(std::holds_alternative<o::Undefined>(receiver.data));reasons.assign(args.begin(),args.end());return o::Value(u"recovered");});
    const auto then=context.get(wrapped,o::Key(u"then"));
    const auto chained=context.callMember(then,wrapped,std::array<o::Value,2>{o::Value(fulfilled),o::Value(onRejected)});
    expect(chained!=wrapped);
    const auto chainedPromise=std::any_cast<async::PromiseRef>(context.payload(std::get<o::ObjectRef>(chained.data)));
    expect(chainedPromise->belongsTo(queue));
    const auto catchMethod=context.get(chained,o::Key(u"catch"));
    const auto recovered=context.callMember(catchMethod,chained,std::array<o::Value,1>{o::Value(onRejected)});
    const auto recoveredPromise=std::any_cast<async::PromiseRef>(context.payload(std::get<o::ObjectRef>(recovered.data)));
    expect(recoveredPromise!=chainedPromise);expect(recoveredPromise->belongsTo(queue));
    const auto passThrough=context.callMember(then,wrapped,std::array<o::Value,1>{o::Value(o::Undefined{})});
    const auto passPromise=std::any_cast<async::PromiseRef>(context.payload(std::get<o::ObjectRef>(passThrough.data)));
    const auto ignoredNonCallable=context.callMember(then,wrapped,std::array<o::Value,1>{o::Value(17)});
    const auto ignoredPromise=std::any_cast<async::PromiseRef>(context.payload(std::get<o::ObjectRef>(ignoredNonCallable.data)));
    const auto noCatch=context.callMember(catchMethod,wrapped,std::span<const o::Value>{});
    const auto noCatchPromise=std::any_cast<async::PromiseRef>(context.payload(std::get<o::ObjectRef>(noCatch.data)));
    const auto ignoredCatch=context.callMember(catchMethod,wrapped,std::array<o::Value,1>{o::Value(17)});
    const auto ignoredCatchPromise=std::any_cast<async::PromiseRef>(context.payload(std::get<o::ObjectRef>(ignoredCatch.data)));
    while(queue->runOne()){}
    expect(reasons==std::vector<o::Value>{o::Value(u"reason")});
    expect(chainedPromise->state()==async::State::fulfilled);
    expect(std::any_cast<o::Value>(chainedPromise->value())==o::Value(u"recovered"));
    for(const auto& propagated:{passPromise,ignoredPromise,noCatchPromise,ignoredCatchPromise}){
      expect(propagated->state()==async::State::rejected);
      const auto reason=std::any_cast<async::ThrownValue>(propagated->value());
      const auto* raw=std::any_cast<o::Value>(&reason.value);
      expect(raw&&*raw==o::Value(u"reason"));
    }
  };

  "function_list_async_rejection_reaches_output_with_missing_callbacks"_test=[] {
    o::Context context;OptionFunctionBindings functions(context);
    auto queue=std::make_shared<async::JobQueue>();auto lease=std::make_shared<std::atomic_bool>(true);
    FunctionListContextBindings bindings(context,functions,lease,queue);
    auto list=std::make_shared<FunctionList>();const o::Value instance(bindings.bind(list));
    const auto items=context.array();context.set(instance,o::Key(u"items"),o::Value(items));
    auto input=async::Promise::create(queue);const auto inputObject=context.object();context.setPayload(inputObject,input);
    const auto callback=context.function(u"rejecting",[inputObject](o::Context&,o::Value,std::span<const o::Value>){return o::Value(inputObject);});
    const auto record=context.object();context.set(o::Value(record),o::Key(u"item"),o::Value(callback));context.set(o::Value(record),o::Key(u"priority"),o::Value(0));
    context.set(o::Value(items),o::Key(u"0"),o::Value(record));
    const auto method=context.get(instance,o::Key(u"asyncExecute"));
    const auto output=context.callMember(method,instance,std::span<const o::Value>{});
    const auto outputPromise=std::any_cast<async::PromiseRef>(context.payload(std::get<o::ObjectRef>(output.data)));
    const o::Value reason(u"original rejection");input->reject(async::ThrownValue{async::Value(reason)});
    while(queue->runOne()){}
    expect(outputPromise->state()==async::State::rejected);
    const auto outputReason=std::any_cast<async::ThrownValue>(outputPromise->value());
    expect(std::any_cast<o::Value>(outputReason.value)==reason);
    const auto then=context.get(output,o::Key(u"then"));
    const auto pass=context.callMember(then,output,std::array<o::Value,1>{o::Value(o::Undefined{})});
    const auto passPromise=std::any_cast<async::PromiseRef>(context.payload(std::get<o::ObjectRef>(pass.data)));
    while(queue->runOne()){}
    expect(passPromise->state()==async::State::rejected);
    const auto passReason=std::any_cast<async::ThrownValue>(passPromise->value());
    expect(std::any_cast<o::Value>(passReason.value)==reason);
    const auto catchMethod=context.get(output,o::Key(u"catch"));
    const auto ignored=context.callMember(catchMethod,output,std::array<o::Value,1>{o::Value(17)});
    const auto ignoredPromise=std::any_cast<async::PromiseRef>(context.payload(std::get<o::ObjectRef>(ignored.data)));
    while(queue->runOne()){}
    expect(ignoredPromise->state()==async::State::rejected);
    const auto ignoredReason=std::any_cast<async::ThrownValue>(ignoredPromise->value());
    expect(std::any_cast<o::Value>(ignoredReason.value)==reason);
  };

  "function_list_async_keeps_captured_context_array_owner"_test=[] {
    o::Context context;
    OptionFunctionBindings functions(context);
    FunctionListContextBindings bindings(context,functions);
    auto list=std::make_shared<FunctionList>();
    const auto instance=o::Value(bindings.bind(list));
    const auto old=context.array();context.set(instance,o::Key(u"items"),o::Value(old));
    auto queue=std::make_shared<async::JobQueue>();
    auto first=async::Promise::create(queue);
    std::vector<std::string> calls;
    const auto promiseObject=context.object();context.payload(promiseObject)=first;
    const auto objectArgument=context.object();const auto symbolArgument=context.symbol(u"arg");
    const auto callbackA=context.function(u"a",[&](o::Context&,o::Value,std::span<const o::Value> args){
      calls.push_back("a");
      expect(args.size()==5);
      expect(args[0]==o::Value(u"property"));expect(args[1]==o::Value(u"string"));
      expect(args[2]==o::Value(objectArgument));expect(args[3]==o::Value(symbolArgument));expect(args[4]==o::Value(12));
      return o::Value(promiseObject);
    });
    const auto callbackB=context.function(u"b",[&](o::Context&,o::Value,std::span<const o::Value>){calls.push_back("b");return o::Value(false);});
    auto makeRecord=[&](o::ObjectRef callback,double priority){const auto record=context.object();context.set(o::Value(record),o::Key(u"item"),o::Value(callback));context.set(o::Value(record),o::Key(u"priority"),o::Value(priority));return record;};
    const auto recA=makeRecord(callbackA,0),recB=makeRecord(callbackB,1);
    context.set(o::Value(old),o::Key(u"0"),o::Value(recA));context.set(o::Value(old),o::Key(u"1"),o::Value(recB));
    const std::array<std::any,5> args{Property("property"),std::string("string"),objectArgument,symbolArgument,FunctionValue{Property(12)}};
    const auto result=list->asyncExecute(queue,args);
    context.set(instance,o::Key(u"items"),o::Value(context.array()));
    first->resolve(std::any(true));
    expect(queue->runOne());
    expect(result->state()==async::State::fulfilled);
    expect(std::any_cast<bool>(result->value())==false);
    expect(calls==std::vector<std::string>{"a","b"});
  };
};
}
