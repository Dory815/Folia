#include "../../tests/support/folia_test.hpp"
import folia.math.output.scene;
import folia.math.output.svg.serializer;
import folia.math.adaptors.lite_adaptor;
import folia.math.adaptors.lite.lite_module;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace scene=folia::math::output::scene;
namespace svg=folia::math::output::svg;

std::u16string render(const scene::Scene& completed) {
    folia::math::options::Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    adaptors::lite::LiteModule module(context,functions,properties,regexp);
    const auto identity=context.construct(module.adaptorConstructor());
    const auto adaptor=module.resolveAdaptor(identity);
    svg::Serializer consumer(*adaptor);
    const auto roots=consumer.materialize(completed);
    if(roots.size()!=1)throw std::runtime_error("Expected one scene root");
    return consumer.serialize(roots.front());
}

suite sceneOwnership=[] {
    "completed_foreign_object_owns_html_and_pixel_to_math_transform"_test=[] {
        scene::Builder builder;
        PropertyList attributes;
        attributes.set("data-mjx-xml",Property(true));
        const auto node=builder.node(u"foreignObject",attributes);
        scene::ForeignObjectGeometry geometry;
        geometry.y=-8;geometry.width=16;geometry.height=12;
        geometry.transform.operations={
            {.kind=scene::TransformKind::Scale,.x=1.0/16,.y=1.0/16,.scalarScale=true},
            {.kind=scene::TransformKind::Matrix,.a=1,.d=-1}};
        node->geometry=geometry;
        node->paint=scene::ShapePaint{.fill=std::string("currentColor"),.stroke=std::string("none")};
        scene::HtmlNodePrimitive html;
        html.kind=u"span";html.namespaceHint=u"http://www.w3.org/1999/xhtml";
        html.attributes={{u"id",u"content"}};
        scene::HtmlNodePrimitive text;
        text.kind=u"#text";text.text=u"x&\U0001F642";
        scene::HtmlNodePrimitive comment;
        comment.kind=u"#comment";comment.text=u"<!--preserved-->";
        html.children={text,comment};
        node->semantic=html;
        builder.addRoot(node);
        const auto completed=std::move(builder).finish();
        const auto before=render(completed);
        expect(before.find(u"width=\"16px\"")!=std::u16string::npos);
        expect(before.find(u"scale(62.5) matrix(1 0 0 -1 0 0)")!=std::u16string::npos);
        expect(before.find(u"x&amp;\U0001F642<!--preserved-->")!=std::u16string::npos);

        std::get<scene::HtmlNodePrimitive>(node->semantic).children.clear();
        std::get<scene::ForeignObjectGeometry>(node->geometry).width=999;
        std::get<scene::ForeignObjectGeometry>(node->geometry).transform.operations.clear();
        node->paint->fill="red";
        const auto& frozenHtml=std::get<scene::HtmlNodePrimitive>(completed.roots().front()->semantic);
        const auto& frozenGeometry=std::get<scene::ForeignObjectGeometry>(completed.roots().front()->geometry);
        expect(frozenHtml.children.size()==2_u);
        expect(eq(frozenGeometry.width,16.0));
        expect(frozenGeometry.transform.operations.size()==2_u);
        expect(completed.roots().front()->paint->fill==std::optional<std::string>("currentColor"));
        expect(render(completed)==before);
    };
    "completed_scene_severs_mutable_builder_and_font_aliases"_test=[] {
        scene::Scene completed;
        scene::NodeRef retainedRoot,retainedGlyph;
        auto externalOutline=std::make_shared<scene::GlyphOutline>();
        externalOutline->sourcePath="M0 0L1000 0L1000 1000Z";
        externalOutline->commands={{'M',{0,0}},{'L',{1000,0}},{'L',{1000,1000}},{'Z',{}}};
        std::weak_ptr<const scene::GlyphOutline> originalOutline=externalOutline;
        {
            scene::Builder builder;
            PropertyList attributes;
            attributes.set("id",Property("saved"));
            retainedRoot=builder.node(u"svg",attributes);
            retainedGlyph=builder.node(u"path");
            builder.geometry(retainedGlyph,scene::PathGeometry{externalOutline});
            retainedGlyph->semantic=scene::GlyphPrimitive{0x78,"normal","test",externalOutline};
            builder.append(retainedRoot,retainedGlyph);
            builder.retain(externalOutline);
            builder.addRoot(retainedRoot);
            completed=std::move(builder).finish();
        }
        const auto before=render(completed);
        const auto frozenGlyph=completed.roots().front()->children.front();
        const auto geometryOutline=std::get<scene::PathGeometry>(frozenGlyph->geometry).outline;
        const auto semanticOutline=std::get<scene::GlyphPrimitive>(frozenGlyph->semantic).outline;
        expect(geometryOutline==semanticOutline);
        expect(geometryOutline==completed.resources().front());
        expect(geometryOutline.get()!=externalOutline.get());

        externalOutline->sourcePath="M9 9Z";
        externalOutline->commands.clear();
        externalOutline->coordinateUnitsPerEm=1;
        retainedGlyph->kind=u"rect";
        retainedGlyph->geometry=scene::RectGeometry{99,99,99,99};
        retainedRoot->attributes.set("id",Property("changed"));
        retainedRoot->children.clear();
        retainedRoot.reset();
        retainedGlyph.reset();
        externalOutline.reset();

        expect(originalOutline.expired());
        expect(geometryOutline->sourcePath=="M0 0L1000 0L1000 1000Z");
        expect(geometryOutline->commands.size()==4u);
        expect(geometryOutline->coordinateUnitsPerEm==1000.0);
        expect(render(completed)==before);
        expect(render(completed)==before);
    };

    "scene_consumers_share_full_precision_geometry_and_ordered_transforms"_test=[] {
        scene::Builder builder;
        const auto root=builder.node(u"svg");
        const auto rect=builder.node(u"rect");
        const scene::RectGeometry rule{.125,-.25,.333333333333,.0625};
        builder.geometry(rect,rule);
        scene::Transform transform;
        transform.operations={{scene::TransformKind::Scale,2,-1},
                              {scene::TransformKind::Translate,.125,-.25}};
        builder.transform(rect,transform);
        builder.append(root,rect);
        builder.addRoot(root);
        const auto completed=std::move(builder).finish();
        const auto child=completed.roots().front()->children.front();
        const auto& typed=std::get<scene::RectGeometry>(child->geometry);
        expect(typed.x==rule.x && typed.y==rule.y && typed.width==rule.width && typed.height==rule.height);
        expect(child->transform->operations.front().kind==scene::TransformKind::Scale);
        expect(child->transform->operations.back().kind==scene::TransformKind::Translate);
        const auto serialized=render(completed);
        expect(serialized.find(u"transform=\"scale(2,-1) translate(125,-250)\"")!=std::u16string::npos);
        expect(serialized.find(u"width=\"333.3\"")!=std::u16string::npos);
        expect(std::get<scene::RectGeometry>(child->geometry).width==rule.width);
        expect(render(completed)==serialized);
    };
};
}
