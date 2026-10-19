module folia.math.output.scene;
import :msubsup;
import :svg_paint;
import std;
namespace folia::math::output::scene {
namespace {
using common::CommonWrapper;
using common::CommonScriptbase;
}
class MsubsupPlacement final:public Placement {
public:
    std::vector<NodeRef> build(CommonWrapper& base,Builder& builder,WrapperFactory& factory,
                               std::span<const NodeRef> parents) override {
        auto* script=dynamic_cast<common::CommonMsubsup*>(&base);
        if(!script)throw options::Exception("TypeError",u"Msubsup placement received a non-Msubsup wrapper");
        if(auto split=tryEmbellishedSvg(base,builder,factory,parents))return *split;
        auto roots=createSvgRoots(base,builder,parents);decorateSvgWrapper(base,builder,roots);
        if(roots.empty())return roots;
        auto* b=script->baseChild();auto* sup=script->supChild();auto* sub=script->subChild();
        if(!b||!sup||!sub)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'toSVG')");
        const double width=script->getBaseWidth(),ic=script->getAdjustedIc();
        const auto uvq=script->getUVQ();
        const auto tail=std::span<const NodeRef>(&roots.back(),1);
        const auto baseScene=scriptChildScene(*b,builder,factory,roots);
        const auto supScene=scriptChildScene(*sup,builder,factory,tail);
        const auto subScene=scriptChildScene(*sub,builder,factory,tail);
        placeScriptRoots(*b,builder,baseScene,0,0);
        placeScriptRoots(*sub,builder,subScene,
            width+(script->baseIsChar?0:ic),(*uvq)[1]);
        placeScriptRoots(*sup,builder,supScene,width+ic,(*uvq)[0]);
        return roots;
    }
};

void registerMsubsupPlacement(WrapperFactory& factory){
    factory.registerKind("msubsup",std::make_shared<MsubsupPlacement>());
}
std::vector<NodeRef> buildMsubsupPlacement(common::CommonWrapper& wrapper,Builder& builder,
    WrapperFactory& factory,std::span<const NodeRef> parents){
    return MsubsupPlacement{}.build(wrapper,builder,factory,parents);
}
}
