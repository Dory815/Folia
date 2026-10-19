#include "support/folia_test.hpp"
import folia.math;
import folia.math.output.scene;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.output.svg.serializer;
import folia.math.output.svg.scene_context_nodes;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace scene=folia::math::output::scene;
namespace svg=folia::math::output::svg;
namespace opts=folia::math::options;
namespace common=folia::math::output::common;
common::FontLoadHost fontHost(){
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&){};
    return host;
}
bool hasAttribute(const scene::FrozenNodeRef& node,std::string_view key){
    for(const auto& item:node->attributes)if(item.first==key)return true;
    return false;
}
bool hasAttributeValue(const scene::FrozenNodeRef& node,std::string_view key,std::string_view value){
    for(const auto& item:node->attributes)if(item.first==key&&item.second==value)return true;
    return false;
}
scene::FrozenNodeRef findKind(const scene::FrozenNodeRef& node,std::u16string_view kind){
    if(!node)return {};
    if(node->kind==kind)return node;
    for(const auto& child:node->children)if(auto found=findKind(child,kind))return found;
    return {};
}
bool hasNestedTooltipSvg(const scene::FrozenNodeRef& node){
    if(!node)return false;
    if(node->kind==u"mjx-tool"){
        const auto tip=findKind(node,u"mjx-tip");
        if(tip&&tip->children.size()==1&&tip->children.front()->kind==u"svg")return true;
    }
    for(const auto& child:node->children)if(hasNestedTooltipSvg(child))return true;
    return false;
}
}

