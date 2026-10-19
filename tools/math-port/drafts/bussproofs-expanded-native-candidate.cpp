// Separate native edge corpus. Include the locked candidate only for its
// canonical session setup, serializer, and compile/error boundary.
#define main bussproofs_locked_candidate_main
#include "../bussproofs-native-candidate.cpp"
#undef main

namespace {
constexpr Case expandedCases[] = {
    {"empty-axiom-content", uR"bsp(\begin{prooftree}\AxiomC{}\end{prooftree})bsp"},
    {"space-only-axiom-content", uR"bsp(\begin{prooftree}\AxiomC{   }\end{prooftree})bsp"},
    {"empty-unary-conclusion", uR"bsp(\begin{prooftree}\AxiomC{A}\UnaryInfC{}\end{prooftree})bsp"},
    {"space-only-unary-conclusion", uR"bsp(\begin{prooftree}\AxiomC{A}\UnaryInfC{   }\end{prooftree})bsp"},
    {"empty-labels", uR"bsp(\begin{prooftree}\AxiomC{A}\LeftLabel{}\RightLabel{}\UnaryInfC{B}\end{prooftree})bsp"},
    {"empty-label-left-only", uR"bsp(\begin{prooftree}\AxiomC{A}\LeftLabel{}\UnaryInfC{B}\end{prooftree})bsp"},
    {"solid-line-command", uR"bsp(\begin{prooftree}\AxiomC{A}\solidLine\UnaryInfC{B}\end{prooftree})bsp"},
    {"always-no-line", uR"bsp(\begin{prooftree}\alwaysNoLine\AxiomC{A}\UnaryInfC{B}\UnaryInfC{C}\end{prooftree})bsp"},
    {"always-single-line", uR"bsp(\begin{prooftree}\alwaysSingleLine\AxiomC{A}\UnaryInfC{B}\UnaryInfC{C}\end{prooftree})bsp"},
    {"always-solid-line", uR"bsp(\begin{prooftree}\alwaysSolidLine\AxiomC{A}\UnaryInfC{B}\UnaryInfC{C}\end{prooftree})bsp"},
    {"always-root-bottom", uR"bsp(\begin{prooftree}\alwaysRootAtBottom\AxiomC{A}\UnaryInfC{B}\UnaryInfC{C}\end{prooftree})bsp"},
    {"quaternary-sequent", uR"bsp(\begin{prooftree}\Axiom$A\fCenter B$\Axiom$C\fCenter D$\Axiom$E\fCenter F$\Axiom$G\fCenter H$\QuaternaryInf$I\fCenter J$\end{prooftree})bsp"},
    {"quinary-sequent", uR"bsp(\begin{prooftree}\Axiom$A\fCenter B$\Axiom$C\fCenter D$\Axiom$E\fCenter F$\Axiom$G\fCenter H$\Axiom$I\fCenter J$\QuinaryInf$K\fCenter L$\end{prooftree})bsp"},
    {"unary-insufficient-premises", uR"bsp(\begin{prooftree}\UnaryInfC{B}\end{prooftree})bsp"},
    {"ternary-insufficient-premises", uR"bsp(\begin{prooftree}\AxiomC{A}\TrinaryInfC{B}\end{prooftree})bsp"},
    {"quaternary-insufficient-premises", uR"bsp(\begin{prooftree}\AxiomC{A}\AxiomC{B}\QuaternaryInfC{C}\end{prooftree})bsp"},
    {"quinary-insufficient-premises", uR"bsp(\begin{prooftree}\AxiomC{A}\AxiomC{B}\AxiomC{C}\AxiomC{D}\QuinaryInfC{E}\end{prooftree})bsp"},
    {"left-label-outside-proof", uR"bsp(\LeftLabel{x})bsp"},
    {"inference-outside-proof", uR"bsp(\UnaryInfC{B})bsp"},
    {"root-command-outside-proof", uR"bsp(\rootAtBottom)bsp"},
    {"multiple-sequent-adjustments", uR"bsp(\begin{prooftree}\Axiom$A\fCenter B$\UnaryInf$B\fCenter C$\UnaryInf$C\fCenter D$\UnaryInf$D\fCenter E$\end{prooftree})bsp"},
};
} // namespace

