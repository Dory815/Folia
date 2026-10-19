#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.textmacros.configuration;
import folia.math.input.tex.textmacros.methods;
import folia.math.input.tex.textcomp.configuration;
import folia.math.input.tex.bboldx.configuration;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.input_jax;
import folia.math.core.math_item;
import folia.math.core.math_render_actions;
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o=folia::math::options;
namespace t=folia::math::tex;

struct TextPackageMathItem final:core::AbstractMathItem,core::MathItemSourceIdentity {
  o::Context& context;
  o::ObjectRef identity;
  TextPackageMathItem(o::Context& context_,o::ObjectRef identity_,std::u16string input,
                      core::MathInputJaxReference jax)
    :core::AbstractMathItem(Property(property_host::debugText(input)),std::move(jax),Property(true)),
     context(context_),identity(identity_){}
  o::Context& sourceContext()const override{return context;}
  o::ObjectRef sourceIdentity()const override{return identity;}
};

struct Graph {
  o::Context context;
  OptionFunctionBindings functions{context};
  regexp_host::RegExpBindings regexp{context};
  Unicode16Normalization unicode;
  core::InputJaxModule input{context};
  core::FindMathModule findMath{context};
  t::FindTeXModule findTeX{context,findMath,regexp};
  t::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
  Graph(){runtime.initializeParserRuntime();}

  bool compile(std::span<const std::u16string_view> packages,std::u16string_view source){
    (void)t::BasePackage::initializeModule(runtime);
    (void)t::TextMacrosConfiguration::initializeModule(runtime);
    (void)t::TextcompConfiguration::initializeModule(runtime);
    (void)t::BboldxConfiguration::initializeModule(runtime);
    const auto packageArray=context.array(static_cast<std::uint32_t>(packages.size()));
    for(std::size_t i=0;i<packages.size();++i){
      const auto index=std::to_string(i);
      context.set(packageArray,o::Key(std::u16string(index.begin(),index.end())),o::Value(std::u16string(packages[i])));
    }
    const auto options=context.object();context.set(options,o::Key(u"packages"),o::Value(packageArray));
    auto jax=t::TeX::make(runtime,o::Value(options));
    auto factory=std::make_shared<mml::MmlFactory>();jax->setMmlFactory(factory);
    const auto identity=context.object();
    auto item=std::make_shared<TextPackageMathItem>(context,identity,std::u16string(source),
        core::MathInputJaxReference{static_cast<core::MathInputJaxService*>(jax.get())});
    context.payload(identity)=std::static_pointer_cast<NativeObject>(item);
    try{
      return jax->compile(*item,core::MathDocumentReference{PropertyNull{}}).isNode();
    }catch(const o::Exception&){return false;}
  }
};

struct SourcePreprocessorFixture {
  o::Context& context;
  std::vector<std::u16string> events;
  std::vector<bool> receiverChecks;
  o::ObjectRef record,outer,packageData,config,inner,nodeFactory,jax,factory;
  o::ObjectRef activeClear;
  std::size_t oldClearCalls=0,replacementClearCalls=0,setFactoryCalls=0,getCalls=0,parseOptionsReads=0;

