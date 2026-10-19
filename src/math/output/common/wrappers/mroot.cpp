module folia.math.output.common;
import :mroot;
import :wrapper;
import folia.math.util.property_host;
import std;
namespace folia::math::output::common {
void CommonMroot::combineRootBBox(BBox& box,const BBox& surdBox,double H)const{
    if(!childNodes||!(*childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '1')");
    const auto rootBox=(*childNodes)->get(*root()).requireNode("getOuterBBox")->getOuterBBox();
    const double h=getRootDimens(surdBox,H)[1];box.combine(*rootBox,0,h);
}
std::array<double,3> CommonMroot::getRootDimens(const BBox& surdBox,double H)const{
    if(!childNodes||!(*childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '1')");
    const auto rootBox=(*childNodes)->get(*root()).requireNode("getOuterBBox")->getOuterBBox();
    const double size=surd&&surd->size?*surd->size:0;
    const double offset=(size<0?0.5:0.6)*surdBox.w;
    const double ratio=offset/rootBox->rscale;
    const double W=(std::isnan(rootBox->w)||std::isnan(ratio))?std::numeric_limits<double>::quiet_NaN():std::max(rootBox->w,ratio);
    const double dx=(std::isnan(W-rootBox->w))?std::numeric_limits<double>::quiet_NaN():std::max(0.0,W-rootBox->w);
    const double h=rootHeight(*rootBox,surdBox,size,H);
    const double x=W*rootBox->rscale-offset;
    return {x,h,dx};
}
double CommonMroot::rootHeight(const BBox& rootBox,const BBox& surdBox,double size,double H)const{
    const double h=surdBox.h+surdBox.d;
    const double b=(size<0?1.9:0.55*h)-(h-H);
    const double extension=rootBox.d*rootBox.rscale;
    return b+(std::isnan(extension)?std::numeric_limits<double>::quiet_NaN():std::max(0.0,extension));
}
double CommonMroot::rootWidth()const{
    if(!childNodes||!(*childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '1')");
    const auto box=(*childNodes)->get(*root()).requireNode("getOuterBBox")->getOuterBBox();
    return 0.4+box->w*box->rscale;
}
}
