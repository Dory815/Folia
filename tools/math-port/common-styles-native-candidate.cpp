// Focused native observer for the locked MathJax 4.1.3 Styles/StyleJson rows.
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <variant>
#include <cmath>
#include <sstream>
#include <iomanip>

import folia.math.util.styles;
import folia.math.util.style_json;
import folia.math.util.property_host;
import std;

namespace {
using namespace folia::math;
namespace st = folia::math::styles;
namespace sj = folia::math::style_json;

std::string utf8(std::u16string_view value) { return property_host::debugText(value); }
std::string quote(std::string_view value) {
    std::string out="\"";
    for (unsigned char c : value) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20) {
                constexpr char hex[]="0123456789abcdef";
                out += "\\u00"; out += hex[c >> 4]; out += hex[c & 15];
            } else out += static_cast<char>(c);
        }
    }
    out += '"';
    return out;
}
std::string number(double value) {
    if (std::isnan(value)) return "{\"$\":\"NaN\"}";
    if (std::isinf(value)) return value < 0 ? "{\"$\":\"-Infinity\"}" : "{\"$\":\"Infinity\"}";
    if (value == 0 && std::signbit(value)) return "{\"$\":\"-0\"}";
    std::ostringstream out; out << std::setprecision(17) << value; return out.str();
}
std::string regexpSource(std::u16string_view pattern) {
    std::u16string source;
    source.reserve(pattern.size()+4);
    for(std::size_t i=0;i<pattern.size();++i){
        if(pattern[i]==u'/'){
            std::size_t slashes=0;for(std::size_t j=i;j>0&&pattern[j-1]==u'\\';--j)++slashes;
            if((slashes&1u)==0)source.push_back(u'\\');
        }
        source.push_back(pattern[i]);
    }
    return utf8(source);
}
std::string plain(const Property& value) {
    if (value.missing()) return "{\"$\":\"undefined\"}";
    if (std::holds_alternative<PropertyNull>(value.value)) return "null";
    if (const auto* b=std::get_if<bool>(&value.value)) return *b ? "true" : "false";
    if (const auto* n=std::get_if<double>(&value.value)) return number(*n);
    if (const auto* s=std::get_if<std::string>(&value.value)) return quote(*s);
    if (const auto* list=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value); list && *list) {
        const PropertyList object=PropertyList::record(value).value();
        std::string out="{"; bool first=true;
        for (const auto& key:object.keys()) {
            if (!first) out+=','; first=false;
            out+=quote(key)+":"+plain(object.getOwn(key));
        }
        out+='}'; return out;
    }
    throw std::runtime_error("Unsupported Property kind in Styles observer");
}
std::string strings(const std::vector<std::string>& values) {
    std::string out="["; for (std::size_t i=0;i<values.size();++i) { if(i)out+=',';out+=quote(values[i]); } return out+"]";
}
std::string strings16(const std::vector<std::u16string>& values) {
    std::string out="["; for (std::size_t i=0;i<values.size();++i) { if(i)out+=',';out+=quote(utf8(values[i])); } return out+"]";
}
std::string props(const st::StyleList& list) {
    std::string out="{"; bool first=true;
    for (const auto& key:list.keys()) {
        if (!first) out+=','; first=false; out+=quote(key)+":"+plain(list.getOwn(key));
    }
    return out+"}";
}
std::string snapshot(const st::Styles& style) {
    std::string out="{\"cssText\":"+quote(utf8(style.cssText()))+
        ",\"styleList\":"+props(style.styleList())+
        ",\"keys\":"+strings(style.styleList().keys())+
        ",\"missing\":"+quote(utf8(style.get(u"missing")))+
        ",\"padding\":[";
    for(std::size_t i=0;i<st::TRBL.size();++i){if(i)out+=',';out+=quote(utf8(style.get(property_host::text("padding-"+st::TRBL[i]))));}
    out+="],\"border\":[";
    for(std::size_t i=0;i<st::TRBL.size();++i){if(i)out+=',';out+='[';
        for(std::size_t j=0;j<st::WSC.size();++j){if(j)out+=',';out+=quote(utf8(style.get(property_host::text("border-"+st::TRBL[i]+"-"+st::WSC[j]))));}
        out+=']';
    }
    return out+"]}";
}
template<class F> std::string observed(F&& action) {
    try { return "{\"value\":"+action()+"}"; }
    catch(const folia::math::options::Exception& e) { return "{\"error\":{\"name\":"+quote(e.name)+",\"message\":"+quote(utf8(e.message))+"}}"; }
    catch(const std::exception& e) { return "{\"error\":{\"name\":\"Error\",\"message\":"+quote(e.what())+"}}"; }
}
struct DerivedStyles : st::Styles {
    static inline st::StylesPatterns derivedPatterns{
        std::make_shared<folia::math::regexp_host::RegExp>(u"$^"),
        st::Styles::pattern.value,
        std::make_shared<folia::math::regexp_host::RegExp>(u"([a-z]+)\\s*=\\s*([^;]*);?",u"g"),
        st::Styles::pattern.comment};
    explicit DerivedStyles(std::u16string_view input):Styles(input,derivedPatterns){}
};
struct SheetProbe : sj::StyleJsonSheet {
    using StyleJsonSheet::StyleJsonSheet;
    const PropertyList& styles() const { return styles_; }
};
bool sameObject(const Property& left,const Property& right) {
    const auto* a=std::get_if<std::shared_ptr<PropertyStorage>>(&left.value);
    const auto* b=std::get_if<std::shared_ptr<PropertyStorage>>(&right.value);
    return a&&b&&*a==*b;
}
std::string setTrace(const std::vector<std::pair<std::u16string,st::StyleValue>>& operations) {
    st::Styles style; std::string out="["; bool first=true;
    for(const auto& [name,value]:operations){if(!first)out+=',';first=false;style.set(name,value);out+=snapshot(style);}
    return out+"]";
}
std::string operationsJson(const std::vector<std::pair<std::u16string,st::StyleValue>>& operations) {
    std::string out="[";
    for(std::size_t i=0;i<operations.size();++i){if(i)out+=',';out+='[';
        out+=quote(utf8(operations[i].first))+',';
        if(const auto* text=std::get_if<std::u16string>(&operations[i].second.value))out+=quote(utf8(*text));
        else if(const auto* n=std::get_if<double>(&operations[i].second.value))out+=(*n==0?"0":number(*n));
        else out+=std::get<bool>(operations[i].second.value)?"true":"false";
        out+=']';
    }
    return out+"]";
}
std::string staticRecord() {
    std::string out="{\"TRBL\":"+strings(st::TRBL)+",\"WSC\":"+strings(st::WSC)+",\"patterns\":{";
    const std::array<std::pair<std::string_view,folia::math::regexp_host::RegExpRef>,4> patterns{{
        {"sanitize",st::Styles::pattern.sanitize},{"value",st::Styles::pattern.value},
        {"style",st::Styles::pattern.style},{"comment",st::Styles::pattern.comment}}};
    for(std::size_t i=0;i<patterns.size();++i){if(i)out+=',';out+=quote(patterns[i].first)+":{\"source\":"+quote(regexpSource(patterns[i].second->pattern()))+",\"flags\":"+quote(utf8(patterns[i].second->flags()))+"}";}
    out+="},\"connections\":{";
    for(std::size_t i=0;i<st::Styles::connect.size();++i){if(i)out+=',';const auto& [name,c]=st::Styles::connect[i];
        out+=quote(name)+":{\"children\":"+strings(c.children)+",\"parts\":"+(c.hasParts?strings(c.parts):"{\"$\":\"undefined\"}")+",\"subPart\":"+(c.subPart?"true":"{\"$\":\"undefined\"}")+",\"split\":"+quote(c.splitName)+",\"combine\":"+quote(c.combineName)+"}";
    }
    return out+"}}";
}
}

