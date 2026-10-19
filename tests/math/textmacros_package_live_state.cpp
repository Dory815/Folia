#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.textmacros.configuration;
import folia.math.input.tex.textcomp.configuration;
import folia.math.core.input_jax_host;
import folia.math.core.input_jax;
import folia.math.core.math_item;
import folia.math.core.math_render_actions;
import folia.math.core.find_math_host;
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
  TextPackageMathItem(o::Context& cx,o::ObjectRef id,std::u16string input,core::MathInputJaxReference jax)
    :core::AbstractMathItem(Property(property_host::debugText(input)),std::move(jax),Property(true)),
     context(cx),identity(id){}
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

  void initializeTextcompPackages(){
    (void)t::BasePackage::initializeModule(runtime);
    (void)t::TextMacrosConfiguration::initializeModule(runtime);
    (void)t::TextcompConfiguration::initializeModule(runtime);
  }

  t::TeXRef makeTextcompJax(){
    const std::array<std::u16string_view,3> packages{u"base",u"textmacros",u"textcomp"};
    const auto packageArray=context.array(static_cast<std::uint32_t>(packages.size()));
    for(std::size_t i=0;i<packages.size();++i){
      const auto index=std::to_string(i);
      context.set(packageArray,o::Key(std::u16string(index.begin(),index.end())),
                  o::Value(std::u16string(packages[i])));
    }
    const auto options=context.object();
    context.set(options,o::Key(u"packages"),o::Value(packageArray));
    auto jax=t::TeX::make(runtime,o::Value(options));
    jax->setMmlFactory(std::make_shared<mml::MmlFactory>());
    return jax;
  }

  mml::NodeValue compileTextcomp(t::TeXRef& jax,const std::u16string& source,
                                 std::shared_ptr<TextPackageMathItem>& item,
                                 o::ObjectRef& itemIdentity){
    itemIdentity=context.object();
    item=std::make_shared<TextPackageMathItem>(context,itemIdentity,source,
        core::MathInputJaxReference{static_cast<core::MathInputJaxService*>(jax.get())});
    context.payload(itemIdentity)=std::static_pointer_cast<NativeObject>(item);
    try{return jax->compile(*item,core::MathDocumentReference{PropertyNull{}});}
    catch(const o::Exception& e){
      std::cerr<<"[g218-live-state] compile threw options::Exception name="<<e.name
               <<" message="<<property_host::debugText(e.message)<<'\n';
      throw;
    }catch(const std::exception& e){
      std::cerr<<"[g218-live-state] compile threw std::exception: "<<e.what()<<'\n';
      throw;
    }catch(...){
      std::cerr<<"[g218-live-state] compile threw unknown exception\n";
      throw;
    }
  }
};

std::string handle(o::ObjectRef id){
  return "context="+std::to_string(id.context)+",index="+std::to_string(id.index);
}
void reportCheck(std::string_view name,bool ok,std::string_view detail){
  std::cerr<<"[g218-live-state] "<<name<<"="<<(ok?"ok":"FAIL")<<" "<<detail<<'\n';
  expect(ok);
}

