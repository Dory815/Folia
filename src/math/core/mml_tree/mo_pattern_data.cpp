// Generated source pattern data from pristine MathJax 4.1.3 mo.ts.
// Apache-2.0; see src/math/LICENSE-MathJax.txt. Root owns this file.
module folia.math.core.mml_tree.mo_patterns;
import std;
namespace folia::math::mml {namespace {
constexpr std::array<char16_t,31> source0={94,91,34,39,42,96,170,176,178,45,180,185,186,8216,45,8223,8242,45,8247,8279,8304,8305,8308,45,8319,8320,45,8334,93,43,36};
constexpr std::array<MoCodeUnitRange,13> ranges0={{{34,34},{39,39},{42,42},{96,96},{170,170},{176,176},{178,180},{185,186},{8216,8223},{8242,8247},{8279,8279},{8304,8305},{8308,8334}}};
constexpr std::array<char16_t,10> source1={94,91,34,39,8216,45,8223,93,43,36};
constexpr std::array<MoCodeUnitRange,3> ranges1={{{34,34},{39,39},{8216,8223}}};
constexpr std::array<char16_t,14> source2={94,91,97,45,122,65,45,90,93,123,50,44,125,36};
constexpr std::array<MoCodeUnitRange,2> ranges2={{{65,90},{97,122}}};
constexpr std::array<char16_t,36> source3={94,91,180,769,714,96,768,715,168,776,126,771,732,175,772,713,728,774,711,780,94,770,710,8400,8401,8406,8407,8417,729,775,730,778,8411,8412,93,36};
constexpr std::array<MoCodeUnitRange,18> ranges3={{{94,94},{96,96},{126,126},{168,168},{175,175},{180,180},{710,711},{713,715},{728,730},{732,732},{768,772},{774,776},{778,778},{780,780},{8400,8401},{8406,8407},{8411,8412},{8417,8417}}};
constexpr std::array<char16_t,11> source4={94,91,8592,8594,8596,9180,9181,9182,9183,93,36};
constexpr std::array<MoCodeUnitRange,4> ranges4={{{8592,8592},{8594,8594},{8596,8596},{9180,9183}}};
const std::array<MoPatternSpec,5> specs={{
{std::u16string_view(source0.data(),source0.size()),"",ranges0,1,std::nullopt},
{std::u16string_view(source1.data(),source1.size()),"",ranges1,1,std::nullopt},
{std::u16string_view(source2.data(),source2.size()),"",ranges2,2,std::nullopt},
{std::u16string_view(source3.data(),source3.size()),"",ranges3,1,1},
{std::u16string_view(source4.data(),source4.size()),"",ranges4,1,1},
}};
}
const MoPatternSpec& moPatternSpec(MoPattern kind){const auto index=static_cast<std::size_t>(kind);if(index>=specs.size())host::typeError("Invalid native MmlMo pattern");return specs[index];}
}
