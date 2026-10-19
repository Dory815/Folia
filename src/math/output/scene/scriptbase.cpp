module folia.math.output.scene;
import :scriptbase;
import :svg_paint;
import std;
namespace folia::math::output::scene {
namespace {
using common::CommonWrapper;
using common::CommonScriptbase;
}
class ScriptPlacement final:public Placement {
public:
    std::vector<NodeRef> build(CommonWrapper& base,Builder& builder,WrapperFactory& factory,
                               std::span<const NodeRef> parents) override {
        auto* script=dynamic_cast<CommonScriptbase*>(&base);
        if(!script)throw options::Exception("TypeError",u"Script placement received a non-script wrapper");
        if(auto split=tryEmbellishedSvg(base,builder,factory,parents))return *split;
        auto roots=createSvgRoots(base,builder,parents);decorateSvgWrapper(base,builder,roots);
        if(roots.empty())return roots;
        auto* commonBase=script->baseChild();auto* scriptChild=script->scriptChild();
        if(!commonBase||!scriptChild)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'toSVG')");
        const double width=script->getBaseWidth();
        auto [x,y]=script->getOffset();
        const auto tail=std::span<const NodeRef>(&roots.back(),1);
        placeScriptChild(*commonBase,builder,factory,0,0,roots);
        placeScriptChild(*scriptChild,builder,factory,width+x,y,tail);
        return roots;
    }
};

void registerScriptbasePlacement(WrapperFactory& factory){
    factory.registerKind("msub",std::make_shared<ScriptPlacement>());
    factory.registerKind("msup",std::make_shared<ScriptPlacement>());
}
std::vector<NodeRef> buildScriptbasePlacement(common::CommonWrapper& wrapper,Builder& builder,
    WrapperFactory& factory,std::span<const NodeRef> parents){
    return ScriptPlacement{}.build(wrapper,builder,factory,parents);
}
}
