#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.boldsymbol.configuration;
import folia.math.input.tex.empheq.configuration;
import folia.math.input.tex.empheq.util;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.math_item;
import folia.math.core.math_render_actions;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t=folia::math::tex;
namespace o=folia::math::options;
struct ProbeParser:t::TexParser { void bindForTest(o::ObjectRef identity){bindIdentity(std::move(identity));} };
struct EmpheqMathItem final:core::AbstractMathItem,core::MathItemSourceIdentity {
  o::Context& context;o::ObjectRef identity;
  EmpheqMathItem(o::Context& context_,o::ObjectRef identity_,Property input,core::MathInputJaxReference jax)
    :core::AbstractMathItem(std::move(input),std::move(jax),Property(true)),context(context_),identity(identity_){}
  o::Context& sourceContext()const override{return context;}
  o::ObjectRef sourceIdentity()const override{return identity;}
};
std::shared_ptr<ProbeParser> bindParser(t::ParserRuntime& runtime,t::ParseOptions& options){
  auto parser=std::make_shared<ProbeParser>();parser->configuration=&options;
  const auto identity=runtime.context().object();const t::TexParserRef reference=parser;
  (void)runtime.parserIdentities.bind(reference,identity,reference);parser->bindForTest(identity);return parser;
}
struct Graph {
  o::Context context; OptionFunctionBindings functions{context};
  regexp_host::RegExpBindings regexp{context}; Unicode16Normalization unicode;
  core::InputJaxModule input{context}; core::FindMathModule findMath{context};
  t::FindTeXModule findTeX{context,findMath,regexp};
  t::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
  Graph(){runtime.initializeParserRuntime();}
};
FunctionRef method(const Property& table,std::u16string_view name){
  const auto value=property_host::get(table,property_host::debugText(name));
  const auto* result=std::get_if<FunctionRef>(&value.value);
  if(!result||!*result)throw std::runtime_error("EmpheqMethods member is not callable");return *result;
}
void expectMetadata(t::ParserRuntime& runtime,const FunctionRef& method,std::u16string_view name,double length){
  const auto identity=runtime.functions().find(method);expect(identity.has_value());
  const auto function=o::Value(*identity);
  expect(runtime.context().get(function,o::Key(u"name"))==o::Value(std::u16string(name)));
  expect(runtime.context().get(function,o::Key(u"length"))==o::Value(length));
}
suite empheqCanonicalOwner=[] {
  "empheq_module_exports_keep_one_runtime_owner_and_source_keys_metadata"_test=[] {
    Graph graph;auto& runtime=graph.runtime;
    const auto methods=t::EmpheqMethods::initializeModule(runtime);
    const auto record=PropertyList::record(methods);expect(record.has_value());
    expect(record->keys()==std::vector<std::string>{"Empheq","EmpheqMO","EmpheqDelim"});
    const auto environment=method(methods,u"Empheq"),mo=method(methods,u"EmpheqMO"),delim=method(methods,u"EmpheqDelim");
    expectMetadata(runtime,environment,u"Empheq",2);expectMetadata(runtime,mo,u"EmpheqMO",3);expectMetadata(runtime,delim,u"EmpheqDelim",2);
    expect(t::EmpheqMethods::initializeModule(runtime)==methods);
    property_host::set(methods,"addedBySource",Property(17));
    expect(property_host::get(t::EmpheqMethods::initializeModule(runtime),"addedBySource")==Property(17));
    (void)t::EmpheqConfiguration::initializeModule(runtime);
    const auto environmentMap=runtime.mapHandler.getMap(Property("empheq-env"));expect(environmentMap!=nullptr);
    const auto mapped=environmentMap->parserFor(Property("empheq"));const auto* mappedFunction=std::get_if<FunctionRef>(&mapped);expect(mappedFunction!=nullptr);expect(*mappedFunction==environment);
    expect(method(t::EmpheqMethods::initializeModule(runtime),u"Empheq")==environment);
    expect(method(t::EmpheqMethods::initializeModule(runtime),u"EmpheqMO")==mo);
    expect(method(t::EmpheqMethods::initializeModule(runtime),u"EmpheqDelim")==delim);
    const auto owner=t::EmpheqUtil::initializeModule(runtime);
    const auto* identity=std::get_if<o::ObjectRef>(&owner.data);expect(identity!=nullptr);
    expect(runtime.empheqUtilModuleExports==owner);
    const auto keys=runtime.context().ownKeys(owner);
    std::vector<std::u16string> names;for(const auto& key:keys)if(const auto* text=std::get_if<std::u16string>(&key.data))names.push_back(*text);
    expect(names==std::vector<std::u16string>{u"splitOptions",u"columnCount",u"cellBlock",u"topRowTable",u"rowspanCell",u"left",u"right",u"adjustTable",u"allowEnv",u"checkEnv"});
    const std::array<std::pair<std::u16string_view,double>,9> metadata{{
      {u"splitOptions",2},{u"columnCount",1},{u"cellBlock",4},{u"topRowTable",2},{u"rowspanCell",5},
      {u"left",5},{u"right",5},{u"adjustTable",2},{u"checkEnv",1}}};
    for(const auto& [name,length]:metadata){
      const auto function=runtime.context().get(owner,o::Key(std::u16string(name)));const auto* id=std::get_if<o::ObjectRef>(&function.data);expect(id!=nullptr);
      expect(runtime.context().get(function,o::Key(u"name"))==o::Value(std::u16string(name)));
      expect(runtime.context().get(function,o::Key(u"length"))==o::Value(length));
    }
    const auto allow=runtime.context().get(owner,o::Key(u"allowEnv"));const auto* allowObject=std::get_if<o::ObjectRef>(&allow.data);expect(allowObject!=nullptr);
    runtime.context().erase(*allowObject,o::Key(u"equation"));
    const std::array<o::Value,1> equation{o::Value(u"equation")};
    const auto accepted=t::EmpheqUtil::call(runtime,u"checkEnv",equation);expect(accepted==o::Value(false));
    runtime.context().set(*allowObject,o::Key(u"custom"),o::Value(true));
    const std::array<o::Value,1> custom{o::Value(u"custom*")};
    expect(t::EmpheqUtil::call(runtime,u"checkEnv",custom)==o::Value(true));
    Graph other;const auto otherOwner=t::EmpheqUtil::initializeModule(other.runtime);
    expect(otherOwner!=owner);
    expect(t::EmpheqMethods::initializeModule(other.runtime)!=methods);
  };
  "empheq_context_call_uses_replaced_export_member_receiver_and_node_identity"_test=[] {
    Graph graph;auto& runtime=graph.runtime;const auto owner=t::EmpheqUtil::initializeModule(runtime);
    auto mmlFactory=std::make_shared<mml::MmlFactory>();(void)runtime.bindMmlFactory(mmlFactory);
    auto* nativeNode=mmlFactory->create("mi",tree::FactoryArgs{});const auto nodeIdentity=runtime.mmlNodeIdentities.find(nativeNode);expect(nodeIdentity.has_value());
    o::Value receiverSeen=o::Undefined{};
    const auto replacement=runtime.context().function(u"checkEnv",[&](o::Context&,o::Value receiver,std::span<const o::Value>)->o::Value{receiverSeen=receiver;return o::Value(*nodeIdentity);});
    const auto* ownerIdentity=std::get_if<o::ObjectRef>(&owner.data);expect(ownerIdentity!=nullptr);
    runtime.context().set(owner,o::Key(u"checkEnv"),o::Value(replacement));
    const auto result=t::EmpheqUtil::call(runtime,u"checkEnv",{});expect(receiverSeen==owner);expect(result==o::Value(*nodeIdentity));
    const auto* returned=std::get_if<o::ObjectRef>(&result.data);expect(returned!=nullptr);
    expect(runtime.mmlNodeIdentities.resolve(*returned)==nativeNode);
    expect(t::EmpheqUtil::checkEnv(runtime,u"ignored"));
  };
  "empheq_fresh_cases_left_dependency_lazily_initializes_util_owner"_test=[] {
    Graph graph;auto& runtime=graph.runtime;t::ParseOptions options(runtime);auto parser=bindParser(runtime,options);
    expect(std::holds_alternative<o::Undefined>(runtime.empheqUtilModuleExports.data));
    bool dispatched=false;
    try{t::EmpheqUtil::left(mml::NodeValue(Missing{}),mml::NodeValue(Missing{}),u"left",*parser);}
    catch(...){dispatched=true;}
    expect(dispatched);
    const auto* owner=std::get_if<o::ObjectRef>(&runtime.empheqUtilModuleExports.data);expect(owner!=nullptr);
    const auto method=runtime.context().get(o::Value(*owner),o::Key(u"left"));
    expect(runtime.context().kind(std::get<o::ObjectRef>(method.data))==o::ObjectKind::function);
  };
  "empheq_public_left_dispatches_to_replaced_export_with_real_receiver"_test=[] {
    Graph graph;auto& runtime=graph.runtime;t::ParseOptions options(runtime);auto parser=bindParser(runtime,options);
    auto mmlFactory=std::make_shared<mml::MmlFactory>();(void)runtime.bindMmlFactory(mmlFactory);
    auto* table=mmlFactory->create("mtable",tree::FactoryArgs{});const auto tableIdentity=runtime.mmlNodeIdentities.find(table);expect(tableIdentity.has_value());
    const auto owner=t::EmpheqUtil::initializeModule(runtime);o::Value receiverSeen=o::Undefined{};std::vector<o::Value> argsSeen;
    const auto replacement=runtime.context().function(u"left",[&](o::Context&,o::Value receiver,std::span<const o::Value> args)->o::Value{
      receiverSeen=receiver;argsSeen.assign(args.begin(),args.end());return o::Value(o::Undefined{});
    });
    runtime.context().set(owner,o::Key(u"left"),o::Value(replacement));
    t::EmpheqUtil::left(mml::NodeValue(table),mml::NodeValue(table),u"\\ell",*parser,u"align");
    expect(receiverSeen==owner);expect(argsSeen.size()==5);
    expect(argsSeen[0]==o::Value(*tableIdentity));expect(argsSeen[1]==o::Value(*tableIdentity));
    expect(argsSeen[2]==o::Value(u"\\ell"));expect(argsSeen[3]==o::Value(parser->instanceIdentity()));expect(argsSeen[4]==o::Value(u"align"));
  };
  "empheq_delimiter_macro_uses_property_record_with_boldsymbol_token_creator"_test=[] {
    Graph graph;auto& runtime=graph.runtime;
    (void)t::BasePackage::initializeModule(runtime);
    (void)t::BoldsymbolConfiguration::initializeModule(runtime);
    (void)t::EmpheqConfiguration::initializeModule(runtime);
    const auto packages=runtime.context().array(3);
    runtime.context().set(packages,o::Key(u"0"),o::Value(u"base"));
    runtime.context().set(packages,o::Key(u"1"),o::Value(u"boldsymbol"));
    runtime.context().set(packages,o::Key(u"2"),o::Value(u"empheq"));
    const auto config=runtime.context().object();runtime.context().set(config,o::Key(u"packages"),o::Value(packages));
    auto jax=t::TeX::make(runtime,o::Value(config));auto factory=std::make_shared<mml::MmlFactory>();jax->setMmlFactory(factory);
    auto item=std::make_shared<EmpheqMathItem>(runtime.context(),runtime.context().object(),Property("\\empheqlbrace"),core::MathInputJaxReference{static_cast<core::MathInputJaxService*>(jax.get())});
    runtime.context().payload(item->sourceIdentity())=std::static_pointer_cast<NativeObject>(item);
    const auto result=jax->compile(*item,core::MathDocumentReference{PropertyNull{}});expect(result.isNode());
    bool found=false;const auto walk=[&](auto&& self,mml::NodeValue value)->void{
      if(!value.isNode())return;auto& current=*value.requireNode();
      if(current.isKind("mo")&&current.attributes()->get("data-latex")==Property("\\empheqlbrace"))found=true;
      const auto children=current.childNodes;for(std::size_t i=0;children&&i<children->size();++i)self(self,children->get(i));
    };walk(walk,result);expect(found);
  };
};
} // namespace
