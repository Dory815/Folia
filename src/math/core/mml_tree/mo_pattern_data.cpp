// Generated from pinned MathJax 4.1.3 MmlNodes/mo.ts by operator-data.cjs.
// Apache-2.0; see src/math/LICENSE-MathJax.txt.
module folia.math.core.mml_tree.mo_patterns;
import std;
namespace folia::math::mml { namespace {
constexpr std::array<MoCodeUnitRange,13> ranges_0{{
  {char16_t{0x22},char16_t{0x22}},
  {char16_t{0x27},char16_t{0x27}},
  {char16_t{0x2a},char16_t{0x2a}},
  {char16_t{0x60},char16_t{0x60}},
  {char16_t{0xaa},char16_t{0xaa}},
  {char16_t{0xb0},char16_t{0xb0}},
  {char16_t{0xb2},char16_t{0xb4}},
  {char16_t{0xb9},char16_t{0xba}},
  {char16_t{0x2018},char16_t{0x201f}},
  {char16_t{0x2032},char16_t{0x2037}},
  {char16_t{0x2057},char16_t{0x2057}},
  {char16_t{0x2070},char16_t{0x2071}},
  {char16_t{0x2074},char16_t{0x208e}}
}};
constexpr std::array<MoCodeUnitRange,3> ranges_1{{
  {char16_t{0x22},char16_t{0x22}},
  {char16_t{0x27},char16_t{0x27}},
  {char16_t{0x2018},char16_t{0x201f}}
}};
constexpr std::array<MoCodeUnitRange,2> ranges_2{{
  {char16_t{0x41},char16_t{0x5a}},
  {char16_t{0x61},char16_t{0x7a}}
}};
constexpr std::array<MoCodeUnitRange,18> ranges_3{{
  {char16_t{0x5e},char16_t{0x5e}},
  {char16_t{0x60},char16_t{0x60}},
  {char16_t{0x7e},char16_t{0x7e}},
  {char16_t{0xa8},char16_t{0xa8}},
  {char16_t{0xaf},char16_t{0xaf}},
  {char16_t{0xb4},char16_t{0xb4}},
  {char16_t{0x2c6},char16_t{0x2c7}},
  {char16_t{0x2c9},char16_t{0x2cb}},
  {char16_t{0x2d8},char16_t{0x2da}},
  {char16_t{0x2dc},char16_t{0x2dc}},
  {char16_t{0x300},char16_t{0x304}},
  {char16_t{0x306},char16_t{0x308}},
  {char16_t{0x30a},char16_t{0x30a}},
  {char16_t{0x30c},char16_t{0x30c}},
  {char16_t{0x20d0},char16_t{0x20d1}},
  {char16_t{0x20d6},char16_t{0x20d7}},
  {char16_t{0x20db},char16_t{0x20dc}},
  {char16_t{0x20e1},char16_t{0x20e1}}
}};
constexpr std::array<MoCodeUnitRange,4> ranges_4{{
  {char16_t{0x2190},char16_t{0x2190}},
  {char16_t{0x2192},char16_t{0x2192}},
  {char16_t{0x2194},char16_t{0x2194}},
  {char16_t{0x23dc},char16_t{0x23df}}
}};
const std::array<MoPatternSpec,5> specs{{
  {u"^[\"'*`\u00aa\u00b0\u00b2-\u00b4\u00b9\u00ba\u2018-\u201f\u2032-\u2037\u2057\u2070\u2071\u2074-\u207f\u2080-\u208e]+$","",ranges_0,1,std::nullopt},
  {u"^[\"'\u2018-\u201f]+$","",ranges_1,1,std::nullopt},
  {u"^[a-zA-Z]{2,}$","",ranges_2,2,std::nullopt},
  {u"^[\u00b4\u0301\u02ca`\u0300\u02cb\u00a8\u0308~\u0303\u02dc\u00af\u0304\u02c9\u02d8\u0306\u02c7\u030c^\u0302\u02c6\u20d0\u20d1\u20d6\u20d7\u20e1\u02d9\u0307\u02da\u030a\u20db\u20dc]$","",ranges_3,1,1},
  {u"^[\u2190\u2192\u2194\u23dc\u23dd\u23de\u23df]$","",ranges_4,1,1}
}};
}
const MoPatternSpec& moPatternSpec(MoPattern kind) {
  const auto index=static_cast<std::size_t>(kind);
  if(index>=specs.size()) throw std::out_of_range("Invalid native MmlMo pattern");
  return specs[index];
}
}
