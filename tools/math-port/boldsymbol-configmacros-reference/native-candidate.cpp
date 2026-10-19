// Actual Source package candidates: full TeX.compile and postfilter path.
#include "../../../tests/support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.boldsymbol.configuration;
import folia.math.input.tex.configmacros.configuration;
import folia.math.input.tex.node_util;
import folia.math.core.math_item;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace tex=folia::math::tex;
namespace opts=folia::math::options;
class CandidateMathItem final:public core::AbstractMathItem {
public:
  CandidateMathItem(Property input,core::MathInputJaxReference jax)
    :core::AbstractMathItem(std::move(input),std::move(jax),Property(true)){}
};

opts::Value strings(opts::Context& cx,std::span<const std::u16string_view> values){
  const auto out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;
  for(const auto value:values){const auto n=std::to_string(i++);cx.set(out,opts::Key(std::u16string(n.begin(),n.end())),opts::Value(std::u16string(value)));}
  return opts::Value(out);
}
opts::Value strings(opts::Context& cx,std::initializer_list<std::u16string_view> values){
  return strings(cx,std::span<const std::u16string_view>(values.begin(),values.size()));
}
struct Fixture {
  opts::Context context;
  OptionFunctionBindings functions{context};
  regexp_host::RegExpBindings regex{context};
  Unicode16Normalization unicode;
  core::InputJaxModule input{context};
  core::FindMathModule findMath{context};
  tex::FindTeXModule findTeX{context,findMath,regex};
  tex::ParserRuntime runtime{context,functions,regex,unicode,input,findTeX};
  mml::MmlFactory mml;
  Fixture(){
    runtime.initializeParserRuntime();
    (void)tex::BasePackage::initializeModule(runtime);
    tex::NewcommandConfiguration::initializeModule(runtime);
    (void)tex::BoldsymbolConfiguration::initializeModule(runtime);
    (void)tex::ConfigMacrosConfiguration::initializeModule(runtime);
  }
  tex::TeXRef jax(std::span<const std::u16string_view> packages,opts::Value overrides=opts::Undefined{}){
    opts::Value config=context.object();
    context.set(config,opts::Key(u"packages"),strings(context,packages));
    if(const auto* object=std::get_if<opts::ObjectRef>(&overrides.data)){
      for(const auto& name:context.ownKeys(*object))context.set(config,name,context.get(*object,name));
    }
    auto result=tex::TeX::make(runtime,config);result->setMmlFactory(&mml);return result;
  }
  tex::TeXRef jax(std::initializer_list<std::u16string_view> packages,opts::Value overrides=opts::Undefined{}){
    return jax(std::span<const std::u16string_view>(packages.begin(),packages.size()),std::move(overrides));
  }
  mml::NodeValue compile(tex::TeX& jax,std::u16string source){
    CandidateMathItem item(Property(property_host::debugText(source)),
      core::MathInputJaxReference{static_cast<core::MathInputJaxService*>(&jax)});
    return jax.compile(item,core::MathDocumentReference{PropertyNull{}});
  }
};
void walk(mml::NodeValue value,const std::function<void(mml::MmlNode&)>& visit){
  auto* node=value.requireNode("native package MML result");visit(*node);
  const auto children=node->childNodes;
  for(std::size_t i=0;children&&i<children->size();++i)walk(children->get(i),visit);
}
opts::Value macroOptions(opts::Context& cx){
  const auto options=cx.object(),macros=cx.object(),active=cx.object(),environments=cx.object();
  cx.set(macros,opts::Key(u"foo"),opts::Value(u"x+y"));
  cx.set(macros,opts::Key(u"bar"),opts::Value(cx.array(3)));
  const auto bar=std::get<opts::ObjectRef>(cx.get(macros,opts::Key(u"bar")).data);
  cx.set(bar,opts::Key(u"0"),opts::Value(u"#1+#2"));cx.set(bar,opts::Key(u"1"),opts::Value(2));
  const auto baz=cx.array(3);cx.set(baz,opts::Key(u"0"),opts::Value(u"#1+#2"));
  cx.set(baz,opts::Key(u"1"),opts::Value(2));cx.set(baz,opts::Key(u"2"),opts::Value(u"d"));
  cx.set(macros,opts::Key(u"baz"),opts::Value(baz));
  const auto qux=cx.array(3),templateArgs=cx.array(2);cx.set(qux,opts::Key(u"0"),opts::Value(u"#1+#2"));
  cx.set(qux,opts::Key(u"1"),opts::Value(2));cx.set(templateArgs,opts::Key(u"0"),opts::Value(u""));
  cx.set(templateArgs,opts::Key(u"1"),opts::Value(u","));cx.set(qux,opts::Key(u"2"),opts::Value(templateArgs));
  cx.set(macros,opts::Key(u"qux"),opts::Value(qux));
  cx.set(active,opts::Key(u"~"),opts::Value(u"z+1"));
  const auto env=cx.array(2);cx.set(env,opts::Key(u"0"),opts::Value(u"a+"));cx.set(env,opts::Key(u"1"),opts::Value(u"+b"));
  cx.set(environments,opts::Key(u"xx"),opts::Value(env));
  const auto optionalEnv=cx.array(4);cx.set(optionalEnv,opts::Key(u"0"),opts::Value(u"#1+"));
  cx.set(optionalEnv,opts::Key(u"1"),opts::Value(u"+end"));cx.set(optionalEnv,opts::Key(u"2"),opts::Value(1));
  cx.set(optionalEnv,opts::Key(u"3"),opts::Value(u"d"));cx.set(environments,opts::Key(u"yy"),opts::Value(optionalEnv));
  cx.set(options,opts::Key(u"macros"),opts::Value(macros));cx.set(options,opts::Key(u"active"),opts::Value(active));
  cx.set(options,opts::Key(u"environments"),opts::Value(environments));return opts::Value(options);
}
opts::Value caseOptions(opts::Context& cx,std::u16string_view category,
                        std::u16string_view name,opts::Value definition){
  const auto options=cx.object(),macros=cx.object(),active=cx.object(),environments=cx.object();
  const auto target=category==u"macros"?macros:category==u"active"?active:environments;
  cx.set(target,opts::Key(std::u16string(name)),std::move(definition));
  cx.set(options,opts::Key(u"macros"),opts::Value(macros));
  cx.set(options,opts::Key(u"active"),opts::Value(active));
  cx.set(options,opts::Key(u"environments"),opts::Value(environments));
  return opts::Value(options);
}
opts::Value array(opts::Context& cx,std::initializer_list<opts::Value> values){
  const auto out=cx.array(static_cast<std::uint32_t>(values.size()));std::uint32_t index=0;
  for(const auto& value:values){const auto text=std::to_string(index++);cx.set(out,opts::Key(std::u16string(text.begin(),text.end())),value);}
  return opts::Value(out);
}

