// MathJax 4.1.3 ts/util/string.ts; Apache-2.0 (LICENSE-MathJax.txt).
// UTF-16 is the source string domain; external UTF-8 belongs to util/utf16.
export module folia.math.util.string;
export import folia.math.util.option_types;
import std;
export namespace folia::math::string_util {
using Error=options::Exception;
double sortLength(std::u16string_view a,std::u16string_view b);
std::u16string quotePattern(std::u16string_view text);
std::vector<double> unicodeChars(std::u16string_view text);
std::u16string unicodeString(std::span<const double> data);
bool isPercent(std::u16string_view text);
std::vector<std::u16string> split(std::u16string_view text);
std::u16string replaceUnicode(std::u16string_view text);
std::u16string toEntity(std::u16string_view text);
}
