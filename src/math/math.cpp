module folia.math;
import folia.math.output.svg.native_document;
import folia.math.cancellation;
import folia.math.util.native_identities;
import folia.math.util.property_host;
import std;

namespace folia::math {
namespace {
namespace svg=output::svg;
namespace common=output::common;
namespace opts=options;

// The package and output profile is the one used by Folia's MathJax entry.
constexpr std::array<std::u16string_view,38> packages={
    u"base",u"action",u"ams",u"amscd",u"bbm",u"bboldx",u"bbox",u"begingroup",
    u"boldsymbol",u"braket",u"bussproofs",u"cancel",u"cases",u"centernot",
    u"color",u"colortbl",u"configmacros",u"dsfont",u"empheq",u"enclose",
    u"extpfeil",u"fontsizev3",u"gensymb",u"html",u"mathtools",u"mhchem",
    u"newcommand",u"noundefined",u"physics",u"setoptions",u"tagformat",u"texhtml",
    u"textcomp",u"textmacros",u"unicode",u"units",u"upgreek",u"verb"};

void fontWarning(const std::any& reason) {
    if(const auto* error=std::any_cast<opts::Exception>(&reason)) {
        std::cerr<<error->name<<": "<<property_host::debugText(error->message)<<'\n';
    } else if(const auto* text=std::any_cast<std::string>(&reason)) {
        std::cerr<<*text<<'\n';
    } else if(const auto* text=std::any_cast<std::u16string>(&reason)) {
        std::cerr<<property_host::debugText(*text)<<'\n';
    } else if(const auto* error=std::any_cast<std::exception_ptr>(&reason)) {
        if(!*error)throw std::invalid_argument("Empty native font exception");
        try {std::rethrow_exception(*error);}
        catch(const opts::Exception& value) {
            std::cerr<<value.name<<": "<<property_host::debugText(value.message)<<'\n';
        }
        catch(const std::exception& value) {std::cerr<<value.what()<<'\n';}
    } else {
        throw std::runtime_error("Native font warning has no text representation");
    }
}

common::FontLoadHost fontHost() {
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=fontWarning;
    host.consoleWarn=[](const std::vector<std::any>& arguments) {
        for(const auto& argument:arguments)fontWarning(argument);
    };
    return host;
}
}

struct Renderer::State {
    svg::NativeRenderSession session;
    std::shared_ptr<handlers::html::HTMLMathDocument> document;
    std::shared_ptr<common::LayoutJob> pendingLayoutRetirement;
    bool unusable=false;
    std::optional<mml::MmlFactory::ProductMark> requestMmlMark;
    std::optional<RequestIdentityCapture> requestMmlCapture;
    std::optional<RequestIdentityCapture> requestNodeArrayCapture;
    std::optional<adaptors::lite::LiteModule::RequestMark> requestLiteMark;
    std::optional<svg::SceneContextNodes::RequestMark> requestSceneMark;
    bool requestOwnersActive=false;
    bool requestItemRetired=false,requestMmlIdentitiesRetired=false,
         requestNodeArraysRetired=false,requestSceneRetired=false,requestProductsRetired=false,requestLiteRetired=false;

    std::array<std::size_t,3> sceneOwnerDiagnostics()const {
        const auto counts=session.services().sceneContextNodes().requestOwnerCountsForDiagnostics();
        return {counts.handles,counts.sourceIdentities,counts.sourceViews};
    }

    Renderer::OwnerCounts diagnostics()const {
        const auto contextCounts=session.context().allocationCountsForDiagnostics();
        auto& services=session.services();
        const auto wrapperCounts=services.commonFactories().ownerCountsForDiagnostics();
        return {contextCounts.objectSlots,contextCounts.symbolSlots,
            services.mmlFactory().productCountForDiagnostics(),
            services.parser().canonicalIdentityHandleCountForDiagnostics(),
            wrapperCounts.wrapperIdentities,wrapperCounts.retainedLayoutJobs,
            services.sceneContextNodes().handleCountForDiagnostics(),
            services.lite().store()->recordCountForDiagnostics(),
            services.lite().nodeIdentityCountForDiagnostics()};
    }