int main() {
  try {
    Fixture fixture;
    Serializer serializer{&fixture.cx, &fixture.functions, &fixture.runtime};
    std::cout << "{\"schema\":1,\"source\":\"MathJax-src 4.1.3 "
                 "fb987178c2d279a99b0db5ea02e751691703955e\",\"files\":["
              << "{\"ts\":\"ts/input/tex/bussproofs/BussproofsConfiguration.ts\",\"tsSha256\":\"2aefb3c006598ccc5ef08a4f5d5e2b596aec048737f8e2aaf571cf63c2f075df\",\"cjs\":\"cjs/input/tex/bussproofs/BussproofsConfiguration.js\",\"cjsSha256\":\"249ab0f53fb4ed0c4bcabe8bc20ba39be5c65dc6ab0912acd252b202261310c9\"},"
              << "{\"ts\":\"ts/input/tex/bussproofs/BussproofsItems.ts\",\"tsSha256\":\"133a085cbeea74c316cee57432876e15c2142b5cc54c5625cb108d9a950f1a7f\",\"cjs\":\"cjs/input/tex/bussproofs/BussproofsItems.js\",\"cjsSha256\":\"e1b0a23de54c9ed334e30d1c1e679c6dcde24a2d1558ac7645abc21a041ab13a\"},"
              << "{\"ts\":\"ts/input/tex/bussproofs/BussproofsMappings.ts\",\"tsSha256\":\"76ef61358cac14baa65bfcfcaf7d862d2e068fbac89d48f2dc9d5e60a58e9c9c\",\"cjs\":\"cjs/input/tex/bussproofs/BussproofsMappings.js\",\"cjsSha256\":\"237e8be03370951f7dac389d79980454d930fcb82be5d4d821dc93e95fe8aff9\"},"
              << "{\"ts\":\"ts/input/tex/bussproofs/BussproofsMethods.ts\",\"tsSha256\":\"25e6a7c51def3f5912df78d8822b6d92b370c9cc9b472f9ffc852c357b7727e5\",\"cjs\":\"cjs/input/tex/bussproofs/BussproofsMethods.js\",\"cjsSha256\":\"c671b2b31b229e73454c70c794a82923f8d059f43b5f1af134588b530055c054\"},"
              << "{\"ts\":\"ts/input/tex/bussproofs/BussproofsUtil.ts\",\"tsSha256\":\"f6e3f1f95cd8e0961f3ddbf28c4bf1d4f3f0b3c30e488f68ae125341014992bd\",\"cjs\":\"cjs/input/tex/bussproofs/BussproofsUtil.js\",\"cjsSha256\":\"8ed15ce9672065e5d4ae2e3f06703bbb456f836287fa1fb6a0aa45ecfb1a305d\"}],"
              << "\"baseOracleSha256\":\"f253b38cb839fe38b5a38be73d54ba3dfe03d0f40d2fb93fe0d26f3c9f966a35\",\"cases\":[";
    bool first = true;
    for (const auto &test : expandedCases) {
      if (!first) std::cout << ',';
      first = false;
      std::cout << "{\"name\":" << q(test.name) << ",\"input\":"
                << q(text(test.input)) << ",\"result\":";
      try {
        std::cout << serializer.node(fixture.compile(test.input)) << ",\"error\":null";
      } catch (const tex::TexError &e) {
        std::cout << "null,\"error\":{\"name\":\"TexError\",\"id\":"
                  << q(text(e.id)) << ",\"message\":" << q(text(e.message)) << "}";
      } catch (const opts::Exception &e) {
        std::cout << "null,\"error\":{\"name\":" << q(e.name)
                  << ",\"id\":null,\"message\":" << q(text(e.message)) << "}";
      } catch (const std::exception &e) {
        std::cout << "null,\"error\":{\"name\":\"NativeException\",\"id\":null,\"message\":" << q(e.what()) << "}";
      } catch (...) {
        std::cout << "null,\"error\":{\"name\":\"UnclassifiedNativeException\",\"id\":null,\"message\":\"\"}";
      }
      std::cout << ",\"packageData\":" << serializer.packageData(*fixture.inputJax) << "}";
    }
    std::cout << "]}\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

