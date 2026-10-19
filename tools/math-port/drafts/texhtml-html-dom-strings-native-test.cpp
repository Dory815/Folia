#include "../../../tests/support/folia_test.hpp"
import folia.math.adaptors.lite.lite_module;
import folia.math.handlers.html.html_dom_strings;
import folia.math.input.tex.texhtml.tex_html_configuration;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.util.context_property_bindings;
import folia.math.util.option_functions;
import folia.math.util.option_types;
import folia.math.util.regexp_binding;
import folia.math.util.runtime_modules;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::options;
using folia::math::adaptors::lite::LiteModule;
using folia::math::handlers::html::HTMLDomStringsModule;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;
namespace tex=folia::math::tex;

common::FontLoadHost texhtmlTestFontHost(){
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&){};
    return host;
}

suite texhtmlHtmlDomStrings=[] {
"uses_source_options_and_the_active_lite_dom"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexps=std::make_shared<regexp_host::RegExpBindings>(context);
    LiteModule lite(context,functions,properties,regexps);
    util::RuntimeModules modules(context);
    auto htmlStrings=HTMLDomStringsModule::initializeModule(modules,properties,*regexps);
    const auto sourceClass=htmlStrings->identity();
    const auto staticOptions=context.get(Value(sourceClass),Key(u"OPTIONS"));
    const auto skip=context.get(staticOptions,Key(u"skipHtmlTags"));
    expect(context.toNumber(context.get(skip,Key(u"length")))==10.0);
    const std::array<std::u16string_view,10> skipped{
        u"script",u"noscript",u"style",u"textarea",u"pre",u"code",u"math",
        u"select",u"option",u"mjx-container"};
    for(std::size_t i=0;i<skipped.size();++i){
        const auto digits=std::to_string(i);
        expect(context.get(skip,Key(std::u16string(digits.begin(),digits.end())))==Value(std::u16string(skipped[i])));
    }
    expect(context.get(staticOptions,Key(u"ignoreHtmlClass"))==Value(u"mathjax_ignore"));
    expect(context.get(staticOptions,Key(u"processHtmlClass"))==Value(u"mathjax_process"));
    const auto include=context.get(staticOptions,Key(u"includeHtmlTags"));
    expect(context.ownKeys(include).size()==3);
    expect(context.get(include,Key(u"br"))==Value(u"\n"));
    expect(context.get(include,Key(u"wbr"))==Value(u""));
    expect(context.get(include,Key(u"#comment"))==Value(u""));

    context.set(include,Key(u"tex-html"),Value(u"custom"));
    htmlStrings->installTexHtmlHook();
    expect(context.get(include,Key(u"tex-html"))==Value(u"custom"));
    context.set(include,Key(u"tex-html"),Value(Undefined{}));
    htmlStrings->installTexHtmlHook();
    const auto hook=context.get(include,Key(u"tex-html"));
    const auto* hookFunction=std::get_if<ObjectRef>(&hook.data);
    expect(hookFunction&&context.kind(*hookFunction)==ObjectKind::function);
    expect(context.toNumber(context.get(hook,Key(u"length")))==2.0);
    const auto constructorLength=context.get(Value(sourceClass),Key(u"length"));
    expect(context.toNumber(constructorLength)==0.0);
    expect(context.get(hook,Key(u"name"))==Value(u""));

    auto adaptor=lite.constructAdaptor();
    const auto adaptorIdentity=lite.bindAdaptor(adaptor);
    const auto instance=htmlStrings->create();
    instance->setAdaptor(Value(adaptorIdentity),adaptor);
    const auto instanceOptions=instance->optionsValue();
    const auto instanceIncludes=context.get(instanceOptions,Key(u"includeHtmlTags"));
    expect(context.ownKeys(instanceOptions).size()==4);
    expect(context.get(instanceOptions,Key(u"ignoreHtmlClass"))==Value(u"mathjax_ignore"));
    expect(context.get(instanceOptions,Key(u"processHtmlClass"))==Value(u"mathjax_process"));
    expect(context.get(instanceIncludes,Key(u"tex-html"))==hook);
    expect(context.ownKeys(instanceIncludes).size()==4);
    const Value adaptorValue(adaptorIdentity);
    const std::array<Value,2> parseArgs{Value(u"<div>before<br>after</div>"),Value(u"text/html")};
    const Value parsed=context.callMember(context.get(adaptorValue,Key(u"parse")),adaptorValue,parseArgs,u"adaptor.parse");
    const std::array<Value,1> bodyArgs{parsed};
    const Value body=context.callMember(context.get(adaptorValue,Key(u"body")),adaptorValue,bodyArgs,u"adaptor.body");
    const std::array<Value,1> childArgs{body};
    const Value children=context.callMember(context.get(adaptorValue,Key(u"childNodes")),adaptorValue,childArgs,u"adaptor.childNodes");
    const Value hookNode=context.get(children,Key(u"0"));
    const auto rootValue=propertyNode(properties.from(hookNode));
    const auto* root=std::get_if<DOMNodeRef>(&rootValue);
    expect(root!=nullptr);
    const std::array<Value,2> hookArgs{hookNode,Value(adaptorIdentity)};
    expect(context.call(*hookFunction,Value(Undefined{}),hookArgs)==Value(u"<tex-html>before<br>after</tex-html>"));
    const Value instanceIdentity(instance->identity());
    const std::array<Value,1> findArgs{hookNode};
    const Value findMethod=context.get(instanceIdentity,Key(u"find"));
    const Value firstResult=context.callMember(findMethod,instanceIdentity,findArgs,u"domStrings.find");
    const Value firstStrings=context.get(firstResult,Key(u"0"));
    const Value firstNodes=context.get(firstResult,Key(u"1"));
    expect(std::get<ObjectRef>(firstStrings.data)!=
        std::get<ObjectRef>(context.get(instanceIdentity,Key(u"strings")).data));
    expect(std::get<ObjectRef>(firstNodes.data)!=
        std::get<ObjectRef>(context.get(instanceIdentity,Key(u"nodes")).data));
    expect(context.toNumber(context.get(firstStrings,Key(u"length")))==1.0);
    expect(context.get(firstStrings,Key(u"0"))==Value(u"before\nafter"));
    expect(context.toNumber(context.get(firstNodes,Key(u"length")))==1.0);
    const Value firstNodeList=context.get(firstNodes,Key(u"0"));
    expect(context.toNumber(context.get(firstNodeList,Key(u"length")))==3.0);
    const std::array<std::u16string_view,3> firstKinds{u"#text",u"br",u"#text"};
    const std::array<std::u16string_view,3> firstText{u"before",u"",u"after"};
    const std::array<std::u16string_view,3> firstMarkup{u"",u"<br>",u""};
    const std::array<std::size_t,3> firstLengths{6,1,5};
    for(std::size_t i=0;i<firstKinds.size();++i){
        const auto digits=std::to_string(i);
        const Value entry=context.get(firstNodeList,Key(std::u16string(digits.begin(),digits.end())));
        const auto nodeValue=propertyNode(properties.from(context.get(entry,Key(u"0"))));
        const auto* node=std::get_if<DOMNodeRef>(&nodeValue);
        expect(node!=nullptr);
        if(!node)continue;
        expect(adaptor->kind(*node)==firstKinds[i]);
        if(firstKinds[i]==u"#text")expect(adaptor->value(*node)==firstText[i]);
        else expect(adaptor->textContent(*node)==firstText[i]);
        if(firstKinds[i]==u"#text")expect(adaptor->value(*node)==firstText[i]);
        else expect(adaptor->outerHTML(*node)==firstMarkup[i]);
        expect(context.get(entry,Key(u"1"))==Value(static_cast<double>(firstLengths[i])));
    }

    const std::array<Value,2> nestedParseArgs{
        Value(u"<div>before<br>middle<tex-html><b>inner</b><tex-html><i>deep</i></tex-html></tex-html>after</div>"),
        Value(u"text/html")};
    const Value nestedDocument=context.callMember(context.get(adaptorValue,Key(u"parse")),
        adaptorValue,nestedParseArgs,u"adaptor.parse");
    const std::array<Value,1> nestedBodyArgs{nestedDocument};
    const Value nestedBody=context.callMember(context.get(adaptorValue,Key(u"body")),
        adaptorValue,nestedBodyArgs,u"adaptor.body");
    const std::array<Value,1> nestedChildrenArgs{nestedBody};
    const Value nestedChildren=context.callMember(context.get(adaptorValue,Key(u"childNodes")),
        adaptorValue,nestedChildrenArgs,u"adaptor.childNodes");
    const Value nestedRoot=context.get(nestedChildren,Key(u"0"));
    const std::array<Value,1> nestedFindArgs{nestedRoot};
    const Value nestedResult=context.callMember(findMethod,instanceIdentity,nestedFindArgs,
        u"domStrings.find");
    const Value nestedStrings=context.get(nestedResult,Key(u"0"));
    const Value nestedNodes=context.get(nestedResult,Key(u"1"));
    expect(context.toNumber(context.get(nestedStrings,Key(u"length")))==1.0);
    expect(context.get(nestedStrings,Key(u"0"))==Value(
        u"before\nmiddle<tex-html n=\"2\"><b>inner</b><tex-html><i>deep</i></tex-html><!2></tex-html>after"));
    expect(context.toNumber(context.get(nestedNodes,Key(u"length")))==1.0);
    const Value nestedNodeList=context.get(nestedNodes,Key(u"0"));
    expect(context.toNumber(context.get(nestedNodeList,Key(u"length")))==5.0);
    const std::array<std::u16string_view,5> nestedKinds{
        u"#text",u"br",u"#text",u"tex-html",u"#text"};
    const std::array<std::u16string_view,5> nestedOuterHtml{
        u"",u"<br>",u"",u"<tex-html><b>inner</b><tex-html><i>deep</i></tex-html></tex-html>",u""};
    const std::array<std::u16string_view,5> nestedText{
        u"before",u"",u"middle",u"innerdeep",u"after"};
    const std::array<std::u16string_view,5> nestedValues{
        u"before",u"",u"middle",u"",u"after"};
    const std::array<double,5> nestedLengths{6,1,6,75,5};
    for(std::size_t i=0;i<nestedKinds.size();++i){
        const auto digits=std::to_string(i);
        const Value entry=context.get(nestedNodeList,Key(std::u16string(digits.begin(),digits.end())));
        const Value node=context.get(entry,Key(u"0"));
        const auto projected=propertyNode(properties.from(node));
        const auto* domNode=std::get_if<DOMNodeRef>(&projected);
        expect(domNode!=nullptr);
        if(!domNode)continue;
        const std::array<Value,1> kindArgs{node};
        const Value kindMethod=context.get(adaptorValue,Key(u"kind"));
        expect(context.callMember(kindMethod,adaptorValue,kindArgs,u"adaptor.kind")==Value(std::u16string(nestedKinds[i])));
        expect(context.get(entry,Key(u"1"))==Value(nestedLengths[i]));
        expect(adaptor->kind(*domNode)==nestedKinds[i]);
        if(nestedKinds[i]==u"#text")expect(adaptor->value(*domNode)==nestedText[i]);
        else expect(adaptor->textContent(*domNode)==nestedText[i]);
        if(nestedKinds[i]==u"#text")expect(adaptor->value(*domNode)==nestedValues[i]);
        else expect(adaptor->outerHTML(*domNode)==nestedOuterHtml[i]);
    }
    expect(context.get(instanceIdentity,Key(u"string"))==Value(u""));
    for(const auto name:{u"strings",u"snodes",u"nodes",u"stack"})
        expect(context.toNumber(context.get(context.get(instanceIdentity,Key(name)),Key(u"length")))==0.0);

    const auto replacement=context.function(u"value",[adaptorIdentity](Context&,Value receiver,
        std::span<const Value> args)->Value{
        expect(std::get<ObjectRef>(receiver.data)==adaptorIdentity);
        expect(args.size()==1);
        return Value(u"source replacement");
    });
    context.set(adaptorValue,Key(u"value"),Value(replacement));
    const Value replaced=context.callMember(findMethod,instanceIdentity,findArgs,u"domStrings.find");
    const Value replacedStrings=context.get(replaced,Key(u"0"));
    expect(context.toNumber(context.get(replacedStrings,Key(u"length")))==1.0);
    expect(context.get(replacedStrings,Key(u"0"))==Value(u"source replacement\nsource replacement"));

    const auto instancePrototype=context.prototype(instance->identity());
    expect(instancePrototype.has_value());
    for(const auto name:{u"init",u"getPatterns",u"pushString",u"extendString",u"handleText",
            u"handleTag",u"handleContainer",u"handleOther",u"find"}){
        const auto method=context.get(Value(*instancePrototype),Key(name));
        const auto* function=std::get_if<ObjectRef>(&method.data);
        expect(function&&context.kind(*function)==ObjectKind::function);
    }
};
};