void runLiveStateScenario(bool warmupJaxFirst){
    Graph graph;
    graph.initializeTextcompPackages();
    t::TeXRef warmup;
    if(warmupJaxFirst){
      warmup=graph.makeTextcompJax(); // Keep the uncompiled default jax alive during the real compile.
      reportCheck("warmup_jax_is_live_uncompiled_owner",static_cast<bool>(warmup),
                  "jax="+handle(warmup->instanceIdentity())+" parseOptions_handles="+
                  std::to_string(graph.runtime.parseOptionsIdentities.canonicalHandleCount()));
    }
    auto jax=graph.makeTextcompJax();
    auto& cx=graph.context;
    auto& runtime=graph.runtime;
    std::shared_ptr<TextPackageMathItem> item;
    o::ObjectRef itemIdentity;
    const std::u16string source=u"\\text{\\textcopyright\\textregistered\\texttrademark}";
    const auto result=graph.compileTextcomp(jax,source,item,itemIdentity);
    reportCheck("compile_result",result.isNode(),"result_is_node="+std::string(result.isNode()?"true":"false"));

    const auto jaxIdentity=jax->instanceIdentity();
    const auto actualOptionsValue=jax->parseOptions();
    const auto* actualOptionsRef=std::get_if<t::ParseOptionsRef>(&actualOptionsValue);
    reportCheck("actual_parse_options_owner",actualOptionsRef&&*actualOptionsRef,
                "jax="+handle(jaxIdentity));
    if(!actualOptionsRef||!*actualOptionsRef)return;
    const auto actualOptions=*actualOptionsRef;
    const auto actualOptionsIdentity=runtime.parseOptionsIdentities.find(actualOptions);
    reportCheck("canonical_parse_options_identity",actualOptionsIdentity.has_value(),
                "parseOptions="+(actualOptionsIdentity?handle(*actualOptionsIdentity):"<missing>"));
    if(!actualOptionsIdentity)return;

    const auto packageData=actualOptions->packageData;
    const auto* packageMap=std::get_if<std::shared_ptr<SourceStringMap<o::Value>>>(&packageData);
    reportCheck("package_data_map",packageMap&&*packageMap,"outer_parse_options="+handle(*actualOptionsIdentity));
    if(!packageMap||!*packageMap)return;
    const auto recordValue=(*packageMap)->get(u"textmacros");
    reportCheck("textmacros_record_exists",recordValue.has_value(),"package=textmacros");
    if(!recordValue)return;
    const auto* recordObject=std::get_if<o::ObjectRef>(&recordValue->data);
    reportCheck("textmacros_record_is_context_object",recordObject!=nullptr,
                recordObject?handle(*recordObject):"record is not ObjectRef");
    if(!recordObject)return;
    const auto record=*recordObject;

    const o::Value recordOptions=cx.get(record,o::Key(u"parseOptions"));
    const auto* recordOptionsIdentity=std::get_if<o::ObjectRef>(&recordOptions.data);
    reportCheck("record_inner_parse_options_identity",recordOptionsIdentity!=nullptr,
                recordOptionsIdentity?handle(*recordOptionsIdentity):"<not ObjectRef>");
    if(recordOptionsIdentity){
      const auto inner=runtime.parseOptionsIdentities.resolve(*recordOptionsIdentity);
      reportCheck("record_inner_options_distinct_from_outer",inner.get()!=actualOptions.get(),
                  "inner="+handle(*recordOptionsIdentity)+" outer="+handle(*actualOptionsIdentity));
      const auto* innerMap=std::get_if<std::shared_ptr<SourceStringMap<o::Value>>>(&inner->packageData);
      reportCheck("record_inner_options_shares_outer_package_data",innerMap&&*innerMap&&
                  (*innerMap)->storage()==(*packageMap)->storage(),
                  "inner="+handle(*recordOptionsIdentity)+" outer="+handle(*actualOptionsIdentity));
    }

    const o::Value recordJax=cx.get(record,o::Key(u"jax"));
    const auto* recordJaxObject=std::get_if<o::ObjectRef>(&recordJax.data);
    reportCheck("record_jax_is_actual_jax",recordJaxObject&&*recordJaxObject==jaxIdentity,
                "actual="+handle(jaxIdentity)+" record="+
                (recordJaxObject?handle(*recordJaxObject):"<not ObjectRef>"));
    const o::Value actualJaxOptions=cx.get(o::Value(jaxIdentity),o::Key(u"_parseOptions"));
    const auto* actualJaxOptionsObject=std::get_if<o::ObjectRef>(&actualJaxOptions.data);
    reportCheck("actual_jax_parse_options_is_live_owner",
                actualJaxOptionsObject&&*actualJaxOptionsObject==*actualOptionsIdentity,
                "jax="+handle(jaxIdentity)+" expected="+handle(*actualOptionsIdentity)+" actual="+
                (actualJaxOptionsObject?handle(*actualJaxOptionsObject):"<not ObjectRef>"));
    const o::Value projectedOptions=cx.get(recordJax,o::Key(u"_parseOptions"));
    const auto* projectedOptionsObject=std::get_if<o::ObjectRef>(&projectedOptions.data);
    reportCheck("record_jax_parse_options_is_actual",projectedOptionsObject&&*projectedOptionsObject==*actualOptionsIdentity,
                "actual="+handle(*actualOptionsIdentity)+" projected="+
                (projectedOptionsObject?handle(*projectedOptionsObject):"<not ObjectRef>"));
    reportCheck("actual_options_owns_math_item",cx.hasOwn(o::Value(*actualOptionsIdentity),o::Key(u"mathItem")),
                "parseOptions="+handle(*actualOptionsIdentity)+" item="+handle(itemIdentity));
    const o::Value actualMathItem=cx.get(o::Value(*actualOptionsIdentity),o::Key(u"mathItem"));
    const auto* actualMathItemObject=std::get_if<o::ObjectRef>(&actualMathItem.data);
    reportCheck("math_item_field_is_compile_item",actualMathItemObject&&*actualMathItemObject==itemIdentity,
                "expected="+handle(itemIdentity)+" actual="+
                (actualMathItemObject?handle(*actualMathItemObject):"<not ObjectRef>"));
    const o::Value root=cx.get(o::Value(*actualOptionsIdentity),o::Key(u"root"));
    const bool rootPresent=!std::holds_alternative<o::Null>(root.data)&&!std::holds_alternative<o::Undefined>(root.data);
    reportCheck("actual_options_root_non_null",rootPresent,
                "root_is_object="+std::string(std::holds_alternative<o::ObjectRef>(root.data)?"true":"false"));

    const o::Value parserValue=cx.get(record,o::Key(u"texParser"));
    const auto* parserIdentity=std::get_if<o::ObjectRef>(&parserValue.data);
    reportCheck("record_tex_parser_identity_present",parserIdentity!=nullptr,
                parserIdentity?handle(*parserIdentity):"<null-or-non-object>");
    if(!parserIdentity)return;
    t::TexParserRef parser;
    try{parser=runtime.parserIdentities.resolve(*parserIdentity);}
    catch(const o::Exception& e){
      std::cerr<<"[g218-live-state] parser identity resolve failed name="<<e.name
               <<" message="<<property_host::debugText(e.message)<<'\n';
    }
    reportCheck("record_tex_parser_identity_resolves",static_cast<bool>(parser),
                "parser="+handle(*parserIdentity)+" parser_handles="+
                std::to_string(runtime.parserIdentities.canonicalHandleCount()));
    if(!parser)return;
    const auto parserCanonicalIdentity=runtime.parserIdentities.find(parser);
    reportCheck("record_tex_parser_is_canonical_owner",
                parserCanonicalIdentity&&*parserCanonicalIdentity==*parserIdentity,
                "record="+handle(*parserIdentity)+" canonical="+
                (parserCanonicalIdentity?handle(*parserCanonicalIdentity):"<missing>")+" parser_handles="+
                std::to_string(runtime.parserIdentities.canonicalHandleCount()));
    const auto parserConfiguration=parser->configuration;
    const auto* parserOptions=std::get_if<t::ParseOptions*>(&parserConfiguration);
    reportCheck("tex_parser_configuration_is_outer_options",parserOptions&&*parserOptions==actualOptions.get(),
                "parser="+handle(*parserIdentity)+" expected_options="+handle(*actualOptionsIdentity));
    reportCheck("record_tex_parser_is_outer_tex_parser",dynamic_cast<t::TextParser*>(parser.get())==nullptr,
                "parser="+handle(*parserIdentity)+"; nested text parser would be an unexpected owner");
}

suite textmacrosPackageLiveState=[] {
  "textmacros_textcomp_package_record_tracks_live_compile_owners_single_jax"_test=[] {
    runLiveStateScenario(false);
  };
  "textmacros_textcomp_package_record_tracks_live_compile_owners_after_uncompiled_warmup"_test=[] {
    runLiveStateScenario(true);
  };
};
} // namespace

