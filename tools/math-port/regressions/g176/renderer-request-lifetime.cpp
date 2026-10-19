import folia.math;
import std;

namespace {
using namespace folia::math;

bool hasGlyph(const output::scene::FrozenNodeRef& node,std::uint32_t value) {
    if(!node)return false;
    if(const auto* glyph=std::get_if<output::scene::GlyphPrimitive>(&node->semantic);
       glyph&&glyph->codepoint==value)return true;
    for(const auto& child:node->children)if(hasGlyph(child,value))return true;
    return false;
}
bool hasGlyph(const RenderResult& result,std::uint32_t value) {
    for(const auto& root:result.scene.roots())if(hasGlyph(root,value))return true;
    return false;
}
bool hasMathError(const RenderResult& result) {
    return result.svg.find("data-mjx-error=")!=std::string::npos||
        result.svg.find("data-mml-node=\"merror\"")!=std::string::npos;
}
bool hasPositiveActionRect(const output::scene::FrozenNodeRef& node) {
    if(!node)return false;
    if(node->kind==u"rect")if(const auto* rect=std::get_if<output::scene::RectGeometry>(&node->geometry))
        if(rect->width>0&&rect->height>0)return true;
    for(const auto& child:node->children)if(hasPositiveActionRect(child))return true;
    return false;
}
bool hasActionListenerAndGeometry(const output::scene::Scene& scene,
                                  std::span<const std::u16string_view> expected) {
    bool found=false;
    const auto visit=[&](const auto& self,const output::scene::FrozenNodeRef& node)->void {
        if(!node)return;
        bool isAction=false;
        for(const auto& [key,value]:node->attributes)
            if(key=="data-mml-node"&&value=="maction"){isAction=true;break;}
        if(isAction&&node->eventTypes.size()==expected.size()&&
           std::equal(node->eventTypes.begin(),node->eventTypes.end(),expected.begin(),
               [](const std::u16string& actual,std::u16string_view wanted){return actual==wanted;})&&
           hasPositiveActionRect(node))found=true;
        for(const auto& child:node->children)self(self,child);
    };
    for(const auto& root:scene.roots())visit(visit,root);
    return found;
}
void collectGlyphs(const output::scene::FrozenNodeRef& node,std::vector<std::uint32_t>& result) {
    if(!node)return;
    if(const auto* glyph=std::get_if<output::scene::GlyphPrimitive>(&node->semantic))
        result.push_back(glyph->codepoint);
    for(const auto& child:node->children)collectGlyphs(child,result);
}
std::vector<std::uint32_t> glyphs(const RenderResult& result) {
    std::vector<std::uint32_t> values;
    for(const auto& root:result.scene.roots())collectGlyphs(root,values);
    return values;
}
}

