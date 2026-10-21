/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/mo.ts glyph placement.
 * This backend-independent pass retains FontData path identity and full-
 * precision glyph offsets; only SVG serialization rounds coordinates.
 *************************************************************/
module folia.math.output.scene;
import :mo;
import :svg_paint;
import :glyph_paint;
import folia.math.core.mml_tree.mml_node;
import folia.math.util.property_host;
import folia.math.util.string;
import std;

namespace folia::math::output::scene {
namespace {
using folia::math::Property;
using folia::math::property_host::truthy;

struct GlyphMetrics {double h=0,d=0,w=0;};
GlyphMetrics glyphMetrics(common::CommonMo& wrapper,std::string_view variant,std::uint32_t cp){
    const auto chars=wrapper.getVariantChar(variant,cp);
    return {property_host::number(glyphSlot(chars,0)),property_host::number(glyphSlot(chars,1)),
            property_host::number(glyphSlot(chars,2))};
}
void stretchAssembly(common::CommonMo& wrapper,Builder& builder,WrapperFactory& placements,
                     std::span<const NodeRef> roots){
    const auto data=wrapper.stretch;
    if(!data||!data->stretch)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'keys')");
    const auto requested=*data->stretch;
    const auto bbox=wrapper.getBBox();
    const double h=bbox->h,d=bbox->d,w=bbox->w;
    const auto chars=glyphUnicodeChars(wrapper,wrapper.getText(),wrapper.variant);
    if(chars->empty())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'codePointAt')");
    const double cp=data->c&&*data->c!=0&&!std::isnan(*data->c)
        ?*data->c:static_cast<double>(chars->front());
    std::vector<std::string> variants;
    variants.reserve(requested.size());
    for(std::size_t i=0;i<requested.size();++i)
        variants.push_back(wrapper.font->getStretchVariant(cp,static_cast<double>(i)).value_or(""));
    auto code=[&](std::size_t i){return i<requested.size()?static_cast<std::uint32_t>(requested[i]):0u;};
    if(data->dir==common::VERTICAL){
        const auto top=code(0),ext=code(1),bot=code(2);
        const auto topMetrics=top?glyphMetrics(wrapper,variants.size()>0?variants[0]:wrapper.variant,top):GlyphMetrics{};
        const auto bottomMetrics=bot?glyphMetrics(wrapper,variants.size()>2?variants[2]:wrapper.variant,bot):GlyphMetrics{};
        const double T=topMetrics.h+topMetrics.d;
        const double B=bottomMetrics.h+bottomMetrics.d;
        if(top)emitGlyphToRoots(wrapper,builder,placements,roots,top,(w-glyphMetrics(wrapper,variants[0],top).w)/2,h-glyphMetrics(wrapper,variants[0],top).h,variants[0]);
        if(bot)emitGlyphToRoots(wrapper,builder,placements,roots,bot,(w-glyphMetrics(wrapper,variants.size()>2?variants[2]:wrapper.variant,bot).w)/2,
            glyphMetrics(wrapper,variants.size()>2?variants[2]:wrapper.variant,bot).d-d,
            variants.size()>2?variants[2]:wrapper.variant);
        if(requested.size()==4){
            const auto mid=code(3);double MH=0,MD=0;
            if(mid){const auto v=variants.size()>3?variants[3]:wrapper.variant;const auto m=glyphMetrics(wrapper,v,mid);
                const double y=(m.d-m.h)/2+wrapper.font->params().axis_height;
                emitGlyphToRoots(wrapper,builder,placements,roots,mid,(w-m.w)/2,y,v);MH=m.h+y;MD=m.d-y;}
            auto add=[&](double H,double D,double topH,double botH){
                const double T0=std::max(0.0,topH-common::VFUZZ),B0=std::max(0.0,botH-common::VFUZZ);
                const double Y=H+D-T0-B0;if(Y<=0)return;
                const auto v=variants.size()>1?variants[1]:wrapper.variant;
                const auto m=glyphMetrics(wrapper,v,ext);const double s=1.5*Y/(m.h+m.d);
                const double yy=(s*(m.h-m.d)-Y)/2;
                emitGlyphExtender(wrapper,builder,placements,roots,ext,v,true,(w-m.w)/2,B0-D,m.w,Y,0,yy,m.w,Y,s);
            };
            add(h,-MH,T,MH);add(-MD,d,MD,B);
        }else if(ext){
            const auto v=variants.size()>1?variants[1]:wrapper.variant;const auto m=glyphMetrics(wrapper,v,ext);
            const double Y=h+d-std::max(0.0,T-common::VFUZZ)-std::max(0.0,B-common::VFUZZ);
            if(Y>0){const double s=1.5*Y/(m.h+m.d);const double yy=(s*(m.h-m.d)-Y)/2;
                emitGlyphExtender(wrapper,builder,placements,roots,ext,v,true,(w-m.w)/2,std::max(0.0,B-common::VFUZZ)-d,m.w,Y,0,yy,m.w,Y,s);}
        }
    }else{
        const auto left=code(0),ext=code(1),right=code(2);
        const auto lv=variants.size()>0?variants[0]:wrapper.variant;
        const auto ev=variants.size()>1?variants[1]:wrapper.variant;
        const auto rv=variants.size()>2?variants[2]:wrapper.variant;
        const double L=left?glyphMetrics(wrapper,lv,left).w:0,R=right?glyphMetrics(wrapper,rv,right).w:0;
        if(left)emitGlyphToRoots(wrapper,builder,placements,roots,left,0,0,lv);
        if(right)emitGlyphToRoots(wrapper,builder,placements,roots,right,w-glyphMetrics(wrapper,rv,right).w,0,rv);
        if(requested.size()==4){
            const auto mid=code(3);double x1=0,x2=0;
            if(mid){const auto mv=variants.size()>3?variants[3]:wrapper.variant;const double mw=glyphMetrics(wrapper,mv,mid).w;
                x1=(w-mw)/2;x2=(w+mw)/2;emitGlyphToRoots(wrapper,builder,placements,roots,mid,x1,0,mv);}
            const double w2=w/2;
            auto addExtH=[&](double W,double left,double right,double offset){
                if(!ext)return;
                left=std::max(0.0,left-common::HFUZZ);right=std::max(0.0,right-common::HFUZZ);
                const double X=W-left-right;if(X<=0)return;
                const auto m=glyphMetrics(wrapper,ev,ext);const double s=1.5*(X/m.w);
                const double Y=m.h+m.d+2*common::VFUZZ,D=-(m.d+common::VFUZZ);
                emitGlyphExtender(wrapper,builder,placements,roots,ext,ev,false,offset+left,D,X,Y,
                    (s*m.w-X)/2,D,X,Y,s);
            };
            addExtH(w2,L,w2-x1,0);
            addExtH(w2,x2-w2,R,w2);
        }else if(ext){
            auto addExtH=[&](double W,double left,double right,double offset){
                left=std::max(0.0,left-common::HFUZZ);right=std::max(0.0,right-common::HFUZZ);
                const double X=W-left-right;if(X<=0)return;
                const auto m=glyphMetrics(wrapper,ev,ext);const double s=1.5*(X/m.w);
                const double Y=m.h+m.d+2*common::VFUZZ,D=-(m.d+common::VFUZZ);
                emitGlyphExtender(wrapper,builder,placements,roots,ext,ev,false,offset+left,D,X,Y,
                    (s*m.w-X)/2,D,X,Y,s);
            };
            addExtH(w,L,R,0);
        }
    }
}


}

