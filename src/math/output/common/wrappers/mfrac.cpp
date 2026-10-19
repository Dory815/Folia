module folia.math.output.common;
import :mfrac;
import :wrapper;
import :mo;
import folia.math.util.property_host;
import folia.math.core.mml_tree.mml_nodes.mfrac;
import std;

namespace folia::math::output::common {
double CommonMfrac::sourceAlignX(double width,const BBox& box,std::string_view align)const{
    return getAlignX(width,box,align);
}
double CommonMfrac::sourceLength2em(const Property& value,double size,std::optional<double> scale)const{
    return length2em(value,size,scale);
}
CommonMfrac::CommonMfrac(CommonWrapperFactory& factory,mml::MmlNode& source,CommonWrapper* parentValue)
    :CommonWrapper(factory,source,parentValue){}
void CommonMfrac::initializeSource(){
    CommonWrapper::initializeSource();
    const auto* node=dynamic_cast<mml::MmlMfrac*>(this->node);
    if(!node)throw options::Exception("TypeError",u"CommonMfrac requires an MmlMfrac node");
    pad=property_host::truthy(node->getProperty("withDelims"))?0:font->params().nulldelimiterspace;
    if(property_host::truthy(node->attributes()->get("bevelled"))){
        const auto H=getBevelData(isDisplay()).H;
        bevel=&createMo(u"/");
        bevel->node->attributes()->set("symmetric",true);
        (void)bevel->canStretch(VERTICAL);
        bevel->getStretchedVariant(std::array<double,1>{H},true);
    }
}
void CommonMfrac::getFractionBBox(BBox& box,bool display,double thickness){
    if(!childNodes||!(*childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    const auto children=*childNodes;
    const auto nbox=children->get(0).requireNode("getOuterBBox")->getOuterBBox();
    const auto dbox=children->get(1).requireNode("getOuterBBox")->getOuterBBox();
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    const auto& tex=font->params();const double axis=tex.axis_height;
    const auto tuv=getTUV(display,thickness);
    box.combine(*nbox,0,axis+tuv.T+std::max(nbox->d*nbox->rscale,tuv.u));
    box.combine(*dbox,0,axis-tuv.T-std::max(dbox->h*dbox->rscale,tuv.v));
    box.w+=2*pad+0.2;
}
CommonMfrac::TUV CommonMfrac::getTUV(bool display,double thickness)const{
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    const auto& tex=font->params();const double axis=tex.axis_height;
    const double T=(display?3.5:1.5)*thickness;
    return {T,(display?tex.num1:tex.num2)-axis-T,(display?tex.denom1:tex.denom2)+axis-T};
}
void CommonMfrac::getAtopBBox(BBox& box,bool display){
    const auto data=getUVQ(display);
    box.combine(data.nbox,0,data.u);box.combine(data.dbox,0,-data.v);box.w+=2*pad;
}
CommonMfrac::UVQ CommonMfrac::getUVQ(bool display)const{
    if(!childNodes||!(*childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    const auto children=*childNodes;
    const auto nbox=children->get(0).requireNode("getOuterBBox")->getOuterBBox();
    const auto dbox=children->get(1).requireNode("getOuterBBox")->getOuterBBox();
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    const auto& tex=font->params();
    double u=display?tex.num1:tex.num3,v=display?tex.denom1:tex.denom2;
    const double p=(display?7:3)*tex.rule_thickness;
    double q=u-nbox->d*nbox->scale-(dbox->h*dbox->scale-v);
    if(q<p){u+=(p-q)/2;v+=(p-q)/2;q=p;}
    return {u,v,q,*nbox,*dbox};
}
void CommonMfrac::getBevelledBBox(BBox& box,bool display){
    const auto data=getBevelData(display);const auto lbox=bevel->getOuterBBox();
    box.combine(data.nbox,0,data.u);box.combine(*lbox,box.w-data.delta/2,0);
    box.combine(data.dbox,box.w-data.delta/2,data.v);
}
CommonMfrac::BevelData CommonMfrac::getBevelData(bool display)const{
    if(!childNodes||!(*childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    const auto children=*childNodes;
    const auto nbox=children->get(0).requireNode("getOuterBBox")->getOuterBBox();
    const auto dbox=children->get(1).requireNode("getOuterBBox")->getOuterBBox();
    const double delta=display?0.4:0.15;
    const double H=std::max(nbox->scale*(nbox->h+nbox->d),dbox->scale*(dbox->h+dbox->d))+2*delta;
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    const double axis=font->params().axis_height;
    const double u=nbox->scale*(nbox->d-nbox->h)/2+axis+delta;
    const double v=dbox->scale*(dbox->d-dbox->h)/2+axis-delta;
    return {H,delta,u,v,*nbox,*dbox};
}
bool CommonMfrac::isDisplay()const{
    const auto attributes=node->attributes();
    return property_host::truthy(attributes->get("displaystyle"))&&attributes->get("scriptlevel")==Property(0);
}
void CommonMfrac::computeBBox(BBox& box,bool recompute){
    box.empty();const auto attributes=node->attributes();
    const Property thicknessValue=attributes->get("linethickness");
    const bool bevelled=property_host::truthy(attributes->get("bevelled"));
    const bool display=isDisplay();std::optional<double> width;
    if(bevelled)getBevelledBBox(box,display);
    else{
        width=-2*pad;
        const double thickness=length2em(Property(property_host::debugText(property_host::string(thicknessValue))),0.06);
        if(thickness==0)getAtopBBox(box,display);
        else{getFractionBBox(box,display,thickness);*width-=0.2;}
        *width+=box.w;
    }
    box.clean();
    setChildPWidths(recompute,width);
}
bool CommonMfrac::canStretch(std::string_view){return false;}
std::string CommonMfrac::getChildAlign(std::size_t index)const{
    const auto attributes=node->attributes();
    if(property_host::truthy(attributes->get("bevelled")))return "left";
    const auto name=index==0?"numalign":index==1?"denomalign":"undefined";
    return property_host::debugText(property_host::string(attributes->get(name)));
}
double CommonMfrac::getWrapWidth(std::size_t index){
    const auto attributes=node->attributes();
    if(property_host::truthy(attributes->get("bevelled"))){if(!childNodes||!(*childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");return (*childNodes)->get(index).requireNode("getOuterBBox")->getOuterBBox()->w;}
    const double width=getBBox()->w;
    const double thickness=length2em(attributes->get("linethickness"));
    return width-(thickness?0.2:0)-2*pad;
}
}
