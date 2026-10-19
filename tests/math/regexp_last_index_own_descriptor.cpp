#include "support/folia_test.hpp"
import folia.math.util.regexp_binding;

namespace {
using namespace boost::ut;
namespace host=folia::math::regexp_host;
namespace opts=folia::math::options;

suite regexp_last_index_own_descriptor=[] {
  "math_port_regexp_host_numeric_last_index_assignment_keeps_double_cursor_contract"_test=[] {
    host::RegExp pattern(u"x",u"g");
    const std::int64_t signedCursor=std::numeric_limits<std::int64_t>::max();
    const std::uint64_t unsignedCursor=std::numeric_limits<std::uint64_t>::max();

    pattern.lastIndex=signedCursor;
    expect(pattern.lastIndex.value==opts::Value(static_cast<double>(signedCursor)));
    pattern.lastIndex=unsignedCursor;
    expect(pattern.lastIndex.value==opts::Value(static_cast<double>(unsignedCursor)));

    const opts::Value rawString(u"1");
    pattern.lastIndex=rawString;
    expect(pattern.lastIndex.value==rawString);
  };

  "math_port_regexp_last_index_is_a_live_non_enumerable_own_data_property"_test=[] {
    opts::Context context;
    host::RegExpBindings bindings{context};
    auto pattern=std::make_shared<host::RegExp>(u"x",u"g");
    const auto object=bindings.bind(pattern);
    const auto alias=bindings.bind(pattern);
    const opts::Value receiver(object);
    expect(object==alias);

    const auto own=context.ownKeys(receiver);
    expect(own==std::vector<opts::Key>{opts::Key(u"lastIndex")});
    const auto descriptor=context.descriptor(receiver,opts::Key(u"lastIndex"));
    expect(descriptor.has_value());
    if(!descriptor)return;
    expect(!descriptor->accessor);
    expect(descriptor->writable&&!descriptor->enumerable&&!descriptor->configurable);
    expect(descriptor->value==opts::Value(0));
    expect(context.enumerableStringKeys(receiver).empty());

    context.set(receiver,opts::Key(u"lastIndex"),opts::Value(u"1"));
    const auto rawString=context.descriptor(receiver,opts::Key(u"lastIndex"));
    expect(rawString&&rawString->value==opts::Value(u"1"));
    expect(bindings.lastIndex(receiver)==1);
    const auto match=bindings.exec(receiver,u"xxx");
    expect(match.has_value()&&match->index==1);
    const auto advanced=context.descriptor(receiver,opts::Key(u"lastIndex"));
    expect(advanced&&advanced->value==opts::Value(2));

    context.set(receiver,opts::Key(u"lastIndex"),opts::Value(4));
    expect(!bindings.exec(alias,u"x"));
    const auto reset=context.descriptor(receiver,opts::Key(u"lastIndex"));
    expect(reset&&reset->value==opts::Value(0));
    expect(context.ownKeys(receiver)==own);
  };

  "math_port_regexp_last_index_readonly_writeback_matches_source_set_semantics"_test=[] {
    opts::Context context;
    host::RegExpBindings bindings{context};
    const opts::Key key(u"lastIndex");
    auto freeze=[&](opts::ObjectRef object,opts::Value value){
      context.define(object,key,opts::Descriptor::data(std::move(value),false,false,false));
    };
    auto expectTypeError=[&](auto&& operation){
      bool threw=false;
      try{operation();}catch(const opts::Exception&error){threw=error.name=="TypeError";}
      expect(threw);
    };
    auto expectRaw=[&](opts::ObjectRef object,opts::Value expected){
      const auto descriptor=context.descriptor(opts::Value(object),key);
      expect(descriptor.has_value());
      if(!descriptor)return;
      expect(!descriptor->writable&&!descriptor->enumerable&&!descriptor->configurable);
      expect(descriptor->value==expected);
    };

    const auto globalSuccess=bindings.create(u"x",u"g");
    freeze(globalSuccess,opts::Value(0));
    expectTypeError([&]{(void)bindings.exec(globalSuccess,u"x");});
    expectRaw(globalSuccess,opts::Value(0));
    expectTypeError([&]{bindings.setLastIndex(globalSuccess,2);});
    expectRaw(globalSuccess,opts::Value(0));

    const auto globalFailure=bindings.create(u"x",u"g");
    bindings.setLastIndex(globalFailure,1);
    freeze(globalFailure,opts::Value(1));
    expectTypeError([&]{(void)bindings.exec(globalFailure,u"a");});
    expectRaw(globalFailure,opts::Value(1));

    const auto globalEmpty=bindings.create(u"(?:)",u"g");
    freeze(globalEmpty,opts::Value(0));
    expectTypeError([&]{(void)bindings.exec(globalEmpty,u"");});
    expectRaw(globalEmpty,opts::Value(0));

    const auto stickySuccess=bindings.create(u"x",u"y");
    bindings.setLastIndex(stickySuccess,1);
    freeze(stickySuccess,opts::Value(1));
    expectTypeError([&]{(void)bindings.exec(stickySuccess,u"xx");});
    expectRaw(stickySuccess,opts::Value(1));

    const auto stickyFailure=bindings.create(u"x",u"y");
    bindings.setLastIndex(stickyFailure,1);
    freeze(stickyFailure,opts::Value(1));
    expectTypeError([&]{(void)bindings.exec(stickyFailure,u"a");});
    expectRaw(stickyFailure,opts::Value(1));

    const auto stickyEmpty=bindings.create(u"(?:)",u"y");
    freeze(stickyEmpty,opts::Value(0));
    expectTypeError([&]{(void)bindings.exec(stickyEmpty,u"");});
    expectRaw(stickyEmpty,opts::Value(0));

    const auto nonStateful=bindings.create(u"x");
    bindings.setLastIndex(nonStateful,1);
    freeze(nonStateful,opts::Value(1));
    const auto plainMatch=bindings.exec(nonStateful,u"x");
    expect(plainMatch&&plainMatch->index==0);
    expectRaw(nonStateful,opts::Value(1));
  };

  "math_port_regexp_last_index_to_length_handles_raw_primitives_and_valueof_once"_test=[] {
    opts::Context context;
    host::RegExpBindings bindings{context};
    const auto regexp=bindings.create(u"x",u"g");
    const opts::Value receiver(regexp);
    auto check=[&](opts::Value raw,std::u16string input,std::optional<std::size_t> expectedIndex,
                   opts::Value expectedAfter){
      context.set(receiver,opts::Key(u"lastIndex"),raw);
      const auto before=context.descriptor(receiver,opts::Key(u"lastIndex"));
      expect(before&&opts::sameValue(before->value,raw));
      const auto result=bindings.exec(receiver,input);
      expect(result.has_value()==expectedIndex.has_value());
      if(result&&expectedIndex)expect(result->index==*expectedIndex);
      const auto after=context.descriptor(receiver,opts::Key(u"lastIndex"));
      expect(after&&opts::sameValue(after->value,expectedAfter));
    };
    check(opts::Value(u"1"),u"xxx",1,opts::Value(2));
    check(opts::Value(true),u"xxx",1,opts::Value(2));
    check(opts::Value(opts::Undefined{}),u"x",0,opts::Value(1));
    check(opts::Value(-4),u"x",0,opts::Value(1));
    check(opts::Value(std::numeric_limits<double>::quiet_NaN()),u"x",0,opts::Value(1));
    check(opts::Value(std::numeric_limits<double>::infinity()),u"x",std::nullopt,opts::Value(0));

    int conversions=0;
    std::vector<std::string> order;
    const auto valueOf=context.function(u"valueOf",[&](opts::Context&,opts::Value,std::span<const opts::Value>){
      ++conversions;order.emplace_back("call:valueOf");return opts::Value(1);
    });
    const auto getValueOf=context.function(u"get valueOf",[&](opts::Context&,opts::Value,std::span<const opts::Value>){
      order.emplace_back("get:valueOf");return opts::Value(valueOf);
    });
    const auto getToPrimitive=context.function(u"get @@toPrimitive",[&](opts::Context&,opts::Value,std::span<const opts::Value>){
      order.emplace_back("get:@@toPrimitive");return opts::Value(opts::Undefined{});
    });
    const auto object=context.object();
    context.define(object,opts::Key(context.intrinsics().symbolToPrimitive),opts::Descriptor::access(getToPrimitive,{},false,true));
    context.define(object,opts::Key(u"valueOf"),opts::Descriptor::access(getValueOf,{},false,true));
    context.set(receiver,opts::Key(u"lastIndex"),opts::Value(object));
    const auto rawObject=context.descriptor(receiver,opts::Key(u"lastIndex"));
    expect(rawObject&&rawObject->value==opts::Value(object));
    const auto match=bindings.exec(receiver,u"xxx");
    expect(match&&match->index==1);
    expect(conversions==1);
    expect(order==std::vector<std::string>{"get:@@toPrimitive","get:valueOf","call:valueOf"});
    const auto afterValueOf=context.descriptor(receiver,opts::Key(u"lastIndex"));
    expect(afterValueOf&&afterValueOf->value==opts::Value(2));

    const auto symbol=context.symbol(u"cursor");
    context.set(receiver,opts::Key(u"lastIndex"),opts::Value(symbol));
    bool threw=false;
    try{(void)bindings.exec(receiver,u"xxx");}catch(const opts::Exception&error){threw=error.name=="TypeError";}
    expect(threw);
    const auto afterError=context.descriptor(receiver,opts::Key(u"lastIndex"));
    expect(afterError&&afterError->value==opts::Value(symbol));

    const auto throwingValueOf=context.function(u"valueOf",[](opts::Context&,opts::Value,std::span<const opts::Value>)->opts::Value{
      throw opts::Exception("RangeError",u"conversion failed");
    });
    const auto throwingObject=context.object();
    context.set(throwingObject,opts::Key(u"valueOf"),opts::Value(throwingValueOf));
    context.set(receiver,opts::Key(u"lastIndex"),opts::Value(throwingObject));
    threw=false;
    try{(void)bindings.exec(receiver,u"xxx");}catch(const opts::Exception&error){threw=error.name=="RangeError";}
    expect(threw);
    const auto afterThrow=context.descriptor(receiver,opts::Key(u"lastIndex"));
    expect(afterThrow&&afterThrow->value==opts::Value(throwingObject));

    const auto plain=bindings.create(u"x");
    const opts::Value plainReceiver(plain);
    context.set(plainReceiver,opts::Key(u"lastIndex"),opts::Value(symbol));
    threw=false;
    try{(void)bindings.exec(plainReceiver,u"x");}catch(const opts::Exception&error){threw=error.name=="TypeError";}
    expect(threw);
    const auto plainAfterError=context.descriptor(plainReceiver,opts::Key(u"lastIndex"));
    expect(plainAfterError&&plainAfterError->value==opts::Value(symbol));
    context.set(plainReceiver,opts::Key(u"lastIndex"),opts::Value(throwingObject));
    threw=false;
    try{(void)bindings.exec(plainReceiver,u"x");}catch(const opts::Exception&error){threw=error.name=="RangeError";}
    expect(threw);
    const auto plainAfterThrow=context.descriptor(plainReceiver,opts::Key(u"lastIndex"));
    expect(plainAfterThrow&&plainAfterThrow->value==opts::Value(throwingObject));

    int plainConversions=0;
    const auto plainValueOf=context.function(u"valueOf",[&](opts::Context&,opts::Value,std::span<const opts::Value>){
      ++plainConversions;return opts::Value(7);
    });
    const auto plainObject=context.object();
    context.set(plainObject,opts::Key(u"valueOf"),opts::Value(plainValueOf));
    context.set(plainReceiver,opts::Key(u"lastIndex"),opts::Value(plainObject));
    const auto plainMatch=bindings.exec(plainReceiver,u"x");
    expect(plainMatch&&plainMatch->index==0);
    expect(plainConversions==1);
    const auto plainAfterMatch=context.descriptor(plainReceiver,opts::Key(u"lastIndex"));
    expect(plainAfterMatch&&plainAfterMatch->value==opts::Value(plainObject));

    const auto sticky=bindings.create(u"x",u"y");
    const opts::Value stickyReceiver(sticky);
    context.set(stickyReceiver,opts::Key(u"lastIndex"),opts::Value(symbol));
    threw=false;
    try{(void)bindings.exec(stickyReceiver,u"xxx");}catch(const opts::Exception&error){threw=error.name=="TypeError";}
    expect(threw);
    const auto stickyAfterError=context.descriptor(stickyReceiver,opts::Key(u"lastIndex"));
    expect(stickyAfterError&&stickyAfterError->value==opts::Value(symbol));
    int stickyConversions=0;
    const auto stickyValueOf=context.function(u"valueOf",[&](opts::Context&,opts::Value,std::span<const opts::Value>){
      ++stickyConversions;return opts::Value(1);
    });
    const auto stickyObject=context.object();
    context.set(stickyObject,opts::Key(u"valueOf"),opts::Value(stickyValueOf));
    context.set(stickyReceiver,opts::Key(u"lastIndex"),opts::Value(stickyObject));
    const auto stickyMatch=bindings.exec(stickyReceiver,u"xxx");
    expect(stickyMatch&&stickyMatch->index==1);
    expect(stickyConversions==1);
    const auto stickyAfter=context.descriptor(stickyReceiver,opts::Key(u"lastIndex"));
    expect(stickyAfter&&stickyAfter->value==opts::Value(2));
  };
};
}