std::vector<NodeRef> MoPlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonMo*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"Mo SVG placement received a non-Mo wrapper");
    const auto text=wrapper->getText();
    const auto chars=glyphUnicodeChars(*wrapper,text,wrapper->variant);
    if(chars->empty())return {};
    const bool stretchy=wrapper->stretch&&wrapper->stretch->dir!=common::NONE;
    if(stretchy&&!wrapper->size)wrapper->getStretchedVariant({});

    auto roots=createSvgRoots(*wrapper,builder,parents);
    decorateSvgWrapper(*wrapper,builder,roots);
    if(roots.size()>1&&wrapper->breakStyle!="duplicate"){
        const auto index=wrapper->breakStyle=="after"?1u:0u;
        if(index<roots.size()){
            const auto removed=roots[index];
            for(const auto& parent:parents){
                if(parent){
                    for(std::size_t childIndex=0;childIndex<parent->children.size();++childIndex){
                        if(parent->children[childIndex]==removed){parent->children.erase(childIndex);break;}
                    }
                }
            }
            roots[index].reset();
        }
    }

    const auto cache=wrapper->jax().optionsContext().get(wrapper->jax().options,options::Key(u"fontCache"));
    const auto* cacheName=std::get_if<std::u16string>(&cache.data);
    if(!cacheName||*cacheName!=u"none")
        throw options::Exception("Unimplemented",u"SVG font caches require the completed Scene writer");

    if(stretchy&&wrapper->size&&*wrapper->size<0){
        stretchAssembly(*wrapper,builder,placements,roots);
        return roots;
    }

    const bool symmetric=truthy(wrapper->node->attributes()->get("symmetric"))&&
        (!wrapper->stretch||wrapper->stretch->dir!=common::HORIZONTAL);
    const double u=(symmetric||
                    truthy(wrapper->node->attributes()->get("largeop")))
        ?wrapper->getCenterOffset():0;
    const double v=truthy(wrapper->node->getProperty("mathaccent"))?wrapper->getAccentOffset():0;
    if(roots.empty())return roots;
    if(u||v){
        TransformOperation translation{TransformKind::Translate,v,u};
        translation.spaceSeparated=true;
        for(const auto& root:roots)
            if(root)builder.transform(root,Transform{0,0,1,1,{translation}});
    }
    if(roots[0])addSvgBaseChildren(*wrapper,builder,placements,
        std::span<const NodeRef>(&roots[0],1));
    if(roots.size()>1&&roots[1]){
        auto* childOwner=wrapper->multChar
            ?static_cast<common::CommonWrapper*>(wrapper->multChar)
            :static_cast<common::CommonWrapper*>(wrapper);
        addSvgBaseChildren(*childOwner,builder,placements,
            std::span<const NodeRef>(&roots[1],1));
    }
    return roots;
}

void registerMoPlacement(WrapperFactory& factory){
    factory.registerKind("mo",std::make_shared<MoPlacement>());
}


}