int main(int argc,char** argv) {
    std::vector<std::pair<std::string,std::string>> rows;
    const std::vector<std::pair<std::u16string,std::string>> constructors{
        {u"", "empty"},{u"color: red; background-color: blue;","ordinary"},
        {u"padding: 1em;","padding-one"},{u"padding: 1em 2em;","padding-two"},
        {u"padding: 1em 2em 3em;","padding-three"},{u"padding: 1em 2em 3em 4em;","padding-four"},
        {u"margin: 1px 2px 3px 4px;","margin-four"},{u"border: 1px solid red;","border"},
        {u"border: solid red;","border-no-width"},{u"border-top: thin dashed rgb(1, 2, 3);","border-top"},
        {u"border-width: 1px 2px 3px 4px; border-style: solid; border-color: red blue;","border-parts"},
        {u"font: italic bold 12px/1.5 \"Times New Roman\", serif;","font"},
        {u"font: normal small-caps 700 semi-condensed 1.2em/120% serif;","font-variants"},
        {u"font-family: \"A; B\"; color: green;","quoted-semicolon"},
        {u"font-family: 'A\\'B'; color: red;","escaped-quote"},
        {u"font-family: \"unclosed; color: red;","unclosed-quote"},
        {u"color: red; /* removed */ padding: 1em;","comment"},
        {u"color: red; /* multiline\ncomment */ color: blue;","multiline-comment"},
        {u"color: red; @invalid; color: blue;","invalid-prefix"},
        {u"color: red; Color: blue; padding: 2em;","uppercase-parser-name"},
        {u"font-family: \"数学😀\"; color: red;","unicode-value"},
        {u"padding: 1em; padding-left: 4em; padding-top: 2em;","sequential-child-overrides"},
        {u"border: 2px solid blue; border-left-width: 1px;","border-child-overrides"},
        {u"color: red; color: blue; color:;","delete-value"}};
    for(const auto& [css,name]:constructors)rows.emplace_back(name,"{\"name\":"+quote(name)+",\"input\":"+quote(utf8(css))+",\"actual\":"+observed([&]{st::Styles style(css);return snapshot(style);})+"}");
    const std::vector<std::pair<std::string,std::vector<std::pair<std::u16string,st::StyleValue>>>> mutations{
        {"set-parent-and-child",{{u"padding",st::StyleValue(u"1em 2em")},{u"paddingLeft",st::StyleValue(u"3em")},{u"paddingTop",st::StyleValue(u"4em")}}},
        {"combine-border",{{u"borderTop",st::StyleValue(u"1px solid red")},{u"borderRight",st::StyleValue(u"1px solid red")},{u"borderBottom",st::StyleValue(u"1px solid red")},{u"borderLeft",st::StyleValue(u"1px solid red")}}},
        {"delete-border-child",{{u"border",st::StyleValue(u"1px solid red")},{u"borderLeftColor",st::StyleValue(u"")}}},
        {"set-font-parts",{{u"fontSize",st::StyleValue(u"12px")},{u"fontWeight",st::StyleValue(u"bold")},{u"fontFamily",st::StyleValue(u"serif")}}},
        {"sanitize-semicolon",{{u"color",st::StyleValue(u"red; background: blue")}}},
        {"sanitize-unclosed-quote",{{u"fontFamily",st::StyleValue(u"\"unclosed")}}},
        {"primitive-values",{{u"opacity",st::StyleValue(0.5)},{u"zIndex",st::StyleValue(-0.0)},{u"flag",st::StyleValue(true)},{u"color",st::StyleValue(false)}}},
        {"ordered-delete-reinsert",{{u"color",st::StyleValue(u"red")},{u"padding",st::StyleValue(u"1em")},{u"color",st::StyleValue(u"")},{u"color",st::StyleValue(u"blue")}}}};
    for(const auto& [name,operations]:mutations)rows.emplace_back(name,"{\"name\":"+quote(name)+",\"operations\":"+operationsJson(operations)+",\"actual\":"+observed([&]{return setTrace(operations);})+"}");
    rows.emplace_back("style-list-is-copy","{\"name\":\"style-list-is-copy\",\"actual\":"+observed([&]{st::Styles style(u"color: red;");auto first=style.styleList();auto second=style.styleList();Property firstHandle(first),secondHandle(second);const auto* a=std::get_if<std::shared_ptr<PropertyStorage>>(&firstHandle.value);const auto* b=std::get_if<std::shared_ptr<PropertyStorage>>(&secondHandle.value);const bool distinct=a&&b&&*a!=*b;first.set("color",Property("blue"));return std::string("{\"distinct\":")+(distinct?"true":"false")+",\"untouched\":"+quote(utf8(style.get(u"color")))+",\"actual\":"+snapshot(style)+"}";})+"}");
    rows.emplace_back("protected-name-and-sanitizing-methods","{\"name\":\"protected-name-and-sanitizing-methods\",\"actual\":"+observed([&]{struct Probe:st::Styles{using Styles::Styles;using Styles::normalizeName;using Styles::parentName;using Styles::childName;using Styles::sanitizeValue;};Probe style;std::vector<std::string> names;for(auto n:{u"fontSize",u"MozTransform",u"borderTopWidth",u"COLOR",u"--X",u"x😀Y"})names.push_back(style.normalizeName(n));std::vector<std::string> parents;for(auto n:{"","color","border-top-width","--x"})parents.push_back(style.parentName(n));std::vector<std::string> children{style.childName("border-width","top"),style.childName("font","line-height"),style.childName("border-top","color")};std::vector<std::u16string> sanitized;for(auto v:{u"red",u"red; blue",u"\"unclosed",u"'a; b'",u"\"a\\\"b\"",u"a\nb"})sanitized.push_back(style.sanitizeValue(v));return std::string("{\"normalize\":")+strings(names)+",\"parent\":"+strings(parents)+",\"child\":"+strings(children)+",\"sanitize\":"+strings16(sanitized)+"}";})+"}");
    rows.emplace_back("subclass-pattern-lookup-and-base-connection-lookup","{\"name\":\"subclass-pattern-lookup-and-base-connection-lookup\",\"actual\":"+observed([&]{DerivedStyles style(u"padding=1em 2em;color=red;");style.set(u"color",st::StyleValue(u"red; blue"));return snapshot(style);})+"}");
    rows.emplace_back("style-sheet-lifecycle-and-nested-alias","{\"name\":\"style-sheet-lifecycle-and-nested-alias\",\"actual\":"+observed([&]{PropertyList nested{{"color",Property("red")}};PropertyList media{{".child",Property(nested)}};PropertyList selector{{"color",Property("blue")},{"opacity",Property(0.5)},{"@media test",Property(media)}};PropertyList input{{".a",Property(selector)}};SheetProbe sheet{Property(input)};std::vector<std::string> trace;trace.push_back("{\"cssText\":"+quote(utf8(sheet.cssText()))+",\"rules\":"+strings16(sheet.getStyleRules())+"}");selector.set("color",Property("green"));nested.set("color",Property("purple"));const auto afterMutation=utf8(sheet.cssText());const Property sourceMedia=selector.getOwn("@media test");const Property sheetSelector=sheet.styles().getOwn(".a");const Property copiedMedia=PropertyList::record(sheetSelector)->getOwn("@media test");trace.push_back("{\"cssText\":"+quote(afterMutation)+",\"shallowPropertyCopy\":"+(afterMutation.find("blue")!=std::string::npos&&afterMutation.find("green")==std::string::npos?"true":"false")+",\"nestedAlias\":"+(sameObject(copiedMedia,sourceMedia)?"true":"false")+"}");PropertyList addA{{"padding",Property("1em")},{"color",Property("black")}};PropertyList addB{{"display",Property("inline")}};PropertyList additions{{".a",Property(addA)},{".b",Property(addB)}};sheet.addStyles(Property(additions));trace.push_back("{\"cssText\":"+quote(utf8(sheet.cssText()))+",\"indented\":"+strings16(sheet.getStyleRules(Missing{},u"  "))+"}");const std::array<std::u16string,2> remove{u".missing",u".a"};sheet.removeStyles(remove);trace.push_back("{\"cssText\":"+quote(utf8(sheet.cssText()))+"}");sheet.clear();trace.push_back("{\"cssText\":"+quote(utf8(sheet.cssText()))+",\"rules\":"+strings16(sheet.getStyleRules())+"}");sheet.addStyles(Property(PropertyNull{}));trace.push_back("{\"cssText\":"+quote(utf8(sheet.cssText()))+"}");std::string out="[";for(std::size_t i=0;i<trace.size();++i){if(i)out+=',';out+=trace[i];}return out+"]";})+"}");
    rows.emplace_back("static-patterns-and-connections","{\"name\":\"static-patterns-and-connections\",\"actual\":"+observed(staticRecord)+"}");
    std::filesystem::path path=argc>1?argv[1]:"build/math-port/common-styles-native.json";std::filesystem::create_directories(path.parent_path());std::ofstream out(path,std::ios::binary);if(!out){std::cerr<<"cannot open candidate output\n";return 1;}out<<"{\"version\":\"4.1.3\",\"records\":[";for(std::size_t i=0;i<rows.size();++i){if(i)out<<',';out<<rows[i].second;}out<<"]}\n";return out?0:1;
}
