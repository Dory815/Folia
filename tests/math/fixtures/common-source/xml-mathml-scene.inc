// Source MathJax 4.1.3 fb987178c2d279a99b0db5ea02e751691703955e
// Input: pinned direct MathML XMLNode/foreignObject witness; not TeX package coverage.
// scope-sha256 9705d389538b2fc8bf782feea4372ace639b3727304f2971937bbcfb0042a0d1
// input/mathml.js 8fc5382e4a81b8be02484c9f6de91618ce8a5c859af97b5175df6badf35035db | input/mathml.ts e027607ae93b8dedf7b8312b1b814c50cf8cd0c6b332f56edba02d1d3bd7be1b
// input/mathml/FindMathML.js fec27373d9b959de3511e5162eb98bb50d0b17a95e4e746a62f55fe625b86c2a | input/mathml/FindMathML.ts 1237e59c4a1b1e4852b4bf6986db1cbbb84efc0726426051021bcc6d2595dfd9
// input/mathml/MathMLCompile.js 0b190d56a673941b6b4d1c8e8632dfa894ed50ddccce484e9e8d142c6d1e8880 | input/mathml/MathMLCompile.ts 2bb1f7258daf68fa6d51ea585b08312c1bc407719bd6711651369db1d5b68349
enum class XmlHdwPolicy { automatic, use, force, ignore };
enum class XmlSourceStyleKind { text, number };
struct XmlSourceStyleEntry { std::u16string_view selector,property; XmlSourceStyleKind kind; std::u16string_view text; double number; };
struct XmlSourceObservation { double w,h,d,scale,rscale; std::u16string_view family,font_size; std::string_view html,wrapper_styles,source_html_hdw; bool can_measure_nodes; double output_scale,metrics_scale,em,ex; std::u16string_view metrics_family; };
struct XmlSourceCase { mathml_scene_test::Case source; XmlHdwPolicy hdw; double scale; bool can_measure_nodes; std::string_view start_kind,start_parent_kind; std::span<std::u16string_view const> payloads; std::span<XmlSourceObservation const> observations; };
constexpr XmlSourceStyleEntry sourceXmlStyleEntries[]{
  {u"foreignObject[data-mjx-html]", u"overflow", XmlSourceStyleKind::text, u"visible", 0},
  {u"mjx-measure-xml", u"position", XmlSourceStyleKind::text, u"absolute", 0},
  {u"mjx-measure-xml", u"left", XmlSourceStyleKind::number, {}, 0},
  {u"mjx-measure-xml", u"top", XmlSourceStyleKind::number, {}, 0},
  {u"mjx-measure-xml", u"display", XmlSourceStyleKind::text, u"inline-block", 0},
  {u"mjx-measure-xml", u"line-height", XmlSourceStyleKind::text, u"normal", 0},
  {u"mjx-measure-xml", u"white-space", XmlSourceStyleKind::text, u"normal", 0},
  {u"mjx-html", u"display", XmlSourceStyleKind::text, u"inline-block", 0},
  {u"mjx-html", u"line-height", XmlSourceStyleKind::text, u"normal", 0},
  {u"mjx-html", u"text-align", XmlSourceStyleKind::text, u"initial", 0},
  {u"mjx-html", u"white-space", XmlSourceStyleKind::text, u"initial", 0},
  {u"mjx-html-holder", u"display", XmlSourceStyleKind::text, u"block", 0},
  {u"mjx-html-holder", u"position", XmlSourceStyleKind::text, u"absolute", 0},
  {u"mjx-html-holder", u"top", XmlSourceStyleKind::number, {}, 0},
  {u"mjx-html-holder", u"left", XmlSourceStyleKind::number, {}, 0},
  {u"mjx-html-holder", u"bottom", XmlSourceStyleKind::number, {}, 0},
  {u"mjx-html-holder", u"right", XmlSourceStyleKind::number, {}, 0}
};
constexpr XmlSourceStyleEntry sourceAnnotationXmlStyleEntries[]{
  {u"foreignObject[data-mjx-xml]", u"font-family", XmlSourceStyleKind::text, u"initial", 0},
  {u"foreignObject[data-mjx-xml]", u"line-height", XmlSourceStyleKind::text, u"normal", 0},
  {u"foreignObject[data-mjx-xml]", u"overflow", XmlSourceStyleKind::text, u"visible", 0}
};
constexpr std::array<mathml_scene_test::Node, 5> xml0Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 2, 0, 1, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 3, 1, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> xml0Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml0XmlPayloads{{
  u"<section class=\"xml-probe\" data-order=\"a&amp;b\"><span title=\"A &amp; B\">Alpha &amp; Beta</span><!--inside--><br><svg xmlns=\"http://www.w3.org/2000/svg\"><text>G</text></svg></section>"
}};
constexpr std::array<XmlSourceObservation, 1> xml0Observations{{
  {0, 0, 0, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="font-family: Times; font-size: 16px;" variant=""><section class="xml-probe" data-order="a&amp;b"><span title="A &amp; B">Alpha &amp; Beta</span><!--inside--><br><svg xmlns="http://www.w3.org/2000/svg"><text>G</text></svg></section></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "force", false, 1, 1, 16, 8, u""}
}};

