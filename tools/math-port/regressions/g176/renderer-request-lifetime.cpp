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

        // These exact profile cases throw from the Lite event binding during
        // layout. Unlike merror, they unwind an active wrapper job.
        for(const auto formula:{R"(\toggle{x}{y}\endtoggle)",R"(\mathtip{x+1}{y^2})"}) {
            bool gotLayoutError=false;
            try {(void)renderer.render(formula);}
            catch(const RenderError& error) {
                gotLayoutError=error.name=="TypeError"&&
                    std::string_view(error.what())=="node.addEventListener is not a function";
            }
            const auto afterLayoutError=RendererDiagnosticsProbe::snapshot(renderer);
            ok=ok&&gotLayoutError&&afterLayoutError[4]==wrapperHandles&&
                afterLayoutError[5]==retainedJobs;
            const auto recovered=renderer.render("x");
            ok=!recovered.svg.empty()&&ok;
        }

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
