module folia.math.input.tex.unit_util;
import folia.math.input.tex.unit_util_host;
import std;
namespace folia::math::tex {
namespace {
Dimension muReplace(UnitUtil& util,Dimension matched){
    if(matched[1]!=Property("mu"))return matched;
    const Property factor=util.UNIT_CASES->get(matched[1]);
    const double numericFactor=property_host::number(factor);
    const double numericValue=unit_util_host::parseFloat(matched[0]);
    const std::string em=util.em(numericFactor*numericValue);
    std::u16string value=property_host::text(em);
    value.resize(value.size()>=2?value.size()-2:0);
    matched[0]=unit_util_host::string(value);
    matched[1]="em";
    return matched;
}
}
UnitMap::UnitMap(UnitEntries entries){
    for(auto& [name,value]:entries){
        const auto found=std::ranges::find_if(map_,[&](const auto& entry){return entry.first==name;});
        if(found==map_.end())map_.emplace_back(std::move(name),value);
        else found->second=value;
    }
    updateDimen();
}
void UnitMap::updateDimen(){
    std::u16string names=u"(";
    bool first=true;
    for(const auto& [name,value]:map_){
        (void)value;
        if(!first)names.push_back(u'|');
        names+=name;
        first=false;
    }
    names.push_back(u')');
    unit=std::move(names);

    const std::u16string endSource=u"^\\s*"+num+u"\\s*"+unit+u"\\s*$";
    dimenEnd=std::make_shared<UnitPattern>(endSource);
    const std::u16string restSource=u"^\\s*"+num+u"\\s*"+unit+u" ?";
    dimenRest=std::make_shared<UnitPattern>(restSource);
}
UnitMap* UnitMap::set(std::u16string name,double ems){
    const auto found=std::ranges::find_if(map_,[&](const auto& entry){return entry.first==name;});
    if(found==map_.end())map_.emplace_back(std::move(name),ems);
    else found->second=ems;
    updateDimen();
    return this;
}
Property UnitMap::get(Property name)const{
    if(const auto* text=std::get_if<std::string>(&name.value)){
        const std::u16string key=property_host::text(*text);
        const auto found=std::ranges::find_if(map_,[&](const auto& entry){return entry.first==key;});
        if(found!=map_.end()&&property_host::truthy(Property(found->second)))return found->second;
    }
    const auto point=std::ranges::find_if(map_,[](const auto& entry){return entry.first==u"pt";});
    if(point!=map_.end())return point->second;
    return Missing{};
}
bool UnitMap::erase(std::u16string_view name){
    const auto found=std::ranges::find_if(map_,[&](const auto& entry){return entry.first==name;});
    if(found==map_.end())return false;
    map_.erase(found);
    updateDimen();
    return true;
}
Dimension UnitUtil::matchDimen(Property dim,Property rest){
    const bool useRest=property_host::truthy(rest);
    const auto cases=UNIT_CASES;
    const UnitPatternRef pattern=useRest?cases->dimenRest:cases->dimenEnd;
    const auto match=pattern->match(dim);
    if(!match)return Dimension{Property(PropertyNull{}),Property(PropertyNull{}),Property(0)};
    const Property value=unit_util_host::firstComma(unit_util_host::slot(*match,1));
    const Property unit=unit_util_host::slot(*match,4);
    const std::size_t length=unit_util_host::text(unit_util_host::slot(*match,0)).size();
    return muReplace(*this,Dimension{value,unit,Property(static_cast<double>(length))});
}
double UnitUtil::dimen2em(Property dim){
    const Dimension match=this->matchDimen(std::move(dim));
    const Property value=property_host::truthy(match[0])?match[0]:Property("1");
    const double number=unit_util_host::parseFloat(value);
    const Property factor=UNIT_CASES->get(match[1]);
    return property_host::number(factor)*number;
}
std::string UnitUtil::em(double value){
    if(std::abs(value)<0.0006)return "0em";
    std::string fixed=unit_util_host::fixed(value);
    fixed=std::regex_replace(fixed,std::regex(R"(\.?0+$)"),"");
    return fixed+"em";
}
Property UnitUtil::trimSpaces(Property text){
    const auto* raw=std::get_if<std::string>(&text.value);
    if(!raw)return text;
    const std::u16string source=property_host::text(*raw);
    std::u16string trimmed=unit_util_host::trim(source);
    if(!trimmed.empty()&&trimmed.back()==u'\\'&&!source.empty()&&source.back()==u' ')
        trimmed.push_back(u' ');
    return unit_util_host::string(trimmed);
}
}