suite texhtmlConfigurationHook=[] {
"runs_the_actual_configuration_hook_and_preserves_custom_include_entries"_test=[] {
    svg::NativeRenderSession session(texhtmlTestFontHost());
    auto& services=session.services();
    auto& context=session.context();
    auto module=HTMLDomStringsModule::initializeModule(services.modules(),
        services.parser().properties,services.parser().regexp());
    auto resolver=[&services](ObjectRef identity){return services.lite().resolveAdaptor(identity);};
    const auto configuration=tex::TexHtmlConfiguration::initializeModule(
        services.parser(),services.modules(),resolver);
    const auto staticOptions=context.get(Value(module->identity()),Key(u"OPTIONS"));
    const auto include=context.get(staticOptions,Key(u"includeHtmlTags"));
    const auto custom=context.function(u"custom",[](Context&,Value,std::span<const Value>){
        return Value(u"custom marker");
    });
    context.set(include,Key(u"tex-html"),Value(custom));
    const Value configHook=configuration->config();
    const std::array<Value,0> noArgs{};
        context.call(std::get<ObjectRef>(configHook.data),Value(Undefined{}),noArgs);
    expect(context.get(include,Key(u"tex-html"))==Value(custom));
    context.set(include,Key(u"tex-html"),Value(Undefined{}));
        context.call(std::get<ObjectRef>(configHook.data),Value(Undefined{}),noArgs);
    const Value installed=context.get(include,Key(u"tex-html"));
    const auto* installedId=std::get_if<ObjectRef>(&installed.data);
    expect(installedId&&context.kind(*installedId)==ObjectKind::function);
    expect(context.get(installed,Key(u"name"))==Value(u""));
    expect(context.get(installed,Key(u"length"))==Value(2.0));
};
};