  explicit SourcePreprocessorFixture(o::Context& context_):context(context_),
      record(context.object()),outer(context.object()),packageData(context.object()),config(context.object()),
      inner(context.object()),nodeFactory(context.object()),jax(context.object()),factory(context.object()) {
    auto getter=[this](o::ObjectRef owner,std::u16string keyName,std::u16string event,
                       std::function<o::Value()> read){
      const auto function=context.function(event,[this,owner,event=std::move(event),read=std::move(read)]
          (o::Context&,o::Value receiver,std::span<const o::Value>)->o::Value{
        events.push_back(event);receiverChecks.push_back(receiver==o::Value(owner));return read();
      });
      context.define(owner,o::Key(std::move(keyName)),o::Descriptor::access(function,std::nullopt));
    };
    auto method=[this](o::ObjectRef owner,std::u16string event,
                       std::function<o::Value(o::Value,std::span<const o::Value>)> call){
      return context.function(event,[this,owner,event=std::move(event),call=std::move(call)]
          (o::Context&,o::Value receiver,std::span<const o::Value> arguments)->o::Value{
        events.push_back(event);receiverChecks.push_back(receiver==o::Value(owner));
        return call(std::move(receiver),arguments);
      });
    };
    auto replacement=method(inner,u"inner.clear.replacement.call",[this](o::Value,std::span<const o::Value>){
      ++replacementClearCalls;return o::Value(o::Undefined{});
    });
    activeClear=method(inner,u"inner.clear.old.call",[this](o::Value,std::span<const o::Value>){
      ++oldClearCalls;return o::Value(o::Undefined{});
    });
    const auto setFactory=method(nodeFactory,u"nodeFactory.setMmlFactory.call",
        [this,replacement](o::Value,std::span<const o::Value> arguments){
          ++setFactoryCalls;
          receiverChecks.push_back(arguments.size()==1&&arguments[0]==o::Value(factory));
          activeClear=replacement;
          return o::Value(o::Undefined{});
        });
    const auto getConfig=method(packageData,u"packageData.get.call",
        [this](o::Value,std::span<const o::Value> arguments){
          ++getCalls;
          receiverChecks.push_back(arguments.size()==1&&arguments[0]==o::Value(u"textmacros"));
          return o::Value(config);
        });
    getter(record,u"data",u"record.data.get",[this]{return o::Value(outer);});
    getter(outer,u"packageData",u"outer.packageData.get",[this]{return o::Value(packageData);});
    getter(packageData,u"get",u"packageData.get.getter",[getConfig]{return o::Value(getConfig);});
    const auto parseOptionsGetter=context.function(u"config.parseOptions.get",
        [this](o::Context&,o::Value receiver,std::span<const o::Value>)->o::Value{
          ++parseOptionsReads;
          events.push_back(parseOptionsReads==1?u"config.parseOptions.get.first":u"config.parseOptions.get.second");
          receiverChecks.push_back(receiver==o::Value(config));
          return o::Value(inner);
        });
    context.define(config,o::Key(u"parseOptions"),o::Descriptor::access(parseOptionsGetter,std::nullopt));
    getter(inner,u"nodeFactory",u"inner.nodeFactory.get",[this]{return o::Value(nodeFactory);});
    getter(nodeFactory,u"setMmlFactory",u"nodeFactory.setMmlFactory.getter",[setFactory]{return o::Value(setFactory);});
    getter(config,u"jax",u"config.jax.get",[this]{return o::Value(jax);});
    getter(jax,u"mmlFactory",u"jax.mmlFactory.get",[this]{return o::Value(factory);});
    getter(inner,u"clear",u"inner.clear.get",[this]{return o::Value(activeClear);});
  }

  void reset(){events.clear();receiverChecks.clear();oldClearCalls=0;replacementClearCalls=0;setFactoryCalls=0;getCalls=0;parseOptionsReads=0;}
  std::vector<std::u16string> expectedEvents()const{return {
    u"record.data.get",u"outer.packageData.get",u"packageData.get.getter",u"packageData.get.call",
    u"config.parseOptions.get.first",u"inner.nodeFactory.get",u"nodeFactory.setMmlFactory.getter",
    u"config.jax.get",u"jax.mmlFactory.get",u"nodeFactory.setMmlFactory.call",
    u"config.parseOptions.get.second",u"inner.clear.get",u"inner.clear.replacement.call"};}
};

