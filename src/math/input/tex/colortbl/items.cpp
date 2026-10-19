// Native implementation of ColorArrayItem from ColortblConfiguration.ts.
module folia.math.input.tex.colortbl.items;
import folia.math.input.tex.parser;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
std::string ColorArrayItem::kind() const { return "array"; }
void ColorArrayItem::EndEntry() {
  ArrayItem::EndEntry();
  const auto *rowValue =
      std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&row);
  if (!rowValue || !*rowValue || (*rowValue)->empty())
    return;
  const std::size_t index = (*rowValue)->size() - 1;
  const std::u16string *colorValue = nullptr;
  if (!color.cell.empty())
    colorValue = &color.cell;
  else if (!color.row.empty())
    colorValue = &color.row;
  else if (index < color.col.size() && color.col[index] &&
           !color.col[index]->empty())
    colorValue = &*color.col[index];
  if (colorValue) {
    auto cell = (*rowValue)->get(index);
    auto *node = cell.requireNode("attributes");
    auto attributes = node->attributes();
    if (!attributes)
      throw options::Exception(
          "TypeError", u"Cannot read properties of null (reading 'set')");
    attributes->set("mathbackground", property_host::debugText(*colorValue));
    color.cell.clear();
    hasColor = true;
  }
}
void ColorArrayItem::EndRow() {
  ArrayItem::EndRow();
  color.row.clear();
}
mml::NodeValue ColorArrayItem::createMml() {
  mml::NodeValue result = ArrayItem::createMml();
  mml::NodeValue table = result;
  auto *root = table.requireNode("isKind");
  if (root->isKind("mrow")) {
    const auto children = root->childNodes;
    if (!children)
      throw options::Exception("TypeError",
                               u"Cannot read properties of null (reading '1')");
    table = children->get(1);
  }
  auto *node = table.requireNode("isKind");
  if (node->isKind("mstyle")) {
    auto outer = node->childNodes;
    if (!outer)
      throw options::Exception("TypeError",
                               u"Cannot read properties of null (reading '0')");
    auto first = outer->get(0);
    auto *firstNode = first.requireNode("childNodes");
    auto inner = firstNode->childNodes;
    if (!inner)
      throw options::Exception("TypeError",
                               u"Cannot read properties of null (reading '0')");
    table = inner->get(0);
    node = table.requireNode("isKind");
  }
  if (node->isKind("menclose")) {
    auto outer = node->childNodes;
    if (!outer)
      throw options::Exception("TypeError",
                               u"Cannot read properties of null (reading '0')");
    auto first = outer->get(0);
    auto *firstNode = first.requireNode("childNodes");
    auto inner = firstNode->childNodes;
    if (!inner)
      throw options::Exception("TypeError",
                               u"Cannot read properties of null (reading '0')");
    table = inner->get(0);
    node = table.requireNode("attributes");
  }
  if (hasColor) {
    auto attributes = node->attributes();
    if (!attributes)
      throw options::Exception(
          "TypeError", u"Cannot read properties of null (reading 'get')");
    const Property frame = attributes->get("frame"),
                   styles = attributes->get("data-frame-styles");
    if (frame == Property("none") &&
        std::holds_alternative<Missing>(styles.value))
      attributes->set("data-frame-styles", Property(""));
  }
  return result;
}
tree::FactoryClassRef<BaseItem>
ColorArrayItem::initializeModule(ParserRuntime &runtime) {
  const auto *module =
      std::get_if<options::ObjectRef>(&runtime.baseItemsModule.data);
  if (!module)
    throw options::Exception("TypeError",
                             u"BaseItems module exports are unavailable");
  const auto baseValue = runtime.context().get(options::Value(*module),
                                               options::Key(u"ArrayItem"));
  const auto *base = std::get_if<options::ObjectRef>(&baseValue.data);
  if (!base)
    throw options::Exception("TypeError",
                             u"ArrayItem constructor is unavailable");
  return StackItemFactoryBindings::registerSubclass(
      runtime, u"ColorArrayItem", "array", 0, typeid(ColorArrayItem),
      [] { return std::make_unique<ColorArrayItem>(); },
      [&runtime](BaseItem &raw, tree::FactoryArgs) {
        auto *item = dynamic_cast<ColorArrayItem *>(&raw);
        if (!item)
          throw options::Exception(
              "TypeError",
              u"ColorArrayItem initializer received another class");
        item->initializeArrayItem(runtime);
      },
      [](const BaseItem *item) {
        return dynamic_cast<const ColorArrayItem *>(item) != nullptr;
      },
      [] { return BaseItem::baseErrors(); }, *base, true,
      StackItemSourceSurface{
          false, {{u"EndEntry", 0}, {u"EndRow", 0}, {u"createMml", 0}}});
}
} // namespace folia::math::tex
