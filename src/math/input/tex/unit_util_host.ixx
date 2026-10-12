// Root engine/value host, no MathJax unit reduction or conversion algorithm.
export module folia.math.input.tex.unit_util_host;
export import folia.math.input.tex.unit_util;
import folia.math.util.js_number;
import folia.math.util.js_numeric_input;
import std;
export namespace folia::math::tex::unit_util_host {
inline std::u16string text(const Property& p){
    if(auto* s=std::get_if<std::string>(&p.value))return property_host::text(*s);
    property_host::typeError("Native UnitUtil string operation receiver is not a string");
}
inline Property string(std::u16string_view s){return property_host::debugText(s);}
inline Property slot(const std::vector<Property>& slots,std::size_t i){return i<slots.size()?slots[i]:Property(Missing{});}
inline double parseFloat(const Property& value){
    // Captures from the Source fixed numeric grammar, plus strings produced
    // by mu's em formatter, are complete numeric literals. No prefix is lost.
    return property_host::number(value);
}
inline Property firstComma(const Property& value){
    auto s=text(value);if(auto i=s.find(u',');i!=s.npos)s[i]=u'.';return string(s);
}
inline std::string fixed(double n){return js_number::toFixed(n,3);}
inline std::u16string trim(std::u16string_view value){
    std::size_t first=0,last=value.size();
    while(first<last&&js_input::isWhitespace(value[first]))++first;
    while(last>first&&js_input::isWhitespace(value[last-1]))--last;
    return std::u16string(value.substr(first,last-first));
}
}
