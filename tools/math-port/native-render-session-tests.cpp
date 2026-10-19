#include "../../tests/support/folia_test.hpp"
import folia.math.output.svg.render_session;
import folia.math.output.svg.native_document;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::options;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

// This fixture exercises bundled static construction only. Unexpected loads
// fail loudly; it does not claim to implement or validate dynamic font loads.
struct StaticConstructionHost {
    std::vector<std::string> requests;
    std::vector<std::any> warnings;
};
common::FontLoadHost staticHost(const std::shared_ptr<StaticConstructionHost>& trace) {
    common::FontLoadHost host;
    host.asyncLoad=[trace](std::shared_ptr<common::FontData>,common::DynamicFileRef,std::string name)->async::PromiseRef {
        trace->requests.push_back(std::move(name));
        throw Exception("Unimplemented",u"Unexpected dynamic font request in static construction fixture");
    };
    host.syncLoad=[trace](common::FontData&,const common::DynamicFileRef&,std::string name) {
        trace->requests.push_back(std::move(name));
        throw Exception("Unimplemented",u"Unexpected synchronous font request in static construction fixture");
    };
    host.asyncIsSynchronous=[] { return false; };
    host.warn=[trace](const std::any& warning) { trace->warnings.push_back(warning); };
    return host;
}

