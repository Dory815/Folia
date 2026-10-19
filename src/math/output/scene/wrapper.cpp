module folia.math.output.scene;
import :wrapper;
import :svg_paint;
import folia.math.util.property_host;
import folia.math.cancellation;

namespace folia::math::output::scene {
std::vector<NodeRef> SvgWrapperPlacement::build(common::CommonWrapper& wrapper,
    Builder& builder,WrapperFactory& placements,std::span<const NodeRef> parents){
    cancellation::checkpoint(cancellation::CheckpointStage::Layout);
    if(auto split=tryEmbellishedSvg(wrapper,builder,placements,parents))return *split;
    auto roots=createSvgRoots(wrapper,builder,parents);
    decorateSvgWrapper(wrapper,builder,roots);
    placeContent(wrapper,builder,placements,roots);
    return roots;
}

void WrapperFactory::registerKind(std::string kind,std::shared_ptr<Placement> placement){
    if(!placement)property_host::typeError("Cannot register null scene placement");
    if(!placements_.emplace(std::move(kind),std::move(placement)).second)
        property_host::typeError("Scene placement kind already registered");
}
std::vector<NodeRef> WrapperFactory::build(common::CommonWrapper& wrapper,Builder& builder,
                                           std::span<const NodeRef> parents){
    const auto selected=wrapper.selectedClass();
    const auto* metadata=selected
        ?std::any_cast<common::CommonWrapperClassData>(&selected->staticData):nullptr;
    if(!metadata)
        throw options::Exception("TypeError",u"Scene wrapper has no selected Source class descriptor");
    // Source dispatches toSVG on the class selected by WrapperFactory.create,
    // including SvgWrapper for nodes such as mstyle. The original MML kind
    // still labels the node, but does not select its rendering algorithm again.
    const auto found=placements_.find(metadata->kind);
    if(found==placements_.end())
        throw options::Exception("Unimplemented",u"SVG scene placement is not registered for this MML kind");
    struct WrapperScope {
        Builder& builder;
        common::CommonWrapper* previous;
        WrapperScope(Builder& value,common::CommonWrapper& current)
            :builder(value),previous(builder.exchangeCurrentWrapper(&current)){}
        ~WrapperScope(){(void)builder.exchangeCurrentWrapper(previous);}
    } scope(builder,wrapper);
    builder.setLayoutObserver(observer_);
    return found->second->build(wrapper,builder,*this,parents);
}

std::optional<std::vector<NodeRef>> tryEmbellishedSvg(common::CommonWrapper& wrapper,
    Builder& builder,WrapperFactory& placements,std::span<const NodeRef> parents){
    if(parents.size()<=1||!wrapper.node->isEmbellished())return {};
    const auto parent=wrapper.node->Parent();
    auto* parentNode=parent.requireNode("isEmbellished");
    if(parentNode->isEmbellished())return {};
    auto* core=wrapper.coreMO();
    if(!core)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'embellishedBreakStyle')");
    const auto style=core->embellishedBreakStyle();
    const std::array<std::string_view,2> positions{"before","after"};
    std::vector<NodeRef> result;
    result.reserve(2);
    for(std::size_t i=0;i<2;++i){
        // The Source destructuring only reads parents[0] and parents[1], even
        // when the array has additional entries.
        const auto& parentNodeRef=parents[i];
        if(style!=positions[i]){
            const auto built=placements.build(wrapper,builder,
                std::span<const NodeRef>(&parentNodeRef,1));
            const auto root=built.empty()?NodeRef{}:built.front();
            if(root)placeSvgNode(wrapper,builder,root,0,0,true);
            result.push_back(root);
        }else{
            const auto roots=createSvgRoots(wrapper,builder,
                std::span<const NodeRef>(&parentNodeRef,1));
            result.push_back(roots.empty()?NodeRef{}:roots.front());
        }
    }
    return result;
}

void addSvgBaseChildren(common::CommonWrapper& wrapper,Builder& builder,
                        WrapperFactory& placements,std::span<const NodeRef> parents){
    // SvgWrapper.addChildren captures the children collection once, then
    // reuses its iterator values while the running x position advances.
    if(!wrapper.childNodes)return;
    const auto children=*wrapper.childNodes;
    if(!children)throw options::Exception("TypeError",
        u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    double x=0;
    for(std::size_t index=0;index<children->size();++index){
        auto* child=children->get(index).requireNode("toSVG");
        auto childRoots=placements.build(*child,builder,parents);
        const auto box=child->getOuterBBox();
        if(!childRoots.empty()){
            placeSvgNode(*child,builder,childRoots.front(),x+box->L*box->rscale,0,true);
        }
        x+=(box->L+box->w+box->R)*box->rscale;
    }
}
}
