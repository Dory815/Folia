#include "support/folia_test.hpp"
import folia.math.output.svg.scene_context_nodes;
import folia.math.output.svg.serializer;
import folia.math.util.context_property_bindings;
import folia.math.util.regexp_host;
import folia.math.util.option_types;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::options;
namespace scene=folia::math::output::scene;
namespace svg=folia::math::output::svg;
namespace lite=folia::math::adaptors::lite;
void expectSameFunction(const Value& left,const Value& right){
    const auto* a=std::get_if<ObjectRef>(&left.data);
    const auto* b=std::get_if<ObjectRef>(&right.data);
    expect(a&&b&&*a==*b);
}

suite scene_context_nodes = [] {
"svg_scene_node_projection_reuses_the_same_handle_for_one_node"_test=[] {
    Context context;
    svg::SceneContextNodes projection(context,u"TestSceneNode");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    scene::Builder builder;
    const auto node=builder.node(u"g");
    const auto owner=context.object(),definition=context.object();
    const auto first=projection.handle(node,owner,definition);
    const auto second=projection.handle(node);
    expect(first==second);
    expect(projection.resolve(first)==node);
    expect(projection.owner(second)==owner);
    expect(projection.definition(second)==definition);
    expect(projection.isHandle(Value(second)));
};

"svg_scene_handle_fields_are_live_views_of_the_same_builder_node"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    svg::SceneContextNodes projection(context,u"TestSceneNode");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    scene::Builder builder;
    const auto node=builder.node(u"g");
    const auto handle=projection.handle(node);
    expect(std::get<std::u16string>(context.get(Value(handle),Key(u"kind")).data)==u"g");
    const auto attributes=context.get(Value(handle),Key(u"attributes"));
    context.set(attributes,Key(u"data-test"),Value(u"value"));
    expect(property_host::string(node->attributes.get("data-test"))==u"value");

    const auto child=builder.node(u"text");
    const auto childHandle=projection.handle(child);
    const auto childrenValue=context.get(Value(handle),Key(u"children"));
    const auto* children=std::get_if<ObjectRef>(&childrenValue.data);
    expect(children!=nullptr);
    context.arrayPush(*children,std::array<Value,1>{Value(childHandle)});
    expect(node->children.size()==1_u);
    expect(node->children[0]==child);
    expect(context.get(Value(handle),Key(u"children"))==childrenValue);
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(Undefined{})); // direct push does not assign parent
};

"svg_scene_children_replacement_keeps_old_array_bound_to_previous_owner"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    svg::SceneContextNodes projection(context,u"TestSceneNode");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    scene::Builder builder;
    const auto node=builder.node(u"g");
    node->semantic=scene::SourceDomPrimitive{};
    const auto handle=projection.handle(node);
    const auto oldChild=builder.node(u"text");
    const auto replacementChild=builder.node(u"path");
    const auto laterOldChild=builder.node(u"circle");
    const auto oldChildHandle=projection.handle(oldChild);
    const auto replacementChildHandle=projection.handle(replacementChild);
    const auto laterOldChildHandle=projection.handle(laterOldChild);

    const auto oldChildrenValue=context.get(Value(handle),Key(u"children"));
    const auto* oldChildren=std::get_if<ObjectRef>(&oldChildrenValue.data);
    expect(oldChildren!=nullptr);
    if(!oldChildren)return;
    context.arrayPush(*oldChildren,std::array<Value,1>{Value(oldChildHandle)});
    expect(context.get(Value(handle),Key(u"children"))==oldChildrenValue);

    const auto replacement=context.array();
    context.arrayPush(replacement,std::array<Value,1>{Value(replacementChildHandle)});
    context.set(Value(handle),Key(u"children"),Value(replacement));
    expect(context.get(Value(handle),Key(u"children"))==Value(replacement));
    context.arrayPush(*oldChildren,std::array<Value,1>{Value(laterOldChildHandle)});
    expect(context.arrayLength(*oldChildren)==2_u);
    expect(context.arrayLength(replacement)==1_u);
    expect(context.get(replacement,Key(u"0"))==Value(replacementChildHandle));
    expect(node->children.size()==1_u);
    expect(node->children[0]==replacementChild);
};

