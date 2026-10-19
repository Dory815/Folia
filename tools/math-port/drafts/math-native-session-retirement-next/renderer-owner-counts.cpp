import folia.math;
import std;

namespace {
using namespace folia::math;

bool hasGlyph(const output::scene::FrozenNodeRef& node,std::uint32_t codepoint) {
    if(!node)return false;
    if(const auto* glyph=std::get_if<output::scene::GlyphPrimitive>(&node->semantic);
       glyph&&glyph->codepoint==codepoint)return true;
    for(const auto& child:node->children)if(hasGlyph(child,codepoint))return true;
    return false;
}
bool hasGlyph(const RenderResult& result,std::uint32_t codepoint) {
    for(const auto& root:result.scene.roots())if(hasGlyph(root,codepoint))return true;
    return false;
}
void appendGlyphs(const output::scene::FrozenNodeRef& node,std::vector<std::uint32_t>& out) {
    if(!node)return;
    if(const auto* glyph=std::get_if<output::scene::GlyphPrimitive>(&node->semantic))
        out.push_back(glyph->codepoint);
    for(const auto& child:node->children)appendGlyphs(child,out);
}
std::vector<std::uint32_t> glyphs(const RenderResult& result) {
    std::vector<std::uint32_t> out;
    for(const auto& root:result.scene.roots())appendGlyphs(root,out);
    return out;
}
}

int main() try {
    RenderResult retained;
    bool persistentStateOk=true;
    {
        Renderer renderer;
        retained=renderer.render("x+1");
        (void)renderer.render("x"); // warm the persistent module/font path
        RendererDiagnosticsProbe::print(renderer,"warm");

        const auto macroDefinition=renderer.render(R"(\newcommand{\FoliaLifeMacro}{x}\FoliaLifeMacro)");
        const auto tagDefinition=renderer.render(
            R"(\begin{equation}\tag{A}\label{folia:lifetime:first}x\end{equation})");
        const auto reference=renderer.render(
            R"(\begin{equation}\tag{B}\label{folia:lifetime:second}\ref{folia:lifetime:first}\end{equation})");
        const auto macroUse=renderer.render(R"(\FoliaLifeMacro)");
        const auto plainX=renderer.render("x");
        persistentStateOk=!macroDefinition.svg.empty()&&!tagDefinition.svg.empty()&&
            glyphs(plainX)==std::vector<std::uint32_t>{0x1D465}&&
            glyphs(macroUse)==glyphs(plainX)&&glyphs(macroDefinition)==glyphs(plainX)&&
            glyphs(tagDefinition)==std::vector<std::uint32_t>{0x1D465,'(','A',')'}&&
            glyphs(reference)==std::vector<std::uint32_t>{'A','(','B',')'};
        RendererDiagnosticsProbe::print(renderer,"post_state_setup");
        for(std::size_t i=0;i<32;++i)(void)renderer.render("x");
        RendererDiagnosticsProbe::print(renderer,"after_32");
        for(std::size_t i=0;i<32;++i)(void)renderer.render("x");
        RendererDiagnosticsProbe::print(renderer,"after_64");
        // Keep `retained` alive while the same Renderer performs 64 more calls.
    }
    if(!persistentStateOk)
        std::cerr<<"persistent macro or tag/reference check failed\n";
    if(retained.svg.empty()||retained.scene.roots().empty()||
       !hasGlyph(retained,static_cast<std::uint32_t>('1'))) {
        std::cerr<<"saved RenderResult was not readable after Renderer destruction\n";
        persistentStateOk=false;
    }
    if(!persistentStateOk)return 2;
    std::cout<<"saved result remains readable after Renderer destruction\n";
    return 0;
} catch(const std::exception& error) {
    std::cerr<<"renderer owner counts: "<<error.what()<<'\n';
    return 1;
} catch(...) {
    std::cerr<<"renderer owner counts: non-standard exception\n";
    return 1;
}