suite textmacrosFilterRecordRoundtrip=[] {
  "textmacros_preprocessor_filter_record_bridge_serves_textcomp_and_bboldx"_test=[] {
    const std::array<std::u16string_view,3> textcomp{u"base",u"textmacros",u"textcomp"};
    const std::array<std::u16string_view,3> bboldx{u"base",u"textmacros",u"bboldx"};
    Graph textcompGraph;
    expect(textcompGraph.compile(textcomp,u"\\text{\\textcopyright\\textregistered}"));
    Graph bboldxGraph;
    expect(bboldxGraph.compile(bboldx,u"\\text{\\textbb{A} \\textbfbb{B}}"));
  };
  "textmacros_preprocessor_runs_one_source_algorithm_for_source_record_and_native_filter_record"_test=[] {
    Graph graph;
    auto& runtime=graph.runtime;
    auto& context=graph.context;
    (void)t::BasePackage::initializeModule(runtime);
    (void)t::TextMacrosConfiguration::initializeModule(runtime);

    const auto loaded=runtime.configurationHandler.get(u"textmacros");
    const auto* configuration=std::get_if<t::ConfigurationRef>(&loaded);
    expect(configuration!=nullptr&&*configuration);
    if(!configuration||!*configuration)return;
    const auto preprocessors=(*configuration)->field(u"preprocessors");
    const auto tuple=context.get(preprocessors,o::Key(u"0"));
    const auto callback=context.get(tuple,o::Key(u"0"));
    const auto nativeFunctionValue=runtime.functions().functionValue(callback);
    const auto* nativeFunction=std::get_if<FunctionRef>(&nativeFunctionValue);
    expect(nativeFunction!=nullptr&&*nativeFunction);
    if(!nativeFunction||!*nativeFunction)return;

    SourcePreprocessorFixture fixture(context);
    (void)context.call(std::get<o::ObjectRef>(callback.data),o::Value(o::Undefined{}),
        std::array<o::Value,1>{o::Value(fixture.record)});
    expect(static_cast<bool>(fixture.events==fixture.expectedEvents()));
    expect(!fixture.receiverChecks.empty()&&std::ranges::all_of(fixture.receiverChecks,[](bool value){return value;}));
    expect(fixture.getCalls==1&&fixture.setFactoryCalls==1);
    expect(fixture.oldClearCalls==0&&fixture.replacementClearCalls==1);

    fixture.reset();
    auto record=std::make_shared<core::InputJaxFilterData>();
    record->initialize(core::MathItemReference{Missing{}},core::MathDocumentReference{PropertyNull{}},
                       std::any(o::Value(fixture.outer)));
    record->encodeData=[](const std::any& data)->o::Value{
      const auto* value=std::any_cast<o::Value>(&data);
      if(!value)throw o::Exception("TypeError",u"test filter data is not a Context Value");
      return *value;
    };
    record->bindSourceRecord(context,record,runtime.executionLifetime());
    const auto recordIdentity=record->sourceIdentity();
    const auto* payload=std::any_cast<core::InputJaxFilterDataRef>(&std::as_const(context).payload(recordIdentity));
    expect(payload!=nullptr&&*payload==record);
    if(!payload||*payload!=record)return;
    expect(context.get(recordIdentity,o::Key(u"data"))==o::Value(fixture.outer));

    const std::array<std::any,1> nativeArguments{std::any(core::InputJaxFilterDataRef(record))};
    (void)(*nativeFunction)->call(FunctionReceiver{Missing{}},nativeArguments);
    expect(record->sourceIdentity()==recordIdentity);
    expect(context.get(recordIdentity,o::Key(u"data"))==o::Value(fixture.outer));
    auto expectedNativeEvents=fixture.expectedEvents();
    expectedNativeEvents.erase(expectedNativeEvents.begin());
    expect(static_cast<bool>(fixture.events==expectedNativeEvents));
    expect(!fixture.receiverChecks.empty()&&std::ranges::all_of(fixture.receiverChecks,[](bool value){return value;}));
    expect(fixture.getCalls==1&&fixture.setFactoryCalls==1);
    expect(fixture.oldClearCalls==0&&fixture.replacementClearCalls==1);
  };
};
suite textmacrosNativeFallbackBoundary=[] {
  "textmacros_bboldx_text_fallback_receives_native_parser_and_token_values"_test=[] {
    const std::array<std::u16string_view,3> packages{u"base",u"textmacros",u"bboldx"};
    Graph graph;
    expect(graph.compile(packages,u"\\text{\\textbb{A} \\textbfbb{B} \\txtbbalpha \\txtbfbbalpha}"));
  };
  "textmacros_fallback_uses_source_visible_compatible_parser_records"_test=[] {
    Graph graph;
    auto& runtime=graph.runtime;
    auto& context=graph.context;
    (void)t::BasePackage::initializeModule(runtime);
    (void)t::TextMacrosConfiguration::initializeModule(runtime);
    const auto loaded=runtime.configurationHandler.get(u"text-base");
    const auto* configuration=std::get_if<t::ConfigurationRef>(&loaded);
    expect(configuration!=nullptr&&*configuration);
    if(!configuration||!*configuration)return;
    const o::Value fallbacks=(*configuration)->field(u"fallback");
    const o::Value macroFallback=context.get(fallbacks,o::Key(u"macro"));
    const o::Value characterFallback=context.get(fallbacks,o::Key(u"character"));
    const auto parser=context.object(),texParser=context.object();
    context.define(parser,o::Key(u"texParser"),o::Descriptor::data(o::Value(texParser)));
    context.define(parser,o::Key(u"text"),o::Descriptor::data(o::Value(u"")));
    std::vector<std::u16string> events;
    const auto lookup=context.function(u"lookup",[&](o::Context&,o::Value receiver,std::span<const o::Value> args){
      events.push_back(u"lookup");
      expect(receiver==o::Value(texParser));
      expect(args.size()==2&&args[0]==o::Value(u"macro")&&args[1]==o::Value(u"unknown"));
      return o::Value(o::Undefined{});
    });
    const auto parse=context.function(u"parse",[&](o::Context& cx,o::Value receiver,std::span<const o::Value> args){
      events.push_back(u"parse");
      expect(receiver==o::Value(texParser));
      expect(args.size()==2&&args[0]==o::Value(u"macro"));
      const o::Value sourceInput=cx.get(args[1],o::Key(u"0"));
      const o::Value token=cx.get(args[1],o::Key(u"1"));
      expect(sourceInput==o::Value(parser)&&token==o::Value(u"unknown"));
      return o::Value(o::Undefined{});
    });
    context.define(texParser,o::Key(u"lookup"),o::Descriptor::data(o::Value(lookup)));
    context.define(texParser,o::Key(u"parse"),o::Descriptor::data(o::Value(parse)));
    (void)context.call(std::get<o::ObjectRef>(macroFallback.data),o::Value(o::Undefined{}),
        std::array<o::Value,2>{o::Value(parser),o::Value(u"unknown")});
    expect(static_cast<bool>(events==std::vector<std::u16string>{u"lookup",u"parse"}));
    (void)context.call(std::get<o::ObjectRef>(characterFallback.data),o::Value(o::Undefined{}),
        std::array<o::Value,2>{o::Value(parser),o::Value(u"x")});
    expect(context.get(parser,o::Key(u"text"))==o::Value(u"x"));

    const Property canonical= t::TextMacrosMethods::get(runtime,u"Macro");
    const o::Value macroFunction=runtime.tokenMaps.parseMethodToValue(functions::fromProperty(canonical));
    const auto macro=context.object();
    context.define(macro,o::Key(u"_func"),o::Descriptor::data(macroFunction));
    o::Value lookupResult(macro);
    std::size_t errorCalls=0,parseCalls=0;
    const auto matchingLookup=context.function(u"lookup",[&](o::Context&,o::Value receiver,std::span<const o::Value> args){
      expect(receiver==o::Value(texParser));expect(args.size()==2&&args[0]==o::Value(u"macro"));return lookupResult;
    });
    const auto error=context.function(u"Error",[&](o::Context&,o::Value receiver,std::span<const o::Value> args){
      ++errorCalls;expect(receiver==o::Value(parser));
      expect(args.size()==3&&args[0]==o::Value(u"MathMacro")&&args[1]==o::Value(u"%1 is only supported in math mode")&&args[2]==o::Value(u"\\other"));
      return o::Value(o::Undefined{});
    });
    const auto parseReturningValue=context.function(u"parse",[&](o::Context&,o::Value receiver,std::span<const o::Value> args){
      ++parseCalls;expect(receiver==o::Value(texParser));expect(args.size()==2);return o::Value(u"ignored parse result");
    });
    context.set(texParser,o::Key(u"lookup"),o::Value(matchingLookup));
    context.set(texParser,o::Key(u"parse"),o::Value(parseReturningValue));
    context.set(parser,o::Key(u"Error"),o::Value(error));
    const o::ObjectRef callback=std::get<o::ObjectRef>(macroFallback.data);
    const auto result=context.call(callback,o::Value(o::Undefined{}),std::array<o::Value,2>{o::Value(parser),o::Value(u"macroName")});
    expect(std::holds_alternative<o::Undefined>(result.data));
    expect(errorCalls==0&&parseCalls==1);
    context.set(macro,o::Key(u"_func"),o::Value(u"not Macro"));
    (void)context.call(callback,o::Value(o::Undefined{}),std::array<o::Value,2>{o::Value(parser),o::Value(u"other")});
    expect(errorCalls==1&&parseCalls==2);

    const auto left=context.object(),right=context.object();
    std::vector<std::u16string> additionOrder;
    const auto leftPrimitive=context.function(u"left primitive",[&](o::Context&,o::Value receiver,std::span<const o::Value> args){
      additionOrder.push_back(u"left");expect(receiver==o::Value(left));expect(args.size()==1&&args[0]==o::Value(u"default"));return o::Value(u"L");
    });
    const auto rightPrimitive=context.function(u"right primitive",[&](o::Context&,o::Value receiver,std::span<const o::Value> args){
      additionOrder.push_back(u"right");expect(receiver==o::Value(right));expect(args.size()==1&&args[0]==o::Value(u"default"));return o::Value(u"R");
    });
    context.set(o::Value(left),o::Key(context.intrinsics().symbolToPrimitive),o::Value(leftPrimitive));
    context.set(o::Value(right),o::Key(context.intrinsics().symbolToPrimitive),o::Value(rightPrimitive));
    context.set(parser,o::Key(u"text"),o::Value(left));
    (void)context.call(std::get<o::ObjectRef>(characterFallback.data),o::Value(o::Undefined{}),
        std::array<o::Value,2>{o::Value(parser),o::Value(right)});
    expect(context.get(parser,o::Key(u"text"))==o::Value(u"LR"));
    expect(static_cast<bool>(additionOrder==std::vector<std::u16string>{u"left",u"right"}));
    context.set(parser,o::Key(u"text"),o::Value(2));
    (void)context.call(std::get<o::ObjectRef>(characterFallback.data),o::Value(o::Undefined{}),
        std::array<o::Value,2>{o::Value(parser),o::Value(3)});
    expect(context.get(parser,o::Key(u"text"))==o::Value(5));
    const bool numericAddition=context.get(parser,o::Key(u"text"))==o::Value(5);
    context.set(parser,o::Key(u"text"),o::Value(u""));
    (void)context.call(std::get<o::ObjectRef>(characterFallback.data),o::Value(o::Undefined{}),
        std::array<o::Value,1>{o::Value(parser)});
    expect(context.get(parser,o::Key(u"text"))==o::Value(u"undefined"));
    expect(numericAddition);
  };
};
} // namespace
