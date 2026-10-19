// Native mapping tables from MathJax 4.1.3 MathtoolsMappings.ts.
module folia.math.input.tex.mathtools.mappings;
import folia.math.input.tex.token_map;
import folia.math.input.tex.mathtools.methods;
import folia.math.util.function_types;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Property tuple(Property method,
               std::initializer_list<Property> arguments = {}) {
  PropertyArray values;
  values.push(std::move(method));
  for (const auto &value : arguments)
    values.push(value);
  return Property(values);
}
Property str(std::u16string_view value) {
  return Property(property_host::debugText(value));
}
Property str8(std::string_view value) { return Property(std::string(value)); }
Property number(double value) { return Property(value); }
void command(ParserRuntime &runtime, std::string_view name,
             PropertyList entries) {
  (void)TokenMapHost::create<CommandMap>(
      runtime.mapHandler, Property(std::string(name)),
      Property(std::move(entries)));
}
} // namespace
void MathtoolsMappings::initializeModule(ParserRuntime &runtime) {
  const Property methods = MathtoolsMethods::initializeModule(runtime);
  auto m = [&](std::u16string_view name) {
    return property_host::get(methods, property_host::debugText(name));
  };
  PropertyList macros;
  macros.setOwn("shoveleft", tuple(m(u"HandleShove"), {Property("left")}));
  macros.setOwn("shoveright", tuple(m(u"HandleShove"), {Property("right")}));
  constexpr std::array<std::tuple<std::string_view, double, double, double>, 17>
      arrows{{{"xleftrightarrow", 0x2194, 10, 10},
              {"xLeftarrow", 0x21d0, 12, 7},
              {"xRightarrow", 0x21d2, 7, 12},
              {"xLeftrightarrow", 0x21d4, 12, 12},
              {"xhookleftarrow", 0x21a9, 10, 5},
              {"xhookrightarrow", 0x21aa, 5, 10},
              {"xmapsto", 0x21a6, 10, 10},
              {"xrightharpoondown", 0x21c1, 5, 10},
              {"xleftharpoondown", 0x21bd, 10, 5},
              {"xrightleftharpoons", 0x21cc, 10, 10},
              {"xrightharpoonup", 0x21c0, 5, 10},
              {"xleftharpoonup", 0x21bc, 10, 5},
              {"xleftrightharpoons", 0x21cb, 10, 10},
              {"xlongrightarrow", 0x27f6, 7, 12},
              {"xlongleftarrow", 0x27f5, 12, 7},
              {"xLongrightarrow", 0x27f9, 7, 12},
              {"xLongleftarrow", 0x27f8, 12, 7}}};
  for (const auto &[key, code, left, right] : arrows) {
    if (key == "xlongrightarrow" || key == "xlongleftarrow" ||
        key == "xLongrightarrow" || key == "xLongleftarrow")
      macros.setOwn(std::string(key),
                    tuple(m(u"xArrow"), {number(code), number(left),
                                         number(right), number(1.45)}));
    else
      macros.setOwn(
          std::string(key),
          tuple(m(u"xArrow"), {number(code), number(left), number(right)}));
  }
  for (const auto &[key, pos] :
       std::array<std::pair<std::string_view, std::string_view>, 3>{
           {{"mathllap", "l"}, {"mathrlap", "r"}, {"mathclap", "c"}}})
    macros.setOwn(std::string(key),
                  tuple(m(u"MathLap"), {str8(pos), Property(false)}));
  for (const auto &[key, pos] :
       std::array<std::pair<std::string_view, std::string_view>, 4>{
           {{"clap", "c"},
            {"textllap", "l"},
            {"textrlap", "r"},
            {"textclap", "c"}}})
    macros.setOwn(std::string(key), tuple(m(u"MtLap"), {str8(pos)}));
  macros.setOwn("cramped", m(u"Cramped"));
  for (const auto &[key, pos] :
       std::array<std::pair<std::string_view, std::string_view>, 3>{
           {{"crampedllap", "l"}, {"crampedrlap", "r"},
            {"crampedclap", "c"}}})
    macros.setOwn(std::string(key),
                  tuple(m(u"MathLap"), {str8(pos), Property(true)}));
  macros.setOwn(
      "crampedsubstack",
      tuple(m(u"Macro"),
            {Property("\\begin{crampedsubarray}{c}#1\\end{crampedsubarray}"),
             number(1)}));
  macros.setOwn("mathmbox", m(u"MathMBox"));
  macros.setOwn("mathmakebox", m(u"MathMakeBox"));
  macros.setOwn("overbracket", m(u"UnderOverBracket"));
  macros.setOwn("underbracket", m(u"UnderOverBracket"));
  macros.setOwn("refeq", m(u"HandleRef"));
  macros.setOwn("MoveEqLeft",
                tuple(m(u"Macro"), {Property("\\hspace{#1em}&\\hspace{-#1em}"),
                                    number(1), Property("2")}));
  macros.setOwn("Aboxed", m(u"Aboxed"));
  macros.setOwn("MakeAboxedCommand", m(u"MakeAboxedCommand"));
  macros.setOwn("ArrowBetweenLines", m(u"ArrowBetweenLines"));
  macros.setOwn("vdotswithin", m(u"VDotsWithin"));
  macros.setOwn("shortvdotswithin", m(u"ShortVDotsWithin"));
  macros.setOwn("MTFlushSpaceAbove", m(u"FlushSpaceAbove"));
  macros.setOwn("MTFlushSpaceBelow", m(u"FlushSpaceBelow"));
  for (const auto &[key, value] :
       std::array<std::pair<std::string_view, std::u16string_view>, 6>{
           {{"DeclarePairedDelimiter", u"DeclarePairedDelimiter"},
            {"DeclarePairedDelimiterX", u"DeclarePairedDelimiterX"},
            {"DeclarePairedDelimiterXPP", u"DeclarePairedDelimiterXPP"},
            {"DeclarePairedDelimiters", u"DeclarePairedDelimiter"},
            {"DeclarePairedDelimitersX", u"DeclarePairedDelimiterX"},
            {"DeclarePairedDelimitersXPP", u"DeclarePairedDelimiterXPP"}}})
    macros.setOwn(std::string(key), m(value));
  macros.setOwn("vcentercolon",
                tuple(m(u"CenterColon"), {Property(true), Property(true)}));
  macros.setOwn("ordinarycolon", tuple(m(u"CenterColon"), {Property(false)}));
  macros.setOwn("MTThinColon",
                tuple(m(u"CenterColon"),
                      {Property(true), Property(true), Property(true)}));
  constexpr std::array<
      std::tuple<std::string_view, std::string_view, std::u16string_view>, 21>
      relations{{{"coloneqq", ":=", u"≔"},
                 {"Coloneqq", "::=", u"⩴"},
                 {"coloneq", ":=", u"≔"},
                 {"Coloneq", "::=", u"⩺"},
                 {"eqqcolon", "=:", u"≕"},
                 {"Eqqcolon", "=::", u""},
                 {"eqcolon", "=:", u"≕"},
                 {"Eqcolon", "=::", u""},
                 {"colonapprox", ":\\approx", u""},
                 {"Colonapprox", "::\\approx", u""},
                 {"colonsim", ":\\sim", u""},
                 {"Colonsim", "::\\sim", u""},
                 {"dblcolon", "::", u"∷"},
                 {"approxcolon", "\\approx:", u""},
                 {"Approxcolon", "\\approx::", u""},
                 {"simcolon", "\\sim:", u""},
                 {"Simcolon", "\\sim::", u""},
                 {"colondash", ":-", u""},
                 {"Colondash", "::-", u""},
                 {"dashcolon", "-:", u"∹"},
                 {"Dashcolon", "-::", u""}}};
  for (const auto &[key, tex, unicode] : relations) {
    if (unicode.empty())
      macros.setOwn(std::string(key), tuple(m(u"Relation"), {str8(tex)}));
    else
      macros.setOwn(std::string(key),
                    tuple(m(u"Relation"), {str8(tex), str(unicode)}));
  }
  macros.setOwn("nuparrow",
                tuple(m(u"NArrow"), {str(u"↑"), Property(".06em")}));
  macros.setOwn("ndownarrow",
                tuple(m(u"NArrow"), {str(u"↓"), Property(".25em")}));
  macros.setOwn(
      "bigtimes",
      tuple(m(u"Macro"), {Property("\\mathop{\\Large\\kern-.1em\\boldsymbol{"
                                   "\\times}\\kern-.1em}")}));
  macros.setOwn("splitfrac", tuple(m(u"SplitFrac"), {Property(false)}));
  macros.setOwn("splitdfrac", tuple(m(u"SplitFrac"), {Property(true)}));
  macros.setOwn("xmathstrut", m(u"XMathStrut"));
  macros.setOwn("prescript", m(u"Prescript"));
  macros.setOwn("newtagform", tuple(m(u"NewTagForm"), {Property(false)}));
  macros.setOwn("renewtagform", tuple(m(u"NewTagForm"), {Property(true)}));
  macros.setOwn("usetagform", m(u"UseTagForm"));
  PropertyArray adjustlimits;
  adjustlimits.push(m(u"MacroWithTemplate"));
  adjustlimits.push(Property(
      "\\mathop{{#1}\\vphantom{{#3}}}_{{#2}\\vphantom{{#4}}}"
      "\\mathop{{#3}\\vphantom{{#1}}}_{{#4}\\vphantom{{#2}}}"));
  adjustlimits.push(number(4));
  adjustlimits.resize(7);
  adjustlimits.set(4, Property("_"));
  adjustlimits.set(6, Property("_"));
  macros.setOwn("adjustlimits", Property(adjustlimits));
  macros.setOwn("mathtoolsset", m(u"SetOptions"));
  command(runtime, "mathtools-macros", std::move(macros));

  PropertyList legacy;
  legacy.setOwn("coloneq", tuple(m(u"Relation"), {Property(":-")}));
  legacy.setOwn("Coloneq", tuple(m(u"Relation"), {Property("::-")}));
  legacy.setOwn("eqcolon", tuple(m(u"Relation"), {Property("-:"), str(u"∹")}));
  legacy.setOwn("Eqcolon", tuple(m(u"Relation"), {Property("-::")}));
  command(runtime, "mathtools-legacycolonsymbols", std::move(legacy));

  Property envParser = property_host::get(runtime.parseMethods, "environment");
  PropertyList environments;
  auto env = [&](std::string key, std::u16string method,
                 std::initializer_list<Property> args) {
    environments.setOwn(std::move(key), tuple(m(method), args));
  };
  env("dcases", u"Array",
      {Property(PropertyNull{}), Property("\\{"), Property(""), Property("ll"),
       Property(PropertyNull{}), Property(".2em"), Property("D")});
  env("rcases", u"Array",
      {Property(PropertyNull{}), Property(""), Property("\\}"), Property("ll"),
       Property(PropertyNull{}), Property(".2em")});
  env("drcases", u"Array",
      {Property(PropertyNull{}), Property(""), Property("\\}"), Property("ll"),
       Property(PropertyNull{}), Property(".2em"), Property("D")});
  for (const auto &[key, open, close, style] :
       std::array<std::tuple<std::string_view, std::string_view,
                             std::string_view, std::string_view>,
                  4>{{{"dcases*", "{", "", "D"},
                      {"rcases*", "", "}", ""},
                      {"drcases*", "", "}", "D"},
                      {"cases*", "{", "", ""}}}) {
    if (!style.empty())
      env(std::string(key), u"Cases",
          {Property(PropertyNull{}), str8(open), str8(close), str8(style)});
    else
      env(std::string(key), u"Cases",
          {Property(PropertyNull{}), str8(open), str8(close)});
  }
  constexpr std::array<
      std::tuple<std::string_view, std::string_view, std::string_view>, 6>
      matrices{{{"matrix*", "", ""},
                {"pmatrix*", "(", ")"},
                {"bmatrix*", "[", "]"},
                {"Bmatrix*", "\\{", "\\}"},
                {"vmatrix*", "\\vert", "\\vert"},
                {"Vmatrix*", "\\Vert", "\\Vert"}}};
  for (const auto &[key, open, close] : matrices) {
    const bool unspecified = key == "matrix*";
    env(std::string(key), u"MtMatrix",
        {Property(PropertyNull{}),
         unspecified && open.empty() ? Property(PropertyNull{}) : str8(open),
         unspecified && close.empty() ? Property(PropertyNull{}) : str8(close)});
  }
  constexpr std::array<std::tuple<std::string_view, std::string_view,
                                  std::string_view, std::string_view>,
                       10>
      small{{{"smallmatrix*", "", "", ""},
             {"psmallmatrix", "(", ")", "c"},
             {"psmallmatrix*", "(", ")", ""},
             {"bsmallmatrix", "[", "]", "c"},
             {"bsmallmatrix*", "[", "]", ""},
             {"Bsmallmatrix", "\\{", "\\}", "c"},
             {"Bsmallmatrix*", "\\{", "\\}", ""},
             {"vsmallmatrix", "\\vert", "\\vert", "c"},
             {"vsmallmatrix*", "\\vert", "\\vert", ""},
             {"Vsmallmatrix", "\\Vert", "\\Vert", "c"}}};
  for (const auto &[key, open, close, align] : small) {
    const bool unspecified = key == "smallmatrix*";
    if (!align.empty())
      env(std::string(key), u"MtSmallMatrix",
          {Property(PropertyNull{}),
           unspecified && open.empty() ? Property(PropertyNull{}) : str8(open),
           unspecified && close.empty() ? Property(PropertyNull{}) : str8(close),
           str8(align)});
    else
      env(std::string(key), u"MtSmallMatrix",
          {Property(PropertyNull{}),
           unspecified && open.empty() ? Property(PropertyNull{}) : str8(open),
           unspecified && close.empty() ? Property(PropertyNull{}) : str8(close)});
  }
  env("Vsmallmatrix*", u"MtSmallMatrix",
      {Property(PropertyNull{}), Property("\\Vert"), Property("\\Vert")});
  env("crampedsubarray", u"Array",
      {Property(PropertyNull{}), Property(PropertyNull{}),
       Property(PropertyNull{}), Property(PropertyNull{}), Property("0em"),
       Property("0.1em"), Property("S'"), number(1)});
  env("multlined", u"MtMultlined", {});
  env("spreadlines", u"SpreadLines", {Property(true)});
  env("lgathered", u"AmsEqnArray",
      {Property(PropertyNull{}), Property(PropertyNull{}),
       Property(PropertyNull{}), Property("l"), Property("t"),
       Property(PropertyNull{}), Property(".5em"), Property("D")});
  env("rgathered", u"AmsEqnArray",
      {Property(PropertyNull{}), Property(PropertyNull{}),
       Property(PropertyNull{}), Property("r"), Property("t"),
       Property(PropertyNull{}), Property(".5em"), Property("D")});
  (void)TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,Property("mathtools-environments"),
                                             functions::fromProperty(envParser),
                                             Property(std::move(environments)));
  PropertyList delimiters{{"\\lparen", Property("(")},
                          {"\\rparen", Property(")")}};
  Property delimiterParser =
      property_host::get(runtime.parseMethods, "delimiter");
  (void)TokenMapHost::create<DelimiterMap>(runtime.mapHandler,
      Property("mathtools-delimiters"),
      functions::fromProperty(delimiterParser),
      Property(std::move(delimiters)));
  PropertyList characters;
  characters.setOwn(":", tuple(m(u"CenterColon"), {Property(true)}));
  command(runtime, "mathtools-characters", std::move(characters));
}
} // namespace folia::math::tex
