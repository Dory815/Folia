module folia.math.output.scene.foundation;
import folia.math.util.property_host;
import folia.math.cancellation;

namespace folia::math::output::scene {
namespace {
using OutlineMap=std::unordered_map<const GlyphOutline*,std::shared_ptr<const GlyphOutline>>;
std::shared_ptr<const GlyphOutline> freezeOutline(
    const std::shared_ptr<const GlyphOutline>& outline,OutlineMap& copies){
    if(!outline)return {};
    if(const auto found=copies.find(outline.get());found!=copies.end())return found->second;
    cancellation::checkpoint(cancellation::CheckpointStage::Scene);
    auto frozen=std::make_shared<const GlyphOutline>(*outline);
    cancellation::checkpoint(cancellation::CheckpointStage::Scene);
    copies.emplace(outline.get(),frozen);
    return frozen;
}
Geometry freezeGeometry(const Geometry& geometry,OutlineMap& outlines){
    if(const auto* path=std::get_if<PathGeometry>(&geometry))
        return PathGeometry{freezeOutline(path->outline,outlines)};
    return geometry;
}
Semantic freezeSemantic(const Semantic& semantic,OutlineMap& outlines){
    if(const auto* glyph=std::get_if<GlyphPrimitive>(&semantic)){
        auto frozen=*glyph;
        frozen.outline=freezeOutline(glyph->outline,outlines);
        return frozen;
    }
    if(const auto* text=std::get_if<SourceTextPrimitive>(&semantic)){
        auto frozen=*text;
        cancellation::checkpoint(cancellation::CheckpointStage::Scene);
        frozen.text=text->currentText();
        cancellation::checkpoint(cancellation::CheckpointStage::Scene);
        frozen.owner.reset();
        return frozen;
    }
    return semantic;
}
FrozenNodeRef freezeNode(const NodeRef& source,
                         std::unordered_map<const Node*,FrozenNodeRef>& copies,
                         OutlineMap& outlines){
    if(!source)return {};
    cancellation::checkpoint(cancellation::CheckpointStage::Scene);
    if(const auto found=copies.find(source.get());found!=copies.end())return found->second;
    // The completed record is independent even if a caller keeps the mutable
    // Builder handle returned by Builder::node().
    auto target=std::make_shared<FrozenNode>();
    copies.emplace(source.get(),target);
    target->kind=source->kind;
    const auto sourceAttributeKeys=source->attributes.keys();
    target->attributes.reserve(sourceAttributeKeys.size());
    for(const auto& key:sourceAttributeKeys){
        cancellation::checkpoint(cancellation::CheckpointStage::Scene);
        const auto value=source->attributes.getOwn(key);
        target->attributes.emplace_back(
            key,property_host::debugText(property_host::string(value)));
    }
    Geometry currentGeometry;
    std::optional<Transform> currentTransform;
    cancellation::checkpoint(cancellation::CheckpointStage::Scene);
    source->applyGeometryOwner(currentGeometry,currentTransform,target->attributes);
    cancellation::checkpoint(cancellation::CheckpointStage::Scene);
    target->geometry=freezeGeometry(currentGeometry,outlines);
    target->semantic=freezeSemantic(source->semantic,outlines);
    target->eventTypes=source->eventTypes;
    auto currentPaint=source->paint;
    if(source->paintOwner)source->paintOwner->apply(currentPaint,currentGeometry,target->attributes);
    target->paint=std::move(currentPaint);
    target->transform=std::move(currentTransform);
    target->transformAttributeIndex=source->transformAttributeIndex;
    target->transformSuffix=source->transformSuffix;
    const auto sourceChildren=source->children.snapshot();
    target->children.reserve(sourceChildren.size());
    for(const auto& child:sourceChildren){
        cancellation::checkpoint(cancellation::CheckpointStage::Scene);
        target->children.push_back(freezeNode(child,copies,outlines));
    }
    return target;
}
}
void Scene::freeze(std::span<const NodeRef> roots,
                   std::span<const std::shared_ptr<const GlyphOutline>> resources){
    std::unordered_map<const Node*,FrozenNodeRef> copies;
    OutlineMap outlines;
    for(const auto& root:roots){
        if(!root)property_host::typeError("Cannot add null scene root");
        roots_.push_back(freezeNode(root,copies,outlines));
    }
    resources_.reserve(resources.size());
    for(const auto& resource:resources){
        cancellation::checkpoint(cancellation::CheckpointStage::Scene);
        if(!resource)property_host::typeError("Cannot retain null scene resource");
        resources_.push_back(freezeOutline(resource,outlines));
    }
}
}
