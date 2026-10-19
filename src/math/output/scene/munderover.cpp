module folia.math.output.scene;
import :munderover;
import :svg_paint;
import std;
namespace folia::math::output::scene {
namespace {
using common::CommonWrapper;
using common::CommonScriptbase;
}
class MunderPlacement final:public Placement {
public:
    std::vector<NodeRef> build(CommonWrapper& base,Builder& builder,WrapperFactory& factory,
                               std::span<const NodeRef> parents) override {
        auto* script=dynamic_cast<common::CommonMunder*>(&base);
        if(!script)throw options::Exception("TypeError",u"Munder placement received a non-Munder wrapper");
        if(auto split=tryEmbellishedSvg(base,builder,factory,parents))return *split;
        if(script->hasMovableLimits())return buildScriptbasePlacement(base,builder,factory,parents);
        auto roots=createSvgRoots(base,builder,parents);decorateSvgWrapper(base,builder,roots);
        if(roots.empty())return roots;
        auto* b=script->baseChild();auto* sub=script->scriptChild();
        if(!b||!sub)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
        auto bbox=b->getOuterBBox(),sbox=sub->getOuterBBox();
        auto baseScene=scriptChildScene(*b,builder,factory,roots);
        auto scriptScene=scriptChildScene(*sub,builder,factory,roots);
        const double delta=script->isLineBelow?0:script->getDelta(*sub,true);
        const double v=script->getUnderKV(*bbox,*sbox)[1];
        const auto widths=script->getDeltaW({*bbox,*sbox},{0,-delta});
        placeScriptRoots(*b,builder,baseScene,widths[0],0);
        placeScriptRoots(*sub,builder,scriptScene,widths[1],v);
        return roots;
    }
};

class MoverPlacement final:public Placement {
public:
    std::vector<NodeRef> build(CommonWrapper& base,Builder& builder,WrapperFactory& factory,
                               std::span<const NodeRef> parents) override {
        auto* script=dynamic_cast<common::CommonMover*>(&base);
        if(!script)throw options::Exception("TypeError",u"Mover placement received a non-Mover wrapper");
        if(auto split=tryEmbellishedSvg(base,builder,factory,parents))return *split;
        if(script->hasMovableLimits())return buildScriptbasePlacement(base,builder,factory,parents);
        auto roots=createSvgRoots(base,builder,parents);decorateSvgWrapper(base,builder,roots);
        if(roots.empty())return roots;
        auto* b=script->baseChild();auto* sup=script->scriptChild();
        if(!b||!sup)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
        auto bbox=b->getOuterBBox(),sbox=sup->getOuterBBox();
        auto baseScene=scriptChildScene(*b,builder,factory,roots);
        auto scriptScene=scriptChildScene(*sup,builder,factory,roots);
        const double delta=script->isLineAbove?0:script->getDelta(*sup);
        const double u=script->getOverKU(*bbox,*sbox)[1];
        const auto widths=script->getDeltaW({*bbox,*sbox},{0,delta});
        placeScriptRoots(*b,builder,baseScene,widths[0],0);
        placeScriptRoots(*sup,builder,scriptScene,widths[1],u);
        return roots;
    }
};

class MunderoverPlacement final:public Placement {
public:
    std::vector<NodeRef> build(CommonWrapper& base,Builder& builder,WrapperFactory& factory,
                               std::span<const NodeRef> parents) override {
        auto* script=dynamic_cast<common::CommonMunderover*>(&base);
        if(!script)throw options::Exception("TypeError",u"Munderover placement received a non-Munderover wrapper");
        if(auto split=tryEmbellishedSvg(base,builder,factory,parents))return *split;
        if(script->hasMovableLimits())return buildMsubsupPlacement(base,builder,factory,parents);
        auto roots=createSvgRoots(base,builder,parents);decorateSvgWrapper(base,builder,roots);
        if(roots.empty())return roots;
        auto* b=script->baseChild();auto* under=script->underChild();auto* over=script->overChild();
        if(!b||!under||!over)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
        auto bbox=b->getOuterBBox(),obox=over->getOuterBBox(),ubox=under->getOuterBBox();
        auto baseScene=scriptChildScene(*b,builder,factory,roots);
        auto underScene=scriptChildScene(*under,builder,factory,roots);
        auto overScene=scriptChildScene(*over,builder,factory,roots);
        const double odelta=script->getDelta(*over),udelta=script->getDelta(*under,true);
        const double u=script->getOverKU(*bbox,*obox)[1],v=script->getUnderKV(*bbox,*ubox)[1];
        const auto widths=script->getDeltaW({*bbox,*ubox,*obox},
            {0,script->isLineBelow?0:-udelta,script->isLineAbove?0:odelta});
        placeScriptRoots(*b,builder,baseScene,widths[0],0);
        placeScriptRoots(*under,builder,underScene,widths[1],v);
        placeScriptRoots(*over,builder,overScene,widths[2],u);
        return roots;
    }
};

void registerMunderoverPlacement(WrapperFactory& factory){
    factory.registerKind("munder",std::make_shared<MunderPlacement>());
    factory.registerKind("mover",std::make_shared<MoverPlacement>());
    factory.registerKind("munderover",std::make_shared<MunderoverPlacement>());
}
}
