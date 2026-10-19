#include "support/folia_test.hpp"
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.util.option_types;

namespace {
using namespace boost::ut;
using namespace folia::math::options;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

struct ProcessMathTestHost {
    std::vector<std::string> requests;
    std::vector<std::any> warnings;
};

common::FontLoadHost processMathHost(const std::shared_ptr<ProcessMathTestHost>& trace){
    common::FontLoadHost host;
    host.asyncLoad=[trace](std::shared_ptr<common::FontData>,common::DynamicFileRef,
                           std::string request)->folia::math::async::PromiseRef{
        trace->requests.push_back(std::move(request));
        throw Exception("Unimplemented",u"Unexpected dynamic font request in processMath test");
    };
    host.syncLoad=[trace](common::FontData&,const common::DynamicFileRef&,std::string request){
        trace->requests.push_back(std::move(request));
        throw Exception("Unimplemented",u"Unexpected synchronous font request in processMath test");
    };
    host.asyncIsSynchronous=[] {return false;};
    host.warn=[trace](const std::any& warning){trace->warnings.push_back(warning);};
    return host;
}

constexpr std::array<std::u16string_view,38> sourcePackages={
    u"base",u"action",u"ams",u"amscd",u"bbm",u"bboldx",u"bbox",u"begingroup",
    u"boldsymbol",u"braket",u"bussproofs",u"cancel",u"cases",u"centernot",
    u"color",u"colortbl",u"configmacros",u"dsfont",u"empheq",u"enclose",
    u"extpfeil",u"fontsizev3",u"gensymb",u"html",u"mathtools",u"mhchem",
    u"newcommand",u"noundefined",u"physics",u"setoptions",u"tagformat",u"texhtml",
    u"textcomp",u"textmacros",u"unicode",u"units",u"upgreek",u"verb"};

suite svg_processmath_methods = [] {
"svg_output_processmath_matches_pinned_cjs_method_metadata"_test=[] {
    auto trace=std::make_shared<ProcessMathTestHost>();
    svg::NativeRenderSession session(processMathHost(trace));
    auto& context=session.context();
    const auto options=context.object();
    context.set(Value(options),Key(u"fontCache"),Value(u"none"));
    auto jax=session.construct(Value(options));
    const auto prototype=context.prototype(jax->instanceIdentity());
    expect(prototype.has_value());
    const auto descriptor=context.descriptor(Value(*prototype),Key(u"processMath"));
    expect(descriptor.has_value());
    expect(descriptor->writable);
    expect(descriptor->enumerable);
    expect(descriptor->configurable);
    const auto* function=std::get_if<ObjectRef>(&descriptor->value.data);
    expect(function&&context.kind(*function)==ObjectKind::function);
    expect(context.get(Value(*function),Key(u"name"))==Value(u""));
    expect(context.toNumber(context.get(Value(*function),Key(u"length")))==2.0);
};

"svg_maction_mathtip_reaches_source_event_handler_error"_test=[] {
    auto trace=std::make_shared<ProcessMathTestHost>();
    auto host=processMathHost(trace);
    host.asyncIsSynchronous=[] {return true;};
    svg::NativeRenderSession session(std::move(host));
    auto& context=session.context();
    const auto jaxOptions=context.object();
    context.set(Value(jaxOptions),Key(u"fontCache"),Value(u"none"));
    (void)session.construct(Value(jaxOptions));
    const auto inputOptions=context.object();
    const auto packages=context.array(static_cast<std::uint32_t>(sourcePackages.size()));
    for(std::size_t index=0;index<sourcePackages.size();++index){
        const auto key=std::to_string(index);
        context.set(Value(packages),Key(std::u16string(key.begin(),key.end())),
            Value(std::u16string(sourcePackages[index])));
    }
    context.set(Value(inputOptions),Key(u"packages"),Value(packages));
    auto document=svg::createNativeSvgDocument(session,Undefined{},Value(inputOptions));
    auto& cx=document->sourceContext();
    const auto settings=cx.object();
    cx.set(Value(settings),Key(u"display"),Value(false));
    cx.set(Value(settings),Key(u"em"),Value(16.0));
    cx.set(Value(settings),Key(u"ex"),Value(8.0));
    cx.set(Value(settings),Key(u"containerWidth"),Value(1280.0));
    bool reachedSourceEventHandler=false;
    try{
        (void)document->convert(Value(u"\\mathtip{x+1}{y^2}"),Value(settings));
    }catch(const Exception& error){
        reachedSourceEventHandler=error.name=="TypeError"&&
            error.message==u"node.addEventListener is not a function";
    }
    expect(reachedSourceEventHandler);
    expect(trace->requests.empty());
};

"svg_maction_texttip_renders_source_title"_test=[] {
    auto trace=std::make_shared<ProcessMathTestHost>();
    auto host=processMathHost(trace);
    host.asyncIsSynchronous=[] {return true;};
    svg::NativeRenderSession session(std::move(host));
    auto& context=session.context();
    const auto jaxOptions=context.object();
    context.set(Value(jaxOptions),Key(u"fontCache"),Value(u"none"));
    (void)session.construct(Value(jaxOptions));
    const auto inputOptions=context.object();
    const auto packages=context.array(static_cast<std::uint32_t>(sourcePackages.size()));
    for(std::size_t index=0;index<sourcePackages.size();++index){
        const auto key=std::to_string(index);
        context.set(Value(packages),Key(std::u16string(key.begin(),key.end())),
            Value(std::u16string(sourcePackages[index])));
    }
    context.set(Value(inputOptions),Key(u"packages"),Value(packages));
    auto document=svg::createNativeSvgDocument(session,Undefined{},Value(inputOptions));
    auto& cx=document->sourceContext();
    const auto settings=cx.object();
    cx.set(Value(settings),Key(u"display"),Value(false));
    cx.set(Value(settings),Key(u"em"),Value(16.0));
    cx.set(Value(settings),Key(u"ex"),Value(8.0));
    cx.set(Value(settings),Key(u"containerWidth"),Value(1280.0));
    const auto result=document->convert(Value(u"\\texttip{x}{tip}"),Value(settings));
    const auto dom=folia::math::propertyNode(document->propertyBindings().from(result));
    const auto* node=std::get_if<folia::math::DOMNodeRef>(&dom);
    expect(node&&*node);
    const auto xml=document->domAdaptor().serializeXML(*node);
    expect(xml.find(u"<title>tip</title>")!=std::u16string::npos);
    expect(trace->requests.empty());
};
};
}