"svg_scene_geometry_attributes_are_live_typed_slots_with_source_spelling"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    svg::SceneContextNodes projection(context,u"TestSceneNode");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    scene::Builder builder;
    PropertyList rectAttributes;
    rectAttributes.setOwn("x",Property("1250"));
    rectAttributes.setOwn("width",Property("2000"));
    const auto rect=builder.node(u"rect",rectAttributes);
    const auto rectHandle=projection.handle(rect);
    const auto projected=context.get(Value(rectHandle),Key(u"attributes"));
    const auto alias=context.get(Value(rectHandle),Key(u"attributes"));
    expect(alias==projected);
    expect(context.get(projected,Key(u"x"))==Value(u"1250"));
    expect(std::get<scene::RectGeometry>(rect->geometry).x==1.25);
    context.set(alias,Key(u"x"),Value(u"3000"));
    expect(std::get<scene::RectGeometry>(rect->geometry).x==3.0);
    expect(context.get(projected,Key(u"x"))==Value(u"3000"));
    context.erase(std::get<ObjectRef>(projected.data),Key(u"x"));
    expect(!context.hasOwn(alias,Key(u"x")));
    expect(std::get<scene::RectGeometry>(rect->geometry).x==0.0);

    PropertyList transformAttributes;
    transformAttributes.setOwn("transform",Property("scale(2,-1) rotate(90 1000 2000)"));
    const auto group=builder.node(u"g",transformAttributes);
    const auto groupHandle=projection.handle(group);
    const auto operations=group->transform->operations;
    expect(operations.size()==2_u);
    expect(operations[0].kind==scene::TransformKind::Scale);
    expect(operations[0].x==2.0&&operations[0].y==-1.0);
    expect(operations[1].kind==scene::TransformKind::Rotate);
    expect(operations[1].degrees==90.0);
    expect(operations[1].centerX==1.0&&operations[1].centerY==2.0);
    const auto groupProjected=context.get(Value(groupHandle),Key(u"attributes"));
    expect(context.get(groupProjected,Key(u"transform"))==Value(u"scale(2,-1) rotate(90 1000 2000)"));
    builder.addRoot(group);
    auto adaptor=module.constructAdaptor();
    svg::Serializer serializer(*adaptor);
    auto serialized=serializer.serialize(serializer.materialize(std::move(builder).finish()).front());
    expect(serialized.find(u"scale(2,-1) rotate(90 1000 2000)")!=std::u16string::npos);
    context.set(groupProjected,Key(u"transform"),Value(u"scale(3,-4)"));
    expect(group->transform->operations.size()==1_u);
    expect(group->transform->operations[0].x==3.0&&group->transform->operations[0].y==-4.0);
    expect(context.get(groupProjected,Key(u"transform"))==Value(u"scale(3,-4)"));

    PropertyList viewportAttributes;
    viewportAttributes.setOwn("width",Property("75%"));
    viewportAttributes.setOwn("height",Property("50%"));
    viewportAttributes.setOwn("preserveAspectRatio",Property("xMinYMax slice"));
    viewportAttributes.setOwn("overflow",Property("visible"));
    const auto viewport=builder.node(u"svg",viewportAttributes);
    const auto viewportHandle=projection.handle(viewport);
    const auto viewportAlias=context.get(Value(viewportHandle),Key(u"attributes"));
    const auto& viewportGeometry=std::get<scene::ViewportGeometry>(viewport->geometry);
    expect(viewportGeometry.widthRelative&&viewportGeometry.width==0.75);
    expect(viewportGeometry.heightRelative&&viewportGeometry.height==0.5);
    expect(viewportGeometry.alignment==scene::ViewportAlignment::XMinYMax);
    expect(viewportGeometry.fit==scene::ViewportFit::Slice&&!viewportGeometry.clipToViewport);
    context.set(viewportAlias,Key(u"width"),Value(u"40%"));
    expect(std::get<scene::ViewportGeometry>(viewport->geometry).width==0.4);
    expect(context.get(viewportAlias,Key(u"width"))==Value(u"40%"));
    context.erase(std::get<ObjectRef>(viewportAlias.data),Key(u"overflow"));
    expect(std::get<scene::ViewportGeometry>(viewport->geometry).clipToViewport);

    PropertyList lineAttributes;
    lineAttributes.setOwn("stroke-width",Property("250"));
    lineAttributes.setOwn("stroke-dasharray",Property("100 250 500"));
    const auto line=builder.node(u"line",lineAttributes);
    const auto lineHandle=projection.handle(line);
    const auto lineAlias=context.get(Value(lineHandle),Key(u"attributes"));
    expect(std::get<scene::LineGeometry>(line->geometry).width==0.25);
    expect(line->paint&&line->paint->dash&&line->paint->dash->lengths.size()==3_u);
    context.set(lineAlias,Key(u"stroke-dasharray"),Value(u"80 120"));
    expect(line->paint->dash->lengths==std::vector<double>{0.08,0.12});
    expect(context.get(lineAlias,Key(u"stroke-dasharray"))==Value(u"80 120"));
    context.set(lineAlias,Key(u"stroke-width"),Value(u"750"));
    expect(std::get<scene::LineGeometry>(line->geometry).width==0.75);

    PropertyList foreignAttributes;
    foreignAttributes.setOwn("x",Property("12px"));
    foreignAttributes.setOwn("width",Property("400px"));
    const auto foreign=builder.node(u"foreignObject",foreignAttributes);
    const auto foreignHandle=projection.handle(foreign);
    const auto foreignAlias=context.get(Value(foreignHandle),Key(u"attributes"));
    expect(std::get<scene::ForeignObjectGeometry>(foreign->geometry).x==12.0);
    expect(std::get<scene::ForeignObjectGeometry>(foreign->geometry).width==400.0);
    context.set(foreignAlias,Key(u"width"),Value(u"700px"));
    expect(std::get<scene::ForeignObjectGeometry>(foreign->geometry).width==700.0);
    expect(context.get(foreignAlias,Key(u"width"))==Value(u"700px"));

    PropertyList typedAttributes;
    typedAttributes.setOwn("width",Property("9000"));
    const auto typedRect=builder.node(u"rect",typedAttributes);
    typedRect->geometry=scene::RectGeometry{.width=4.0};
    const auto typedHandle=projection.handle(typedRect);
    const auto typedAlias=context.get(Value(typedHandle),Key(u"attributes"));
    expect(std::get<scene::RectGeometry>(typedRect->geometry).width==4.0);
    expect(context.get(typedAlias,Key(u"width"))==Value(u"4000"));
};