suite texhtmlDocumentWiring=[] {
"separates_html_dom_options_and_preserves_the_source_domstrings_field"_test=[] {
    svg::NativeRenderSession session(texhtmlTestFontHost());
    auto& context=session.context();
    const auto customConstructor=context.nativeConstructor(u"CustomDomStrings",
        [](Context& constructorContext,std::span<const Value>)->ObjectRef{
            return constructorContext.object();
        });
    const auto custom=context.instance(customConstructor);
    const auto customDocumentOptions=context.object();
    context.set(customDocumentOptions,Key(u"DomStrings"),Value(custom));
    const auto skip=context.array(1);
    context.set(skip,Key(u"0"),Value(u"custom-skip"));
    context.set(customDocumentOptions,Key(u"skipHtmlTags"),Value(skip));
    const auto customIncludes=context.object();
    context.set(customIncludes,Key(u"br"),Value(u"line marker"));
    context.set(customDocumentOptions,Key(u"includeHtmlTags"),Value(customIncludes));
    const auto customDocument=svg::createNativeSvgDocument(
        session,Value(customDocumentOptions));
    const auto customIdentity=customDocument->sourceIdentity();
    const Value customDocumentValue(customIdentity);
    expect(context.get(customDocumentValue,Key(u"domStrings"))==Value(custom)) << "custom DomStrings identity";
    expect(context.get(custom,Key(u"adaptor"))==Value(session.adaptorObject())) << "custom DomStrings adaptor";
    const Value customEffectiveOptions=context.get(customDocumentValue,Key(u"options"));
    expect(context.get(customEffectiveOptions,Key(u"DomStrings"))==Value(custom)) << "separated custom DomStrings";
    expect(!context.hasOwn(customEffectiveOptions,Key(u"skipHtmlTags"))) << "skip options separated";
    expect(!context.hasOwn(customEffectiveOptions,Key(u"includeHtmlTags"))) << "include options separated";

    const auto nativeDocumentOptions=context.object();
    context.set(nativeDocumentOptions,Key(u"skipHtmlTags"),Value(u"custom-skip"));
    const auto nativeIncludes=context.object();
    context.set(nativeIncludes,Key(u"br"),Value(u"line marker"));
    context.set(nativeDocumentOptions,Key(u"includeHtmlTags"),Value(nativeIncludes));
    const auto nativeDocument=svg::createNativeSvgDocument(
        session,Value(nativeDocumentOptions));
    const Value nativeDocumentValue(nativeDocument->sourceIdentity());
    const Value nativeIdentity=context.get(nativeDocumentValue,Key(u"domStrings"));
    const auto* nativeObject=std::get_if<ObjectRef>(&nativeIdentity.data);
    expect(nativeObject!=nullptr) << "native DomStrings object";
    if(!nativeObject)return;
    const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(
        &context.payload(*nativeObject));
    const auto nativeOwner=payload?std::dynamic_pointer_cast<
        folia::math::handlers::html::HTMLDomStrings>(*payload):nullptr;
    expect(nativeOwner!=nullptr) << "native DomStrings owner";
    expect(context.get(nativeIdentity,Key(u"adaptor"))==Value(session.adaptorObject())) << "native actual adaptor identity";
    const Value effective=context.get(nativeIdentity,Key(u"options"));
    const Value effectiveSkip=context.get(effective,Key(u"skipHtmlTags"));
    expect(effectiveSkip==Value(u"custom-skip")) << "preserved Source string option";
    const Value effectiveIncludes=context.get(effective,Key(u"includeHtmlTags"));
    expect(context.get(effectiveIncludes,Key(u"br"))==Value(u"line marker")) << "preserved include override";
    expect(context.get(effectiveIncludes,Key(u"wbr"))==Value(u"")) << "default include entry";
};
};
}
