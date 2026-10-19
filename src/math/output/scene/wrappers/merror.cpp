/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/merror.ts placement.
 *************************************************************/
module folia.math.output.scene;
import :merror;
import :svg_paint;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
std::vector<NodeRef> MerrorPlacement::build(common::CommonWrapper& wrapper,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto roots=createSvgRoots(wrapper,builder,parents);
    decorateSvgWrapper(wrapper,builder,roots);
    const auto box=wrapper.getBBox();
    if(roots.empty())property_host::typeError("Cannot read properties of undefined (reading 'append')");
    {
        const auto& root=roots.front();
        if(!root)property_host::typeError("Cannot read properties of null (reading 'append')");
        const auto fixed=[&](double value){return Property(wrapper.jax().fixed(value*1000,1));};
        auto background=builder.node(u"rect",PropertyList{{"data-background",Property(true)},
            {"width",fixed(box->w)},{"height",fixed(box->h+box->d)},{"y",fixed(-box->d)}});
        RectGeometry rect{0,-box->d,box->w,box->h+box->d};
        rect.fields={RectField::Width,RectField::Height,RectField::Y,RectField::X};
        rect.fieldCount=3;
        background->geometry=std::move(rect);
        ShapePaint paint;paint.fill="yellow";paint.stroke="none";paint.emitAttributes=false;
        background->paint=std::move(paint);
        builder.append(root,background);
        const auto attrs=wrapper.node->attributes();
        if(!attrs)property_host::typeError("Cannot read properties of null (reading 'get')");
        const auto title=attrs->get("title");
        if(property_host::truthy(title)){
            const auto text=builder.node(u"#text",{},property_host::string(title));
            const auto titleNode=builder.node(u"title",{}, {},1);
            builder.append(titleNode,text);
            builder.append(root,titleNode);
        }
    }
    addSvgBaseChildren(wrapper,builder,placements,roots);
    for(const auto& root:roots)if(root)for(const auto& child:root->children.snapshot()){
        if(child&&child->kind==u"g"){
            ShapePaint paint;paint.fill="red";paint.stroke="red";paint.emitAttributes=false;
            child->paint=std::move(paint);
        }
    }
    return roots;
}

void registerMerrorPlacement(WrapperFactory& factory){
    factory.registerKind("merror",std::make_shared<MerrorPlacement>());
}
}