"svg_scene_node_target_restores_nested_targets_after_exception"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    const auto adaptorObject=context.construct(module.adaptorConstructor());
    const auto adaptor=module.resolveAdaptor(adaptorObject);
    svg::SceneContextNodes projection(context,u"TestSceneNode");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    scene::Builder outerBuilder,innerBuilder;
    const auto outer=outerBuilder.node(u"g"),inner=innerBuilder.node(u"g");
    outerBuilder.append(outer,inner);
    const auto outerHandle=projection.handle(outer),innerHandle=projection.handle(inner);
    auto originalAppend=context.get(Value(adaptorObject),Key(u"append"));
    auto originalAttribute=context.get(Value(adaptorObject),Key(u"setAttribute"));
    {
        auto outerScope=projection.installTargets(module,outerBuilder);
        {
            auto innerScope=projection.installTargets(module,innerBuilder);
            bool threw=false;
            try{
                const auto append=context.get(Value(adaptorObject),Key(u"append"));
                const auto* function=std::get_if<ObjectRef>(&append.data);
                const std::array<Value,2> args{Value(innerHandle),Value(outerHandle)};
                (void)context.call(*function,Value(adaptorObject),args);
            }catch(const Exception& error){threw=error.name=="TypeError";}
            expect(threw);
        }
        expectSameFunction(context.get(Value(adaptorObject),Key(u"append")),originalAppend);
        expectSameFunction(context.get(Value(adaptorObject),Key(u"setAttribute")),originalAttribute);
        const auto append=context.get(Value(adaptorObject),Key(u"append"));
        const auto* function=std::get_if<ObjectRef>(&append.data);
        const std::array<Value,2> args{Value(outerHandle),Value(innerHandle)};
        (void)context.call(*function,Value(adaptorObject),args);
        expect(outer->children.size()==1_u);
        expect(inner->children.empty());
    }
    expectSameFunction(context.get(Value(adaptorObject),Key(u"append")),originalAppend);
    expectSameFunction(context.get(Value(adaptorObject),Key(u"setAttribute")),originalAttribute);
};

"svg_scene_presentation_writes_keep_full_precision_and_only_match_the_exact_source_value"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    const auto adaptorObject=context.construct(module.adaptorConstructor());
    svg::SceneContextNodes projection(context,u"TestSceneNode");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    scene::Builder builder;
    const auto node=builder.node(u"g");
    scene::TransformOperation rotate;
    rotate.kind=scene::TransformKind::Rotate;
    rotate.degrees=90.123456789;
    rotate.centerX=1.000123456;
    rotate.centerY=2.000456789;
    node->transform=scene::Transform{};
    node->transform->operations.push_back(rotate);
    const auto handle=projection.handle(node);
    {
        auto target=projection.installTargets(module,builder);
        const auto setAttribute=context.get(Value(adaptorObject),Key(u"setAttribute"));
        const std::array<Value,3> roundedArgs={Value(handle),Value(u"transform"),Value(u"rotate(90.1 1000.1 2000.5)")};
        projection.withPresentationAttribute(handle,u"transform",roundedArgs[2],[&]{
            (void)context.call(std::get<ObjectRef>(setAttribute.data),Value(adaptorObject),roundedArgs);
        });
        expect(node->transform->operations[0].degrees==90.123456789);
        expect(node->transform->operations[0].centerX==1.000123456);
        expect(property_host::string(node->attributes.get("transform"))==u"rotate(90.1 1000.1 2000.5)");

        const std::array<Value,3> replacementArgs={Value(handle),Value(u"transform"),Value(u"scale(3,-4)")};
        projection.withPresentationAttribute(handle,u"transform",Value(u"scale(2,-1)"),[&]{
            (void)context.call(std::get<ObjectRef>(setAttribute.data),Value(adaptorObject),replacementArgs);
        });
        expect(node->transform->operations.size()==1_u);
        expect(node->transform->operations[0].x==3.0&&node->transform->operations[0].y==-4.0);

        bool threw=false;
        try{
            projection.withPresentationAttribute(handle,u"transform",Value(u"scale(5)"),[]{
                throw 73;
            });
        }catch(int value){threw=value==73;}
        expect(threw);
        const std::array<Value,3> afterThrowArgs={Value(handle),Value(u"transform"),Value(u"scale(5)")};
        (void)context.call(std::get<ObjectRef>(setAttribute.data),Value(adaptorObject),afterThrowArgs);
        expect(node->transform->operations[0].x==5.0&&node->transform->operations[0].y==5.0);
    }
};