    State():session(fontHost(),{svg::NewcmFontExtension::bboldx,
            svg::NewcmFontExtension::bbm,svg::NewcmFontExtension::dsfont}) {
        auto& context=session.context();
        const auto outputOptions=context.object();
        context.set(outputOptions,opts::Key(u"fontCache"),opts::Value(u"none"));
        context.set(outputOptions,opts::Key(u"exFactor"),opts::Value(.5));
        context.set(outputOptions,opts::Key(u"displayOverflow"),opts::Value(u"linebreak"));
        const auto linebreaks=context.object();
        context.set(linebreaks,opts::Key(u"inline"),opts::Value(true));
        context.set(outputOptions,opts::Key(u"linebreaks"),opts::Value(linebreaks));
        (void)session.construct(opts::Value(outputOptions));
        const auto inputOptions=context.object();
        const auto selected=context.array(static_cast<std::uint32_t>(packages.size()));
        for(std::size_t index=0;index<packages.size();++index) {
            const auto key=std::to_string(index);
            context.set(selected,opts::Key(std::u16string(key.begin(),key.end())),
                opts::Value(std::u16string(packages[index])));
        }
        context.set(inputOptions,opts::Key(u"packages"),opts::Value(selected));
        document=svg::createNativeSvgDocument(session,opts::Undefined{},opts::Value(inputOptions));
    }

    void retireCurrentLayoutJob(){
        if(!pendingLayoutRetirement){
            auto jax=session.outputJax();
            pendingLayoutRetirement=jax
                ?jax->takeCurrentLayoutJobForPrivateRendererRetirement()
                :std::shared_ptr<common::LayoutJob>{};
        }
        if(pendingLayoutRetirement){
            (void)session.services().commonFactories().retireRendererLayoutJob(pendingLayoutRetirement);
            pendingLayoutRetirement.reset();
        }
    }

    void beginRequestOwners(){
        retireCurrentRequestOwners();
        auto& services=session.services();auto& runtime=services.parser();
        requestSceneMark=services.sceneContextNodes().beginRequestScope();
        try{
            requestMmlMark=services.mmlFactory().productMark();
            requestLiteMark=services.lite().requestMark();
            requestMmlCapture=runtime.mmlNodeIdentities.beginRequestCapture();
            try{requestNodeArrayCapture=runtime.nodeArrayIdentities.beginRequestCapture();}
            catch(...){runtime.mmlNodeIdentities.retireCapturedRequestBindings(*requestMmlCapture);requestMmlCapture.reset();throw;}
            try{document->beginRequestOwnerScope();}
            catch(...){runtime.nodeArrayIdentities.retireCapturedRequestBindings(*requestNodeArrayCapture);requestNodeArrayCapture.reset();runtime.mmlNodeIdentities.retireCapturedRequestBindings(*requestMmlCapture);requestMmlCapture.reset();throw;}
        }catch(...){
            services.sceneContextNodes().retireRequestScope(*requestSceneMark);
            requestSceneMark.reset();throw;
        }
        requestItemRetired=requestMmlIdentitiesRetired=requestNodeArraysRetired=
            requestSceneRetired=requestProductsRetired=requestLiteRetired=false;
        requestOwnersActive=true;
    }

    void retireCurrentRequestOwners(){
        if(!requestOwnersActive)return;
        auto& services=session.services();auto& runtime=services.parser();
        if(!requestItemRetired){document->retireCompletedRequestItem();requestItemRetired=true;}
        if(!requestMmlIdentitiesRetired){runtime.mmlNodeIdentities.retireCapturedRequestBindings(*requestMmlCapture);requestMmlCapture.reset();requestMmlIdentitiesRetired=true;}
        if(!requestNodeArraysRetired){runtime.nodeArrayIdentities.retireCapturedRequestBindings(*requestNodeArrayCapture);requestNodeArrayCapture.reset();requestNodeArraysRetired=true;}
        if(!requestSceneRetired){services.sceneContextNodes().retireRequestScope(*requestSceneMark);requestSceneRetired=true;}
        if(!requestProductsRetired){services.mmlFactory().retireProductsFrom(*requestMmlMark);requestProductsRetired=true;}
        if(!requestLiteRetired){services.lite().retireRequestNodesSince(*requestLiteMark);requestLiteRetired=true;}
        (void)runtime.retireTagInfoOwnersOutsidePrivateRendererRoots();
        (void)runtime.retireLabelOwnersOutsidePrivateRendererRoots();
        requestMmlMark.reset();requestLiteMark.reset();requestSceneMark.reset();requestOwnersActive=false;
    }

    struct RequestLayoutScope {
        State& state;
        bool closed=false;
        explicit RequestLayoutScope(State& value):state(value){
            state.retireCurrentLayoutJob();state.retireCurrentRequestOwners();state.beginRequestOwners();
        }
        void close(){state.retireCurrentLayoutJob();state.retireCurrentRequestOwners();closed=true;}
        ~RequestLayoutScope(){
            if(!closed){
                bool layoutRetired=false;
                try{state.retireCurrentLayoutJob();layoutRetired=true;}catch(...){}
                // Do not destroy MML products while a failed job retirement
                // may leave wrappers pointing into the request product suffix.
                if(layoutRetired)try{state.retireCurrentRequestOwners();}catch(...){}
            }
        }
    };