int main() try {
    RenderResult retained;
    bool ok=true;
    {
        Renderer renderer;
        retained=renderer.render("x+1");
        (void)renderer.render("x");
        const auto afterWarm=RendererDiagnosticsProbe::snapshot(renderer);
        const auto wrapperHandles=afterWarm[4];
        const auto retainedJobs=afterWarm[5];

        const auto macro=renderer.render(
            R"(\newcommand{\FoliaRetireMacro}{x}\FoliaRetireMacro)");
        const auto firstLabel=renderer.render(
            R"(\begin{equation}\tag{A}\label{folia:retire:first}x\end{equation})");
        ok=ok&&!macro.svg.empty()&&!firstLabel.svg.empty();

        for(std::size_t i=0;i<256;++i) {
            const auto value=renderer.render("x+\\frac{1}{2}");
            ok=ok&&!value.svg.empty();
        }
        const auto repeated=RendererDiagnosticsProbe::snapshot(renderer);
        ok=ok&&repeated[4]==wrapperHandles&&repeated[5]==retainedJobs;

        // A TeX parse error is returned as a rendered merror, not a C++
        // exception. It still completes a copied result and its wrapper job
        // must be retired; a following conversion must remain usable.
        const auto parseError=renderer.render(R"(\frac{)");
        ok=ok&&hasMathError(parseError);
        const auto afterError=RendererDiagnosticsProbe::snapshot(renderer);
        ok=ok&&afterError[4]==wrapperHandles&&afterError[5]==retainedJobs;
        const auto afterParseErrorRetry=renderer.render("x");
        ok=ok&&!afterParseErrorRetry.svg.empty();

        // Pinned Source renders both action nodes successfully. Their event
        // registrations and positive action rectangle are frozen in the Scene.
        const std::array<std::u16string_view,1> toggleEvents{u"click"};
        const auto toggle=renderer.render(R"(\toggle{x}{y}\endtoggle)");
        ok=ok&&!toggle.svg.empty()&&!hasMathError(toggle)&&
            toggle.svg.find("data-toggle=\"1\"")!=std::string::npos&&
            hasActionListenerAndGeometry(toggle.scene,toggleEvents);
        const auto afterToggle=RendererDiagnosticsProbe::snapshot(renderer);
        ok=ok&&afterToggle[4]==wrapperHandles&&afterToggle[5]==retainedJobs;

        const std::array<std::u16string_view,2> tooltipEvents{u"mouseover",u"mouseout"};
        const auto tooltip=renderer.render(R"(\mathtip{x+1}{y^2})");
        ok=ok&&!tooltip.svg.empty()&&!hasMathError(tooltip)&&
            tooltip.svg.find("<foreignObject")!=std::string::npos&&
            tooltip.svg.find("<mjx-tip>")!=std::string::npos&&
            hasActionListenerAndGeometry(tooltip.scene,tooltipEvents);
        const auto afterTooltip=RendererDiagnosticsProbe::snapshot(renderer);
        ok=ok&&afterTooltip[4]==wrapperHandles&&afterTooltip[5]==retainedJobs;

        for(std::size_t i=0;i<16;++i) {
            const auto scene=renderer.renderScene("x+1");
            ok=ok&&!scene.roots().empty();
        }
        const auto afterSceneOnly=RendererDiagnosticsProbe::snapshot(renderer);
        ok=ok&&afterSceneOnly[4]==wrapperHandles&&afterSceneOnly[5]==retainedJobs;

        // A pre-request cancellation has no completed job to retire and leaves
        // the same Renderer available for later calls.
        std::stop_source stopped;
        stopped.request_stop();
        bool gotCancellation=false;
        try {(void)renderer.render("x",stopped.get_token(),{});}
        catch(const RenderError& error) { gotCancellation=error.name=="Cancelled"; }
        ok=ok&&gotCancellation;

        const auto macroUse=renderer.render(R"(\FoliaRetireMacro)");
        const auto plainX=renderer.render("x");
        const auto reference=renderer.render(
            R"(\begin{equation}\tag{B}\label{folia:retire:second}\ref{folia:retire:first}\end{equation})");
        ok=ok&&glyphs(macroUse)==glyphs(plainX)&&
            hasGlyph(reference,static_cast<std::uint32_t>('A'))&&
            hasGlyph(reference,static_cast<std::uint32_t>('B'));
        const auto afterState=RendererDiagnosticsProbe::snapshot(renderer);
        ok=ok&&afterState[4]==wrapperHandles&&afterState[5]==retainedJobs;

        // Scene is a copied immutable result; retirement only removes private
        // Source wrapper identities after this copy and SVG serialization.
        const auto savedScene=retained.scene;
        ok=ok&&!savedScene.roots().empty();
    }
    ok=ok&&!retained.svg.empty()&&hasGlyph(retained,static_cast<std::uint32_t>('1'));

    // A separate Renderer exercises cleanup during an actual in-flight
    // cancellation. The deadline is deliberately not used to assert which
    // internal checkpoint wins; it must, however, leave no request wrapper
    // or LayoutJob lease behind before the unusable Renderer is destroyed.
    {
        Renderer cancelledRenderer;
        (void)cancelledRenderer.render("x");
        const auto before=RendererDiagnosticsProbe::snapshot(cancelledRenderer);
        // Prepare UTF-16 before starting the stop request. The UTF-8 overload
        // can cancel during decoding, before the native session is entered;
        // that correctly leaves the Renderer usable and is a different case.
        std::u16string largeFormula;
        largeFormula.reserve(36000);
        for(std::size_t i=0;i<12000;++i)largeFormula+=u"x+";
        largeFormula+=u'x';
        std::stop_source inFlightStop;
        std::jthread stopRequest([&]{
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            inFlightStop.request_stop();
        });
        bool cancelledDuringRequest=false;
        try {
            (void)cancelledRenderer.render(largeFormula,inFlightStop.get_token(),{});
        } catch(const RenderError& error) {
            cancelledDuringRequest=error.name=="Cancelled"&&
                std::string_view(error.what()).find("session is unusable")!=std::string_view::npos;
            if(!cancelledDuringRequest)
                std::cerr<<"in-flight cancellation: "<<error.name<<": "<<error.what()<<'\n';
        }
        stopRequest.join();
        const auto after=RendererDiagnosticsProbe::snapshot(cancelledRenderer);
        ok=ok&&cancelledDuringRequest&&after[4]==before[4]&&after[5]==before[5];
    }
    if(!ok) {
        std::cerr<<"layout-job retirement candidate check failed\n";
        return 1;
    }
    std::cout<<"completed wrapper jobs retired; macro, labels, and saved results remain usable\n";
    return 0;
} catch(const std::exception& error) {
    std::cerr<<"renderer request lifetime: "<<error.what()<<'\n';
    return 1;
} catch(...) {
    std::cerr<<"renderer request lifetime: non-standard exception\n";
    return 1;
}
