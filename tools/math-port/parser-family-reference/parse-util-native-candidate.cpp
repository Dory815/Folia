// Focused native behavior candidates for the independently locked
// parse-util-original-source.json observations. Uses actual TeX, ParseOptions,
// Stack, BaseItems, NodeFactory and MmlFactory owners.
#include "../../../tests/support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.input.tex.tex_error;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace tex=folia::math::tex;
namespace opts=folia::math::options;

opts::Value strings(opts::Context& context,std::initializer_list<std::u16string_view> values){
    const auto array=context.array(static_cast<std::uint32_t>(values.size()));
    std::uint32_t index=0;
    for(const auto value:values){
        const auto digits=std::to_string(index++);
        context.set(array,opts::Key(std::u16string(digits.begin(),digits.end())),opts::Value(std::u16string(value)));
    }
    return opts::Value(array);
}
struct ParseUtilRuntime {
    opts::Context context;
    OptionFunctionBindings functions{context};
    regexp_host::RegExpBindings regex{context};
    Unicode16Normalization unicode;
    core::InputJaxModule input{context};
    core::FindMathModule findMath{context};
    tex::FindTeXModule findTeX{context,findMath,regex};
    tex::ParserRuntime runtime{context,functions,regex,unicode,input,findTeX};
    std::shared_ptr<mml::MmlFactory> factory=std::make_shared<mml::MmlFactory>();
    tex::TeXRef jax;
    ParseUtilRuntime(){
        runtime.initializeParserRuntime();tex::BasePackage::initializeModule(runtime);
        const auto config=context.object();context.set(config,opts::Key(u"packages"),strings(context,{u"base"}));
        jax=tex::TeX::make(runtime,opts::Value(config));jax->setMmlFactory(factory);
    }
    tex::ParseOptionsRef options()const{
        const auto parsed=jax->parseOptions();const auto* value=std::get_if<tex::ParseOptionsRef>(&parsed);
        if(!value||!*value)throw std::runtime_error("actual TeX ParseOptions unavailable");return *value;
    }
    std::shared_ptr<tex::TexParser> parser(){
        return tex::TexParser::make(std::u16string{},PropertyList{},*options());
    }
};

