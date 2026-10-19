// Native port of MultlinedItem from MathtoolsItems.ts.
module folia.math.input.tex.mathtools.items;
import folia.math.input.tex.node_util;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.tex_constants;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
std::shared_ptr<tree::ChildArray<mml::MmlNode>> array(
    const std::variant<Missing, PropertyNull,
                       std::shared_ptr<tree::ChildArray<mml::MmlNode>>> &value,
    std::string_view member) {
  if (const auto *result =
          std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&value);
      result && *result)
    return *result;
  property_host::typeError(
      std::string("Cannot read properties of undefined (reading '") +
      std::string(member) + "')");
}
} // namespace
std::string MultlinedItem::kind() const { return "multlined"; }
void MultlinedItem::initializeMultlinedItem(FunctionArgs args) {
  initializeMultlineItem(std::move(args));
}
void MultlinedItem::EndTable() {
  const auto makeSpace = [this](Property width) {
    const std::array<std::any, 3> args{
        std::any(Property("mspace")),
        std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>()),
        std::any(Property(PropertyList{{"width", std::move(width)}}))};
    return create("node", args);
  };
  if (Size() || !array(row, "length")->empty()) {
    EndEntry();
    EndRow();
  }
  auto rows = array(table, "length");
  if (rows->size() > 1) {
    const auto factoryValue = sourceFactory();
    const auto *factory = std::get_if<StackItemFactoryService *>(&factoryValue);
    if (!factory || !*factory)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'configuration')");
    const auto *configuration = std::get_if<StackItemFactoryConfiguration *>(
        &(*factory)->configuration);
    if (!configuration || !*configuration)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'options')");
    auto *parseOptions = dynamic_cast<ParseOptions *>(*configuration);
    if (!parseOptions)
      property_host::typeError(
          "Mathtools MultlinedItem configuration is not ParseOptions");
    const auto *optionsObject =
        std::get_if<ObjectRef>(&parseOptions->options.data);
    if (!optionsObject)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'mathtools')");
    const Value mt = parseOptions->runtime().context().get(*optionsObject,
                                                           Key(u"mathtools"));
    auto option = [&](std::u16string_view name) {
      const auto *object = std::get_if<ObjectRef>(&mt.data);
      if (!object)
        property_host::typeError(
            "Cannot read properties of undefined (reading 'multlined-gap')");
      return parseOptions->runtime().properties.from(
          parseOptions->runtime().context().get(*object,
                                                Key(std::u16string(name))));
    };
    const Property gap = option(u"multlined-gap");
    const Property firstSkip =
        property_host::truthy(option(u"firstline-afterskip"))
            ? option(u"firstline-afterskip")
            : gap;
    const Property lastSkip = property_host::truthy(option(u"lastline-preskip"))
                                  ? option(u"lastline-preskip")
                                  : gap;
    const mml::NodeValue first = rows->get(0);
    const mml::NodeValue firstCell = NodeUtil::instance().getChildAt(first, 0);
    const Property firstAlign =
        NodeUtil::instance().getAttribute(firstCell, "columnalign");
    if (firstAlign !=
        property_host::get(property_host::get(TexConstant, "Align"), "RIGHT")) {
      auto space = makeSpace(firstSkip);
      NodeUtil::instance().appendChildren(
          firstCell, std::make_shared<tree::ChildArray<mml::MmlNode>>(
                         std::initializer_list<mml::NodeValue>{space}));
    }
    const mml::NodeValue last = rows->get(rows->size() - 1);
    const mml::NodeValue lastCell = NodeUtil::instance().getChildAt(last, 0);
    const Property align =
        NodeUtil::instance().getAttribute(lastCell, "columnalign");
    if (align !=
        property_host::get(property_host::get(TexConstant, "Align"), "LEFT")) {
      const mml::NodeValue top = NodeUtil::instance().getChildAt(lastCell, 0);
      const auto children = NodeUtil::instance().getChildren(top);
      if (!children)
        property_host::typeError(
            "Cannot read properties of undefined (reading 'unshift')");
      const std::array<mml::NodeValue, 1> none{mml::NodeValue(Missing{})};
      children->splice(0, 0, none);
      auto space = makeSpace(lastSkip);
      NodeUtil::instance().setChild(top, 0, space);
    }
  }
  MultlineItem::EndTable();
}
StackItemClassRef MultlinedItem::initializeModule(ParserRuntime &runtime) {
  const auto *module = std::get_if<ObjectRef>(&runtime.amsItemsModule.data);
  if (!module)
    throw Exception("TypeError", u"AMS items module exports are unavailable");
  const Value parentValue =
      runtime.context().get(Value(*module), Key(u"MultlineItem"));
  const auto *parent = std::get_if<ObjectRef>(&parentValue.data);
  if (!parent)
    throw Exception("TypeError", u"MultlineItem constructor is unavailable");
  return StackItemFactoryBindings::registerSubclass(
      runtime, u"MultlinedItem", "multlined", 0, typeid(MultlinedItem),
      [] { return std::make_unique<MultlinedItem>(); },
      [&runtime](BaseItem &raw, tree::FactoryArgs args) {
        auto *item = dynamic_cast<MultlinedItem *>(&raw);
        if (!item)
          throw Exception("TypeError",
                          u"MultlinedItem initializer received another class");
        item->initializeArrayItem(runtime);
        item->initializeMultlinedItem(std::move(args));
      },
      [](const BaseItem *item) {
        return dynamic_cast<const MultlinedItem *>(item) != nullptr;
      },
      [] { return BaseItem::baseErrors(); }, *parent, false,
      StackItemSourceSurface{true, {{u"EndTable", 0, false, {}}}, {}});
}
} // namespace folia::math::tex
