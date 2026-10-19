#include "support/folia_test.hpp"
import folia.math;
import folia.math.input.tex.bussproofs.configuration;
import folia.math.output.common;
import folia.math.output.svg.render_session;
import folia.math.output.svg.native_document;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts=folia::math::options;
namespace tex=folia::math::tex;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

common::FontLoadHost fontHost(){
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&){};
    return host;
}

bool contains(const RenderResult& result,std::string_view value){
    return result.svg.find(value)!=std::string::npos;
}

bool hasGlyph(const output::scene::FrozenNodeRef& node,std::uint32_t codepoint){
    if(!node)return false;
    if(const auto* glyph=std::get_if<output::scene::GlyphPrimitive>(&node->semantic);
       glyph&&glyph->codepoint==codepoint)return true;
    for(const auto& child:node->children)if(hasGlyph(child,codepoint))return true;
    return false;
}

bool hasGlyph(const RenderResult& result,std::uint32_t codepoint){
    for(const auto& root:result.scene.roots())if(hasGlyph(root,codepoint))return true;
    return false;
}

suite bussproofsRendererSession=[] {
    "bussproofs is registered once in the native render graph"_test=[] {
        svg::NativeRenderSession session(fontHost(),{
            svg::NewcmFontExtension::bboldx,
            svg::NewcmFontExtension::bbm,
            svg::NewcmFontExtension::dsfont});
        auto& services=session.services();
        const auto loaded=services.parser().configurationHandler.get(u"bussproofs");
        const auto* original=std::get_if<std::shared_ptr<tex::Configuration>>(&loaded);
        expect(original!=nullptr&&*original) << "session graph registers BussproofsConfiguration";
        if(!original||!*original)return;
        const auto repeated=tex::BussproofsConfiguration::initializeModule(
            services.parser(),services.modules());
        expect(repeated==*original) << "reinitialization returns the canonical configuration owner";
    };

    "public renderer reuses one Bussproofs session across proofs and formulas"_test=[] {
        constexpr std::string_view proofA=
            R"(\begin{prooftree}\AxiomC{A}\UnaryInfC{B}\end{prooftree})";
        constexpr std::string_view proofB=
            R"(\begin{prooftree}\AxiomC{P}\AxiomC{Q}\BinaryInfC{P\land Q}\end{prooftree})";
        Renderer renderer;
        const auto first=renderer.render(proofA);
        const auto ordinary=renderer.render("x+1");
        const auto second=renderer.render(proofB);
        expect(!first.scene.roots().empty()) << "proof renders through the Scene path";
        expect(!ordinary.scene.roots().empty()) << "ordinary formula still renders in the same session";
        expect(!second.scene.roots().empty()) << "a second proof renders in the same session";
        expect(contains(first,"<svg")) << "first proof has SVG output";
        expect(hasGlyph(first,static_cast<std::uint32_t>('A'))&&
               hasGlyph(first,static_cast<std::uint32_t>('B')))
            << "first proof premises and conclusion reach the completed Scene";
        expect(hasGlyph(second,static_cast<std::uint32_t>('P'))&&
               hasGlyph(second,static_cast<std::uint32_t>('Q')))
            << "second proof reaches the completed Scene";
        expect(hasGlyph(ordinary,0x1D465u)&&
               hasGlyph(ordinary,static_cast<std::uint32_t>('1')))
            << "ordinary formula reaches the completed Scene";
        expect(first.svg!=ordinary.svg&&ordinary.svg!=second.svg)
            << "proof and ordinary formula outputs reflect their inputs";
    };

    "independent public render sessions do not share proof state"_test=[] {
        constexpr std::string_view proof=
            R"(\begin{prooftree}\AxiomC{L}\UnaryInfC{R}\end{prooftree})";
        Renderer first;
        Renderer second;
        const auto firstResult=first.render(proof);
        const auto secondResult=second.render(proof);
        expect(!firstResult.scene.roots().empty()&&!secondResult.scene.roots().empty())
            << "both independent sessions complete a Scene";
        expect(firstResult.svg==secondResult.svg)
            << "independent sessions produce identical SVG";
    };

    "scene-only render consumes the same frozen owner without adding SVG DOM"_test=[] {
        svg::NativeRenderSession session(fontHost(),{
            svg::NewcmFontExtension::bboldx,
            svg::NewcmFontExtension::bbm,
            svg::NewcmFontExtension::dsfont});
        auto document=svg::createNativeSvgDocument(session);
        auto jax=session.outputJax();
        auto adaptor=session.adaptor();
        expect(jax!=nullptr&&adaptor!=nullptr) << "native session owns its SVG producer and adaptor";
        if(!jax||!adaptor)return;

        std::shared_ptr<const output::scene::Scene> delivered;
        DOMNodeRef parent{};
        std::size_t calls=0;
        auto previous=jax->exchangeSceneOutputConsumer(
            [&](std::shared_ptr<const output::scene::Scene> value,DOMNodeRef target){
                ++calls;delivered=std::move(value);parent=std::move(target);
            });
        struct ConsumerRestore {
            svg::SvgOutputJax& jax;
            svg::SvgOutputJax::SceneOutputConsumer previous;
            ~ConsumerRestore(){(void)jax.exchangeSceneOutputConsumer(std::move(previous));}
        } restore{*jax,std::move(previous)};
        auto& context=session.context();
        const auto settings=context.object();
        context.set(settings,opts::Key(u"em"),opts::Value(16));
        context.set(settings,opts::Key(u"ex"),opts::Value(8));
        context.set(settings,opts::Key(u"containerWidth"),opts::Value(1280));
        (void)document->convert(opts::Value(u"x+1"),opts::Value(settings));

        expect(calls==1&&delivered!=nullptr) << "one conversion delivers its owned completed Scene";
        expect(delivered&&delivered.get()==jax->completedScene())
            << "scene consumer and SvgOutputJax expose the exact same frozen Scene owner";
        expect(parent&&adaptor->tags(parent,u"svg").empty())
            << "capture-only consumer bypasses Serializer::materialize and leaves no SVG child";
        if(delivered)expect(!delivered->roots().empty())
            << "scene-only route retains complete native geometry";
    };

    "native Lite unknown-text measurement is shared by scene-only and SVG output"_test=[] {
        svg::NativeRenderSession session(fontHost(),{
            svg::NewcmFontExtension::bboldx,
            svg::NewcmFontExtension::bbm,
            svg::NewcmFontExtension::dsfont});
        auto document=svg::createNativeSvgDocument(session);
        auto jax=session.outputJax();
        auto adaptor=session.adaptor();
        expect(jax!=nullptr&&adaptor!=nullptr) << "native session exposes its live output owners";
        if(!jax||!adaptor)return;

        std::size_t materializeCalls=0,serializeXmlCalls=0;
        auto previousMaterializeObserver=svg::Serializer::exchangeMaterializationObserver(
            [&]{++materializeCalls;});
        struct RestoreMaterializeObserver {
            svg::Serializer::MaterializationObserver previous;
            ~RestoreMaterializeObserver(){
                (void)svg::Serializer::exchangeMaterializationObserver(std::move(previous));
            }
        } restoreMaterializeObserver{std::move(previousMaterializeObserver)};
        adaptor->setSerializeXmlObserver([&]{++serializeXmlCalls;});
        std::shared_ptr<const output::scene::Scene> sceneOnly;
        DOMNodeRef sceneParent{};
        auto previous=jax->exchangeSceneOutputConsumer(
            [&](std::shared_ptr<const output::scene::Scene> value,DOMNodeRef target){
                sceneOnly=std::move(value);sceneParent=std::move(target);
            });
        {
            struct ConsumerRestore {
                svg::SvgOutputJax& jax;
                svg::SvgOutputJax::SceneOutputConsumer previous;
                ~ConsumerRestore(){(void)jax.exchangeSceneOutputConsumer(std::move(previous));}
            } restore{*jax,std::move(previous)};
            auto& context=session.context();
            const auto settings=context.object();
            context.set(settings,opts::Key(u"em"),opts::Value(16));
            context.set(settings,opts::Key(u"ex"),opts::Value(8));
            context.set(settings,opts::Key(u"containerWidth"),opts::Value(1280));
            (void)document->convert(opts::Value(u"\\text{漢字😀 é}+\\mathsf{ABC}"),
                                    opts::Value(settings));
        }
        expect(sceneOnly!=nullptr&&sceneParent)
            << "unknown-text conversion delivers the scene through the capture consumer";
        expect(materializeCalls==0&&serializeXmlCalls==0)
            << "unknown-text scene-only conversion enters neither Serializer::materialize nor serializeXML";
        expect(adaptor->tags(sceneParent,u"svg").empty())
            << "scene-only unknown-text conversion does not append an SVG tree";

        auto& context=session.context();
        const auto settings=context.object();
        context.set(settings,opts::Key(u"em"),opts::Value(16));
        context.set(settings,opts::Key(u"ex"),opts::Value(8));
        context.set(settings,opts::Key(u"containerWidth"),opts::Value(1280));
        (void)document->convert(opts::Value(u"\\text{漢字😀 é}+\\mathsf{ABC}"),
                                opts::Value(settings));
        const auto* svgScene=jax->completedScene();
        expect(svgScene!=nullptr) << "ordinary SVG conversion completes the same producer";
        if(!sceneOnly||!svgScene)return;
        svg::Serializer serializer(*adaptor);
        const auto sceneNodes=serializer.materialize(*sceneOnly);
        const auto svgNodes=serializer.materialize(*svgScene);
        expect(sceneNodes.size()==svgNodes.size()) << "both consumers receive the same root set";
        if(sceneNodes.size()==svgNodes.size()){
            bool equal=true;
            for(std::size_t i=0;i<sceneNodes.size();++i)
                equal=equal&&adaptor->outerHTML(sceneNodes[i])==adaptor->outerHTML(svgNodes[i]);
            expect(equal) << "scene-only and SVG routes freeze identical unknown-text geometry";
        }
        expect(materializeCalls>0)
            << "the observer records materialization when the normal SVG consumer is restored";
    };

    "native Lite measurement probe recovers after nodeSize option failure"_test=[] {
        svg::NativeRenderSession session(fontHost(),{
            svg::NewcmFontExtension::bboldx,
            svg::NewcmFontExtension::bbm,
            svg::NewcmFontExtension::dsfont});
        auto document=svg::createNativeSvgDocument(session);
        auto adaptor=session.adaptor();
        expect(adaptor!=nullptr) << "native session retains the actual measurement adaptor";
        if(!adaptor)return;

        auto original=adaptor->exchangeSourceNumberOption({});
        expect(static_cast<bool>(original)) << "actual Source option provider is installed";
        if(!original)return;
        bool reachedNodeSizeOption=false;
        adaptor->exchangeSourceNumberOption([source=original,&reachedNodeSizeOption](
            std::string_view field){
            if(field=="unknownCharWidth"){
                reachedNodeSizeOption=true;
                throw opts::Exception("TypeError",u"measurement probe failure witness");
            }
            return source(field);
        });
        bool failedAsInjected=false;
        {
            struct RestoreSourceOption {
                adaptors::lite::LiteAdaptor& adaptor;
                std::function<double(std::string_view)> source;
                ~RestoreSourceOption(){(void)adaptor.exchangeSourceNumberOption(std::move(source));}
            } restore{*adaptor,std::move(original)};
            auto& context=session.context();
            const auto settings=context.object();
            context.set(settings,opts::Key(u"em"),opts::Value(16));
            context.set(settings,opts::Key(u"ex"),opts::Value(8));
            context.set(settings,opts::Key(u"containerWidth"),opts::Value(1280));
            try{
                (void)document->convert(opts::Value(u"\\text{漢字 measurement}"),
                                        opts::Value(settings));
            }catch(const opts::Exception& error){
                failedAsInjected=error.name=="TypeError"&&
                    property_host::debugText(error.message)=="measurement probe failure witness";
            }
        }
        expect(reachedNodeSizeOption&&failedAsInjected)
            << "the failure occurs inside real LiteAdaptor::nodeSize after the probe text is set";

        bool recovered=false;
        auto& context=session.context();
        const auto settings=context.object();
        context.set(settings,opts::Key(u"em"),opts::Value(16));
        context.set(settings,opts::Key(u"ex"),opts::Value(8));
        context.set(settings,opts::Key(u"containerWidth"),opts::Value(1280));
        try{
            const auto converted=document->convert(opts::Value(u"\\text{漢字 measurement}"),
                                                   opts::Value(settings));
            recovered=std::holds_alternative<opts::ObjectRef>(converted.data);
        }catch(...){recovered=false;}
        expect(recovered)
            << "the same session measures the same text after exception-safe probe restoration";
    };

    "native Lite measurement probe restores its prior text after nested measurement"_test=[] {
        const auto packages=std::vector<svg::NewcmFontExtension>{
            svg::NewcmFontExtension::bboldx,
            svg::NewcmFontExtension::bbm,
            svg::NewcmFontExtension::dsfont};
        svg::NativeRenderSession nestedSession(fontHost(),packages);
        auto nestedDocument=svg::createNativeSvgDocument(nestedSession);
        auto nestedAdaptor=nestedSession.adaptor();
        auto nestedJax=nestedSession.outputJax();
        expect(nestedAdaptor&&nestedJax) << "nested session exposes its real Lite measurement owners";
        if(!nestedAdaptor||!nestedJax)return;

        int resetMeasurementCalls=0;
        const auto resetMeasure=[&]{
            ++resetMeasurementCalls;
            nestedJax->reset();
            return common::UnknownBBox{7,.75,.2};
        };
        const auto beforeReset=nestedJax->measureTextWithCache(
            u"reset cache witness","-reset-witness",{},resetMeasure);
        const auto afterReset=nestedJax->measureTextWithCache(
            u"reset cache witness","-reset-witness",{},[&]{
                ++resetMeasurementCalls;
                return common::UnknownBBox{9,.75,.2};
            });
        expect(resetMeasurementCalls==1&&beforeReset.w==7&&afterReset.w==7)
            << "Source reset clears SVG font data while retaining the Common unknown-text Map";

        int sameKeyMeasurementCalls=0;
        common::UnknownBBox nestedSameKey;
        const auto outerSameKey=nestedJax->measureTextWithCache(
            u"same-key witness","-same-key-witness",{},[&]{
                ++sameKeyMeasurementCalls;
                nestedSameKey=nestedJax->measureTextWithCache(
                    u"same-key witness","-same-key-witness",{},[&]{
                        ++sameKeyMeasurementCalls;
                        return common::UnknownBBox{3,.75,.2};
                    });
                return common::UnknownBBox{7,.75,.2};
            });
        const auto afterSameKey=nestedJax->measureTextWithCache(
            u"same-key witness","-same-key-witness",{},[&]{
                ++sameKeyMeasurementCalls;
                return common::UnknownBBox{11,.75,.2};
            });
        expect(sameKeyMeasurementCalls==2&&nestedSameKey.w==3&&
               outerSameKey.w==7&&afterSameKey.w==7)
            << "the outer Source Map.set overwrites a nested same-key measurement";

        auto& nestedContext=nestedSession.context();
        auto settingsFor=[&](opts::Context& context){
            const auto settings=context.object();
            context.set(settings,opts::Key(u"em"),opts::Value(16));
            context.set(settings,opts::Key(u"ex"),opts::Value(8));
            context.set(settings,opts::Key(u"containerWidth"),opts::Value(1280));
            return settings;
        };
        auto sourceOptions=nestedAdaptor->exchangeSourceNumberOption({});
        auto sourceTextContent=nestedAdaptor->exchangeSourceTextContent({});
        struct RestoreMeasurementCallbacks {
            adaptors::lite::LiteAdaptor& adaptor;
            std::function<double(std::string_view)> numberOption;
            std::function<Property(adaptors::lite::LiteNodeRef)> textContent;
            ~RestoreMeasurementCallbacks(){
                (void)adaptor.exchangeSourceNumberOption(std::move(numberOption));
                (void)adaptor.exchangeSourceTextContent(std::move(textContent));
            }
        } restore{*nestedAdaptor,std::move(sourceOptions),std::move(sourceTextContent)};
        expect(static_cast<bool>(restore.numberOption))
            << "native session installs the actual Source option callback";
        expect(static_cast<bool>(restore.textContent))
            << "native session installs the actual Source textContent callback";
        if(!restore.numberOption||!restore.textContent)return;

        auto sourceProvider=nestedJax->exchangeTextMeasurementProvider({});
        struct RestoreMeasurementProvider {
            svg::SvgOutputJax& jax;
            svg::SvgOutputJax::TextMeasurementProvider provider;
            ~RestoreMeasurementProvider(){
                (void)jax.exchangeTextMeasurementProvider(std::move(provider));
            }
        } restoreProvider{*nestedJax,std::move(sourceProvider)};
        expect(static_cast<bool>(restoreProvider.provider))
            << "native session installs its actual Lite measurement provider";
        if(!restoreProvider.provider)return;
        nestedJax->exchangeTextMeasurementProvider(restoreProvider.provider);

        DOMNodeRef lastMeasurementNode{},outerProbeNode{},nestedProbeNode{};
        std::u16string lastMeasurementText,outerProbeText,outerProbeSourceText;
        bool nestedStarted=false,nestedCompleted=false,nestedMeasurementActive=false;
        bool nestedProbeCaptured=false,providerProbeBaselineReady=false;
        bool sameProviderProbeNode=false,providerRestoredBaseline=false;
        bool initialSourceTextMatches=false,providerExchangedDuringCall=false,reentryArmed=false;
        DOMNodeRef providerProbeBaselineNode{};
        std::u16string providerProbeBaselineText,providerProbeRestoredText;
        nestedAdaptor->exchangeSourceTextContent(
            [&,source=restore.textContent](adaptors::lite::LiteNodeRef node){
                const auto value=source(node);
                lastMeasurementNode=node;
                if(const auto* text=std::get_if<std::string>(&value.value))
                    lastMeasurementText=property_host::text(*text);
                return value;
            });
        nestedAdaptor->exchangeSourceNumberOption(
            [&,source=restore.numberOption](std::string_view field){
                if(field=="unknownCharWidth"){
                    if(!nestedStarted&&reentryArmed){
                        nestedStarted=true;
                        outerProbeNode=lastMeasurementNode;
                        outerProbeSourceText=lastMeasurementText;
                        if(outerProbeNode)outerProbeText=nestedAdaptor->textContent(outerProbeNode);
                        struct NestedMeasurementScope {
                            bool& active;
                            explicit NestedMeasurementScope(bool& value):active(value){active=true;}
                            ~NestedMeasurementScope(){active=false;}
                        } nestedScope(nestedMeasurementActive);
                        try{
                            auto activeProvider=nestedJax->exchangeTextMeasurementProvider({});
                            providerExchangedDuringCall=static_cast<bool>(activeProvider);
                            struct RestoreActiveProvider {
                                svg::SvgOutputJax& jax;
                                svg::SvgOutputJax::TextMeasurementProvider provider;
                                ~RestoreActiveProvider(){
                                    (void)jax.exchangeTextMeasurementProvider(std::move(provider));
                                }
                            } restoreActive{*nestedJax,std::move(activeProvider)};
                            const auto measured=nestedJax->measureTextWithCache(
                                u"Ω nested measurement","-reentrant-test",{},
                                [provider=restoreProvider.provider]{
                                    return common::UnknownBBox{
                                        provider(u"Ω nested measurement"),.75,.2};
                                });
                            (void)measured;
                            nestedCompleted=true;
                        }catch(...){nestedCompleted=false;}
                        const auto afterText=outerProbeNode
                            ?nestedAdaptor->textContent(outerProbeNode):std::u16string{};
                        providerProbeRestoredText=nestedProbeNode
                            ?nestedAdaptor->textContent(nestedProbeNode):std::u16string{};
                        sameProviderProbeNode=providerProbeBaselineNode&&nestedProbeNode&&
                            providerProbeBaselineNode==nestedProbeNode;
                        providerRestoredBaseline=providerProbeBaselineReady&&nestedProbeNode&&
                            providerProbeRestoredText==providerProbeBaselineText;
                        initialSourceTextMatches=outerProbeText==outerProbeSourceText;
                        expect(afterText==outerProbeText)
                            << "nested measurement preserves the outer Source node's text";
                    }else if(nestedMeasurementActive&&!nestedProbeCaptured){
                        nestedProbeCaptured=true;
                        nestedProbeNode=lastMeasurementNode;
                    }
                }
                return source(field);
            });

        // Seed the actual shared provider probe and record its own prior value.
        // The Source node being measured can be a different Lite element.
        (void)restoreProvider.provider(u"Ω baseline probe");
        providerProbeBaselineNode=lastMeasurementNode;
        providerProbeBaselineReady=static_cast<bool>(providerProbeBaselineNode);
        if(providerProbeBaselineReady)
            providerProbeBaselineText=nestedAdaptor->textContent(providerProbeBaselineNode);
        reentryArmed=true;

        bool outerCompleted=false;
        try{
            const auto settings=settingsFor(nestedContext);
            const auto value=nestedDocument->convert(
                opts::Value(u"\\text{漢字 outer measurement}"),opts::Value(settings));
            outerCompleted=std::holds_alternative<opts::ObjectRef>(value.data);
        }catch(...){outerCompleted=false;}
        const auto outerScene=nestedJax->completedScene();
        expect(nestedStarted&&nestedCompleted&&outerCompleted&&outerScene)
            << "a real nodeSize option callback completes a nested Lite measurement";
        expect(nestedProbeCaptured)
            << "nested Lite measurement reaches the real text probe";
        expect(sameProviderProbeNode)
            << "nested Lite measurement uses the same shared provider probe";
        expect(providerRestoredBaseline)
            << "nested Lite measurement restores the provider probe's prior text";
        expect(initialSourceTextMatches)
            << "Lite and Source textContent agree before nested measurement";
        expect(nestedProbeCaptured&&sameProviderProbeNode&&providerRestoredBaseline&&
               initialSourceTextMatches)
            << "nested Lite measurement restores the shared real text probe to its prior text";
        expect(providerExchangedDuringCall)
            << "the active provider can be replaced while its copied callable completes";

        svg::NativeRenderSession baselineSession(fontHost(),packages);
        auto baselineDocument=svg::createNativeSvgDocument(baselineSession);
        const auto baselineSettings=settingsFor(baselineSession.context());
        bool baselineCompleted=false;
        try{
            const auto value=baselineDocument->convert(
                opts::Value(u"\\text{漢字 outer measurement}"),opts::Value(baselineSettings));
            baselineCompleted=std::holds_alternative<opts::ObjectRef>(value.data);
        }catch(...){baselineCompleted=false;}
        const auto baselineScene=baselineSession.outputJax()->completedScene();
        expect(baselineCompleted&&baselineScene)
            << "the same outer conversion completes without callback reentry";
        if(!outerScene||!baselineScene)return;

        svg::Serializer nestedSerializer(*nestedAdaptor),baselineSerializer(*baselineSession.adaptor());
        const auto nestedNodes=nestedSerializer.materialize(*outerScene);
        const auto baselineNodes=baselineSerializer.materialize(*baselineScene);
        bool same=nestedNodes.size()==baselineNodes.size();
        if(same)for(std::size_t i=0;i<nestedNodes.size();++i)
            same=same&&nestedAdaptor->outerHTML(nestedNodes[i])==
                baselineSession.adaptor()->outerHTML(baselineNodes[i]);
        expect(same)
            << "nested measurement preserves the outer frozen geometry";
    };

    "public Renderer renderScene returns the Common-produced geometry"_test=[] {
        Renderer renderer;
        const auto result=renderer.renderScene("\\text{漢字😀 é}+x");
        expect(!result.roots().empty()) << "public scene-only call returns owned geometry";
    };

    "scene-only public result owns geometry after renderer destruction"_test=[] {
        std::optional<output::scene::Scene> detached;
        {
            Renderer renderer;
            detached.emplace(renderer.renderScene("x+1"));
        }
        expect(detached&& !detached->roots().empty())
            << "returned FrozenScene remains readable after the renderer and session are destroyed";
    };
};
} // namespace
