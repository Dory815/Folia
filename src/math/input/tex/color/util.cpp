/*************************************************************
 * Native MathJax 4.1.3 input/tex/color/ColorUtil.ts algorithms.
 *************************************************************/
module folia.math.input.tex.color.util;
import std;

namespace folia::math::tex {
namespace {
using View = std::u16string_view;

bool sourceWhitespace(char16_t c) noexcept {
    return c==0x0009 || c==0x000A || c==0x000B || c==0x000C || c==0x000D ||
        c==0x0020 || c==0x00A0 || c==0x1680 || (c>=0x2000 && c<=0x200A) ||
        c==0x2028 || c==0x2029 || c==0x202F || c==0x205F || c==0x3000 || c==0xFEFF;
}

View trimSource(View value) noexcept {
    while (!value.empty() && sourceWhitespace(value.front())) value.remove_prefix(1);
    while (!value.empty() && sourceWhitespace(value.back())) value.remove_suffix(1);
    return value;
}

std::vector<View> splitSourceCommas(View value) {
    value=trimSource(value); // Source rgb/RGB calls trim() before split().
    std::vector<View> result;
    std::size_t start=0;
    while (start<=value.size()) {
        const auto comma=value.find(u',',start);
        const auto end=comma==View::npos ? value.size() : comma;
        result.push_back(trimSource(value.substr(start,end-start)));
        if (comma==View::npos) break;
        start=comma+1;
    }
    return result;
}

bool decimal(View value) noexcept {
    std::size_t i=0, digits=0;
    while (i<value.size() && value[i]>=u'0' && value[i]<=u'9') { ++i; ++digits; }
    if (i<value.size() && value[i]==u'.') {
        ++i;
        while (i<value.size() && value[i]>=u'0' && value[i]<=u'9') { ++i; ++digits; }
    }
    return digits!=0 && i==value.size();
}

bool integer(View value) noexcept {
    if (value.empty()) return false;
    for (char16_t c:value) if(c<u'0'||c>u'9') return false;
    return true;
}

double parseAsciiNumber(View value) {
    // The caller validated the Source decimal grammar. Floating from_chars is
    // locale-independent, unlike strtod, and converts directly to the same
    // IEEE-754 double domain used by JavaScript Number/parseFloat.
    std::string ascii;
    ascii.reserve(value.size());
    for(char16_t c:value) ascii.push_back(static_cast<char>(c));
    double result{};
    const auto [end,error]=std::from_chars(ascii.data(),ascii.data()+ascii.size(),
                                            result,std::chars_format::fixed);
    if(error==std::errc{} && end==ascii.data()+ascii.size()) return result;
    if(error!=std::errc::result_out_of_range || end!=ascii.data()+ascii.size())
        throw std::runtime_error("validated color decimal conversion failed");

    // Source parseFloat returns +/-Infinity on overflow and zero on decimal
    // underflow. from_chars reports result_out_of_range for both; determine
    // which side of the representable range the validated unsigned decimal is
    // on without a locale-sensitive secondary conversion.
    // In this unsigned, exponent-free grammar, a nonzero integer part is
    // at least one and cannot underflow. A value below one cannot overflow.
    const auto integerPart=value.substr(0,value.find(u'.'));
    return std::ranges::any_of(integerPart, [](char16_t c) { return c!=u'0'; })
        ? std::numeric_limits<double>::infinity() : 0.0;
}

std::string hex(double value) {
    // Every caller has already enforced a nonnegative finite value no larger
    // than 255. Conversion truncates, matching JS Number#toString(16) for the
    // integer values used by the Source algorithms.
    const auto n=static_cast<unsigned>(value);
    char buffer[16]{};
    const auto [end,error]=std::to_chars(buffer,buffer+sizeof(buffer),n,16);
    if(error!=std::errc{}) throw std::runtime_error("color hex conversion failed");
    std::string result(buffer,end);
    if(result.size()<2) result.insert(result.begin(),'0');
    return result;
}

[[noreturn]] void rangeError(View model,View low,View high) {
    const std::array<std::u16string,3> args{
        std::u16string(model),std::u16string(low),std::u16string(high)};
    throw TexError("ModelArg2",
        u"Color values for the %1 model must be between %2 and %3",args);
}

[[noreturn]] void badValue() {
    throw TexError("BadColorValue",u"Invalid color value");
}

[[noreturn]] void invalidDecimal() {
    throw TexError("InvalidDecimalNumber",u"Invalid decimal number");
}

[[noreturn]] void invalidInteger() {
    throw TexError("InvalidNumber",u"Invalid number");
}
}

std::u16string ColorModel::normalizeColor(View model,View definition) const {
    if(model.empty() || model==u"named") {
        if(definition.find(u';')!=View::npos) badValue();
        return std::u16string(definition);
    }

    if(model==u"rgb" || model==u"RGB") {
        const auto parts=splitSourceCommas(definition);
        if(parts.size()!=3) {
            const std::array<std::u16string,1> args{std::u16string(model)};
            throw TexError("ModelArg1",u"Color values for the %1 model require 3 numbers",args);
        }
        std::u16string result=u"#";
        for(View part:parts) {
            if(model==u"rgb") {
                if(!decimal(part)) invalidDecimal();
                const double n=parseAsciiNumber(part);
                if(n<0 || n>1) rangeError(u"rgb",u"0",u"1");
                const auto encoded=hex(std::floor(n*255));
                for(char c:encoded) result.push_back(static_cast<char16_t>(c));
            } else {
                if(!integer(part)) invalidInteger();
                const double n=parseAsciiNumber(part); // Source parseInt; regex makes it all digits.
                if(n>255) rangeError(u"RGB",u"0",u"255");
                const auto encoded=hex(n);
                for(char c:encoded) result.push_back(static_cast<char16_t>(c));
            }
        }
        return result;
    }

    if(model==u"gray") {
        const View value=trimSource(definition);
        if(!decimal(value)) invalidDecimal();
        const double n=parseAsciiNumber(value);
        if(n<0 || n>1) rangeError(u"gray",u"0",u"1");
        const auto encoded=hex(std::floor(n*255));
        std::u16string result=u"#";
        for(int i=0;i<3;++i) for(char c:encoded) result.push_back(static_cast<char16_t>(c));
        return result;
    }

    const std::array<std::u16string,1> args{std::u16string(model)};
    throw TexError("UndefinedColorModel",u"Color model '%1' not defined",args);
}

std::u16string ColorModel::getColorByName(View name) const {
    if(const auto found=user_colors_.find(std::u16string(name));found!=user_colors_.end())
        return found->second;
    if(const auto found=ColorConstants::find(name)) {
        std::u16string result;
        result.reserve(found->size());
        for(char c:*found) result.push_back(static_cast<char16_t>(c));
        return result;
    }
    if(name.find(u';')!=View::npos) badValue();
    return std::u16string(name);
}

std::u16string ColorModel::getColor(View model,View definition) const {
    if(model.empty() || model==u"named") return getColorByName(definition);
    return normalizeColor(model,definition);
}

void ColorModel::defineColor(View model,View name,View definition) {
    auto normalized=normalizeColor(model,definition);
    user_colors_.insert_or_assign(std::u16string(name),std::move(normalized));
}
}