suite renderSessionOwnership=[] {
    "native_svg_filter_reads_the_live_context_record_and_actual_owners"_test=[] {
        auto trace=std::make_shared<StaticConstructionHost>();
        svg::NativeRenderSession session(staticHost(trace));
        auto document=svg::createNativeSvgDocument(session);
        auto jax=session.outputJax();
        auto& context=session.context();
        std::vector<std::string> order;
        bool actualMath=false,actualDocument=false,stableRecord=false;
        auto filter=std::make_shared<Function>();
        filter->call=[&](const FunctionReceiver&,FunctionArgs args)->std::any {
            if(args.size()!=1)throw Exception("TypeError",u"one output filter record required");
            auto record=std::any_cast<core::OutputJaxFilterDataRef>(args[0]);
            const auto identity=record->sourceIdentity();
            stableRecord=identity==record->sourceRecord->identity();
            const auto keys=context.enumerableStringKeys(Value(identity));
            expect(keys==std::vector<std::u16string>{u"math",u"document",u"data"});
            const auto originalDescriptor=context.descriptor(Value(identity),Key(u"data"));
            expect(originalDescriptor&&originalDescriptor->enumerable&&originalDescriptor->configurable&&originalDescriptor->writable);
            auto mathBacking=record->rawMathValue();
            auto documentBacking=record->rawDocumentValue();
            auto dataBacking=record->rawDataValue();
            for(const auto& [key,backing]:std::array<std::pair<std::u16string,Value>,3>{{
                    {u"math",mathBacking},{u"document",documentBacking},{u"data",dataBacking}}}){
                context.erase(identity,Key(key));
                expect(!context.descriptor(Value(identity),Key(key)));
                context.set(identity,Key(key),backing);
                expect(context.descriptor(Value(identity),Key(key)).has_value());
            }
            expect(std::holds_alternative<core::MathItem*>(record->mathReference()));
            expect(std::holds_alternative<core::MathDocumentService*>(record->documentReference()));
            expect(record->dataValue()==dataBacking);
            auto accessor=[&](std::u16string key,Value backing){
                auto cell=std::make_shared<Value>(std::move(backing));
                auto getter=context.function(u"filter field getter",[&,key,cell](Context&,Value,std::span<const Value>)->Value{
                    order.push_back("get-"+property_host::debugText(key));return *cell;
                });
                auto setter=context.function(u"filter field setter",[&,key,cell](Context&,Value,std::span<const Value> values)->Value{
                    order.push_back("set-"+property_host::debugText(key));*cell=values.empty()?Value(Undefined{}):values[0];return Value(Undefined{});
                });
                context.define(identity,Key(key),Descriptor::access(getter,setter,true,true));
            };
            accessor(u"math",mathBacking);
            accessor(u"document",documentBacking);
            accessor(u"data",dataBacking);
            const auto math=record->mathReference();
            actualMath=std::holds_alternative<core::MathItem*>(math)&&
                std::get<core::MathItem*>(math)!=nullptr;
            const auto doc=record->documentReference();
            actualDocument=std::holds_alternative<core::MathDocumentService*>(doc)&&
                std::get<core::MathDocumentService*>(doc)==document.get();
            for(const auto key:{u"math",u"document",u"data"}){
                const auto accessorDescriptor=context.descriptor(Value(identity),Key(key));
                expect(accessorDescriptor&&accessorDescriptor->accessor&&accessorDescriptor->getter.has_value()&&accessorDescriptor->setter.has_value());
            }
            const auto live=record->dataValue();
            context.set(identity,Key(u"math"),mathBacking);
            context.set(identity,Key(u"document"),documentBacking);
            context.set(identity,Key(u"data"),live);
            expect(record->dataValue()==live);
            return {};
        };
        jax->preFilters->add(filter);

        const auto settings=context.object();
        context.set(settings,Key(u"display"),Value(false));
        context.set(settings,Key(u"em"),Value(16));
        context.set(settings,Key(u"ex"),Value(8));
        context.set(settings,Key(u"containerWidth"),Value(1280));
        const auto result=document->convert(Value(u"x+y"),Value(settings));
        expect(actualMath);
        expect(actualDocument);
        expect(stableRecord);
        expect(std::holds_alternative<ObjectRef>(result.data));
        const std::vector<std::string> expectedOrder{"get-math","get-document","get-data",
            "set-math","set-document","set-data","get-data","get-data"};
        expect(order==expectedOrder);
    };

    "actual_svg_constructor_services_live_until_context_teardown"_test=[] {
        try {
        auto trace=std::make_shared<StaticConstructionHost>();
        std::weak_ptr<StaticConstructionHost> weakTrace=trace;
        auto context=std::make_shared<Context>();
        auto graph=svg::SvgRenderServiceGraph::create(*context,staticHost(trace));
        std::weak_ptr<svg::SvgRenderServiceGraph> weakGraph=graph;
        expect(&graph->mapBindings()==&graph->svgJax().mapBindings())
            << "SVG output and render graph share one Map binding owner";
        expect(&graph->mapBindings().context()==context.get())
            << "the shared Map owner belongs to the render Context";
        expect(graph->mapBindings().constructor()==graph->svgJax().mapBindings().constructor())
            << "SVG consumers see the same actual Map constructor identity";
        expect(graph->mapBindings().prototype()==graph->svgJax().mapBindings().prototype())
            << "SVG consumers see the same actual Map prototype identity";
        const auto constructor=graph->svgJax().constructor();
        graph.reset();
        trace.reset();
        expect(!weakGraph.expired());
        expect(!weakTrace.expired());

        const auto object=context->construct(constructor);
        auto retained=weakGraph.lock();
        auto jax=retained->svgJax().resolve(object);
        expect(jax->instanceIdentity()==object);
        expect(&jax->optionsContext()==context.get());
        expect(jax->font!=nullptr);
        expect(jax->factory!=nullptr);
        expect(&jax->factory->jax()==jax.get());
        expect(jax->linebreaks!=nullptr);
        expect(jax->styleJson!=nullptr);
        expect(weakTrace.lock()->requests.empty());
        jax.reset();
        retained.reset();
        context.reset();
        expect(weakGraph.expired());
        expect(weakTrace.expired());
        } catch(const Exception& error) {
            std::cerr<<error.name<<": "<<property_host::debugText(error.message)<<'\n';
            throw;
        }
    };

    "public_render_handles_retain_the_actual_context_and_native_providers"_test=[] {
        try {
        auto trace=std::make_shared<StaticConstructionHost>();
        std::weak_ptr<StaticConstructionHost> weakTrace=trace;
        std::shared_ptr<svg::SvgOutputJax> jax;
        std::shared_ptr<adaptors::lite::LiteAdaptor> adaptor;
        std::shared_ptr<mml::MmlFactory> factory;
        std::weak_ptr<svg::SvgRenderServiceGraph> weakGraph;
        {
            svg::NativeRenderSession session(staticHost(trace));
            weakGraph=session.services().shared_from_this();
            jax=session.construct();
            adaptor=session.adaptor();
            factory=session.mathFactory();
            expect(jax==session.outputJax());
            expect(&jax->domAdaptor()==adaptor.get());
            expect(&session.services().mmlFactory()==factory.get());
            expect(jax->completedScene()==nullptr);
        }
        trace.reset();
        expect(!weakGraph.expired());
        expect(!weakTrace.expired());
        expect(jax->optionsContext().get(jax->options,Key(u"fontCache"))==Value(u"local"));
        const auto node=adaptor->text(u"live");
        const bool exactText=adaptor->value(node)==u"live";
        expect(exactText);
        const auto mi=factory->create("mi",tree::FactoryArgs{});
        expect(mi->kind()=="mi");
        jax.reset();
        expect(!weakGraph.expired());
        adaptor.reset();
        expect(!weakGraph.expired());
        factory.reset();
        expect(weakGraph.expired());
        expect(weakTrace.expired());
        } catch(const Exception& error) {
            std::cerr<<error.name<<": "<<property_host::debugText(error.message)<<'\n';
            throw;
        }
    };
};
}
