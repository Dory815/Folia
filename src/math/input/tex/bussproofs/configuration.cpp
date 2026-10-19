// Native port of MathJax 4.1.3 input/tex/bussproofs/BussproofsConfiguration.ts.
module folia.math.input.tex.bussproofs.configuration;
import folia.math.input.tex.bussproofs.items;
import folia.math.input.tex.bussproofs.mappings;
import folia.math.input.tex.bussproofs.methods;
import folia.math.input.tex.bussproofs.util;
import folia.math.input.tex.handler_types;
import folia.math.core.input_jax;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Value object(
    Context &cx,
    std::initializer_list<std::pair<std::u16string_view, Value>> fields) {
  Value result(cx.object());
  for (const auto &[name, value] : fields)
    cx.set(result, Key(std::u16string(name)), value);
  return result;
}
Value array(Context &cx, std::initializer_list<Value> values) {
  Value result(cx.array(static_cast<std::uint32_t>(values.size())));
  std::size_t index = 0;
  for (const auto &value : values) {
    const auto digits = std::to_string(index++);
    cx.set(result, Key(std::u16string(digits.begin(), digits.end())), value);
  }
  return result;
}
Value processor(Context &cx, Value fn, double priority) {
  auto tuple = cx.array(2);
  cx.set(Value(tuple), Key(u"0"), std::move(fn));
  cx.set(Value(tuple), Key(u"1"), Value(priority));
  return Value(tuple);
}
} // namespace
ConfigurationRef
BussproofsConfiguration::initializeModule(ParserRuntime &runtime,
                                           util::RuntimeModules &modules) {
  const auto loaded = runtime.configurationHandler.get(u"bussproofs");
  if (const auto *existing = std::get_if<ConfigurationRef>(&loaded);
      existing && *existing)
    return *existing;
  auto &cx = runtime.context();
  const auto constructor = ProofTreeItem::initializeModule(runtime);
  const auto identity = runtime.itemClassIdentities.find(constructor);
  if (!identity)
    throw Exception("Unimplemented",
                    u"ProofTreeItem constructor identity was not registered");
  BussproofsMappings::initializeModule(runtime);
  const auto utilModule = bussproofs_util::BussproofsUtilModule::initializeModule(runtime, modules);
  const Value save = utilModule->function(u"saveDocument");
  const Value clear = utilModule->function(u"clearDocument");
  const Value attributes = utilModule->function(u"makeBsprAttributes");
  const Value balance = utilModule->function(u"balanceRules");
  const auto handlers = object(
      cx, {{u"macro", array(cx, {Value(u"Bussproofs-macros")})},
           {u"environment", array(cx, {Value(u"Bussproofs-environments")})}});
  const auto items = object(cx, {{u"proofTree", Value(*identity)}});
  const auto preprocessors = array(cx, {processor(cx, save, 1)});
  const auto postprocessors =
      array(cx, {processor(cx, clear, 3), processor(cx, attributes, 2),
                 processor(cx, balance, 1)});
  return Configuration::create(
      runtime, u"bussproofs",
      object(cx, {{u"handler", handlers},
                  {u"items", items},
                  {u"preprocessors", preprocessors},
                  {u"postprocessors", postprocessors}}));
}
} // namespace folia::math::tex
