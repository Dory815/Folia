#include "../../../tests/support/folia_test.hpp"
import folia.math.adaptors.lite.lite_module;
import folia.math.input.tex.texhtml.tex_html_services;
import folia.math.util.option_types;
import folia.math.util.runtime_modules;
import folia.math.util.string_map;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::options;
namespace tex=folia::math::tex;
namespace lite=folia::math::adaptors::lite;

struct LiteGraph {
    Context context;
    OptionFunctionBindings functions{context};
    ContextPropertyBindings properties{context,functions};
    std::shared_ptr<regexp_host::RegExpBindings> regex=
        std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module{context,functions,properties,regex};
    tex::TexHtmlAdaptorResolver resolver=[this](ObjectRef identity)
        ->std::shared_ptr<AbstractDOMAdaptor>{return module.resolveAdaptor(identity);};

    std::shared_ptr<lite::LiteAdaptor> makeAdaptor(ObjectRef& identity){
        auto result=module.constructAdaptor();
        identity=module.bindAdaptor(result);
        return result;
    }
};

tex::TexHtmlServices currentServices(Context& context,SourceStringMap<Value>& packageData,
    const tex::TexHtmlAdaptorResolver& resolver){
    const auto record=packageData.get(u"texhtml");
    if(!record)throw Exception("TypeError",u"missing texhtml record in test fixture");
    const Value adaptor=context.get(*record,Key(u"adaptor"));
    return tex::resolveTexHtmlServices(context,adaptor,resolver);
}

DOMNodeRef parseViaSourceMethods(Context& context,ContextPropertyBindings& properties,
    tex::TexHtmlServices& services,std::u16string html){
    const Value adaptor(services.identity);
    const std::array<Value,1> parseArgs{Value(std::move(html))};
    const Value document=context.callMember(services.parseMethod,adaptor,parseArgs,u"adaptor.parse");
    const std::array<Value,1> bodyArgs{document};
    const Value body=context.callMember(services.bodyMethod,adaptor,bodyArgs,u"adaptor.body");
    const std::array<Value,1> childArgs{body};
    const Value children=context.callMember(services.childNodesMethod,adaptor,childArgs,u"adaptor.childNodes");
    const auto length=context.get(children,Key(u"length"));
    if(context.toNumber(length)==0)throw Exception("TypeError",u"expected one node for native bridge observation");
    Value node=context.get(children,Key(u"0"));
    if(context.toNumber(length)>1){
        const Value definition=context.object();
        const std::array<Value,3> nodeArgs{Value(u"div"),definition,children};
        const Value nodeMethod=context.get(adaptor,Key(u"node"));
        node=context.callMember(nodeMethod,adaptor,nodeArgs,u"adaptor.node");
    }
    const auto projected=propertyNode(properties.from(node));
    if(const auto* actual=std::get_if<DOMNodeRef>(&projected))return *actual;
    throw Exception("TypeError",u"Source LiteAdaptor did not return a canonical DOM node");
}

suite texhtmlBridge=[] {

"tracks_replaced_package_records_and_alternate_lite_adaptors"_test=[] {
    LiteGraph graph;
    ObjectRef firstIdentity,secondIdentity;
    auto first=graph.makeAdaptor(firstIdentity);
    auto second=graph.makeAdaptor(secondIdentity);
    SourceStringMap<Value> packageData;
    auto firstRecord=graph.context.object();
    graph.context.set(firstRecord,Key(u"adaptor"),Value(firstIdentity));
    packageData.set(u"texhtml",Value(firstRecord));

    auto services=currentServices(graph.context,packageData,graph.resolver);
    expect(services.dom==first);
    expect(services.identity==firstIdentity);
    auto dom=parseViaSourceMethods(graph.context,graph.properties,services,u"<b>first</b>");
    expect(services.dom->kind(dom)==u"b");

    // Replacing the entire Source package record still resolves its live
    // adaptor field through the one canonical LiteModule identity table.
    auto replacement=graph.context.object();
    graph.context.set(replacement,Key(u"adaptor"),Value(secondIdentity));
    packageData.set(u"texhtml",Value(replacement));
    services=currentServices(graph.context,packageData,graph.resolver);
    expect(services.dom==second);
    expect(services.identity==secondIdentity);
    dom=parseViaSourceMethods(graph.context,graph.properties,services,u"<i>second</i><u>root</u>");
    expect(services.dom->kind(dom)==u"div");
    const auto children=services.dom->childNodes(dom);
    expect(children.slots.size()==2);

    graph.context.set(replacement,Key(u"adaptor"),Value(firstIdentity));
    services=currentServices(graph.context,packageData,graph.resolver);
    expect(services.dom==first);
    expect(services.identity==firstIdentity);
};

"nullish_adaptors_fail_at_source_method_lookup"_test=[] {
    LiteGraph graph;
    for(const auto adaptor:{Value(Null{}),Value(Undefined{})}){
        bool failed=false;
        try{(void)tex::resolveTexHtmlServices(graph.context,adaptor,graph.resolver);}
        catch(const Exception& error){
            failed=error.name=="TypeError"&&
                error.message.find(u"childNodes")!=std::u16string::npos;
        }
        expect(failed);
    }
};

"xml_ref_retains_the_resolved_lite_owner"_test=[] {
    std::weak_ptr<AbstractDOMAdaptor> weakAdaptor;
    std::weak_ptr<mml::XmlDomAdaptor> weakBridge;
    mml::XmlRef retained;
    {
        Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regex=std::make_shared<regexp_host::RegExpBindings>(context);
        lite::LiteModule module(context,functions,properties,regex);
        tex::TexHtmlAdaptorResolver resolver=[&module](ObjectRef identity)
            ->std::shared_ptr<AbstractDOMAdaptor>{return module.resolveAdaptor(identity);};
        ObjectRef adaptorIdentity;
        auto adaptor=module.constructAdaptor();
        adaptorIdentity=module.bindAdaptor(adaptor);
        weakAdaptor=adaptor;
        auto record=context.object();
        context.set(record,Key(u"adaptor"),Value(adaptorIdentity));
        SourceStringMap<Value> packageData;
        packageData.set(u"texhtml",Value(record));
        auto services=currentServices(context,packageData,resolver);
        auto node=parseViaSourceMethods(context,properties,services,u"<span>kept</span>");
        retained=services.xml->fromDOM(node);
        weakBridge=services.xml;
        adaptor.reset();
    }
    expect(!weakBridge.expired());
    expect(!weakAdaptor.expired());
    retained.reset();
    expect(weakBridge.expired());
    expect(weakAdaptor.expired());
};
};
}