suite boldsymbol_configmacros_native=[] {
  "package_callback_metadata_matches_source_lengths_and_expandable_defaults"_test=[] {
    Fixture f;opts::Context& cx=f.context;
    const auto boldValue=f.runtime.configurationHandler.get(u"boldsymbol");
    const auto* bold=std::get_if<tex::ConfigurationRef>(&boldValue);expect(bold&&*bold);
    if(bold&&*bold){
      const auto nodes=(*bold)->field(u"nodes"),creator=cx.get(nodes,opts::Key(u"token"));
      expect((cx.get(creator,opts::Key(u"length"))==opts::Value(4))==true);
      const auto post=(*bold)->field(u"postprocessors"),tuple=cx.get(post,opts::Key(u"0"));
      const auto callback=cx.get(tuple,opts::Key(u"0"));
      expect((cx.get(callback,opts::Key(u"length"))==opts::Value(1))==true);
    }
    const auto configValue=f.runtime.configurationHandler.get(u"configmacros");
    const auto* config=std::get_if<tex::ConfigurationRef>(&configValue);expect(config&&*config);
    if(config&&*config){
      const auto options=(*config)->field(u"options");
      for(const auto name:{u"active",u"macros",u"environments"}){
        const auto value=cx.get(options,opts::Key(name));
        expect(cx.prototype(std::get<opts::ObjectRef>(value.data))==cx.intrinsics().expandablePrototype);
      }
      const auto init=(*config)->field(u"initMethod"),initFunction=cx.get(init,opts::Key(u"0"));
      const auto conf=(*config)->field(u"configMethod"),configFunction=cx.get(conf,opts::Key(u"0"));
      expect((cx.get(initFunction,opts::Key(u"length"))==opts::Value(1))==true);
      expect((cx.get(configFunction,opts::Key(u"length"))==opts::Value(2))==true);
    }
  };
  "boldsymbol_rewrites_tokens_only_after_full_compile_and_removes_fix_property"_test=[] {
    Fixture f;auto jax=f.jax({u"base",u"boldsymbol"});
    const auto configuredFilters=jax->postFilters?jax->postFilters->storageRef():nullptr;
    expect(configuredFilters&&configuredFilters->size()>=8)
      <<"boldsymbol rewrite postprocessor must be registered after the seven TeX filters";
    const auto loaded=f.runtime.configurationHandler.get(u"boldsymbol");
    const auto* configuration=std::get_if<tex::ConfigurationRef>(&loaded);
    bool exactRewriteRegistered=false;
    if(configuration&&*configuration){
      const auto definitions=(*configuration)->field(u"postprocessors");
      const auto tuple=f.context.get(definitions,opts::Key(u"0"));
      const auto callback=f.context.get(tuple,opts::Key(u"0"));
      const auto expected=f.runtime.functions().functionValue(callback);
      const auto storage=jax->postFilters->storageRef();
      for(const auto& record:*storage)if(record&&record->item==expected)exactRewriteRegistered=true;
    }
    expect(exactRewriteRegistered)<<"the configured rewriteBoldTokens identity must be present in TeX.postFilters";
    const auto root=f.compile(*jax,u"\\boldsymbol{x+\\alpha+\\mathit{y}}");
    std::size_t bold=0,stale=0;
    walk(root,[&](mml::MmlNode& node){
      if(node.isKind("mi")&&tex::NodeUtil::instance().getAttribute(mml::NodeValue(&node),"mathvariant")==Property("bold-italic"))++bold;
      if(property_host::truthy(tex::NodeUtil::instance().getProperty(mml::NodeValue(&node),"fixBold")))++stale;
    });
    expect(bold>=2);expect(stale==0);
  };
  "boldsymbol_excludes_mtext_and_preserves_unmapped_variants"_test=[] {
    Fixture f;auto jax=f.jax({u"base",u"boldsymbol"});
    const auto root=f.compile(*jax,u"\\boldsymbol{\\text{abc}+\\mathbb{R}}");
    bool sawText=false,sawDoubleStruck=false;
    walk(root,[&](mml::MmlNode& node){
      if(node.isKind("mtext")){sawText=true;expect(tex::NodeUtil::instance().getAttribute(mml::NodeValue(&node),"mathvariant")==Property("normal"));}
      if(node.isKind("mi")&&tex::NodeUtil::instance().getAttribute(mml::NodeValue(&node),"mathvariant")==Property("double-struck"))sawDoubleStruck=true;
    });
    expect(sawText);expect(sawDoubleStruck);
  };
  "configmacros_installs_live_definitions_active_map_and_environments"_test=[] {
    Fixture f;auto jax=f.jax({u"base",u"newcommand",u"configmacros"},macroOptions(f.context));
    const auto root=f.compile(*jax,u"\\foo+\\bar{a}{b}+\\baz{x}+\\qux a,b+~+\\begin{xx}x\\end{xx}+\\begin{yy}x\\end{yy}");
    expect(root.requireNode("compiled TeX root")->isKind("math"));
    std::size_t errors=0;
    walk(root,[&](mml::MmlNode& node){if(node.isKind("merror"))++errors;});
    expect(errors==0);
  };
  "configmacros_options_are_owned_by_each_actual_tex_instance"_test=[] {
    Fixture f;auto configured=f.jax({u"base",u"newcommand",u"configmacros"},macroOptions(f.context));
    auto plain=f.jax({u"base",u"newcommand",u"configmacros"});
    const auto first=f.compile(*configured,u"\\foo"),second=f.compile(*plain,u"\\foo");
    bool configuredX=false,plainError=false;
    walk(first,[&](mml::MmlNode& node){if(node.isKind("mi")&&tex::NodeUtil::instance().getText(mml::NodeValue(&node))==u"x")configuredX=true;});
    walk(second,[&](mml::MmlNode& node){if(node.isKind("merror"))plainError=true;});
    expect(configuredX);expect(plainError);
  };
  "configmacros_user_macros_do_not_require_selecting_newcommand_package"_test=[] {
    Fixture f;const auto options=f.context.object(),macros=f.context.object();
    f.context.set(macros,opts::Key(u"foo"),opts::Value(u"x+y"));
    f.context.set(options,opts::Key(u"macros"),opts::Value(macros));
    auto jax=f.jax({u"base",u"configmacros"},opts::Value(options));
    const auto root=f.compile(*jax,u"\\foo");std::size_t errors=0;
    walk(root,[&](mml::MmlNode& node){if(node.isKind("merror"))++errors;});expect(errors==0);
  };
  "all_locked_original_inputs_run_through_actual_tex_compile"_test=[] {
    Fixture f;
    struct Case { std::u16string_view input; std::vector<std::u16string_view> packages; opts::Value options; bool error; };
    const auto macro=[&](std::u16string_view input,opts::Value value,bool error=false){
      return Case{input,{u"base",u"newcommand",u"configmacros"},caseOptions(f.context,u"macros",u"foo",std::move(value)),error};
    };
    const auto bold=[&](std::u16string_view input){return Case{input,{u"base",u"boldsymbol"},opts::Undefined{},false};};
    const auto config=[&](std::u16string_view input,std::u16string_view category,std::u16string_view name,opts::Value value){
      return Case{input,{u"base",u"configmacros"},caseOptions(f.context,category,name,std::move(value)),false};
    };
    const auto xx=array(f.context,{opts::Value(u"a+"),opts::Value(u"+b")});
    const auto yy=array(f.context,{opts::Value(u"#1+"),opts::Value(u"+end"),opts::Value(1),opts::Value(u"d")});
    const auto cases=std::array<Case,16>{
      bold(u"\\boldsymbol{x}"),bold(u"\\boldsymbol{\\alpha+x}"),
      bold(u"\\boldsymbol{x+\\boldsymbol{y}}+z"),bold(u"\\boldsymbol{\\text{abc}+x}"),
      bold(u"\\boldsymbol{\\mathit{x}+\\mathfrak{y}+\\mathcal{z}}"),bold(u"\\boldsymbol{\\mathbb{R}}"),
      macro(u"\\foo",opts::Value(u"x+y")),macro(u"\\foo",array(f.context,{opts::Value(u"x+y")})),
      macro(u"\\foo{a}{b}",array(f.context,{opts::Value(2),opts::Value(u"#1+#2")})),
      macro(u"\\foo{x}",array(f.context,{opts::Value(2),opts::Value(u"#1+#2"),opts::Value(u"d")})),
      macro(u"\\foo a,b",array(f.context,{opts::Value(u"#1+#2"),opts::Value(2),array(f.context,{opts::Value(u""),opts::Value(u",")})})),
      config(u"~",u"active",u"~",opts::Value(u"x+y")),
      config(u"\\begin{xx}x\\end{xx}",u"environments",u"xx",xx),
      config(u"\\begin{xx}{q}x\\end{xx}",u"environments",u"xx",yy),
      macro(u"\\foo",array(f.context,{opts::Value(4),opts::Value(u"#1")})),
      Case{u"\\foo",{u"base",u"configmacros"},caseOptions(f.context,u"macros",u"foo",opts::Value(u"x+y")),false}
    };
    for(const auto& row:cases){
      auto jax=f.jax(row.packages,row.options);
      const auto root=f.compile(*jax,std::u16string(row.input));
      expect(root.isNode());expect(tex::NodeUtil::instance().isType(root,"math"));
      std::size_t errors=0;walk(root,[&](mml::MmlNode& node){if(node.isKind("merror"))++errors;});
      expect(row.error?errors>0:errors==0);
    }
  };
};
} // namespace
