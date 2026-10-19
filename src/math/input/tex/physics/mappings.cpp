// Native port of MathJax 4.1.3 input/tex/physics/PhysicsMappings.ts.
module folia.math.input.tex.physics.mappings;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.physics.methods;
import folia.math.input.tex.token_map;
import folia.math.input.tex.tex_constants;
import folia.math.util.function_types;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Property callable(const PhysicsMethodSet &methods, std::u16string_view name) {
  return methods.get(name);
}
Property tuple(Property method, std::initializer_list<Property> args) {
  PropertyArray a;
  a.push(std::move(method));
  for (const auto &v : args)
    a.push(v);
  return Property(a);
}
Property character(std::u16string_view text) {
  return Property(property_host::debugText(text));
}
void command(MapHandler &registry, std::string_view name,
             PropertyList entries) {
  (void)TokenMapHost::create<CommandMap>(registry, Property(std::string(name)),
                                         Property(std::move(entries)));
}
} // namespace
void PhysicsMappings::initializeModule(ParserRuntime &runtime) {
  const auto m = PhysicsMethods::create(runtime);
  const auto macro = callable(m, u"Macro"), namedFn = callable(m, u"NamedFn");
  PropertyList automatic;
  automatic.setOwn("quantity", callable(m, u"Quantity"));
  automatic.setOwn("qty", callable(m, u"Quantity"));
  automatic.setOwn("pqty",
                   tuple(callable(m, u"Quantity"),
                         {Property("("), Property(")"), Property(true)}));
  automatic.setOwn("bqty",
                   tuple(callable(m, u"Quantity"),
                         {Property("["), Property("]"), Property(true)}));
  automatic.setOwn("vqty",
                   tuple(callable(m, u"Quantity"),
                         {Property("|"), Property("|"), Property(true)}));
  automatic.setOwn("Bqty",
                   tuple(callable(m, u"Quantity"),
                         {Property("\\{"), Property("\\}"), Property(true)}));
  automatic.setOwn("absolutevalue",
                   tuple(callable(m, u"Quantity"),
                         {Property("|"), Property("|"), Property(true)}));
  automatic.setOwn("abs",
                   tuple(callable(m, u"Quantity"),
                         {Property("|"), Property("|"), Property(true)}));
  automatic.setOwn("norm",
                   tuple(callable(m, u"Quantity"),
                         {Property("\\|"), Property("\\|"), Property(true)}));
  automatic.setOwn("evaluated", callable(m, u"Eval"));
  automatic.setOwn("eval", callable(m, u"Eval"));
  automatic.setOwn("order",
                   tuple(callable(m, u"Quantity"),
                         {Property("("), Property(")"), Property(true),
                          Property("O"), Property("-tex-calligraphic")}));
  automatic.setOwn("commutator", callable(m, u"Commutator"));
  automatic.setOwn("comm", callable(m, u"Commutator"));
  for (const auto name : {"anticommutator", "acomm", "poissonbracket", "pb"})
    automatic.setOwn(name, tuple(callable(m, u"Commutator"),
                                 {Property("\\{"), Property("\\}")}));
  command(runtime.mapHandler, "Physics-automatic-bracing-macros",
          std::move(automatic));

  auto characterParser =
      property_host::get(runtime.parseMethods, "mathchar0mo");
  PropertyList mo{
      {"dotproduct",
       PropertyArray{character(u"⋅"), Property(PropertyList{
                                          {"mathvariant", Property("bold")}})}},
      {"vdot",
       PropertyArray{character(u"⋅"), Property(PropertyList{
                                          {"mathvariant", Property("bold")}})}},
      {"crossproduct", character(u"×")},
      {"cross", character(u"×")},
      {"cp", character(u"×")},
      {"gradientnabla",
       PropertyArray{character(u"∇"), Property(PropertyList{
                                          {"mathvariant", Property("bold")}})}},
      {"divsymbol", character(u"÷")},
      {"divisionsymbol", character(u"÷")}};
  (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,
      Property("Physics-vector-mo"), functions::fromProperty(characterParser),
      Property(std::move(mo)));
  auto miParser = property_host::get(runtime.parseMethods, "mathchar0mi");
  PropertyList mi{
      {"real", PropertyArray{character(u"ℜ"),
                             Property(PropertyList{
                                 {"mathvariant", Property("normal")}})}},
      {"imaginary", PropertyArray{character(u"ℑ"),
                                  Property(PropertyList{
                                      {"mathvariant", Property("normal")}})}}};
  (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("Physics-vector-mi"),
                                           functions::fromProperty(miParser),
                                           Property(std::move(mi)));

  PropertyList vectors{{"vnabla", callable(m, u"Vnabla")},
                       {"vectorbold", callable(m, u"VectorBold")},
                       {"vb", callable(m, u"VectorBold")}};
  vectors.setOwn("vectorarrow", tuple(callable(m, u"StarMacro"),
                                      {Property(1), Property("\\vec{\\vb"),
                                       Property("{#1}}")}));
  vectors.setOwn(
      "va", tuple(callable(m, u"StarMacro"),
                  {Property(1), Property("\\vec{\\vb"), Property("{#1}}")}));
  vectors.setOwn("vectorunit", tuple(callable(m, u"StarMacro"),
                                     {Property(1), Property("\\hat{\\vb"),
                                      Property("{#1}}")}));
  vectors.setOwn(
      "vu", tuple(callable(m, u"StarMacro"),
                  {Property(1), Property("\\hat{\\vb"), Property("{#1}}")}));
  for (const auto name : {"gradient", "grad"})
    vectors.setOwn(name,
                   tuple(callable(m, u"OperatorApplication"),
                         {Property("\\vnabla"), Property("("), Property("[")}));
  for (const auto name : {"divergence", "div"})
    vectors.setOwn(name, tuple(callable(m, u"VectorOperator"),
                               {Property("\\vnabla\\vdot"), Property("("),
                                Property("[")}));
  vectors.setOwn("curl", tuple(callable(m, u"VectorOperator"),
                               {Property("\\vnabla\\crossproduct"),
                                Property("("), Property("[")}));
  vectors.setOwn("laplacian",
                 tuple(callable(m, u"OperatorApplication"),
                       {Property("\\nabla^2"), Property("("), Property("[")}));
  command(runtime.mapHandler, "Physics-vector-macros", std::move(vectors));

  PropertyList expressions;
  for (const auto name :
       {"sin", "sinh", "arcsin", "asin", "cos", "cosh", "arccos", "acos",
        "tan", "tanh", "arctan", "atan", "csc", "csch", "arccsc", "acsc",
        "sec", "sech", "arcsec", "asec", "cot", "coth", "arccot", "acot"})
    expressions.setOwn(name, callable(m, u"Expression"));
  expressions.setOwn("exp",
                     tuple(callable(m, u"Expression"), {Property(false)}));
  expressions.setOwn("log", callable(m, u"Expression"));
  expressions.setOwn("ln", callable(m, u"Expression"));
  expressions.setOwn("det",
                     tuple(callable(m, u"Expression"), {Property(false)}));
  expressions.setOwn("Pr",
                     tuple(callable(m, u"Expression"), {Property(false)}));
  expressions.setOwn("tr",
                     tuple(callable(m, u"Expression"), {Property(false)}));
  expressions.setOwn("trace", tuple(callable(m, u"Expression"),
                                    {Property(false), Property("tr")}));
  expressions.setOwn("Tr",
                     tuple(callable(m, u"Expression"), {Property(false)}));
  expressions.setOwn("Trace", tuple(callable(m, u"Expression"),
                                    {Property(false), Property("Tr")}));
  expressions.setOwn("rank", namedFn);
  expressions.setOwn("erf",
                     tuple(callable(m, u"Expression"), {Property(false)}));
  expressions.setOwn("Residue", tuple(macro, {Property("\\mathrm{Res}")}));
  expressions.setOwn("Res", tuple(callable(m, u"OperatorApplication"),
                                  {Property("\\Residue"), Property("("),
                                   Property("["), Property("{")}));
  for (const auto name : {"principalvalue", "pv"})
    expressions.setOwn(name, tuple(callable(m, u"OperatorApplication"),
                                   {Property("{\\cal P}")}));
  expressions.setOwn("PV", tuple(callable(m, u"OperatorApplication"),
                                 {Property("{\\rm P.V.}")}));
  expressions.setOwn("Re", tuple(callable(m, u"OperatorApplication"),
                                 {Property("\\mathrm{Re}"), Property("{")}));
  expressions.setOwn("Im", tuple(callable(m, u"OperatorApplication"),
                                 {Property("\\mathrm{Im}"), Property("{")}));
  constexpr std::array<std::pair<std::string_view, std::string_view>, 29>
      aliases{{{"sine", "sin"},
               {"hypsine", "sinh"},
               {"arcsine", "arcsin"},
               {"asine", "asin"},
               {"cosine", "cos"},
               {"hypcosine", "cosh"},
               {"arccosine", "arccos"},
               {"acosine", "acos"},
               {"tangent", "tan"},
               {"hyptangent", "tanh"},
               {"arctangent", "arctan"},
               {"atangent", "atan"},
               {"cosecant", "csc"},
               {"hypcosecant", "csch"},
               {"arccosecant", "arccsc"},
               {"acosecant", "acsc"},
               {"secant", "sec"},
               {"hypsecant", "sech"},
               {"arcsecant", "arcsec"},
               {"asecant", "asec"},
               {"cotangent", "cot"},
               {"hypcotangent", "coth"},
               {"arccotangent", "arccot"},
               {"acotangent", "acot"},
               {"exponential", "exp"},
               {"logarithm", "log"},
               {"naturallogarithm", "ln"},
               {"determinant", "det"},
               {"Probability", "Pr"}}};
  for (const auto &[alias, target] : aliases)
    if (!alias.empty())
      expressions.setOwn(std::string(alias),
                         tuple(namedFn, {Property(std::string(target))}));
  command(runtime.mapHandler, "Physics-expressions-macros",
          std::move(expressions));

  PropertyList qq{{"qqtext", callable(m, u"Qqtext")},
                  {"qq", callable(m, u"Qqtext")},
                  {"qcomma", tuple(macro, {Property("\\qqtext*{,}")})},
                  {"qc", tuple(macro, {Property("\\qqtext*{,}")})}};
  constexpr std::array<std::pair<std::string_view, std::string_view>, 20>
      qqAliases{{{"qcc", "c.c."},
                 {"qif", "if"},
                 {"qthen", "then"},
                 {"qelse", "else"},
                 {"qotherwise", "otherwise"},
                 {"qunless", "unless"},
                 {"qgiven", "given"},
                 {"qusing", "using"},
                 {"qassume", "assume"},
                 {"qsince", "since"},
                 {"qlet", "let"},
                 {"qfor", "for"},
                 {"qall", "all"},
                 {"qeven", "even"},
                 {"qodd", "odd"},
                 {"qinteger", "integer"},
                 {"qand", "and"},
                 {"qor", "or"},
                 {"qas", "as"},
                 {"qin", "in"}}};
  for (auto [n, t] : qqAliases)
    qq.setOwn(std::string(n),
              tuple(callable(m, u"Qqtext"), {Property(std::string(t))}));
  command(runtime.mapHandler, "Physics-quick-quad-macros", std::move(qq));

  PropertyList derivative{
      {"diffd", callable(m, u"DiffD")},
      {"flatfrac",
       tuple(macro, {Property("\\left.#1\\middle/#2\\right."), Property(2)})}};
  for (const auto name : {"differential", "dd"})
    derivative.setOwn(
        name, tuple(callable(m, u"Differential"), {Property("\\diffd")}));
  for (const auto name : {"variation", "var"})
    derivative.setOwn(
        name, tuple(callable(m, u"Differential"), {Property("\\delta")}));
  for (const auto name : {"derivative", "dv"})
    derivative.setOwn(name, tuple(callable(m, u"Derivative"),
                                  {Property(2), Property("\\diffd")}));
  for (const auto name : {"partialderivative", "pderivative", "pdv"})
    derivative.setOwn(name, tuple(callable(m, u"Derivative"),
                                  {Property(3), Property("\\partial")}));
  for (const auto name : {"functionalderivative", "fderivative", "fdv"})
    derivative.setOwn(name, tuple(callable(m, u"Derivative"),
                                  {Property(2), Property("\\delta")}));
  command(runtime.mapHandler, "Physics-derivative-macros",
          std::move(derivative));

  PropertyList braket{{"bra", callable(m, u"Bra")},
                      {"ket", callable(m, u"Ket")},
                      {"innerproduct", callable(m, u"BraKet")},
                      {"ip", callable(m, u"BraKet")},
                      {"braket", callable(m, u"BraKet")},
                      {"outerproduct", callable(m, u"KetBra")},
                      {"dyad", callable(m, u"KetBra")},
                      {"ketbra", callable(m, u"KetBra")},
                      {"op", callable(m, u"KetBra")},
                      {"expectationvalue", callable(m, u"Expectation")},
                      {"expval", callable(m, u"Expectation")},
                      {"ev", callable(m, u"Expectation")},
                      {"matrixelement", callable(m, u"MatrixElement")},
                      {"matrixel", callable(m, u"MatrixElement")},
                      {"mel", callable(m, u"MatrixElement")}};
  command(runtime.mapHandler, "Physics-bra-ket-macros", std::move(braket));

  PropertyList matrix{{"matrixquantity", callable(m, u"MatrixQuantity")},
                      {"mqty", callable(m, u"MatrixQuantity")}};
  matrix.setOwn("pmqty", tuple(macro, {Property("\\mqty(#1)"), Property(1)}));
  matrix.setOwn("Pmqty", tuple(macro, {Property("\\mqty*(#1)"), Property(1)}));
  matrix.setOwn("bmqty", tuple(macro, {Property("\\mqty[#1]"), Property(1)}));
  matrix.setOwn("vmqty", tuple(macro, {Property("\\mqty|#1|"), Property(1)}));
  for (const auto name : {"smallmatrixquantity", "smqty"})
    matrix.setOwn(name,
                  tuple(callable(m, u"MatrixQuantity"), {Property(true)}));
  matrix.setOwn("spmqty", tuple(macro, {Property("\\smqty(#1)"), Property(1)}));
  matrix.setOwn("sPmqty",
                tuple(macro, {Property("\\smqty*(#1)"), Property(1)}));
  matrix.setOwn("sbmqty", tuple(macro, {Property("\\smqty[#1]"), Property(1)}));
  matrix.setOwn("svmqty", tuple(macro, {Property("\\smqty|#1|"), Property(1)}));
  matrix.setOwn("matrixdeterminant",
                tuple(macro, {Property("\\vmqty{#1}"), Property(1)}));
  matrix.setOwn("mdet", tuple(macro, {Property("\\vmqty{#1}"), Property(1)}));
  matrix.setOwn("smdet", tuple(macro, {Property("\\svmqty{#1}"), Property(1)}));
  matrix.setOwn("identitymatrix", callable(m, u"IdentityMatrix"));
  matrix.setOwn("imat", callable(m, u"IdentityMatrix"));
  matrix.setOwn("xmatrix", callable(m, u"XMatrix"));
  matrix.setOwn("xmat", callable(m, u"XMatrix"));
  matrix.setOwn("zeromatrix",
                tuple(macro, {Property("\\xmat{0}{#1}{#2}"), Property(2)}));
  matrix.setOwn("zmat",
                tuple(macro, {Property("\\xmat{0}{#1}{#2}"), Property(2)}));
  matrix.setOwn("paulimatrix", callable(m, u"PauliMatrix"));
  matrix.setOwn("pmat", callable(m, u"PauliMatrix"));
  matrix.setOwn("diagonalmatrix", callable(m, u"DiagonalMatrix"));
  matrix.setOwn("dmat", callable(m, u"DiagonalMatrix"));
  matrix.setOwn("antidiagonalmatrix",
                tuple(callable(m, u"DiagonalMatrix"), {Property(true)}));
  matrix.setOwn("admat",
                tuple(callable(m, u"DiagonalMatrix"), {Property(true)}));
  command(runtime.mapHandler, "Physics-matrix-macros", std::move(matrix));

  auto envParser = property_host::get(runtime.parseMethods, "environment");
  PropertyList small{
      {"smallmatrix",
       PropertyArray{callable(m, u"Array"), Property(PropertyNull{}),
                     Property(PropertyNull{}), Property(PropertyNull{}),
                     Property("c"), Property("0.333em"), Property(".2em"),
                     Property("S"), Property(1)}}};
  (void)TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,Property("Physics-aux-envs"),
                                             functions::fromProperty(envParser),
                                             Property(std::move(small)));
  PropertyList chars{
      {"|", tuple(callable(m, u"AutoClose"), {Property(mml::TEXCLASS.ORD)})},
      {")", callable(m, u"AutoClose")},
      {"]", callable(m, u"AutoClose")}};
  (void)TokenMapHost::create<MacroMap>(runtime.mapHandler,Property("Physics-characters"),
                                       Property(std::move(chars)));
}
} // namespace folia::math::tex
