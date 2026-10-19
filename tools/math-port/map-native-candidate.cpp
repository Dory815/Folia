// Unregistered focused candidate for map-original-oracle.cjs. Root owns the
// Foundation target and final comparison; this file uses the actual Context
// Map constructor/prototype and dynamic member calls.
#include "../../tests/support/folia_test.hpp"
import folia.math.util.map_bindings;

namespace {
using namespace boost::ut;
using namespace folia::math::options;

Value member(Context& context,Value receiver,const char16_t* name,
             std::span<const Value> args={}) {
  const Value function=context.get(receiver,Key(name));
  const auto* object=std::get_if<ObjectRef>(&function.data);
  if(!object)throw Exception("TypeError",u"Map method is not callable in native candidate");
  return context.call(*object,receiver,args);
}

std::u16string text(Value value) {
  return std::get<std::u16string>(std::move(value.data));
}

struct MapCandidateTests {
  MapCandidateTests() {
    "map_context_same_value_zero_order_and_identity"_test = [] {
      Context context;
      MapContextBindings maps(context);
      const auto map=maps.create();
      const Value nan(std::numeric_limits<double>::quiet_NaN());
      const ObjectRef objectKey=context.object();
      const std::array<Value,2> n1{nan,Value(u"first")};
      const std::array<Value,2> n2{nan,Value(u"updated")};
      const std::array<Value,2> z1{Value(-0.0),Value(u"negative")};
      const std::array<Value,2> z2{Value(+0.0),Value(u"positive")};
      (void)member(context,Value(map),u"set",n1);
      (void)member(context,Value(map),u"set",n2);
      (void)member(context,Value(map),u"set",z1);
      (void)member(context,Value(map),u"set",z2);
      const std::array<Value,2> objectEntry{Value(objectKey),Value(u"object")};
      (void)member(context,Value(map),u"set",objectEntry);
      const auto size=context.get(Value(map),Key(u"size"));
      expect(size==Value(3.0));
      const std::array<Value,1> nanArg{nan};
      expect(member(context,Value(map),u"get",nanArg)==Value(u"updated"));
      const std::array<Value,1> zeroArg{Value(0.0)};
      expect(member(context,Value(map),u"get",zeroArg)==Value(u"positive"));
      const std::array<Value,1> objectArg{Value(objectKey)};
      expect(member(context,Value(map),u"get",objectArg)==Value(u"object"));
      const std::array<Value,1> otherObject{Value(context.object())};
      expect(member(context,Value(map),u"has",otherObject)==Value(false));
      const Value iterator=member(context,Value(map),u"entries");
      const Value first=context.iteratorNext(iterator);
      const auto firstEntry=context.get(first,Key(u"value"));
      const ObjectRef firstPair=std::get<ObjectRef>(firstEntry.data);
      expect(std::isnan(std::get<double>(context.get(Value(firstPair),Key(u"0")).data)));
      const Value second=context.iteratorNext(iterator);
      const ObjectRef secondPair=std::get<ObjectRef>(context.get(second,Key(u"value")).data);
      expect(std::get<double>(context.get(Value(secondPair),Key(u"0")).data)==0.0);
      expect(!std::signbit(std::get<double>(context.get(Value(secondPair),Key(u"0")).data)));
    };

    "map_iterator_observes_delete_append_and_clear"_test = [] {
      Context context;
      MapContextBindings maps(context);
      const auto map=maps.create();
      const std::array<Value,2> setA{Value(u"a"),Value(1)};
      const std::array<Value,2> setB{Value(u"b"),Value(2)};
      (void)member(context,Value(map),u"set",setA);
      (void)member(context,Value(map),u"set",setB);
      const Value iterator=member(context,Value(map),u"entries");
      const Value first=context.iteratorNext(iterator);
      const ObjectRef firstPair=std::get<ObjectRef>(context.get(first,Key(u"value")).data);
      expect(context.get(Value(firstPair),Key(u"0"))==Value(u"a"));
      const std::array<Value,1> deleteB{Value(u"b")};
      (void)member(context,Value(map),u"delete",deleteB);
      const std::array<Value,2> setC{Value(u"c"),Value(3)};
      (void)member(context,Value(map),u"set",setC);
      const Value second=context.iteratorNext(iterator);
      const ObjectRef secondPair=std::get<ObjectRef>(context.get(second,Key(u"value")).data);
      expect(context.get(Value(secondPair),Key(u"0"))==Value(u"c"));
      const Value exhausted=context.iteratorNext(iterator);
      expect(std::get<bool>(context.get(exhausted,Key(u"done")).data));
      const std::array<Value,2> setD{Value(u"d"),Value(4)};
      (void)member(context,Value(map),u"set",setD);
      const Value afterDone=context.iteratorNext(iterator);
      expect(std::get<bool>(context.get(afterDone,Key(u"done")).data));
      (void)member(context,Value(map),u"clear");
      expect(context.get(Value(map),Key(u"size"))==Value(0.0));

      const auto liveMap=maps.create();
      (void)member(context,Value(liveMap),u"set",setA);
      (void)member(context,Value(liveMap),u"set",setB);
      const Value liveIterator=member(context,Value(liveMap),u"entries");
      const Value beforeClear=context.iteratorNext(liveIterator);
      expect(!std::get<bool>(context.get(beforeClear,Key(u"done")).data));
      (void)member(context,Value(liveMap),u"clear");
      const std::array<Value,2> setAfterClear{Value(u"c"),Value(3)};
      (void)member(context,Value(liveMap),u"set",setAfterClear);
      const Value afterClear=context.iteratorNext(liveIterator);
      const ObjectRef afterClearPair=std::get<ObjectRef>(context.get(afterClear,Key(u"value")).data);
      expect(context.get(Value(afterClearPair),Key(u"0"))==Value(u"c"));

      const auto appendMap=maps.create();
      (void)member(context,Value(appendMap),u"set",setA);
      (void)member(context,Value(appendMap),u"set",setB);
      const Value appendIterator=member(context,Value(appendMap),u"entries");
      (void)context.iteratorNext(appendIterator);
      (void)member(context,Value(appendMap),u"delete",deleteB);
      (void)member(context,Value(appendMap),u"set",setC);
      (void)context.iteratorNext(appendIterator);
      (void)member(context,Value(appendMap),u"set",setD);
      const Value appended=context.iteratorNext(appendIterator);
      const ObjectRef appendedPair=std::get<ObjectRef>(context.get(appended,Key(u"value")).data);
      expect(context.get(Value(appendedPair),Key(u"0"))==Value(u"d"));
      expect(std::get<bool>(context.get(context.iteratorNext(appendIterator),Key(u"done")).data));
    };

    "map_for_each_observes_live_mutations_and_receiver"_test = [] {
      Context context;
      MapContextBindings maps(context);
      const auto map=maps.create();
      const std::array<Value,2> first{Value(u"a"),Value(1)};
      const std::array<Value,2> second{Value(u"b"),Value(2)};
      (void)member(context,Value(map),u"set",first);
      (void)member(context,Value(map),u"set",second);
      std::vector<std::u16string> seen;
      const auto callback=context.function(u"callback",[&](Context& cx,Value receiver,std::span<const Value> args){
        expect(receiver==Value(u"receiver"));
        expect(args.size()==3);
        seen.push_back(text(args[1]));
        expect(args[2]==Value(map));
        if(text(args[1])==u"a"){
          const std::array<Value,1> del{Value(u"b")};(void)member(cx,Value(map),u"delete",del);
          const std::array<Value,2> add{Value(u"c"),Value(3)};(void)member(cx,Value(map),u"set",add);
        }
        return Value(Undefined{});
      });
      const std::array<Value,2> args{Value(callback),Value(u"receiver")};
      (void)member(context,Value(map),u"forEach",args);
      expect(seen.size()==2);
      expect(seen[0]==u"a");
      expect(seen[1]==u"c");
    };

    "map_iterable_constructor_and_prototype_descriptors"_test = [] {
      Context context;
      MapContextBindings maps(context);
      const ObjectRef entries=context.array(1);
      const ObjectRef pair=context.array(2);
      context.set(Value(pair),Key(u"0"),Value(u"x"));
      context.set(Value(pair),Key(u"1"),Value(7));
      context.set(Value(entries),Key(u"0"),Value(pair));
      const std::array<Value,1> ctorArgs{Value(entries)};
      const auto map=context.construct(maps.constructor(),ctorArgs);
      const std::array<Value,1> x{Value(u"x")};
      expect(member(context,Value(map),u"get",x)==Value(7));
      const auto proto=maps.prototype();
      const auto constructorPrototype=context.descriptor(Value(maps.constructor()),Key(u"prototype"));
      expect(constructorPrototype.has_value()&&!constructorPrototype->writable&&
             !constructorPrototype->enumerable&&!constructorPrototype->configurable);
      const auto descriptor=context.descriptor(Value(proto),Key(u"set"));
      expect(descriptor.has_value());
      expect(!descriptor->enumerable);
      const auto size=context.descriptor(Value(proto),Key(u"size"));
      expect(size.has_value()&&size->accessor&&!size->setter.has_value());
      expect(context.get(Value(proto),Key(context.intrinsics().symbolIterator))==context.get(Value(proto),Key(u"entries")));
      const auto storage=maps.createStorage();
      expect(maps.bind(storage)==maps.bind(storage));
      Context otherContext;
      MapContextBindings otherMaps(otherContext);
      expect(maps.constructor()!=otherMaps.constructor());
    };

    "map_method_returns_and_key_value_iterators"_test = [] {
      Context context;
      MapContextBindings maps(context);
      const auto map=maps.create();
      const std::array<Value,2> a{Value(u"a"),Value(1)};
      const std::array<Value,2> b{Value(u"b"),Value(2)};
      expect(member(context,Value(map),u"set",a)==Value(map));
      (void)member(context,Value(map),u"set",b);
      const std::array<Value,1> aKey{Value(u"a")};
      expect(member(context,Value(map),u"has",aKey)==Value(true));
      const Value keys=member(context,Value(map),u"keys");
      const Value values=member(context,Value(map),u"values");
      const auto keyIterator=context.getIterator(keys);
      const auto valueIterator=context.getIterator(values);
      expect(context.get(context.iteratorNext(keyIterator),Key(u"value"))==Value(u"a"));
      expect(context.get(context.iteratorNext(keyIterator),Key(u"value"))==Value(u"b"));
      expect(context.get(context.iteratorNext(valueIterator),Key(u"value"))==Value(1));
      expect(context.get(context.iteratorNext(valueIterator),Key(u"value"))==Value(2));
      expect(member(context,Value(map),u"delete",aKey)==Value(true));
      expect(member(context,Value(map),u"delete",aKey)==Value(false));
      expect(std::holds_alternative<Undefined>(member(context,Value(map),u"clear").data));
      expect(context.get(Value(map),Key(u"size"))==Value(0.0));
    };

    "map_constructor_captures_adder_before_live_iterator_lookup"_test = [] {
      Context context;
      MapContextBindings maps(context);
      const ObjectRef proto=maps.prototype();
      const Value builtinSet=context.get(Value(proto),Key(u"set"));
      std::vector<std::u16string> trace;
      const auto capturedSet=context.function(u"set",[&](Context& cx,Value receiver,std::span<const Value> args){
        trace.push_back(u"captured-set");
        const auto* target=std::get_if<ObjectRef>(&builtinSet.data);
        if(!target)throw Exception("TypeError",u"Map.prototype.set is not callable");
        return cx.call(*target,receiver,args);
      });
      const auto replacementSet=context.function(u"set",[&](Context&,Value,std::span<const Value>){
        trace.push_back(u"replacement-set");
        return Value(Undefined{});
      });
      context.set(Value(proto),Key(u"set"),Value(capturedSet));

      const ObjectRef entry=context.object();
      context.set(Value(entry),Key(u"0"),Value(u"key"));
      context.set(Value(entry),Key(u"1"),Value(17));
      const auto customEntryIterator=context.function(u"entry iterator",[&](Context&,Value,std::span<const Value>){
        trace.push_back(u"entry-iterator-should-not-run");
        return Value(context.object());
      });
      context.set(Value(entry),Key(context.intrinsics().symbolIterator),Value(customEntryIterator));

      const ObjectRef outerIterator=context.object();
      bool yielded=false;
      const auto next=context.function(u"next",[&](Context& cx,Value,std::span<const Value>){
        const ObjectRef result=cx.object();
        cx.define(result,Key(u"done"),Descriptor::data(Value(yielded)));
        if(!yielded){yielded=true;cx.define(result,Key(u"value"),Descriptor::data(Value(entry)));}
        return Value(result);
      });
      context.set(Value(outerIterator),Key(u"next"),Value(next));
      const auto iteratorMethod=context.function(u"iterator",[&](Context&,Value,std::span<const Value>){
        trace.push_back(u"call-iterator");return Value(outerIterator);
      });
      const ObjectRef iterable=context.object();
      const auto iteratorGetter=context.function(u"get iterator",[&](Context& cx,Value,std::span<const Value>){
        trace.push_back(u"get-iterator");
        cx.set(Value(proto),Key(u"set"),Value(replacementSet));
        return Value(iteratorMethod);
      });
      context.define(iterable,Key(context.intrinsics().symbolIterator),Descriptor::access(iteratorGetter,{},true,true));
      const std::array<Value,1> args{Value(iterable)};
      const ObjectRef map=context.construct(maps.constructor(),args);
      expect(trace.size()==3);
      if(trace.size()==3){
        expect(trace[0]==u"get-iterator");
        expect(trace[1]==u"call-iterator");
        expect(trace[2]==u"captured-set");
      }
      const Value get=context.get(Value(map),Key(u"get"));
      const auto* getObject=std::get_if<ObjectRef>(&get.data);
      expect(getObject!=nullptr);
      if(getObject){const std::array<Value,1> key{Value(u"key")};expect(context.call(*getObject,Value(map),key)==Value(17));}
    };

    "map_constructor_closes_outer_iterator_for_invalid_entry"_test = [] {
      Context context;
      MapContextBindings maps(context);
      std::vector<std::u16string> trace;
      const ObjectRef iterator=context.object();
      bool yielded=false;
      const auto next=context.function(u"next",[&](Context& cx,Value,std::span<const Value>){
        trace.push_back(u"next");
        const ObjectRef result=cx.object();
        cx.define(result,Key(u"done"),Descriptor::data(Value(yielded)));
        if(!yielded){yielded=true;cx.define(result,Key(u"value"),Descriptor::data(Value(1)));}
        return Value(result);
      });
      const auto close=context.function(u"return",[&](Context& cx,Value,std::span<const Value>){
        trace.push_back(u"return");return Value(cx.object());
      });
      const auto iteratorMethod=context.function(u"iterator",[&](Context&,Value,std::span<const Value>){
        trace.push_back(u"iterator");return Value(iterator);
      });
      context.set(Value(iterator),Key(u"next"),Value(next));
      context.set(Value(iterator),Key(u"return"),Value(close));
      const ObjectRef iterable=context.object();
      context.set(Value(iterable),Key(context.intrinsics().symbolIterator),Value(iteratorMethod));
      bool threw=false;
      try{const std::array<Value,1> args{Value(iterable)};(void)context.construct(maps.constructor(),args);}
      catch(const Exception&){threw=true;}
      expect(threw);
      expect(trace.size()==3);
      if(trace.size()==3){expect(trace[0]==u"iterator");expect(trace[1]==u"next");expect(trace[2]==u"return");}
    };

    "map_constructor_does_not_close_on_iterator_step_or_value_abrupt"_test = [] {
      for(const auto stage:{std::u16string(u"next"),std::u16string(u"done"),std::u16string(u"value")}){
        Context context;
        MapContextBindings maps(context);
        std::vector<std::u16string> trace;
        const ObjectRef iterator=context.object();
        const auto next=context.function(u"next",[&](Context& cx,Value,std::span<const Value>){
          trace.push_back(u"next");
          if(stage==u"next")throw Exception("TypeError",u"next failed");
          const ObjectRef result=cx.object();
          const auto doneGetter=cx.function(u"get done",[&](Context&,Value,std::span<const Value>){
            trace.push_back(u"done");
            if(stage==u"done")throw Exception("TypeError",u"done failed");
            return Value(false);
          });
          const auto valueGetter=cx.function(u"get value",[&](Context&,Value,std::span<const Value>){
            trace.push_back(u"value");
            if(stage==u"value")throw Exception("TypeError",u"value failed");
            return Value(cx.array(0));
          });
          cx.define(result,Key(u"done"),Descriptor::access(doneGetter,{},true,true));
          cx.define(result,Key(u"value"),Descriptor::access(valueGetter,{},true,true));
          return Value(result);
        });
        const auto close=context.function(u"return",[&](Context& cx,Value,std::span<const Value>){
          trace.push_back(u"return");return Value(cx.object());
        });
        context.set(Value(iterator),Key(u"next"),Value(next));
        context.set(Value(iterator),Key(u"return"),Value(close));
        const auto iteratorMethod=context.function(u"iterator",[&](Context&,Value,std::span<const Value>){
          trace.push_back(u"iterator");return Value(iterator);
        });
        const ObjectRef iterable=context.object();
        context.set(Value(iterable),Key(context.intrinsics().symbolIterator),Value(iteratorMethod));
        bool expectedError=false;
        try{
          const std::array<Value,1> args{Value(iterable)};
          (void)context.construct(maps.constructor(),args);
        }catch(const Exception& error){
          expectedError=error.name=="TypeError"&&error.message==stage+u" failed";
        }
        expect(expectedError);
        const std::vector<std::u16string> expected=stage==u"next"
          ?std::vector<std::u16string>{u"iterator",u"next"}
          :stage==u"done"?std::vector<std::u16string>{u"iterator",u"next",u"done"}
                         :std::vector<std::u16string>{u"iterator",u"next",u"done",u"value"};
        expect(trace==expected);
      }
    };

    "map_constructor_closes_on_entry_property_or_adder_abrupt"_test = [] {
      for(const auto stage:{std::u16string(u"key"),std::u16string(u"adder")}){
        Context context;
        MapContextBindings maps(context);
        std::vector<std::u16string> trace;
        const ObjectRef iterator=context.object();
        const ObjectRef entry=context.object();
        if(stage==u"key"){
          const auto keyGetter=context.function(u"get 0",[&](Context&,Value,std::span<const Value>)->Value{
            trace.push_back(u"key");throw Exception("TypeError",u"key failed");
          });
          context.define(entry,Key(u"0"),Descriptor::access(keyGetter,{},true,true));
        }else context.set(Value(entry),Key(u"0"),Value(u"key"));
        context.set(Value(entry),Key(u"1"),Value(1));
        const auto next=context.function(u"next",[&](Context& cx,Value,std::span<const Value>){
          trace.push_back(u"next");const ObjectRef result=cx.object();
          cx.set(Value(result),Key(u"done"),Value(false));cx.set(Value(result),Key(u"value"),Value(entry));return Value(result);
        });
        const auto close=context.function(u"return",[&](Context& cx,Value,std::span<const Value>){
          trace.push_back(u"return");return Value(cx.object());
        });
        context.set(Value(iterator),Key(u"next"),Value(next));
        context.set(Value(iterator),Key(u"return"),Value(close));
        const auto iteratorMethod=context.function(u"iterator",[&](Context&,Value,std::span<const Value>){
          trace.push_back(u"iterator");return Value(iterator);
        });
        const ObjectRef iterable=context.object();
        context.set(Value(iterable),Key(context.intrinsics().symbolIterator),Value(iteratorMethod));
        if(stage==u"adder"){
          const auto failingSet=context.function(u"set",[&](Context&,Value,std::span<const Value>)->Value{
            trace.push_back(u"adder");throw Exception("TypeError",u"adder failed");
          });
          context.set(Value(maps.prototype()),Key(u"set"),Value(failingSet));
        }
        bool expectedError=false;
        try{const std::array<Value,1> args{Value(iterable)};(void)context.construct(maps.constructor(),args);}
        catch(const Exception& error){expectedError=error.name=="TypeError"&&error.message==stage+u" failed";}
        expect(expectedError);
        const std::vector<std::u16string> expected=stage==u"key"
          ?std::vector<std::u16string>{u"iterator",u"next",u"key",u"return"}
          :std::vector<std::u16string>{u"iterator",u"next",u"adder",u"return"};
        expect(trace==expected);
      }
    };

    "map_context_methods_retain_the_actual_binding_owner_until_context_teardown"_test = [] {
      Context context;
      ObjectRef map;
      {
        MapContextBindings maps(context);
        map=maps.create();
        const std::array<Value,2> pair{Value(u"key"),Value(23)};
        (void)member(context,Value(map),u"set",pair);
      }
      const std::array<Value,1> key{Value(u"key")};
      expect(member(context,Value(map),u"get",key)==Value(23));
      const Value iterator=member(context,Value(map),u"entries");
      const Value next=context.iteratorNext(iterator);
      const auto pairValue=context.get(next,Key(u"value"));
      const auto* pairObject=std::get_if<ObjectRef>(&pairValue.data);
      expect(pairObject!=nullptr);
      if(pairObject){
        expect(context.get(Value(*pairObject),Key(u"0"))==Value(u"key"));
        expect(context.get(Value(*pairObject),Key(u"1"))==Value(23));
      }
    };
  }
} map_candidate_tests;
}