suite parse_util_native_candidate=[] {
    "parseutil_cols_matches_locked_unit_em_observations"_test=[] {
        const std::array<double,4> widths{0,1,-0.5,2.25};
        const std::array<double,0> empty{};
        expect(tex::ParseUtil::cols(widths)==u"0em 1em -0.5em 2.25em");
        expect(tex::ParseUtil::cols(empty).empty());
    };
    "parseutil_keyval_options_matches_source_boolean_and_l3_brace_rules"_test=[] {
        ParseUtilRuntime fixture;
        const auto parsed=tex::ParseUtil::keyvalOptions(fixture.runtime,u"a={x},b=2",opts::Value(opts::Null{}));
        const auto* record=std::get_if<std::shared_ptr<PropertyStorage>>(&parsed.value);
        expect(record&&*record);
        if(record&&*record){
            const auto values=PropertyList::record(parsed).value();
            expect(values.get("a")==Property("x"));
            expect(values.get("b")==Property("2"));
        }
        const auto l3=tex::ParseUtil::keyvalOptions(fixture.runtime,u"x={{ a,b }}",opts::Value(opts::Null{}),false,true);
        const auto l3Record=PropertyList::record(l3).value();
        expect(l3Record.get("x")==Property("{ a,b }"));
        bool gotSourceError=false;
        try{(void)tex::ParseUtil::keyvalOptions(fixture.runtime,u"x=1}",opts::Value(opts::Null{}));}
        catch(const tex::TexError& error){gotSourceError=error.id==u"ExtraCloseMissingOpen";}
        expect(gotSourceError);
    };
    "parseutil_substitute_args_and_max_buffer_preserve_argument_and_limit_semantics"_test=[] {
        ParseUtilRuntime fixture;auto parser=fixture.parser();
        const auto args=fixture.context.array(2);
        fixture.context.set(args,opts::Key(u"0"),opts::Value(u"\\alpha"));
        fixture.context.set(args,opts::Key(u"1"),opts::Value(u"beta"));
        expect(tex::ParseUtil::substituteArgs(*parser,opts::Value(args),std::u16string(u"#1+#2"))==u"\\alpha+beta");
        const auto parserOptions=parser->options();
        const auto saved=fixture.context.get(parserOptions,opts::Key(u"maxBuffer"));
        fixture.context.set(parserOptions,opts::Key(u"maxBuffer"),opts::Value(2.0));
        bool limited=false;
        try{(void)tex::ParseUtil::addArgs(*parser,std::u16string(u"ab"),std::u16string(u"c"));}
        catch(const tex::TexError& error){limited=error.id==u"MaxBufferSize";}
        expect(limited);
        fixture.context.set(parserOptions,opts::Key(u"maxBuffer"),saved);
    };
    "parseutil_internal_text_and_array_alignment_use_real_node_and_item_owners"_test=[] {
        ParseUtilRuntime fixture;auto parser=fixture.parser();
        const auto text=tex::ParseUtil::internalText(*parser,u" hello\nworld ",Property(PropertyList{{"mathvariant","bold"}}));
        expect(eq(text.requireNode("kind")->kind(),std::string("mtext")));
        const auto itemFactory=parser->itemFactory();
        const auto* factory=std::get_if<std::shared_ptr<tex::StackItemFactory>>(&itemFactory);
        expect(factory&&*factory);
        if(!factory||!*factory)return;
        auto* raw=(*factory)->create("array",{});
        auto* item=dynamic_cast<tex::ArrayItem*>(raw);
        expect(item!=nullptr);if(!item)return;
        auto& same=tex::ParseUtil::setArrayAlign(*item,u"t");
        expect(&same==item);
        const auto alignValue=fixture.context.get(item->arraydef,opts::Key(u"align"));
        const auto* align=std::get_if<std::u16string>(&alignValue.data);
        expect(align&&*align==u"baseline 1");
    };
    "parseutil_internal_math_splits_real_math_text_and_reports_unterminated_delimiters"_test=[] {
        ParseUtilRuntime fixture;auto parser=fixture.parser();
        const auto traced=[&](std::u16string source){
            try{return tex::ParseUtil::internalMath(*parser,source);}
            catch(const opts::Exception& error){throw opts::Exception(error.name,
                u"ParseUtil.internalMath input="+source+u"; "+error.message);}
            catch(const tex::TexError& error){throw opts::Exception("NativeTest",
                u"ParseUtil.internalMath input="+source+u"; "+error.id+u": "+error.message);}
            catch(...){throw opts::Exception("NativeTest",
                u"ParseUtil.internalMath input="+source+u"; unknown native exception");}
        };
        const auto children=traced(u"before $x$ after");
        expect(children&&children->size()==1)<<"internalMath result child count="<<(children?children->size():0);
        if(children&&children->size()==1){
            const auto row=children->get(0);
            expect(eq(row.requireNode("kind")->kind(),std::string("mrow")))<<"internalMath wraps mixed text/math in mrow";
            const auto rowChildren=row.requireNode("childNodes")->childNodes;
            expect(rowChildren&&rowChildren->size()==3)<<"internalMath mrow child count="<<(rowChildren?rowChildren->size():0);
            if(rowChildren&&rowChildren->size()==3){
                expect(eq(rowChildren->get(0).requireNode("kind")->kind(),std::string("mtext")))<<"internalMath text child0";
                expect(eq(rowChildren->get(1).requireNode("kind")->kind(),std::string("TeXAtom")))<<"internalMath math child";
                expect(eq(rowChildren->get(2).requireNode("kind")->kind(),std::string("mtext")))<<"internalMath text child2";
            }
        }
        bool unterminated=false;
        try{(void)tex::ParseUtil::internalMath(*parser,u"bad $x");}
        catch(const tex::TexError& error){unterminated=error.id==u"MathNotTerminated";}
        expect(unterminated)<<"unterminated delimiter error";
        const auto unicode=traced(u"\\U{1F600}");
        expect(unicode&&unicode->size()==1)<<"Unicode internalMath child count="<<(unicode?unicode->size():0);
    };
    "parseutil_movablelimits_clears_the_actual_node_property"_test=[] {
        ParseUtilRuntime fixture;auto parser=fixture.parser();
        const std::array<std::any,3> args{
            std::any(Property("mo")),std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>()),
            std::any(Property(PropertyList{{"movablelimits",true}}))};
        const auto base=parser->create(u"node",args);
        tex::ParseUtil::checkMovableLimits(base);
        const Property value=base.requireNode("properties")->getProperty("movablelimits");
        expect(value==Property(false));
    };
    "parseutil_max_macro_errors_and_unicode_normalized_script_characters"_test=[] {
        ParseUtilRuntime fixture;auto parser=fixture.parser();
        fixture.context.set(parser->options(),opts::Key(u"maxMacros"),opts::Value(0.0));
        bool macroError=false;
        try{tex::ParseUtil::checkMaxMacros(*parser);}
        catch(const tex::TexError& error){macroError=error.id==u"MaxMacroSub1";}
        expect(macroError);
        expect(tex::ParseUtil::isLatinOrGreekChar(fixture.runtime,u"é"));
        expect(tex::ParseUtil::isLatinOrGreekChar(fixture.runtime,u"Ω"));
        expect(!tex::ParseUtil::isLatinOrGreekChar(fixture.runtime,u"𝑥"));
        expect(!tex::ParseUtil::isLatinOrGreekChar(fixture.runtime,u"中"));
    };
};
}