    DOMNodeRef convertMath(std::u16string_view tex,RenderOptions options) {
        auto& context=session.context();
        const auto settings=context.object();
        const double em=options.em&&!std::isnan(options.em)?options.em:16;
        const double width=options.containerWidth&&!std::isnan(options.containerWidth)
            ?options.containerWidth:1280;
        context.set(settings,opts::Key(u"display"),opts::Value(options.display));
        context.set(settings,opts::Key(u"em"),opts::Value(em));
        context.set(settings,opts::Key(u"ex"),opts::Value(em*.5));
        context.set(settings,opts::Key(u"containerWidth"),opts::Value(width));
        const auto result=document->convert(opts::Value(std::u16string(tex)),opts::Value(settings));
        const auto nodeValue=propertyNode(document->propertyBindings().from(result));
        const auto* node=std::get_if<DOMNodeRef>(&nodeValue);
        if(!node||!*node)
            throw opts::Exception("TypeError",u"Native math conversion did not return a DOM node");
        return *node;
    }

    RenderResult render(std::u16string_view tex,RenderOptions options) {
        RequestLayoutScope requestScope(*this);
        const auto node=convertMath(tex,options);
        const auto* completed=session.outputJax()->completedScene();
        if(!completed)
            throw opts::Exception("TypeError",u"Native math conversion did not complete a Scene");
        RenderResult result{property_host::debugText(document->domAdaptor().serializeXML(node)),*completed};
        requestScope.close();
        return result;
    }

