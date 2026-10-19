module folia.math.output.scene;
import :mmultiscripts;
import :svg_paint;
import folia.math.util.property_host;
import folia.math.util.string;
import std;
namespace folia::math::output::scene {
namespace {
using common::CommonWrapper;
}
class MmultiscriptsPlacement final:public Placement {
    static double align(double w,double W,std::u16string_view value){
        if(value==u"center")return (W-w)/2;
        if(value==u"right")return W-w;
        return 0;
    }
    double addScripts(common::CommonMmultiscripts& wrapper,Builder& builder,
                      WrapperFactory& factory,const NodeRef& root,double x,double u,double v,
                      std::size_t index,std::size_t count,std::u16string_view alignment){
        const auto supRow=builder.node(u"g"),subRow=builder.node(u"g");
        builder.append(root,supRow);builder.append(root,subRow);
        placeSvgNode(wrapper,builder,supRow,x,u,false);
        placeSvgNode(wrapper,builder,subRow,x,v,false);
        const auto children=wrapper.childNodes?*wrapper.childNodes:tree::FactoryChildren<CommonWrapper>{};
        if(!children)throw options::Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        const std::size_t end=index+2*count;double dx=0;
        while(index<end){
            auto* sub=dynamic_cast<CommonWrapper*>(children->get(index++).requireNode("getOuterBBox"));
            auto* sup=dynamic_cast<CommonWrapper*>(children->get(index++).requireNode("getOuterBBox"));
            auto subbox=sub->getOuterBBox(),supbox=sup->getOuterBBox();
            const double width=std::max(subbox->w*subbox->rscale,supbox->w*supbox->rscale);
            const auto subParents=std::span<const NodeRef>(&subRow,1);
            const auto supParents=std::span<const NodeRef>(&supRow,1);
            const auto subScene=scriptChildScene(*sub,builder,factory,subParents);
            const auto supScene=scriptChildScene(*sup,builder,factory,supParents);
            placeScriptRoots(*sub,builder,subScene,
                dx+align(subbox->w*subbox->rscale,width,alignment),0);
            placeScriptRoots(*sup,builder,supScene,
                dx+align(supbox->w*supbox->rscale,width,alignment),0);
            dx+=width;
        }
        return x+dx;
    }
public:
    std::vector<NodeRef> build(CommonWrapper& base,Builder& builder,WrapperFactory& factory,
                               std::span<const NodeRef> parents) override {
        auto* wrapper=dynamic_cast<common::CommonMmultiscripts*>(&base);
        if(!wrapper)throw options::Exception("TypeError",u"Mmultiscripts placement received a non-Mmultiscripts wrapper");
        if(auto split=tryEmbellishedSvg(base,builder,factory,parents))return *split;
        auto roots=createSvgRoots(base,builder,parents);decorateSvgWrapper(base,builder,roots);
        if(roots.empty())return roots;
        const auto alignValue=wrapper->node->getProperty("scriptalign");
        const std::u16string alignment=property_host::truthy(alignValue)
            ?property_host::string(alignValue):std::u16string(u"right left");
        const auto parts=string_util::split(alignment+u" "+alignment);
        const auto preAlign=parts.empty()?std::u16string_view(u"right"):std::u16string_view(parts[0]);
        const auto postAlign=parts.size()<2?std::u16string_view(u"left"):std::u16string_view(parts[1]);
        const auto [u,v]=wrapper->getCombinedUV();
        const auto& data=wrapper->scriptsData();
        double x=0;
        const auto firstRoot=roots.front();
        if(data.numPrescripts)x=addScripts(*wrapper,builder,factory,firstRoot,
            wrapper->font->params().scriptspace,u,v,wrapper->firstPrescript(),data.numPrescripts,preAlign);
        auto* main=wrapper->baseChild();
        placeScriptChild(*main,builder,factory,x,0,roots);
        if(wrapper->breakCount())x=0;
        x+=main->getLineBBox(main->breakCount())->w;
        if(data.numScripts)addScripts(*wrapper,builder,factory,roots.back(),x,u,v,1,data.numScripts,postAlign);
        return roots;
    }
};

void registerMmultiscriptsPlacement(WrapperFactory& factory){
    factory.registerKind("mmultiscripts",std::make_shared<MmultiscriptsPlacement>());
}
}