suite maction_tooltip_scene_projection_g219=[] {
    "nested tooltip append is visible through the canonical frozen Scene tree"_test=[] {
        folia::math::Renderer renderer;
        const auto rendered=renderer.renderScene(uR"(\mathtip{x+1}{y^2})");
        expect(!rendered.roots().empty());
        if(rendered.roots().empty())return;
        expect(hasNestedTooltipSvg(rendered.roots().front()))
            << "the nested Lite processMath append is observed by the shared SourceDom child owner";
    };

    "Source helper absence and explicit typed foreignObject geometry stay distinct"_test=[] {
        folia::math::Renderer renderer;
        const auto rendered=renderer.renderScene(uR"(\mathtip{x+1}{y^2})");
        expect(!rendered.roots().empty());
        if(rendered.roots().empty())return;
        const auto foreign=findKind(rendered.roots().front(),u"foreignObject");
        expect(foreign!=nullptr);
        if(foreign){
            expect(std::holds_alternative<scene::ForeignObjectGeometry>(foreign->geometry));
            const auto* sourceGeometry=std::get_if<scene::ForeignObjectGeometry>(&foreign->geometry);
            if(sourceGeometry)expect(!sourceGeometry->synthesizeMissingAttributes)
                << "the Source svg helper uses current attributes as the presence authority";
            const bool hasGeometryAttribute=hasAttribute(foreign,"y")||hasAttribute(foreign,"width")||
                hasAttribute(foreign,"height")||hasAttribute(foreign,"transform");
            expect(!hasGeometryAttribute)
                << "Source helper attributes do not contain default geometry fields";
        }

        scene::Builder builder;
        auto typed=builder.node(u"foreignObject");
        builder.geometry(typed,scene::ForeignObjectGeometry{
            .x=2,.y=3,.width=4,.height=5});
        builder.addRoot(typed);
        auto typedScene=std::move(builder).finish();
        const auto& typedRoot=typedScene.roots().front();
        const auto* geometry=std::get_if<scene::ForeignObjectGeometry>(&typedRoot->geometry);
        expect(geometry!=nullptr);
        if(geometry){
            expect(geometry->x==2.0&&geometry->y==3.0&&
                geometry->width==4.0&&geometry->height==5.0);
            expect(geometry->synthesizeMissingAttributes)
                << "typed Builder geometry keeps its declared emission defaults";
            expect(std::holds_alternative<std::monostate>(typedRoot->semantic))
                << "typed Builder geometry retains its existing non-SourceDom serialization contract";
        }
    };

    "Source helper attributes remain authoritative across add and remove edits"_test=[] {
        scene::Builder builder;
        PropertyList attributes;
        attributes.setOwn("style",Property("display: none;"));
        attributes.setOwn("y",Property("0px"));
        auto helper=builder.node(u"foreignObject",attributes);
        scene::ForeignObjectGeometry helperGeometry;
        helperGeometry.synthesizeMissingAttributes=false;
        builder.geometry(helper,helperGeometry);
        builder.addRoot(helper);

        auto snapshot=[&] {
            scene::Builder capture;
            capture.addRoot(helper);
            return std::move(capture).finish().roots().front();
        };
        auto frozen=snapshot();
        expect(hasAttributeValue(frozen,"y","0px"));

        helper->attributes.erase("y");
        frozen=snapshot();
        expect(!hasAttribute(frozen,"y"))
            << "removing an initially explicit field does not restore typed zero geometry";

        helper->attributes.setOwn("width",Property("0px"));
        frozen=snapshot();
        expect(hasAttributeValue(frozen,"width","0px"));
        helper->attributes.erase("width");
        frozen=snapshot();
        expect(!hasAttribute(frozen,"width"))
            << "adding then removing a default-valued field leaves it absent";
        const auto* geometry=std::get_if<scene::ForeignObjectGeometry>(&frozen->geometry);
        expect(geometry&&!geometry->synthesizeMissingAttributes);
    };

    "SceneContextNodes replacement and geometry-owner freeze serialize current attributes"_test=[] {
        svg::NativeRenderSession session(fontHost());
        auto& context=session.context();
        auto adaptor=session.adaptor();
        expect(adaptor!=nullptr);
        if(!adaptor)return;
        auto& sceneNodes=session.services().sceneContextNodes();

        const auto serializeNode=[&](scene::NodeRef node){
            scene::Builder builder;
            const auto root=builder.node(u"svg");
            builder.append(root,node);
            builder.addRoot(root);
            const auto frozen=std::move(builder).finish();
            svg::Serializer serializer(*adaptor);
            const auto materialized=serializer.materialize(frozen);
            return materialized.empty()?std::u16string{}:adaptor->outerHTML(materialized.front());
        };
        const auto foreignTag=[](const std::u16string& markup){
            const auto start=markup.find(u"<foreignObject");
            if(start==std::u16string::npos)return std::u16string{};
            const auto end=markup.find(u'>',start);
            return end==std::u16string::npos?std::u16string{}:markup.substr(start,end-start+1);
        };
        const auto attributesObject=[&](bool includeY,bool includeWidth){
            const auto attributes=context.object();
            context.set(attributes,opts::Key(u"style"),opts::Value(u"display: none;"));
            if(includeY)context.set(attributes,opts::Key(u"y"),opts::Value(u"0px"));
            if(includeWidth)context.set(attributes,opts::Key(u"width"),opts::Value(u"0px"));
            return attributes;
        };

        scene::Builder sourceHelperBuilder;
        auto sourceHelper=sourceHelperBuilder.node(u"foreignObject",PropertyList{
            {"style",Property("display: none;")},{"y",Property("7px")},
            {"width",Property("8px")},{"height",Property("9px")}});
        const auto helperHandle=sceneNodes.handle(sourceHelper);
        auto* normalizedGeometry=std::get_if<scene::ForeignObjectGeometry>(&sourceHelper->geometry);
        expect(normalizedGeometry!=nullptr);
        if(normalizedGeometry){
            expect(normalizedGeometry->y==7.0&&normalizedGeometry->width==8.0&&
                normalizedGeometry->height==9.0)
                << "public handle normalization imports explicit helper dimensions";
            normalizedGeometry->synthesizeMissingAttributes=false;
        }
        const auto initialTag=foreignTag(serializeNode(sourceHelper));
        expect(initialTag.find(u"y=\"7px\"")!=std::u16string::npos);
        expect(initialTag.find(u"width=\"8px\"")!=std::u16string::npos);
        expect(initialTag.find(u"height=\"9px\"")!=std::u16string::npos);

        context.set(opts::Value(helperHandle),opts::Key(u"attributes"),
            opts::Value(attributesObject(false,false)));
        const auto removedTag=foreignTag(serializeNode(sourceHelper));
        expect(!removedTag.empty());
        expect(removedTag.find(u"y=")==std::u16string::npos&&removedTag.find(u"width=")==std::u16string::npos&&
            removedTag.find(u"height=")==std::u16string::npos&&removedTag.find(u"transform=")==std::u16string::npos)
            << "whole replacement removes explicit y without typed fallback fields";

        context.set(opts::Value(helperHandle),opts::Key(u"attributes"),
            opts::Value(attributesObject(false,true)));
        const auto addedTag=foreignTag(serializeNode(sourceHelper));
        expect(addedTag.find(u"width=\"0px\"")!=std::u16string::npos);
        expect(addedTag.find(u"y=")==std::u16string::npos&&addedTag.find(u"height=")==std::u16string::npos&&
            addedTag.find(u"transform=")==std::u16string::npos)
            << "an explicitly added default value serializes without synthesizing other fields";

        context.set(opts::Value(helperHandle),opts::Key(u"attributes"),
            opts::Value(attributesObject(false,false)));
        const auto removedAgainTag=foreignTag(serializeNode(sourceHelper));
        expect(removedAgainTag.find(u"width=")==std::u16string::npos)
            << "removing the added width leaves it absent in serialized SVG";

        const auto outputOptions=context.object();
        context.set(outputOptions,opts::Key(u"fontCache"),opts::Value(u"none"));
        const auto jax=session.construct(opts::Value(outputOptions));
        auto document=svg::createNativeSvgDocument(session);
        (void)document;
        const auto sourceJax=jax->instanceIdentity();
        const auto originalSvg=context.get(opts::Value(sourceJax),opts::Key(u"svg"));
        const auto* originalSvgFunction=std::get_if<opts::ObjectRef>(&originalSvg.data);
        expect(originalSvgFunction!=nullptr);
        if(!originalSvgFunction)return;
        const auto sourceAttributes=attributesObject(true,false);
        const std::array<opts::Value,3> sourceSvgArgs={opts::Value(u"foreignObject"),
            opts::Value(sourceAttributes),opts::Value(context.array())};
        const auto sourceIdentity=context.call(*originalSvgFunction,
            opts::Value(sourceJax),sourceSvgArgs);
        auto sourceOwned=std::make_shared<scene::Node>();
        sourceOwned->kind=u"foreignObject";
        sourceOwned->semantic=scene::SourceDomPrimitive{};
        sourceOwned->geometry=scene::ForeignObjectGeometry{
            .synthesizeMissingAttributes=false};
        sceneNodes.associateSourceNode(sourceOwned,sourceIdentity);
        const auto sourceHandle=sceneNodes.handle(sourceOwned);
        context.set(opts::Value(sourceHandle),opts::Key(u"attributes"),
            opts::Value(attributesObject(false,false)));

        const auto sourceOwnedTag=foreignTag(serializeNode(sourceOwned));
        expect(!sourceOwnedTag.empty());
        expect(sourceOwnedTag.find(u"y=")==std::u16string::npos&&sourceOwnedTag.find(u"width=")==std::u16string::npos&&
            sourceOwnedTag.find(u"height=")==std::u16string::npos&&sourceOwnedTag.find(u"transform=")==std::u16string::npos)
            << "SourceDom attributes remain authoritative through replacement and serialization";
    };
};