"svg_scene_targets_preserve_children_order_namespace_and_styles_owner"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    const auto adaptorObject=context.construct(module.adaptorConstructor());
    svg::SceneContextNodes projection(context,u"TestSceneNode");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    scene::Builder builder;
    const auto parent=builder.node(u"g"),otherParent=builder.node(u"g");
    const auto first=builder.node(u"text"),second=builder.node(u"path");
    const auto parentHandle=projection.handle(parent),otherParentHandle=projection.handle(otherParent);
    const auto firstHandle=projection.handle(first),secondHandle=projection.handle(second);
    {
        auto scope=projection.installTargets(module,builder);
        const auto append=context.get(Value(adaptorObject),Key(u"append"));
        const std::array<Value,2> appendFirst{Value(parentHandle),Value(firstHandle)};
        const std::array<Value,2> appendSecond{Value(parentHandle),Value(secondHandle)};
        (void)context.call(std::get<ObjectRef>(append.data),Value(adaptorObject),appendFirst);
        (void)context.call(std::get<ObjectRef>(append.data),Value(adaptorObject),appendSecond);
        (void)context.call(std::get<ObjectRef>(append.data),Value(adaptorObject),appendFirst);
        (void)context.call(std::get<ObjectRef>(append.data),Value(adaptorObject),
            std::array<Value,2>{Value(otherParentHandle),Value(firstHandle)});
        const auto setAttribute=context.get(Value(adaptorObject),Key(u"setAttribute"));
        const std::array<Value,4> attrArgs{Value(parentHandle),Value(u"href"),Value(u"#ref"),Value(u"http://www.w3.org/1999/xlink")};
        (void)context.call(std::get<ObjectRef>(setAttribute.data),Value(adaptorObject),attrArgs);
    const std::array<Value,3> transformArgs{Value(parentHandle),Value(u"transform"),
        Value(u"scale(2,-1) rotate(90 1000 2000)")};
    (void)context.call(std::get<ObjectRef>(setAttribute.data),Value(adaptorObject),transformArgs);
        const auto setStyle=context.get(Value(adaptorObject),Key(u"setStyle"));
        const std::array<Value,3> styleArgs{Value(parentHandle),Value(u"color"),Value(u"red")};
        (void)context.call(std::get<ObjectRef>(setStyle.data),Value(adaptorObject),styleArgs);
    }
    expect(parent->children.size()==1_u);
    expect(parent->children[0]==second);
    expect(otherParent->children.size()==1_u&&otherParent->children[0]==first);
    expect(property_host::string(parent->attributes.get("xlink:href"))==u"#ref");
    expect(parent->transform->operations.size()==2_u);
    expect(parent->transform->operations[0].x==2.0&&parent->transform->operations[0].y==-1.0);
    expect(parent->transform->operations[1].degrees==90.0);
    expect(property_host::string(parent->attributes.get("style"))==u"color: red;");
    expect(context.get(Value(firstHandle),Key(u"parent"))==Value(otherParentHandle));
    expect(context.get(Value(secondHandle),Key(u"parent"))==Value(parentHandle));
    const auto styles=context.get(Value(parentHandle),Key(u"styles"));
    const auto* stylesObject=std::get_if<ObjectRef>(&styles.data);
    expect(stylesObject!=nullptr);
    expect(context.get(styles,Key(u"cssText"))==Value(u"color: red;"));
    const auto parentAttributes=context.get(Value(parentHandle),Key(u"attributes"));
    context.set(parentAttributes,Key(u"style"),Value(u"color: blue;"));
    const auto refreshedStyles=context.get(Value(parentHandle),Key(u"styles"));
    expect(std::get<ObjectRef>(refreshedStyles.data)!=*stylesObject);
    expect(context.get(refreshedStyles,Key(u"cssText"))==Value(u"color: blue;"));
};
};
}
