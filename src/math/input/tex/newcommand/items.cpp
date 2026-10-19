// MathJax 4.1.3 input/tex/newcommand/NewcommandItems.ts semantic methods.
module folia.math.input.tex.newcommand.items;
import folia.math.input.tex.parser;
import folia.math.input.tex.tex_error;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
StackItemFactoryService &service(const StackFactoryReference &factory) {
  const auto *value = std::get_if<StackItemFactoryService *>(&factory);
  if (!value || !*value)
    throw options::Exception(
        "TypeError", u"Cannot read properties of undefined (reading 'create')");
  return **value;
}
CheckType checked(StackItemRef item) {
  auto nodes = std::make_shared<std::vector<StackEntry>>();
  nodes->emplace_back(std::move(item));
  auto result = std::make_shared<CheckTuple>();
  result->items = std::move(nodes);
  result->success = true;
  return result;
}
std::u16string itemName(const BaseItem *item) {
  if (!item)
    return {};
  const StackProperty value = item->getName();
  const auto scalar = stack_property_host::scalar(value);
  return property_host::string(scalar);
}
StackItemRef mmlItem(const StackFactoryReference &factory,
                     mml::NodeValue value) {
  auto &owner = service(factory);
  const std::any argument(value);
  const tree::FactoryArgs args(&argument, 1);
  BaseItem *item = owner.itemFactory().create("mml", args);
  if (!item)
    throw options::Exception(
        "TypeError", u"Cannot read properties of null (reading 'isKind')");
  return owner.reference(item);
}
} // namespace
std::string BeginEnvItem::kind() const { return "beginEnv"; }
bool BeginEnvItem::isOpen() const { return true; }
CheckType BeginEnvItem::checkItem(BaseItem *item) {
  if (item && item->isKind("end")) {
    const std::u16string expected = itemName(this), actual = itemName(item);
    if (actual != expected) {
      const std::array<Property, 2> args{property_host::debugText(expected),
                                         property_host::debugText(actual)};
      throw TexError(u"EnvBadEnd", u"\\begin{%1} ended with \\end{%2}", args);
    }
    return checked(mmlItem(factory(), toMml()));
  }
  if (item && item->isKind("stop")) {
    const std::array<Property, 1> args{
        property_host::debugText(property_host::string(
            stack_property_host::scalar(getName())))};
    throw TexError(u"EnvMissingEnd", u"Missing \\end{%1}", args);
  }
  return BaseItem::checkItem(item);
}
void NewcommandItems::initializeModule(ParserRuntime &runtime) {
  Context &cx = runtime.context();
  if (std::holds_alternative<options::Undefined>(
          runtime.newcommandItemsModule.data))
    runtime.newcommandItemsModule = Value(cx.object());
  const auto *module =
      std::get_if<ObjectRef>(&runtime.newcommandItemsModule.data);
  if (!module)
    throw options::Exception("TypeError",
                             u"NewcommandItems module exports are unavailable");
  const Value existing = cx.get(Value(*module), Key(u"BeginEnvItem"));
  if (const auto *constructor = std::get_if<ObjectRef>(&existing.data)) {
    // Newcommand and ConfigMacros import this same Source module. Reuse its
    // own actual constructor identity and reject another class in that slot.
    const auto registered = runtime.itemClassIdentities.resolve(*constructor);
    if (registered && registered->nativeType == typeid(BeginEnvItem))
      return;
    throw options::Exception(
        "TypeError", u"BeginEnvItem export belongs to a different native class");
  }
  const auto *baseModule =
      std::get_if<ObjectRef>(&runtime.stackItemModule.data);
  if (!baseModule)
    throw options::Exception("TypeError",
                             u"StackItem module exports are unavailable");
  const Value baseConstructor = cx.get(Value(*baseModule), Key(u"BaseItem"));
  const auto *base = std::get_if<ObjectRef>(&baseConstructor.data);
  if (!base)
    throw options::Exception("TypeError",
                             u"BaseItem constructor is unavailable");
  const auto type = StackItemFactoryBindings::registerSubclass(
      runtime, u"BeginEnvItem", "beginEnv", 0, typeid(BeginEnvItem),
      [] { return std::make_unique<BeginEnvItem>(); },
      [](BaseItem &, tree::FactoryArgs) {},
      [](const BaseItem *item) {
        return dynamic_cast<const BeginEnvItem *>(item) != nullptr;
      },
      [] { return BaseItem::baseErrors(); }, *base, true);
  const auto ctor = runtime.itemClassIdentities.find(type);
  if (!ctor)
    throw options::Exception(
        "Unimplemented",
        u"BeginEnvItem constructor identity was not registered");
  cx.set(Value(*module), Key(u"BeginEnvItem"), Value(*ctor));
}
} // namespace folia::math::tex
