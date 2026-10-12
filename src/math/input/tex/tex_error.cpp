// MathJax 4.1.3 TexError.ts semantic methods; Apache-2.0.
module folia.math.input.tex.tex_error;
import folia.math.core.tree.factory_types;
import folia.math.input.tex.tex_error_pattern;
import folia.math.util.property_host;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import std;
namespace folia::math::tex {
TexError::TexError(std::u16string errorId,std::u16string_view sourceMessage,
                   std::span<const Property> rest)
    : id(std::move(errorId)), message(processString(sourceMessage,rest)) {
}
std::u16string TexError::processString(std::u16string_view source,
                                       std::span<const Property> args) {
    auto parts=error_pattern::split(source);
    std::vector<std::optional<Property>> deferred(parts.size());
    const auto argument=[&](std::u16string_view digits) -> Property {
        const auto parsed=js_input::parseInt(
            digits,js_input::IntegerRadix::Decimal);
        // Source subtracts one and performs an ordinary indexed read. The
        // rest-parameter array has no numeric prototype entries in this host.
        if(!std::isfinite(parsed)||parsed<1||parsed>static_cast<double>(args.size()))
            return Missing{};
        return args[static_cast<std::size_t>(parsed)-1];
    };
    const auto joined=[&](const Property& value) -> std::u16string {
        if(value.missing()||std::holds_alternative<PropertyNull>(value.value))return {};
        return property_host::string(value);
    };
    const auto isNumber=[](const Property& value) {
        return std::holds_alternative<double>(value.value);
    };
    for(std::size_t i=1;i<parts.size();i+=2) {
        auto& part=parts[i];
        const std::u16string c=part.empty()?std::u16string{}:std::u16string(1,part[0]);
        if(c>=u"0"&&c<=u"9") {
            auto value=argument(part);
            if(isNumber(value))part=property_host::text(js_number::toString(std::get<double>(value.value)));
            else deferred[i]=std::move(value);
        } else if(c==u"{") {
            // This deliberately compares the entire substring(1), as Source
            // does, rather than testing just its first code unit.
            const auto remainder=part.substr(1);
            if(remainder>=u"0"&&remainder<=u"9") {
                const auto digits=part.size()>=2
                    ?std::u16string_view(part).substr(1,part.size()-2)
                    :std::u16string_view{};
                auto value=argument(digits);
                if(isNumber(value))part=property_host::text(js_number::toString(std::get<double>(value.value)));
                else deferred[i]=std::move(value);
            } else if(error_pattern::plural(part)) {
                part.insert(part.begin(),u'%');
            }
        }
    }
    std::u16string result;
    std::size_t length=0;
    for(std::size_t i=0;i<parts.size();++i)
        length+=deferred[i]?0:parts[i].size();
    result.reserve(length);
    for(std::size_t i=0;i<parts.size();++i) {
        if(deferred[i])result+=joined(*deferred[i]);
        else result+=parts[i];
    }
    return result;
}
}
