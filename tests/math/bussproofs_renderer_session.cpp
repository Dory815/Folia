#include "support/folia_test.hpp"
import folia.math;
import folia.math.input.tex.bussproofs.configuration;
import folia.math.output.common;
import folia.math.output.svg.render_session;

namespace {
using namespace boost::ut;
using namespace folia::math;
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
};
} // namespace