    output::scene::Scene renderScene(std::u16string_view tex,RenderOptions options) {
        RequestLayoutScope requestScope(*this);
        std::shared_ptr<const output::scene::Scene> captured;
        auto jax=session.outputJax();
        if(!jax)throw opts::Exception("TypeError",u"Render session has no SVG output Jax");
        auto previous=jax->exchangeSceneOutputConsumer(
            [&captured](std::shared_ptr<const output::scene::Scene> value,DOMNodeRef){
                captured=std::move(value);
            });
        struct ConsumerScope {
            svg::SvgOutputJax& jax;
            svg::SvgOutputJax::SceneOutputConsumer previous;
            ~ConsumerScope(){(void)jax.exchangeSceneOutputConsumer(std::move(previous));}
        } scope{*jax,std::move(previous)};
        (void)convertMath(tex,options);
        if(!captured)
            throw opts::Exception("TypeError",u"Native math conversion did not produce a Scene");
        auto result=*captured;
        requestScope.close();
        return result;
    }
};

Renderer::OwnerCounts Renderer::diagnosticsForTest()const{return state_->diagnostics();}
std::array<std::size_t,24> RendererDiagnosticsProbe::parserIdentitySnapshot(const Renderer& renderer){
    return renderer.state_->session.services().parser().identityRegistryBreakdownForDiagnostics();
}
std::array<std::size_t,3> RendererDiagnosticsProbe::sceneOwnerSnapshot(const Renderer& renderer){
    return renderer.state_->sceneOwnerDiagnostics();
}
std::vector<std::pair<std::uint64_t,int>> RendererDiagnosticsProbe::liteNodeIdentitySnapshot(const Renderer& renderer){
    std::vector<std::pair<std::uint64_t,int>> result;
    for(const auto& [id,kind]:renderer.state_->session.services().lite().nodeIdentitySnapshotForDiagnostics())
        result.emplace_back(id,static_cast<int>(kind));
    return result;
}
std::vector<std::tuple<std::uint64_t,int,std::string>> RendererDiagnosticsProbe::liteNodeIdentityNamedSnapshot(const Renderer& renderer){
    std::vector<std::tuple<std::uint64_t,int,std::string>> result;
    for(const auto& [id,kind,name]:renderer.state_->session.services().lite().nodeIdentityNamedSnapshotForDiagnostics())
        result.emplace_back(id,static_cast<int>(kind),name);
    return result;
}
std::array<std::size_t,9> RendererDiagnosticsProbe::snapshot(const Renderer& renderer){
    const auto c=renderer.diagnosticsForTest();
    return {c.contextObjectSlots,c.contextSymbolSlots,c.mmlFactoryProducts,
        c.parserRegistryEntries,c.wrapperHandles,c.wrapperRetainedJobs,
        c.sceneHandles,c.liteRecords,c.liteNodeIdentities};
}
void RendererDiagnosticsProbe::print(const Renderer& renderer,std::string_view stage){
    const auto c=renderer.diagnosticsForTest();
    std::cout<<stage
        <<" context_object_slots="<<c.contextObjectSlots
        <<" context_symbol_slots="<<c.contextSymbolSlots
        <<" mml_factory_products="<<c.mmlFactoryProducts
        <<" parser_registry_entries="<<c.parserRegistryEntries
        <<" wrapper_handles="<<c.wrapperHandles
        <<" wrapper_retained_jobs="<<c.wrapperRetainedJobs
        <<" scene_handles="<<c.sceneHandles
        <<" lite_records="<<c.liteRecords
        <<" lite_node_identities="<<c.liteNodeIdentities
        <<'\n';
}

Renderer::Renderer() try :state_(std::make_unique<State>()){}
catch(const opts::Exception& error) {
    throw RenderError(error.name,property_host::debugText(error.message));
}
Renderer::~Renderer()=default;
RenderResult Renderer::render(std::u16string_view tex,RenderOptions options) {
    return render(tex,std::stop_token{},options);
}
RenderResult Renderer::render(std::u16string_view tex,std::stop_token stop,RenderOptions options) {
    cancellation::Request request(stop,options.deadline);
    if(state_->unusable)
        throw RenderError("SessionUnusable","Native math renderer session is unusable after in-flight cancellation");
    try { request.checkpoint(); }
    catch(const cancellation::Cancelled&) { throw RenderError("Cancelled","Math rendering cancelled"); }
    cancellation::Scope scope(request);
    try {
        auto completed=state_->render(tex,options);
        request.checkpoint();
        return completed;
    } catch(const cancellation::Cancelled&) {
        state_->unusable=true;
        throw RenderError("Cancelled","Math rendering cancelled; native session is unusable");
    }

    catch(const opts::Exception& error) {
        throw RenderError(error.name,property_host::debugText(error.message));
    }
}
RenderResult Renderer::render(std::string_view tex,RenderOptions options) {
    return render(tex,std::stop_token{},options);
}
RenderResult Renderer::render(std::string_view tex,std::stop_token stop,RenderOptions options) {
    cancellation::Request request(stop,options.deadline);
    if(state_->unusable)
        throw RenderError("SessionUnusable","Native math renderer session is unusable after in-flight cancellation");
    try {
        request.checkpoint();
        cancellation::Scope scope(request);
        auto converted=property_host::text(tex);
        request.checkpoint();
        return render(std::u16string_view(converted),stop,options);
    } catch(const cancellation::Cancelled&) {
        throw RenderError("Cancelled","Math rendering cancelled");
    }
}
output::scene::Scene Renderer::renderScene(std::u16string_view tex,RenderOptions options) {
    return renderScene(tex,std::stop_token{},options);
}
output::scene::Scene Renderer::renderScene(std::u16string_view tex,std::stop_token stop,RenderOptions options) {
    cancellation::Request request(stop,options.deadline);
    if(state_->unusable)
        throw RenderError("SessionUnusable","Native math renderer session is unusable after in-flight cancellation");
    try { request.checkpoint(); }
    catch(const cancellation::Cancelled&) { throw RenderError("Cancelled","Math rendering cancelled"); }
    cancellation::Scope scope(request);
    try {
        auto completed=state_->renderScene(tex,options);
        request.checkpoint();
        return completed;
    } catch(const cancellation::Cancelled&) {
        state_->unusable=true;
        throw RenderError("Cancelled","Math rendering cancelled; native session is unusable");
    } catch(const opts::Exception& error) {
        throw RenderError(error.name,property_host::debugText(error.message));
    }
}
output::scene::Scene Renderer::renderScene(std::string_view tex,RenderOptions options) {
    return renderScene(tex,std::stop_token{},options);
}
output::scene::Scene Renderer::renderScene(std::string_view tex,std::stop_token stop,RenderOptions options) {
    cancellation::Request request(stop,options.deadline);
    if(state_->unusable)
        throw RenderError("SessionUnusable","Native math renderer session is unusable after in-flight cancellation");
    try {
        request.checkpoint();
        cancellation::Scope scope(request);
        auto converted=property_host::text(tex);
        request.checkpoint();
        return renderScene(std::u16string_view(converted),stop,options);
    } catch(const cancellation::Cancelled&) {
        throw RenderError("Cancelled","Math rendering cancelled");
    }
}
std::string Renderer::toSvg(std::u16string_view tex,RenderOptions options) {
    return render(tex,options).svg;
}
std::string Renderer::toSvg(std::string_view tex,RenderOptions options) {
    return render(tex,options).svg;
}
std::string Renderer::toSvg(std::u16string_view tex,std::stop_token stop,RenderOptions options) {
    return render(tex,stop,options).svg;
}
std::string Renderer::toSvg(std::string_view tex,std::stop_token stop,RenderOptions options) {
    return render(tex,stop,options).svg;
}
}