constexpr std::array<mathml_scene_test::Node, 5> xml1Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 2, 0, 1, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 3, 1, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> xml1Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml1XmlPayloads{{
  u"<div data-mjx-hdw=\"1em .25em 3em\"><span>F</span></div>"
}};
constexpr std::array<XmlSourceObservation, 1> xml1Observations{{
  {3, 1, 0.25, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="height: 1.25em; width: 3em; vertical-align: -0.25em;" variant=""><mjx-html-holder style="font-family: Times; font-size: 16px;"><div data-mjx-hdw="1em .25em 3em"><span>F</span></div></mjx-html-holder></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "force", false, 1, 1, 16, 8, u""}
}};

constexpr std::array<mathml_scene_test::Node, 5> xml2Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 2, 0, 1, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 3, 1, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> xml2Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml2XmlPayloads{{
  u"<div data-mjx-hdw=\".75em .125em 2.5em\"><i>U</i></div>"
}};
constexpr std::array<XmlSourceObservation, 1> xml2Observations{{
  {2.5, 0.75, 0.125, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="font-family: Times; font-size: 16px;" variant=""><div data-mjx-hdw=".75em .125em 2.5em"><i>U</i></div></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "use", true, 1, 1, 16, 8, u""}
}};

constexpr std::array<mathml_scene_test::Node, 5> xml3Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 2, 0, 1, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 3, 1, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> xml3Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml3XmlPayloads{{
  u"<div data-mjx-hdw=\"1em .25em 3em\"><b>I</b></div>"
}};
constexpr std::array<XmlSourceObservation, 1> xml3Observations{{
  {0, 0, 0, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="font-family: Times; font-size: 16px;" variant=""><div data-mjx-hdw="1em .25em 3em"><b>I</b></div></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "ignore", false, 1, 1, 16, 8, u""}
}};

constexpr std::array<mathml_scene_test::Node, 5> xml4Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 2, 0, 1, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 3, 1, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> xml4Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml4XmlPayloads{{
  u"<div data-mjx-hdw=\".5em .2em 2em\"><span>A</span></div>"
}};
constexpr std::array<XmlSourceObservation, 1> xml4Observations{{
  {0, 0, 0, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="font-family: Times; font-size: 16px;" variant=""><div data-mjx-hdw=".5em .2em 2em"><span>A</span></div></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "ignore", true, 1, 1, 16, 8, u""}
}};

constexpr std::array<mathml_scene_test::Node, 5> xml5Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 2, 0, 1, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 3, 1, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> xml5Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml5XmlPayloads{{
  u"<div data-mjx-hdw=\".5em .2em 2em\"><span>B</span></div>"
}};
constexpr std::array<XmlSourceObservation, 1> xml5Observations{{
  {2, 0.5, 0.2, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="height: 0.7em; width: 2em; vertical-align: -0.2em;" variant=""><mjx-html-holder style="font-family: Times; font-size: 16px;"><div data-mjx-hdw=".5em .2em 2em"><span>B</span></div></mjx-html-holder></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "force", false, 1, 1, 16, 8, u""}
}};

constexpr std::array<mathml_scene_test::Node, 5> xml6Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 2, 0, 1, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 3, 1, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> xml6Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml6XmlPayloads{{
  u"<article><em>S</em></article>"
}};
constexpr std::array<XmlSourceObservation, 1> xml6Observations{{
  {0, 0, 0, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="height: 0; width: 0; vertical-align: 0;" variant=""><mjx-html-holder style="font-family: Times; font-size: 16px;"><article><em>S</em></article></mjx-html-holder></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "ignore", false, 2, 1, 16, 8, u""}
}};

