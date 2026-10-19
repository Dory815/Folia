/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mpadded.ts.
 *************************************************************/
module folia.math.output.common;
import :mpadded;
import :wrapper;
import folia.math.core.mml_tree.mml_nodes.mpadded;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
namespace {
CommonWrapper& child(CommonMpadded& wrapper,std::size_t index){
    if(!wrapper.childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    if(!*wrapper.childNodes)throw options::Exception("TypeError",u"Cannot read properties of null (reading '0')");
    auto* result=dynamic_cast<CommonWrapper*>((*wrapper.childNodes)->get(index).nodeOrNull());
    if(!result)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getOuterBBox')");
    return *result;
}
}

double CommonMpadded::containerWidth() const {
    const auto width = property_host::string(node->attributes()->get("width"));
    const auto overflow = node->attributes()->get("data-overflow");
    const bool relative = !width.empty() &&
        (width.front() == u'+' || width.front() == u'-' || width.back() == u'%');
    if (!relative && property_host::string(overflow) == u"linebreak")
        return length2em(Property(property_host::debugText(width)));
    if(!parent)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'containerWidth')");
    return parent->containerWidth();
}

std::array<double,9> CommonMpadded::getDimens(){
    const auto values=node->attributes()->getList({"width","height","depth","lspace","voffset"});
    const auto box=child(*this,0).getOuterBBox();
    const double W=box->w,H=box->h,D=box->d;
    double w=W,h=H,d=D,x=0,y=0,dx=0;
    const auto width=values.get("width"),height=values.get("height"),depth=values.get("depth");
    const auto lspace=values.get("lspace"),voffset=values.get("voffset");
    if(property_host::string(width)!=u"")w=dimen(width,*box,"w",0);
    if(property_host::string(height)!=u"")h=dimen(height,*box,"h",0);
    if(property_host::string(depth)!=u"")d=dimen(depth,*box,"d",0);
    if(property_host::string(voffset)!=u"")y=dimen(voffset,*box);
    if(property_host::string(lspace)!=u"")x=dimen(lspace,*box);
    const auto align=node->attributes()->get("data-align");
    if(property_host::truthy(align))dx=getAlignX(w,*box,property_host::debugText(property_host::string(align)));
    return {H,D,W,h-H,d-D,w-W,x,y,dx};
}

double CommonMpadded::dimen(const Property& length,const BBox& box,std::string_view direction,std::optional<double> minimum)const{
    const auto text=property_host::string(length);
    std::string d(direction);
    double size=0;
    const auto w=text.find(u"width"),h=text.find(u"height"),dep=text.find(u"depth");
    const auto at=std::min({w,h,dep});
    if(at!=std::u16string::npos){
        if(at==w)size=box.w;
        else if(at==h)size=box.h;
        else size=box.d;
    }else if(!d.empty())size=d=="w"?box.w:d=="h"?box.h:d=="d"?box.d:0;
    double result=length2em(length,size);
    if(result==0||std::isnan(result))result=0;
    if(!text.empty()&&(text.front()==u'+'||text.front()==u'-')&&!d.empty())result+=size;
    if(minimum)result=std::max(*minimum,result);
    return result;
}

void CommonMpadded::setBBoxDimens(BBox& bbox){
    const auto [H,D,W,dh,dd,dw,x,y,dx]=getDimens();
    (void)x;(void)y;(void)dx;
    bbox.w=W+dw;bbox.h=H+dh;bbox.d=D+dd;
}

void CommonMpadded::computeBBox(BBox& bbox,bool recompute){
    setBBoxDimens(bbox);
    auto& content=child(*this,0);
    const double width=content.getOuterBBox()->w;
    if(width>bbox.w){
        const auto overflow=node->attributes()->get("data-overflow");
        if(!jax().math)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'root')");
        auto* root=jax().math->root.nodeOrNull();
        if(!root)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'attributes')");
        const bool rootLinebreak=property_host::string(root->attributes()->get("overflow"))==u"linebreak";
        if(property_host::string(overflow)==u"linebreak"||
           (property_host::string(overflow)==u"auto"&&rootLinebreak)){
            content.breakToWidth(bbox.w);setBBoxDimens(bbox);
        }
    }
    setChildPWidths(recompute,bbox.w);
}
std::string CommonMpadded::getChildAlign(std::size_t)const{
    const auto align=node->attributes()->get("data-align");
    return property_host::truthy(align)?property_host::debugText(property_host::string(align)):"left";
}
double CommonMpadded::getWrapWidth(std::size_t){return getBBox()->w;}
}
