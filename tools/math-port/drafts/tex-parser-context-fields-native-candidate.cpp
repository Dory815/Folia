// Focused native candidate for the TexParser/TextParser Source-own-field patch.
// Run only after the shared StackContextBindings integration is enrolled.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.textmacros.configuration;
import folia.math.input.tex.textmacros.methods;
import folia.math.input.tex.textcomp.configuration;
import folia.math.input.tex.tex_parser_context;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.handlers.html.html_math_document;
import folia.math.input.tex.find_tex_host;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.output.common.font_data_surface;
import folia.math.util.options;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;

namespace {
namespace tex=folia::math::tex;
namespace opts=folia::math::options;
namespace svg=folia::math::output::svg;
namespace common=folia::math::output::common;
namespace core=folia::math::core;
using namespace folia::math;

common::FontLoadHost fontHost(){
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&){};
    return host;
}
opts::ObjectRef packageList(opts::Context& cx){
    const auto result=cx.array(2);
    cx.set(result,opts::Key(u"0"),opts::Value(u"base"));
    cx.set(result,opts::Key(u"1"),opts::Value(u"textmacros"));
    return result;
}
}
int main(){
    try{
        svg::NativeRenderSession session(fontHost());
        auto& cx=session.context();
        auto& runtime=session.services().parser();
        (void)tex::BasePackage::initializeModule(runtime);
        (void)tex::TextMacrosConfiguration::initializeModule(runtime);
        (void)tex::TextcompConfiguration::initializeModule(runtime);
        const auto inputJaxOptions=cx.object();
        cx.set(inputJaxOptions,opts::Key(u"packages"),opts::Value(packageList(cx)));
        auto document=svg::createNativeSvgDocument(session,opts::Undefined{},opts::Value(inputJaxOptions));
        const auto jaxArray=cx.get(opts::Value(document->sourceIdentity()),opts::Key(u"inputJax"));
        const auto jaxValue=cx.get(jaxArray,opts::Key(u"0"));
        const auto* jaxIdentity=std::get_if<opts::ObjectRef>(&jaxValue.data);
        if(!jaxIdentity)throw std::runtime_error("document input jax identity missing");
        const auto jax=runtime.texIdentities.resolve(*jaxIdentity);
        const auto parseValue=jax->parseOptions();
        const auto* parseOwner=std::get_if<tex::ParseOptionsRef>(&parseValue);
        if(!parseOwner||!*parseOwner)throw std::runtime_error("TeX ParseOptions owner missing");
        auto parser=tex::TextParser::make(runtime,u"",Property(PropertyList{}),**parseOwner);
        const auto parserIdentity=parser->instanceIdentity();
        const std::vector<std::u16string> expectedOwnKeys{
            u"_string",u"configuration",u"macroCount",u"i",u"currentCS",u"saveI",
            u"stack",u"text",u"nodes",u"envStack",u"level"};
        if(cx.enumerableStringKeys(opts::Value(parserIdentity))!=expectedOwnKeys)
            throw std::runtime_error("TexParser/TextParser own fields do not follow Source initialization order");
        const auto parseIdentity=runtime.parseOptionsIdentities.find(*parseOwner);
        if(!parseIdentity)throw std::runtime_error("ParseOptions identity missing");
        if(cx.get(opts::Value(parserIdentity),opts::Key(u"configuration"))!=opts::Value(*parseIdentity))
            throw std::runtime_error("TexParser.configuration lost its canonical ParseOptions owner");
        const auto boundParser=runtime.parserIdentities.find(std::static_pointer_cast<tex::TexParser>(parser));
        if(!boundParser||*boundParser!=parserIdentity)throw std::runtime_error("TextParser identity is not the same parser owner");
        const auto stackOwner=std::get<std::shared_ptr<tex::Stack>>(parser->stack);
        const auto stackIdentity=runtime.bindStack(stackOwner);
        const auto factoryIdentity=cx.get(opts::Value(stackIdentity),opts::Key(u"_factory"));
        const auto configFactoryIdentity=cx.get(opts::Value(*parseIdentity),opts::Key(u"itemFactory"));
        if(factoryIdentity!=configFactoryIdentity)throw std::runtime_error("Stack._factory lost the actual ParseOptions.itemFactory alias");
        if(cx.get(opts::Value(parserIdentity),opts::Key(u"stack"))!=opts::Value(stackIdentity))
            throw std::runtime_error("TexParser.stack does not expose the actual Stack identity");
        const auto nodeIdentity=runtime.nodeArrayIdentities.find(parser->nodes());
        if(!nodeIdentity||cx.get(opts::Value(parserIdentity),opts::Key(u"nodes"))!=opts::Value(*nodeIdentity))
            throw std::runtime_error("TextParser.nodes does not expose its canonical node-array identity");
        const auto envIdentityValue=cx.get(opts::Value(parserIdentity),opts::Key(u"envStack"));
        const auto* envIdentity=std::get_if<opts::ObjectRef>(&envIdentityValue.data);
        if(!envIdentity)throw std::runtime_error("TextParser.envStack identity missing");
        const auto stack=std::get<std::shared_ptr<tex::Stack>>(parser->stack);
        const Property originalEnvironment=stack->env();
        const auto replacement=cx.array(1);
        cx.set(replacement,opts::Key(u"0"),runtime.properties.to(originalEnvironment));
        cx.set(opts::Value(parserIdentity),opts::Key(u"envStack"),opts::Value(replacement));
        const auto currentEnvValue=cx.get(opts::Value(parserIdentity),opts::Key(u"envStack"));
        const auto* currentEnvIdentity=std::get_if<opts::ObjectRef>(&currentEnvValue.data);
        if(!currentEnvIdentity||*currentEnvIdentity!=replacement)
            throw std::runtime_error("TextParser.envStack setter did not retain the exact assigned array");
        const auto openBrace=std::get<FunctionRef>(tex::TextMacrosMethods::get(runtime,u"OpenBrace").value);
        const auto closeBrace=std::get<FunctionRef>(tex::TextMacrosMethods::get(runtime,u"CloseBrace").value);
        (void)tex::invokeParseMethod(FunctionValue{openBrace},Missing{},*parser,Property("{"),{});
        const auto openedLength=cx.arrayLength(replacement);
        if(openedLength!=2)throw std::runtime_error("TextMacros OpenBrace did not push to the exact replacement array");
        (void)tex::invokeParseMethod(FunctionValue{closeBrace},Missing{},*parser,Property("}"),{});
        const auto closedLength=cx.arrayLength(replacement);
        if(closedLength!=1||cx.get(opts::Value(replacement),opts::Key(u"0"))!=runtime.properties.to(originalEnvironment))
            throw std::runtime_error("TextMacros CloseBrace did not pop from the exact replacement array");
        cx.set(opts::Value(parserIdentity),opts::Key(u"i"),opts::Value(7));
        cx.set(opts::Value(parserIdentity),opts::Key(u"_string"),opts::Value(u"updated"));
        cx.set(opts::Value(parserIdentity),opts::Key(u"text"),opts::Value(u"live text"));
        if(parser->i!=7||parser->string()!=u"updated"||parser->text()!=u"live text")
            throw std::runtime_error("TexParser/TextParser Context fields did not mutate actual native storage");
        parser->Parse();
        const auto nextEnvValue=cx.get(opts::Value(parserIdentity),opts::Key(u"envStack"));
        const auto* nextEnvIdentity=std::get_if<opts::ObjectRef>(&nextEnvValue.data);
        if(!nextEnvIdentity||*nextEnvIdentity==replacement)
            throw std::runtime_error("Repeated TextParser.Parse did not publish a fresh envStack array identity");
        if(cx.arrayLength(replacement)!=1||cx.get(opts::Value(replacement),opts::Key(u"0"))!=runtime.properties.to(originalEnvironment))
            throw std::runtime_error("Repeated Parse detached or mutated the previous replacement-array alias");
        if(cx.arrayLength(*nextEnvIdentity)!=0)
            throw std::runtime_error("Repeated TextParser.Parse did not clear the actual new envStack owner");
        const auto nextNodes=runtime.nodeArrayIdentities.find(parser->nodes());
        if(!nextNodes||cx.get(opts::Value(parserIdentity),opts::Key(u"nodes"))!=opts::Value(*nextNodes))
            throw std::runtime_error("Repeated TextParser.Parse did not publish its new canonical nodes owner");
        const bool oldFirstIsOriginal=cx.get(opts::Value(replacement),opts::Key(u"0"))==runtime.properties.to(originalEnvironment);
        std::cout<<"{\"openLength\":"<<openedLength<<",\"closeLength\":"<<closedLength<<",\"oldLength\":"
                 <<cx.arrayLength(replacement)<<",\"oldFirstIsOriginal\":"<<(oldFirstIsOriginal?"true":"false")
                 <<",\"newLength\":"<<cx.arrayLength(*nextEnvIdentity)<<",\"freshIdentity\":"
                 <<(replacement!=*nextEnvIdentity?"true":"false")<<"}\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<error.what()<<'\n';
        return 1;
    }
}