constexpr std::array<mathml_scene_test::Node, 5> xml7Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 2, 0, 1, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 3, 1, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> xml7Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml7XmlPayloads{{
  u"<div data-mjx-hdw=\"1em .25em 3em\"><span>Scale</span></div>"
}};
constexpr std::array<XmlSourceObservation, 1> xml7Observations{{
  {3, 1, 0.25, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="height: 1.25em; width: 3em; vertical-align: -0.25em;" variant=""><mjx-html-holder style="font-family: Times; font-size: 16px;"><div data-mjx-hdw="1em .25em 3em"><span>Scale</span></div></mjx-html-holder></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "force", false, 2, 1, 16, 8, u""}
}};

constexpr std::array<mathml_scene_test::Node, 5> xml8Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 2, 0, 2, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 3, 2, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 2> xml8Attributes{{
  {u"style", mathml_scene_test::AttributeKind::text, u"font-family: ProbeFamily", 0, false},
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml8XmlPayloads{{
  u"<div>font</div>"
}};
constexpr std::array<XmlSourceObservation, 1> xml8Observations{{
  {0, 0, 0, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="font-family: Times; font-size: 16px;" variant=""><div>font</div></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "ignore", false, 1, 1, 16, 8, u""}
}};

constexpr std::array<mathml_scene_test::Node, 8> xml9Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"mrow", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"mi", false, 2, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"text", false, 3, 0, 0, u"x", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 2, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 5, 0, 1, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 6, 1, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> xml9Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml9XmlPayloads{{
  u"<div><br><svg xmlns=\"http://www.w3.org/2000/svg\"><text>Y</text></svg></div>"
}};
constexpr std::array<XmlSourceObservation, 1> xml9Observations{{
  {0, 0, 0, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="font-family: Times; font-size: 16px;" variant=""><div><br><svg xmlns="http://www.w3.org/2000/svg"><text>Y</text></svg></div></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "force", false, 1, 1, 16, 8, u""}
}};

constexpr std::array<mathml_scene_test::Node, 5> xml10Nodes{{
  {"math", false, std::numeric_limits<std::size_t>::max(), 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"inferredMrow", true, 0, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"semantics", false, 1, 0, 0, u"", std::numeric_limits<std::size_t>::max()},
  {"annotation-xml", false, 2, 0, 1, u"", std::numeric_limits<std::size_t>::max()},
  {"XML", false, 3, 1, 0, u"", 0}
}};
constexpr std::array<mathml_scene_test::Attribute, 1> xml10Attributes{{
  {u"encoding", mathml_scene_test::AttributeKind::text, u"application/xhtml+xml", 0, false}
}};
constexpr std::array<std::u16string_view, 1> xml10XmlPayloads{{
  u"<div></div>"
}};
constexpr std::array<XmlSourceObservation, 1> xml10Observations{{
  {0, 0, 0, 1, 1, u"Times", u"16px", R"FOLIAXML(<mjx-html style="font-family: Times; font-size: 16px;" variant=""><div></div></mjx-html>)FOLIAXML", R"FOLIAXML({"foreignObject[data-mjx-html]":{"overflow":"visible"},"mjx-measure-xml":{"position":"absolute","left":0,"top":0,"display":"inline-block","line-height":"normal","white-space":"normal"},"mjx-html":{"display":"inline-block","line-height":"normal","text-align":"initial","white-space":"initial"},"mjx-html-holder":{"display":"block","position":"absolute","top":0,"left":0,"bottom":0,"right":0}})FOLIAXML", "ignore", false, 1, 1, 16, 8, u""}
}};

constexpr XmlSourceCase sourceXmlCases[]{
  {mathml_scene_test::Case{"xml-unknown-measurement-mixed-payload",mathml_scene_test::Metrics{16,8,640,false},xml0Nodes,xml0Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: 0;" xmlns="http://www.w3.org/2000/svg" width="0.036ex" height="0.036ex" role="img" focusable="false" viewBox="0 0 16 16"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="0px" width="0px" height="0px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="font-family: Times; font-size: 16px;" variant=""><section class="xml-probe" data-order="a&amp;b"><span title="A &amp; B">Alpha &amp; Beta</span><!--inside--><br/><svg xmlns="http://www.w3.org/2000/svg"><text>G</text></svg></section></mjx-html></foreignObject></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::automatic, 1, false, "body", "html", xml0XmlPayloads, xml0Observations},
  {mathml_scene_test::Case{"xml-force-nonzero-hdw",mathml_scene_test::Metrics{16,8,640,false},xml1Nodes,xml1Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.566ex;" xmlns="http://www.w3.org/2000/svg" width="6.787ex" height="2.828ex" role="img" focusable="false" viewBox="0 -1000 3000 1250"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="-16px" width="48px" height="20px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="height: 1.25em; width: 3em; vertical-align: -0.25em;" variant=""><mjx-html-holder style="font-family: Times; font-size: 16px;"><div data-mjx-hdw="1em .25em 3em"><span>F</span></div></mjx-html-holder></mjx-html></foreignObject></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::force, 1, false, "body", "html", xml1XmlPayloads, xml1Observations},
  {mathml_scene_test::Case{"xml-use-nonzero-hdw",mathml_scene_test::Metrics{16,8,640,false},xml2Nodes,xml2Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.283ex;" xmlns="http://www.w3.org/2000/svg" width="5.656ex" height="1.98ex" role="img" focusable="false" viewBox="0 -750 2500 875"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="-12px" width="40px" height="14px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="font-family: Times; font-size: 16px;" variant=""><div data-mjx-hdw=".75em .125em 2.5em"><i>U</i></div></mjx-html></foreignObject></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::use, 1, true, "body", "html", xml2XmlPayloads, xml2Observations},
  {mathml_scene_test::Case{"xml-ignore-hdw-measures",mathml_scene_test::Metrics{16,8,640,false},xml3Nodes,xml3Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: 0;" xmlns="http://www.w3.org/2000/svg" width="0.036ex" height="0.036ex" role="img" focusable="false" viewBox="0 0 16 16"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="0px" width="0px" height="0px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="font-family: Times; font-size: 16px;" variant=""><div data-mjx-hdw="1em .25em 3em"><b>I</b></div></mjx-html></foreignObject></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::ignore, 1, false, "body", "html", xml3XmlPayloads, xml3Observations},
  {mathml_scene_test::Case{"xml-automatic-measurable-policy",mathml_scene_test::Metrics{16,8,640,false},xml4Nodes,xml4Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: 0;" xmlns="http://www.w3.org/2000/svg" width="0.036ex" height="0.036ex" role="img" focusable="false" viewBox="0 0 16 16"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="0px" width="0px" height="0px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="font-family: Times; font-size: 16px;" variant=""><div data-mjx-hdw=".5em .2em 2em"><span>A</span></div></mjx-html></foreignObject></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::automatic, 1, true, "body", "html", xml4XmlPayloads, xml4Observations},
  {mathml_scene_test::Case{"xml-automatic-force-policy",mathml_scene_test::Metrics{16,8,640,false},xml5Nodes,xml5Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.452ex;" xmlns="http://www.w3.org/2000/svg" width="4.525ex" height="1.584ex" role="img" focusable="false" viewBox="0 -500 2000 700"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="-8px" width="32px" height="11.2px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="height: 0.7em; width: 2em; vertical-align: -0.2em;" variant=""><mjx-html-holder style="font-family: Times; font-size: 16px;"><div data-mjx-hdw=".5em .2em 2em"><span>B</span></div></mjx-html-holder></mjx-html></foreignObject></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::automatic, 1, false, "body", "html", xml5XmlPayloads, xml5Observations},
  {mathml_scene_test::Case{"xml-scale-not-one",mathml_scene_test::Metrics{16,8,640,false},xml6Nodes,xml6Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak" style="font-size: 200%;"><svg style="vertical-align: 0;" xmlns="http://www.w3.org/2000/svg" width="0.036ex" height="0.036ex" role="img" focusable="false" viewBox="0 0 16 16"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="0px" width="0px" height="0px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="height: 0; width: 0; vertical-align: 0;" variant=""><mjx-html-holder style="font-family: Times; font-size: 16px;"><article><em>S</em></article></mjx-html-holder></mjx-html></foreignObject></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::ignore, 2, false, "body", "html", xml6XmlPayloads, xml6Observations},
  {mathml_scene_test::Case{"xml-scale-two-force-hdw",mathml_scene_test::Metrics{16,8,640,false},xml7Nodes,xml7Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak" style="font-size: 200%;"><svg style="vertical-align: -0.566ex;" xmlns="http://www.w3.org/2000/svg" width="6.787ex" height="2.828ex" role="img" focusable="false" viewBox="0 -1000 3000 1250"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="-16px" width="48px" height="20px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="height: 1.25em; width: 3em; vertical-align: -0.25em;" variant=""><mjx-html-holder style="font-family: Times; font-size: 16px;"><div data-mjx-hdw="1em .25em 3em"><span>Scale</span></div></mjx-html-holder></mjx-html></foreignObject></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::force, 2, false, "body", "html", xml7XmlPayloads, xml7Observations},
  {mathml_scene_test::Case{"xml-authored-font-style-removed-times-fallback",mathml_scene_test::Metrics{16,8,640,false},xml8Nodes,xml8Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: 0;" xmlns="http://www.w3.org/2000/svg" width="0.036ex" height="0.036ex" role="img" focusable="false" viewBox="0 0 16 16"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="0px" width="0px" height="0px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="font-family: Times; font-size: 16px;" variant=""><div>font</div></mjx-html></foreignObject></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::ignore, 1, false, "body", "html", xml8XmlPayloads, xml8Observations},
  {mathml_scene_test::Case{"xml-after-preceding-math",mathml_scene_test::Metrics{16,8,640,false},xml9Nodes,xml9Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: -0.025ex;" xmlns="http://www.w3.org/2000/svg" width="1.294ex" height="1.025ex" role="img" focusable="false" viewBox="0 -442 572 453"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="mrow"><g data-mml-node="mi"><path data-c="1D465" d="M527 373C527 419 482 442 432 442C389 442 355 419 329 373C308 419 273 442 222 442C173 442 133 419 101 374C74 335 60 306 60 287C60 278 65 273 75 273C84 273 90 278 92 287C111 345 153 413 220 413C253 413 269 392 269 351C269 330 251 252 216 118C199 51 169 18 126 18C112 18 99 21 88 26C114 36 127 54 127 80C127 106 114 119 87 119C54 119 29 91 29 58C29 12 76-11 125-11C167-11 201 12 228 58C247 12 283-11 335-11C383-11 423 12 455 57C482 96 496 125 496 144C496 153 491 158 481 158C472 158 467 153 464 144C447 87 402 18 337 18C304 18 287 38 287 79C287 92 292 120 303 165L337 300C356 375 387 413 431 413C445 413 458 410 469 405C442 396 429 378 429 351C429 325 443 312 470 312C502 312 527 341 527 373Z"/></g><g data-mml-node="semantics" transform="translate(572,0)"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="0px" width="0px" height="0px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="font-family: Times; font-size: 16px;" variant=""><div><br/><svg xmlns="http://www.w3.org/2000/svg"><text>Y</text></svg></div></mjx-html></foreignObject></g></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::force, 1, false, "body", "html", xml9XmlPayloads, xml9Observations},
  {mathml_scene_test::Case{"xml-empty-element",mathml_scene_test::Metrics{16,8,640,false},xml10Nodes,xml10Attributes,R"FOLIAXML(<mjx-container class="MathJax" jax="SVG" overflow="linebreak"><svg style="vertical-align: 0;" xmlns="http://www.w3.org/2000/svg" width="0.036ex" height="0.036ex" role="img" focusable="false" viewBox="0 0 16 16"><g stroke="currentColor" fill="currentColor" stroke-width="0" transform="scale(1,-1)"><g data-mml-node="math"><g data-mml-node="semantics"><g data-mml-node="annotation-xml"><foreignObject data-mjx-xml="true" y="0px" width="0px" height="0px" transform="scale(62.5) matrix(1 0 0 -1 0 0)"><mjx-html style="font-family: Times; font-size: 16px;" variant=""><div/></mjx-html></foreignObject></g></g></g></g></svg></mjx-container>)FOLIAXML"}, XmlHdwPolicy::ignore, 1, false, "body", "html", xml10XmlPayloads, xml10Observations}
};
