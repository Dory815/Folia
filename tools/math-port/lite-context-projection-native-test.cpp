#include "../../tests/support/folia_test.hpp"
import folia.math.adaptors.lite_adaptor;
import folia.math.adaptors.lite.lite_module;
import folia.math.util.context_property_bindings;
import folia.math.util.option_functions;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::adaptors::lite;
namespace opts=folia::math::options;

suite liteContextProjection=[] {
    "lite_font_family_context_method_preserves_source_option_primitives"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        LiteModule module(context,functions,properties,regexp);
        const auto adaptor=module.bindAdaptor(module.constructAdaptor());
        const auto options=context.get(opts::Value(adaptor),opts::Key(u"options"));
        const auto classOptions=context.get(opts::Value(module.adaptorConstructor()),opts::Key(u"OPTIONS"));
        expect(context.get(classOptions,opts::Key(u"fontFamily"))==opts::Value(u"Times"));
        expect(context.get(options,opts::Key(u"fontFamily"))==opts::Value(u"Times"));
        const auto node=module.objectFor(module.resolveAdaptor(adaptor)->text(u""));
        const auto method=context.get(opts::Value(adaptor),opts::Key(u"fontFamily"));
        const auto actual=module.resolveAdaptor(adaptor);
        const std::array<opts::Value,1> args{opts::Value(node)};
        expect(actual->fontFamily(module.nodeFor(node))==u"Times");
        expect(context.callMember(method,opts::Value(adaptor),args,u"LiteAdaptor.fontFamily")==opts::Value(u"Times"));
        const std::array<opts::Value,4> probes{opts::Value(false),opts::Value(0),opts::Value(opts::Null{}),opts::Value(opts::Undefined{})};
        for(const auto& probe:probes){
            context.set(options,opts::Key(u"fontFamily"),probe);
            expect(context.callMember(method,opts::Value(adaptor),args,u"LiteAdaptor.fontFamily")==probe);
        }
        context.set(options,opts::Key(u"fontFamily"),opts::Value(u"Times"));
        const std::array<opts::Value,2> ignoredArguments{opts::Value(opts::Null{}),opts::Value(opts::Undefined{})};
        for(const auto& ignored:ignoredArguments){
            const std::array<opts::Value,1> malformed{ignored};
            expect(context.callMember(method,opts::Value(adaptor),malformed,u"LiteAdaptor.fontFamily")==opts::Value(u"Times"));
        }
        expect(context.callMember(method,opts::Value(adaptor),{},u"LiteAdaptor.fontFamily")==opts::Value(u"Times"));
        actual->badCSS(false);
        for(const auto& ignored:ignoredArguments){
            const std::array<opts::Value,1> malformed{ignored};
            expect(context.callMember(method,opts::Value(adaptor),malformed,u"LiteAdaptor.fontFamily")==opts::Value(u""));
        }
        expect(context.callMember(method,opts::Value(adaptor),{},u"LiteAdaptor.fontFamily")==opts::Value(u""));
        expect(actual->fontFamily({}).empty());
    };
    "lite_node_dispatch_observes_live_methods_and_source_attribute_order"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        LiteModule module(context,functions,properties,regexp);
        const auto adaptor=module.bindAdaptor(module.constructAdaptor());
        std::vector<std::u16string> calls;
        for(const auto name:{u"create",u"setAttributes",u"setStyle",u"setAttribute",u"append"}){
            const auto original=context.get(adaptor,opts::Key(name));
            const auto method=context.function(name,[&,original,name](opts::Context& cx,opts::Value receiver,
                std::span<const opts::Value> args){
                calls.emplace_back(name);
                if(std::u16string_view(name)==u"append"){
                    const auto next=cx.function(u"append",[&,original](opts::Context& current,opts::Value self,
                        std::span<const opts::Value> values){
                        calls.emplace_back(u"append-next");
                        return current.callMember(original,self,values,u"original append");
                    });
                    cx.set(adaptor,opts::Key(u"append"),opts::Value(next));
                }
                return cx.callMember(original,receiver,args,u"original Lite method");
            });
            context.set(adaptor,opts::Key(name),opts::Value(method));
        }
        const auto definition=context.object(),style=context.object(),fields=context.object();
        context.set(style,opts::Key(u"border-left-width"),opts::Value(u"2px"));
        context.set(fields,opts::Key(u"sample"),opts::Value(17));
        context.set(definition,opts::Key(u"style"),opts::Value(style));
        context.set(definition,opts::Key(u"properties"),opts::Value(fields));
        context.set(definition,opts::Key(u"title"),opts::Value(u"name"));
        const auto children=context.array(2);
        const auto first=module.objectFor(module.resolveAdaptor(adaptor)->text(u"a"));
        const auto second=module.objectFor(module.resolveAdaptor(adaptor)->text(u"b"));
        context.set(children,opts::Key(u"0"),opts::Value(first));
        context.set(children,opts::Key(u"1"),opts::Value(second));
        const std::array<opts::Value,3> args{opts::Value(u"span"),opts::Value(definition),opts::Value(children)};
        const auto node=context.callMember(context.get(adaptor,opts::Key(u"node")),
            opts::Value(adaptor),args,u"adaptor.node");
        // Pinned original DOMAdaptor/LiteBase records this sequence; replacing
        // append during its first call affects the next child immediately.
        const std::vector<std::u16string> expected{u"create",u"setAttributes",u"setStyle",
            u"setAttribute",u"append",u"append-next"};
        expect(calls==expected);
        expect(context.get(node,opts::Key(u"sample"))==opts::Value(17));
        const auto attributes=context.get(node,opts::Key(u"attributes"));
        expect(context.get(attributes,opts::Key(u"style"))==opts::Value(u"border-left-width: 2px;"));
        expect(context.get(attributes,opts::Key(u"title"))==opts::Value(u"name"));
        expect(context.get(first,opts::Key(u"parent"))==node);
        expect(context.get(second,opts::Key(u"parent"))==node);
        const auto stop=context.function(u"append",[](opts::Context&,opts::Value,
            std::span<const opts::Value>)->opts::Value{throw opts::Exception("TypeError",u"local append test");});
        context.set(adaptor,opts::Key(u"append"),opts::Value(stop));
        bool propagated=false;
        try{(void)context.callMember(context.get(adaptor,opts::Key(u"node")),opts::Value(adaptor),args,u"adaptor.node");}
        catch(const opts::Exception& error){propagated=error.name=="TypeError"&&error.message==u"local append test";}
        expect(propagated);
    };
    "lite_node_property_context_roundtrip_preserves_object_and_owner_identity"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        LiteModule module(context,functions,properties,regexp);
        const auto cjkPattern=context.get(opts::Value(module.adaptorConstructor()),opts::Key(u"cjkPattern"));
        const auto sharedPattern=regexp->resolve(cjkPattern);
        expect(sharedPattern->pattern()==LiteAdaptor::cjkPatternSource());
        expect(sharedPattern->flags()==u"gu");
        const auto actualAdaptor=module.constructAdaptor();
        const auto cjkText=actualAdaptor->text(u"漢");
        const auto cjkSpan=LiteElement(std::u16string(u"span"),PropertyList{},
            PropertyArray{nodeProperty(cjkText)},module.store()).ref();
        const auto measured=actualAdaptor->nodeSize(cjkSpan);
        expect(measured.first==1.0&&measured.second==0.8);
        bool rejectedTextReceiver=false;
        try{(void)actualAdaptor->nodeSize(cjkText);}
        catch(const opts::Exception& error){rejectedTextReceiver=error.name=="TypeError";}
        expect(rejectedTextReceiver);
        const auto store=module.store();
        const auto node=LiteElement(std::u16string(u"math"),PropertyList{},PropertyArray{},store).ref();
        const auto property=nodeProperty(node);

        const auto first=properties.to(property);
        const auto second=properties.to(nodeProperty(node));
        const auto* object=std::get_if<opts::ObjectRef>(&first.data);
        expect(object!=nullptr);
        if(!object)return;
        expect(second==first);
        const auto roundTrip=properties.from(first);
        expect(roundTrip==property);
        const auto projected=propertyNode(roundTrip);
        const auto* recovered=std::get_if<LiteNodeRef>(&projected);
        expect(recovered!=nullptr);
        if(!recovered)return;
        expect(recovered->store==node.store&&recovered->id==node.id);
        expect(context.get(*object,u"kind")==opts::Value(u"math"));

        context.set(*object,u"kind",opts::Value(u"mi"));
        expect(recordConst(node).kind==Property("mi"));
        auto typed=PropertyList::record(roundTrip);
        expect(typed.has_value());
        if(typed){
            typed->set("kind",Property("mo"));
            expect(context.get(*object,u"kind")==opts::Value(u"mo"));
        }
    };
    "two_lite_adaptors_share_the_same_node_store_and_context_projection"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        LiteModule module(context,functions,properties,regexp);
        const auto store=module.store();
        auto left=std::make_shared<LiteAdaptor>(LiteNodeRef{},PropertyList{},store);
        auto right=std::make_shared<LiteAdaptor>(LiteNodeRef{},PropertyList{},store);
        const auto leftObject=module.bindAdaptor(left);
        const auto rightObject=module.bindAdaptor(right);
        expect(leftObject!=rightObject);

        const auto parent=LiteElement(std::u16string(u"body"),PropertyList{},PropertyArray{},store).ref();
        const auto child=left->text(u"x");
        left->append(parent,child);
        const auto observedParent=right->parent(child);
        const auto* observedNode=std::get_if<DOMNodeRef>(&observedParent);
        expect(observedNode!=nullptr);
        if(observedNode)expect(observedNode->store==parent.store&&observedNode->id==parent.id);
        const auto projected=std::get<opts::ObjectRef>(properties.to(nodeProperty(child)).data);
        expect(properties.to(nodeProperty(child))==opts::Value(projected));
        const auto recovered=propertyNode(properties.from(opts::Value(projected)));
        const auto* recoveredNode=std::get_if<DOMNodeRef>(&recovered);
        expect(recoveredNode!=nullptr);
        if(recoveredNode)expect(recoveredNode->store==child.store&&recoveredNode->id==child.id);
    };
    "lite_parser_attribute_conversion_uses_dynamic_string_hint_primitive_order"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        LiteModule module(context,functions,properties,regexp);
        const auto parser=context.construct(module.parserConstructor());
        const auto object=context.object();
        std::vector<std::string> trace;
        bool receiverMatches=false;
        const auto toString=context.function(u"toString",[&](auto&,auto receiver,auto)->opts::Value{
            trace.emplace_back("call toString");receiverMatches=receiver==opts::Value(object);
            return opts::Value(u"a&\"<b>");
        });
        const auto getter=context.function(u"get toString",[&](auto&,auto receiver,auto)->opts::Value{
            trace.emplace_back("get toString");return opts::Value(toString);
        });
        context.define(object,opts::Key(u"toString"),opts::Descriptor::access(getter,{},true,true));
        const auto method=std::get<opts::ObjectRef>(context.get(opts::Value(parser),opts::Key(u"protectAttribute")).data);
        const std::array<opts::Value,2> args{opts::Value(object),opts::Value(true)};
        const auto result=context.call(method,opts::Value(parser),args);
        expect(result==opts::Value(u"a&amp;&quot;&lt;b&gt;"));
        expect(receiverMatches);
        const bool order=trace.size()==2&&trace[0]=="get toString"&&trace[1]=="call toString";
        expect(order);
    };
    "lite_context_retained_source_objects_keep_module_graph_until_context_teardown"_test=[] {
        std::weak_ptr<LiteNodeStore> storeLease;
        {
            opts::Context context;
            OptionFunctionBindings functions(context);
            ContextPropertyBindings properties(context,functions);
            auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
            opts::ObjectRef parserConstructor{},parser{},adaptor{},body{},children{};
            {
                auto module=std::make_unique<LiteModule>(context,functions,properties,regexp);
                const auto store=module->store();
                storeLease=store;
                adaptor=context.construct(module->adaptorConstructor());
                parserConstructor=module->parserConstructor();
                parser=context.construct(parserConstructor);
                const auto bodyNode=LiteElement(std::u16string(u"body"),PropertyList{},PropertyArray{},store).ref();
                body=module->objectFor(bodyNode);
                const auto textNode=module->resolveAdaptor(adaptor)->text(u"x&<");
                const auto textObject=module->objectFor(textNode);
                children=std::get<opts::ObjectRef>(context.get(body,opts::Key(u"children")).data);
                const auto append=context.get(adaptor,opts::Key(u"append"));
                const std::array<opts::Value,2> appendArgs{opts::Value(body),opts::Value(textObject)};
                (void)context.callMember(append,opts::Value(adaptor),appendArgs,u"adaptor.append");
            }
            expect(!storeLease.expired());
            const auto parserAfterModuleRelease=context.construct(parserConstructor);
            const auto protectHTML=context.get(parserAfterModuleRelease,opts::Key(u"protectHTML"));
            const auto* protectMethod=std::get_if<opts::ObjectRef>(&protectHTML.data);
            expect(protectMethod!=nullptr);
            if(protectMethod)expect(context.kind(*protectMethod)==opts::ObjectKind::function);
            expect(context.get(children,opts::Key(u"length"))==opts::Value(1));
            const auto serialize=context.get(parser,opts::Key(u"serialize"));
            const std::array<opts::Value,3> serializeArgs{opts::Value(adaptor),opts::Value(body),opts::Value(false)};
            expect(context.callMember(serialize,opts::Value(parser),serializeArgs,u"parser.serialize")==
                   opts::Value(u"<body>x&amp;&lt;</body>"));
        }
        expect(storeLease.expired());
    };
};
}
