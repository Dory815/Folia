module folia.math.output.common;
import :msqrt;
import :wrapper;
import :mo;
import folia.math.util.property_host;
import std;
namespace folia::math::output::common {
CommonMsqrt::CommonMsqrt(CommonWrapperFactory& factory,mml::MmlNode& node,CommonWrapper* parentValue)
    :CommonWrapper(factory,node,parentValue){}
void CommonMsqrt::initializeSource(){
    CommonWrapper::initializeSource();
    surd=&createMo(u"\u221A");
    (void)surd->canStretch(VERTICAL);
    getStretchedSurd();
}
void CommonMsqrt::combineRootBBox(BBox&,const BBox&,double)const{}
std::array<double,2> CommonMsqrt::getPQ(const BBox& surdBox)const{
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    const auto& params=font->params();const double t=params.rule_thickness,s=params.surd_height;
    const double p=property_host::truthy(node->attributes()->get("displaystyle"))?params.x_height:t;
    const double q=surdBox.h+surdBox.d>surdH
        ?(surdBox.h+surdBox.d-(surdH-t-s-p/2))/2:s+p/4;
    return {p,q};
}
std::array<double,3> CommonMsqrt::getRootDimens(const BBox&,double)const{return {0,0,0};}
double CommonMsqrt::rootWidth()const{return 1.25;}
void CommonMsqrt::getStretchedSurd(){
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    const auto& params=font->params();const double t=params.rule_thickness,s=params.surd_height;
    const double p=property_host::truthy(node->attributes()->get("displaystyle"))?params.x_height:t;
    if(!childNodes||!(*childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    const auto child=(*childNodes)->get(base());
    const auto box=child.requireNode("getOuterBBox")->getOuterBBox();
    surdH=box->h+box->d+t+s+p/4;
    surd->getStretchedVariant(std::array<double,2>{surdH-box->d,box->d},true);
}
void CommonMsqrt::computeBBox(BBox& box,bool recompute){
    box.empty();const auto surdBox=surd->getBBox();
    if(!childNodes||!(*childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    const auto baseBox=(*childNodes)->get(base()).requireNode("getOuterBBox")->getOuterBBox();
    const double q=getPQ(*surdBox)[1];
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    const double t=font->params().rule_thickness,s=font->params().surd_height;
    const double H=baseBox->h+q+t;const auto dims=getRootDimens(*surdBox,H);const double x=dims[0];
    box.h=H+s;combineRootBBox(box,*surdBox,H);box.combine(*surdBox,x,H-surdBox->h);
    box.combine(*baseBox,x+surdBox->w,0);box.clean();setChildPWidths(recompute);
}
void CommonMsqrt::invalidateBBox(bool bubble){
    CommonWrapper::invalidateBBox(bubble);
    if(!surd||!surd->childNodes||!(*surd->childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    (*surd->childNodes)->get(0).requireNode("invalidateBBox")->invalidateBBox();
}
}
