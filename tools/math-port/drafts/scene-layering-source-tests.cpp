#include "../../../tests/support/folia_test.hpp"
import folia.math;

namespace {
using namespace boost::ut;
namespace scene=folia::math::output::scene;
using folia::math::Renderer;
using folia::math::RenderOptions;

void walkScene(const scene::FrozenNodeRef& node,
               const std::function<void(const scene::FrozenNode&)>& visit){
    if(!node)return;
    visit(*node);
    for(const auto& child:node->children)walkScene(child,visit);
}
void walkScene(const scene::Scene& completed,
               const std::function<void(const scene::FrozenNode&)>& visit){
    for(const auto& root:completed.roots())walkScene(root,visit);
}
bool attribute(const scene::FrozenNode& node,std::string_view key){
    return std::ranges::any_of(node.attributes,[&](const auto& entry){return entry.first==key;});
}
double verticalScale(const scene::Transform& transform){
    if(transform.operations.empty())return transform.scaleY;
    double scale=1;
    for(const auto& operation:transform.operations)
        if(operation.kind==scene::TransformKind::Scale)scale*=operation.y;
    return scale;
}

suite sceneLayeringSource=[] {
    "scene_layering_unknown_text_keeps_font_size_and_local_inversion"_test=[] {
        scene::Scene completed;
        {
            Renderer renderer;
            completed=renderer.render(u"\\text{漢字\U0001F600}").scene;
        }
        std::size_t textRuns=0;
        walkScene(completed,[&](const scene::FrozenNode& node){
            if(node.kind!=u"text")return;
            const auto* text=std::get_if<scene::TextRunPrimitive>(&node.semantic);
            if(!text||text->text.empty())return;
            ++textRuns;
            // Pinned NewCM x_height=.442, ex=8, em=16. The original
            // unknownText call writes font-size=884px and scale(1,-1).
            expect(text->fontSize.has_value());
            if(text->fontSize)expect(std::abs(*text->fontSize-.884)<1e-12);
            expect(node.transform.has_value());
            if(node.transform)expect(verticalScale(*node.transform)==-1.0);
        });
        expect(textRuns>0_u);
    };

    "scene_layering_broken_borders_keep_typed_dash_lengths_and_caps"_test=[] {
        Renderer renderer;
        for(const bool dotted:{false,true}){
            const auto tex=dotted?u"\\bbox[border:2px dotted red]{x+y}":
                u"\\bbox[border:2px dashed red]{x+y}";
            const auto result=renderer.render(tex);
            std::size_t lines=0;
            std::vector<std::array<double,7>> observations;
            walkScene(result.scene,[&](const scene::FrozenNode& node){
                if(node.kind!=u"line"||!node.paint||!node.paint->dash)return;
                ++lines;
                const auto& dash=*node.paint->dash;
                expect(dash.lengths.size()==2_u);
                expect(node.paint->lineCap==(dotted?scene::LineCap::Round:scene::LineCap::Square));
                if(dash.lengths.size()!=2)return;
                if(dotted)expect(dash.lengths[0]==.001);
                else expect(std::abs(dash.lengths[1]-3*dash.lengths[0])<1e-12);
                expect(node.paint->stroke==std::optional<std::string>("red"));
                const auto* geometry=std::get_if<scene::LineGeometry>(&node.geometry);
                expect(geometry!=nullptr);
                if(geometry)observations.push_back({geometry->from.x,geometry->from.y,
                    geometry->to.x,geometry->to.y,geometry->width,dash.lengths[0],dash.lengths[1]});
            });
            expect(lines==4_u);
            // Original addBorderBroken fixed() arguments, captured by the
            // companion observer before SVG rounding in the full profile.
            const std::vector<std::array<double,7>> expected=dotted
                ?std::vector<std::array<double,7>>{
                    {-.06025,.64325,2.3446944444444444,.64325,.1105,.001,.21663131313131312},
                    {2.3446944444444444,-.26525,2.3446944444444444,.64325,.1105,.001,.1797},
                    {-.06025,-.26525,2.3446944444444444,-.26525,.1105,.001,.21663131313131312},
                    {-.06025,-.26525,-.06025,.64325,.1105,.001,.1797}}
                :std::vector<std::array<double,7>>{
                    {-.06025,.64325,2.3446944444444444,.64325,.1105,.09619777777777777,.2885933333333333},
                    {2.3446944444444444,-.26525,2.3446944444444444,.64325,.1105,.10094444444444445,.30283333333333334},
                    {-.06025,-.26525,2.3446944444444444,-.26525,.1105,.09619777777777777,.2885933333333333},
                    {-.06025,-.26525,-.06025,.64325,.1105,.10094444444444445,.30283333333333334}};
            for(const auto& wanted:expected)
                expect(std::ranges::any_of(observations,[&](const auto& actual){
                    for(std::size_t i=0;i<wanted.size();++i)
                        if(std::abs(actual[i]-wanted[i])>1e-10)return false;
                    return true;
                }));
        }
    };

    "scene_layering_nested_viewports_retain_source_clip_and_alignment_policy"_test=[] {
        Renderer renderer;
        const auto stretched=renderer.render(u"\\left(\\rule{1em}{8em}\\right)");
        std::size_t clipped=0,unclipped=0;
        walkScene(stretched.scene,[&](const scene::FrozenNode& node){
            const auto* viewport=std::get_if<scene::ViewportGeometry>(&node.geometry);
            if(!viewport)return;
            if(viewport->clipToViewport)++clipped;else ++unclipped;
            expect(viewport->alignment==scene::ViewportAlignment::XMidYMid);
            expect(viewport->fit==scene::ViewportFit::Meet);
        });
        expect(clipped>0_u);
        expect(unclipped==1_u);

        const auto labelled=renderer.render(
            u"\\begin{align}x&=1\\tag{A}\\\\y&=2\\tag{B}\\end{align}",RenderOptions{true});
        std::size_t relativeViews=0;
        walkScene(labelled.scene,[&](const scene::FrozenNode& node){
            const auto* viewport=std::get_if<scene::ViewportGeometry>(&node.geometry);
            if(!viewport||(!attribute(node,"data-table")&&!attribute(node,"data-labels")))return;
            ++relativeViews;
            expect(!viewport->clipToViewport);
            expect(viewport->widthRelative&&viewport->heightRelative);
            expect(viewport->width==1.0&&viewport->height==1.0);
            expect(viewport->hasViewBox&&viewport->viewWidth==.001);
            if(attribute(node,"data-labels"))expect(viewport->alignment==scene::ViewportAlignment::XMaxYMid);
        });
        expect(relativeViews==2_u);
    };

    "scene_layering_notation_paths_and_arrow_transforms_keep_unrounded_operands"_test=[] {
        Renderer renderer;
        const auto result=renderer.render(u"\\enclose{longdiv phasorangle northeastarrow}{x+y}");
        std::size_t paths=0;
        bool preciseArc=false,preciseEndpoint=false,preciseRotation=false;
        walkScene(result.scene,[&](const scene::FrozenNode& node){
            if(std::holds_alternative<scene::GlyphPrimitive>(node.semantic))return;
            const auto* path=std::get_if<scene::PathGeometry>(&node.geometry);
            if(!path||!path->outline)return;
            ++paths;
            // Original path()/fill() receive these values before fixed().
            // Source d spells the radius as 183.3 and endpoint as 3001.1.
            for(const auto& command:path->outline->commands){
                if(command.operation=='a'&&command.arguments.size()>=2&&
                   std::abs(command.arguments[0]-183.25)<1e-10&&
                   std::abs(command.arguments[1]-493.5)<1e-10)preciseArc=true;
                if(command.operation=='L'&&!command.arguments.empty()&&
                   std::abs(command.arguments[0]-3001.133260779457)<1e-10)preciseEndpoint=true;
            }
            if(node.transform)for(const auto& operation:node.transform->operations)
                if(operation.kind==scene::TransformKind::Rotate&&
                   std::abs(operation.degrees-22.46798288170405)<1e-10&&
                   std::abs(operation.centerX-1.5173166303897285)<1e-10)
                    preciseRotation=true;
        });
        expect(paths==3_u);
        expect(preciseArc&&preciseEndpoint&&preciseRotation);
    };
};
}
