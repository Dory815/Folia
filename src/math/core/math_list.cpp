// MathJax 4.1.3 core/MathList.ts; Apache-2.0.
module folia.math.core.math_list;
import folia.math.util.property_host;
import std;

namespace folia::math::core {
namespace {
bool abstractLess(const Property& left,const Property& right){
    // ECMAScript abstract relational comparison first converts both operands
    // with the Number hint, then compares two strings lexically or otherwise
    // applies numeric conversion. Keep the string branch in UTF-16 code-unit
    // order as required by the Source comparator.
    const Property a=property_host::primitive(left,false);
    const Property b=property_host::primitive(right,false);
    const auto* x=std::get_if<std::string>(&a.value);
    const auto* y=std::get_if<std::string>(&b.value);
    if(x&&y){
        const auto xs=property_host::text(*x),ys=property_host::text(*y);
        return std::lexicographical_compare(xs.begin(),xs.end(),ys.begin(),ys.end());
    }
    const double an=property_host::number(a),bn=property_host::number(b);
    return !std::isnan(an)&&!std::isnan(bn)&&an<bn;
}
bool strictEqual(const Property& left,const Property& right){
    if(left.value.index()!=right.value.index())return false;
    if(const auto* a=std::get_if<double>(&left.value)){
        const auto b=std::get<double>(right.value);
        return !std::isnan(*a)&&!std::isnan(b)&&*a==b;
    }
    if(const auto* a=std::get_if<std::shared_ptr<PropertyStorage>>(&left.value))
        return *a==std::get<std::shared_ptr<PropertyStorage>>(right.value);
    if(const auto* a=std::get_if<FunctionRef>(&left.value))return *a==std::get<FunctionRef>(right.value);
    if(const auto* a=std::get_if<ContextFunctionIdentity>(&left.value))return *a==std::get<ContextFunctionIdentity>(right.value);
    return left==right;
}
}
Property AbstractMathList::locationProperty(MathItem& item,std::string_view member){
    return readMathLocation(item.start,member,locationAccess_);
}
Property AbstractMathList::readStartField(MathItem& item,std::string_view field){
    // Source evaluates a.start for each member access; item.start retains the
    // actual location handle and this call performs its live property Get.
    return locationProperty(item,field);
}
bool AbstractMathList::isBefore(MathItem* const& a,MathItem* const& b){
    if(!a||!b)property_host::typeError("Cannot read properties of null or undefined (reading 'start')");
    auto& left=*a;auto& right=*b;
    // Preserve the source expression's short-circuit and repeated live reads.
    const auto leftIndex=readStartField(left,"i");
    const auto rightIndex=readStartField(right,"i");
    if(abstractLess(leftIndex,rightIndex))return true;
    const auto leftIndexAgain=readStartField(left,"i");
    const auto rightIndexAgain=readStartField(right,"i");
    if(!strictEqual(leftIndexAgain,rightIndexAgain))return false;
    const auto leftNode=readStartField(left,"n");
    const auto rightNode=readStartField(right,"n");
    return abstractLess(leftNode,rightNode);
}
void AbstractMathList::forEach(const std::function<void(MathItem&)>& visit){
    for(auto* item:*this){
        if(!item)property_host::typeError("Cannot read properties of null (reading 'math')");
        visit(*item);
    }
}
}
