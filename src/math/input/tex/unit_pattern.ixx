// Native UTF-16 ECMAScript-pattern host for UnitUtil's generated expressions.
// No unit conversion, dimension reduction or MathJax algorithm lives here.
export module folia.math.input.tex.unit_pattern;
export import folia.math.util.property_host;
export import folia.math.util.option_types;
import folia.math.util.js_numeric_input;
import std;
export namespace folia::math::tex {
class UnitPattern {
    std::wregex compiled_;
    static std::wstring wide(std::u16string_view s){
        std::wstring out;for(auto c:s)out.push_back(static_cast<wchar_t>(c));return out;
    }
    static std::wstring translate(std::u16string_view pattern){
        // std::regex's locale \s is narrower than ECMAScript. Expand each
        // actual escape, preserving literal escaped backslashes and classes.
        std::wstring out;bool inClass=false;
        constexpr std::wstring_view spaces=L"\x0009-\x000D\x0020\x00A0\x1680\x2000-\x200A\x2028\x2029\x202F\x205F\x3000\xFEFF";
        for(std::size_t i=0;i<pattern.size();++i){
            const auto c=pattern[i];
            if(c==u'\\'&&i+1<pattern.size()){
                const auto next=pattern[++i];
                if(next==u's'){if(!inClass)out+=L'[';out+=spaces;if(!inClass)out+=L']';}
                else if(next==u'd'){if(!inClass)out+=L'[';out+=L"0-9";if(!inClass)out+=L']';}
                else{out+=L'\\';out+=static_cast<wchar_t>(next);}
            }else{if(c==u'[')inClass=true;if(c==u']')inClass=false;out+=static_cast<wchar_t>(c);}
        }
        return out;
    }
public:
    const std::u16string source;
    explicit UnitPattern(std::u16string pattern):source(std::move(pattern)){
        try{compiled_=std::wregex(translate(source),std::regex_constants::ECMAScript);}
        catch(const std::regex_error&){throw options::Exception("SyntaxError",u"Native UnitUtil pattern compilation failed");}
    }
    std::optional<std::vector<Property>> match(const Property& value)const{
        auto* text=std::get_if<std::string>(&value.value);
        if(!text){
            if(property_host::nullish(value))property_host::typeError(
                std::string("Cannot read properties of ")+(value.missing()?"undefined":"null")+" (reading 'match')");
            property_host::typeError("dim.match is not a function");
        }
        const auto input=wide(property_host::text(*text));std::wsmatch result;
        if(!std::regex_search(input,result,compiled_))return {};
        std::vector<Property> slots;slots.reserve(result.size());
        for(const auto& sub:result){
            if(!sub.matched){slots.emplace_back(Missing{});continue;}
            std::u16string captured;for(auto c:sub.str())captured.push_back(static_cast<char16_t>(c));
            slots.emplace_back(property_host::debugText(captured));
        }
        return slots;
    }
};
using UnitPatternRef=std::shared_ptr<UnitPattern>;
}
