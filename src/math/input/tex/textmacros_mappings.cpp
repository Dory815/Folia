// MathJax 4.1.3 input/tex/textmacros/TextMacrosMappings.ts; Apache-2.0.
module folia.math.input.tex.textmacros.mappings;
import std;
namespace folia::math::tex {
void TextMacrosMappings::initializeModule(MapHandler &maps) {
    using M = TextMacrosMethods;
    maps.registerMap(std::make_shared<MacroMap>(MacroMapInit{
        {u"text-special", std::nullopt},
        {Macro(u"$", &M::Math), Macro(u"%", &M::Comment), Macro(u"^", &M::MathModeOnly),
         Macro(u"_", &M::MathModeOnly), Macro(u"&", &M::Misplaced), Macro(u"#", &M::Misplaced),
         Macro(u"~", &M::Tilde), Macro(u" ", &M::Space), Macro(u"\t", &M::Space),
         Macro(u"\r", &M::Space), Macro(u"\n", &M::Space), Macro(u"\u00A0", &M::Tilde),
         Macro(u"{", &M::OpenBrace), Macro(u"}", &M::CloseBrace), Macro(u"`", &M::OpenQuote),
         Macro(u"'", &M::CloseQuote)}}));
    std::vector<Macro> commands;
    auto add = [&](std::u16string_view name, MacroHandler method,
                   std::vector<TexArgument> arguments = {}) {
        commands.emplace_back(std::u16string(name), method, std::move(arguments));
    };
    add(u"(", &M::Math);
    for (auto name : {u"$", u"_", u"%", u"{", u"}", u" ", u"&", u"#"})
        add(name, &M::SelfQuote);
    add(u"\\", M::Macro, {std::u16string(u"$\\\\$")});
    const std::pair<std::u16string_view, std::u16string_view> accents[]{
        {u"'", u"\u00B4"}, {u"\u2019", u"\u00B4"}, {u"`", u"`"},     {u"\u2018", u"`"},
        {u"^", u"^"},      {u"\"", u"\u00A8"},     {u"~", u"~"},     {u"=", u"\u00AF"},
        {u".", u"\u02D9"}, {u"u", u"\u02D8"},      {u"v", u"\u02C7"}};
    for (auto const &[name, glyph] : accents)
        add(name, &M::Accent, {std::u16string(glyph)});
    add(u"emph", &M::Emph);
    const std::pair<std::u16string_view, std::u16string_view> fonts[]{
        {u"rm", u"normal"},
        {u"mit", u"italic"},
        {u"oldstyle", u"-tex-oldstyle"},
        {u"cal", u"-tex-calligraphic"},
        {u"it", u"-tex-mathit"},
        {u"bf", u"bold"},
        {u"sf", u"sans-serif"},
        {u"tt", u"monospace"}};
    for (auto const &[name, font] : fonts)
        add(name, &M::SetFont, {std::u16string(font)});
    add(u"frak", &M::TextFont, {std::u16string(u"fraktur")});
    add(u"Bbb", &M::TextFont, {std::u16string(u"double-struck")});
    const std::pair<std::u16string_view, double> sizes[]{
        {u"Tiny", .5},     {u"tiny", .6},         {u"scriptsize", .7}, {u"SMALL", .7},
        {u"Small", .8},    {u"footnotesize", .8}, {u"small", .9},      {u"normalsize", 1},
        {u"large", 1.095}, {u"Large", 1.2},       {u"LARGE", 1.44},    {u"huge", 1.73},
        {u"Huge", 2.07},   {u"HUGE", 2.49}};
    for (auto const &[name, size] : sizes)
        add(name, &M::SetSize, {size});
    const std::pair<std::u16string_view, std::u16string_view> textMacros[]{
        {u"textnormal", u"{\\rm #1}"}, {u"textup", u"{\\rm #1}"}, {u"textrm", u"{\\rm #1}"},
        {u"textit", u"{\\it #1}"},     {u"textbf", u"{\\bf #1}"}, {u"textsf", u"{\\sf #1}"},
        {u"texttt", u"{\\tt #1}"}};
    for (auto const &[name, replacement] : textMacros)
        add(name, M::Macro, {std::u16string(replacement), 1.0});
    const std::pair<std::u16string_view, std::u16string_view> inserts[]{
        {u"dagger", u"\u2020"}, {u"ddagger", u"\u2021"}, {u"S", u"\u00A7"},
        {u"AA", u"\u212B"},     {u"ldots", u"\u2026"},   {u"vdots", u"\u22EE"}};
    for (auto const &[name, glyph] : inserts)
        add(name, &M::Insert, {std::u16string(glyph)});
    const std::pair<std::u16string_view, double> spaces[]{{u",", 3.0 / 18},
                                                          {u":", 4.0 / 18},
                                                          {u">", 4.0 / 18},
                                                          {u";", 5.0 / 18},
                                                          {u"!", -3.0 / 18},
                                                          {u"enspace", .5},
                                                          {u"quad", 1},
                                                          {u"qquad", 2},
                                                          {u"thinspace", 3.0 / 18},
                                                          {u"negthinspace", -3.0 / 18}};
    for (auto const &[name, width] : spaces)
        add(name, M::Spacer, {width});
    for (auto name : {u"hskip", u"hspace", u"kern", u"mskip", u"mspace", u"mkern"})
        add(name, M::Hskip);
    add(u"rule", M::rule);
    add(u"Rule", M::Rule);
    add(u"Space", M::Rule, {std::u16string(u"blank")});
    for (auto name : {u"color", u"textcolor", u"colorbox", u"fcolorbox", u"href", u"style",
                      u"class", u"data", u"cssId", u"unicode", u"U", u"char"})
        add(name, &M::CheckAutoload);
    add(u"ref", M::HandleRef, {false});
    add(u"eqref", M::HandleRef, {true});
    add(u"underline", M::UnderOver, {std::u16string(u"2015")});
    add(u"llap", M::Lap);
    add(u"rlap", M::Lap);
    add(u"phantom", M::Phantom);
    add(u"vphantom", M::Phantom, {1.0, 0.0});
    add(u"hphantom", M::Phantom, {0.0, 1.0});
    add(u"smash", M::Smash);
    add(u"mmlToken", M::MmlToken);
    maps.registerMap(std::make_shared<CommandMap>(
        CommandMapInit{MacroMapInit{{u"text-macros", std::nullopt}, std::move(commands)}}));
}
} // namespace folia::math::tex
