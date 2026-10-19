// MathJax 4.1.3 input/tex/ams/AmsItems.ts semantic methods.
module folia.math.input.tex.ams.items;
import folia.math.input.tex.parser;
import folia.math.input.tex.node_util;
import folia.math.input.tex.tex_error;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
StackItemFactoryService &service(const StackFactoryReference &factory) {
  const auto *value = std::get_if<StackItemFactoryService *>(&factory);
  if (!value || !*value)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'configuration')");
  return **value;
}
ParseOptions &parseOptions(const StackFactoryReference &factory) {
  auto &owner = service(factory);
  const auto *c =
      std::get_if<StackItemFactoryConfiguration *>(&owner.configuration);
  if (!c || !*c)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'tags')");
  auto *p = dynamic_cast<ParseOptions *>(*c);
  if (!p)
    property_host::typeError(
        "Stack item factory configuration is not ParseOptions");
  return *p;
}
Tags &tags(const StackFactoryReference &factory) {
  auto &po = parseOptions(factory);
  const auto *value = std::get_if<std::shared_ptr<Tags>>(&po.tags);
  if (!value || !*value)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'start')");
  return **value;
}
template <class T>
std::shared_ptr<T> shared(const ParserShared<T> &value,
                          std::string_view member) {
  if (const auto *p = std::get_if<std::shared_ptr<T>>(&value); p && *p)
    return *p;
  property_host::typeError(
      std::string("Cannot read properties of null or undefined (reading '") +
      std::string(member) + "')");
}
Property property(const FunctionArgs &args, std::size_t i) {
  if (i >= args.size())
    return Missing{};
  if (const auto *p = std::any_cast<Property>(&args[i]))
    return *p;
  if (std::any_cast<Value>(&args[i]))
    return Missing{};
  if (const auto *s = std::any_cast<std::u16string>(&args[i]))
    return property_host::debugText(*s);
  if (const auto *d = std::any_cast<double>(&args[i]))
    return *d;
  if (const auto *b = std::any_cast<bool>(&args[i]))
    return *b;
  return Missing{};
}
void eraseProperty(const Property &receiver, std::string_view key) {
  auto object = PropertyList::record(receiver);
  if (!object)
    property_host::typeError("Cannot convert undefined or null to object");
  object->erase(key);
}
Value value(ParserRuntime &runtime, const FunctionArgs &args, std::size_t i) {
  if (i >= args.size())
    return Undefined{};
  if (const auto *v = std::any_cast<Value>(&args[i]))
    return *v;
  const Property p = property(args, i);
  if (p.missing())
    return Undefined{};
  return runtime.properties.to(p);
}
mml::NodeValue node(BaseItem &item, std::string_view kind,
                    mml::Children children, Property attrs = PropertyList{}) {
  const std::array<std::any, 3> args{std::any(Property(std::string(kind))),
                                     std::any(std::move(children)),
                                     std::any(std::move(attrs))};
  return item.create("node", args);
}
void setShove(BaseItem &item) {
  item.setProperty("shove", Property(PropertyNull{}));
}
} // namespace
void MultlineItem::initializeMultlineItem(FunctionArgs args) {
  auto &rt = runtime();
  tags(factory()).start(Value(u"multline"), Value(true), value(rt, args, 0));
}
std::string MultlineItem::kind() const { return "multline"; }
void MultlineItem::EndEntry() {
  auto tableValue = shared(table, "length");
  auto rowValue = shared(row, "push");
  if (!tableValue->empty())
    ParseUtil::fixInitialMO(parseOptions(factory()), *this);
  const StackProperty shoveValue = getProperty("shove");
  const Property shove = stack_property_host::scalar(shoveValue);
  Property attrs = property_host::truthy(shove)
                       ? Property(PropertyList{{"columnalign", shove}})
                       : Property(PropertyList{});
  mml::NodeValue cell;
  if(hasSourceNodesOwner()){
    auto& access=sourceAccess();const auto create=access.getReceiver(u"create");const auto children=sourceNodesPropertyValue();
    const std::array<options::Value,4> args{options::Value(u"node"),options::Value(u"mtd"),children,access.encodeProperty(StackProperty(std::move(attrs)))};
    cell=access.decodeNode(access.callMember(create,access.receiver(),args));
  }else cell=node(*this,"mtd",nodes(),std::move(attrs));
  rowValue->push_back(cell);
  setShove(*this);
  Clear();
}
void MultlineItem::EndRow() {
  auto current = shared(row, "length");
  if (current->size() != 1)
    throw TexError(
        u"MultlineRowsOneCol",
        u"The rows within the %1 environment must have exactly one column",
        std::array<Property, 1>{property_host::debugText(u"multline")});
  shared(table, "push")->push_back(node(*this, "mtr", current));
  row = std::make_shared<tree::ChildArray<mml::MmlNode>>();
}
void MultlineItem::EndTable() {
  ArrayItem::EndTable();
  auto values = shared(table, "length");
  if (!values->empty()) {
    const auto first = NodeUtil::instance().getChildAt(values->get(0), 0);
    if (!property_host::truthy(
            NodeUtil::instance().getAttribute(first, "columnalign")))
      NodeUtil::instance().setAttribute(first, "columnalign", Property("left"));
    const auto last =
        NodeUtil::instance().getChildAt(values->get(values->size() - 1), 0);
    if (!property_host::truthy(
            NodeUtil::instance().getAttribute(last, "columnalign")))
      NodeUtil::instance().setAttribute(last, "columnalign", Property("right"));
    const mml::NodeValue tag = tags(factory()).getTag();
    if (tag.isNode()) {
      const auto side = runtime().context().get(arraydef, Key(u"side"));
      const std::size_t label = side == Value(u"left") ? 0 : values->size() - 1;
      const mml::NodeValue rowNode = values->get(label);
      auto children = NodeUtil::instance().getChildren(rowNode);
      auto list = std::make_shared<tree::ChildArray<mml::MmlNode>>();
      list->push_back(tag);
      if (children)
        for (std::size_t i = 0; i < children->size(); ++i)
          if (children->has(i))
            list->push_back(children->get(i));
      const mml::NodeValue labeled = node(*this, "mlabeledtr", list);
      NodeUtil::instance().copyAttributes(rowNode, labeled);
      values->set(label, labeled);
    }
  }
  tags(factory()).end();
}
void FlalignItem::initializeFlalignItem(FunctionArgs args) {
  auto &rt = runtime();
  name = property_host::string(property(args, 0));
  numbered = property_host::truthy(property(args, 1));
  padded = property_host::truthy(property(args, 2));
  center = property_host::truthy(property(args, 3));
  tags(factory()).start(rt.properties.to(property_host::debugText(name)),
                        rt.properties.to(Property(numbered)),
                        rt.properties.to(Property(numbered)));
}
std::string FlalignItem::kind() const { return "flalign"; }
void FlalignItem::EndEntry() {
  EqnArrayItem::EndEntry();
  const double n = property_host::number(
      stack_property_host::scalar(getProperty("xalignat")));
  if (!n)
    return;
  const auto cells = shared(row, "length");
  if (cells->size() > n)
    throw TexError(u"XalignOverflow", u"Extra %1 in row of %2",
                   std::array<Property, 2>{property_host::debugText(u"&"),
                                           property_host::debugText(name)});
}
void FlalignItem::EndRow() {
  auto rowValue = shared(row, "length");
  const double n = property_host::number(
      stack_property_host::scalar(getProperty("xalignat")));
  while (rowValue->size() < n)
    rowValue->push_back(
        node(*this, "mtd", std::make_shared<tree::ChildArray<mml::MmlNode>>()));
  const auto original = rowValue;
  auto newRow = std::make_shared<tree::ChildArray<mml::MmlNode>>();
  if (padded)
    newRow->push_back(
        node(*this, "mtd", std::make_shared<tree::ChildArray<mml::MmlNode>>()));
  while (!original->empty()) {
    const auto first = original->get(0);
    original->splice(0, 1);
    if (!first.isNode())
      break;
    newRow->push_back(first);
    if (!original->empty()) {
      const auto second = original->get(0);
      original->splice(0, 1);
      if (second.isNode())
        newRow->push_back(second);
    }
    if (!original->empty() || padded)
      newRow->push_back(node(
          *this, "mtd", std::make_shared<tree::ChildArray<mml::MmlNode>>()));
  }
  row = newRow;
  const auto max = property_host::number(maxrow);
  if (static_cast<double>(newRow->size()) > max)
    maxrow = static_cast<double>(newRow->size());
  EqnArrayItem::EndRow();
  auto tableValue = shared(table, "length");
  const mml::NodeValue mtr = tableValue->size()
                                 ? tableValue->get(tableValue->size() - 1)
                                 : mml::NodeValue(Missing{});
  if (property_host::truthy(
          stack_property_host::scalar(getProperty("zeroWidthLabel"))) &&
      mtr.requireNode("isKind")->isKind("mlabeledtr")) {
    const auto labelCell = NodeUtil::instance().getChildAt(mtr, 0);
    const auto side = runtime().context().get(parseOptions(factory()).options,
                                              Key(u"tagSide"));
    PropertyList def{{"width", 0}};
    if (side == Value(u"right"))
      def.setOwn("lspace", Property("-1width"));
    const auto content = NodeUtil::instance().getChildren(labelCell);
    const auto mpadded = node(*this, "mpadded", content, Property(def));
    labelCell.requireNode("setChildren")
        ->setChildren(std::make_shared<tree::ChildArray<mml::MmlNode>>(
            std::initializer_list<mml::NodeValue>{mpadded}));
  }
}
void FlalignItem::EndTable() {
  EqnArrayItem::EndTable();
  if (center && property_host::number(maxrow) <= 2) {
    if (const auto *def = std::get_if<ObjectRef>(&arraydef.data))
      runtime().context().erase(*def, Key(u"width"));
    eraseProperty(global, "indentalign");
  }
}
void AmsItems::initializeModule(ParserRuntime &runtime) {
  Context &cx = runtime.context();
  if (std::holds_alternative<Undefined>(runtime.amsItemsModule.data))
    runtime.amsItemsModule = Value(cx.object());
  const auto *module = std::get_if<ObjectRef>(&runtime.amsItemsModule.data);
  if (!module)
    throw Exception("TypeError", u"AmsItems module exports are unavailable");
  const auto *baseModule = std::get_if<ObjectRef>(&runtime.baseItemsModule.data);
  if (!baseModule)
    throw Exception("TypeError", u"BaseItems module exports are unavailable");
  const auto parent = [&](std::u16string_view name) {
    const Value value = cx.get(Value(*baseModule), Key(std::u16string(name)));
    const auto *ctor = std::get_if<ObjectRef>(&value.data);
    if (!ctor)
      throw Exception("TypeError",
                      u"Base stack item constructor is unavailable");
    return *ctor;
  };
  const auto add = [&](std::u16string_view name, StackItemClassRef type) {
    const auto ctor = runtime.itemClassIdentities.find(type);
    if (!ctor)
      throw Exception(
          "Unimplemented",
          u"AMS stack item constructor identity was not registered");
    cx.set(Value(*module), Key(std::u16string(name)), Value(*ctor));
  };
  const auto existing = [&](std::u16string_view name,
                            std::type_index nativeType)
      -> std::optional<StackItemClassRef> {
    const Value value = cx.get(Value(*module), Key(std::u16string(name)));
    const auto *constructor = std::get_if<ObjectRef>(&value.data);
    if (!constructor) return std::nullopt;
    const auto type = runtime.itemClassIdentities.resolve(*constructor);
    if (!type || type->nativeType != nativeType)
      throw Exception("TypeError",
                      u"AMS stack item export belongs to a different native class");
    return type;
  };
  auto multline = existing(u"MultlineItem", typeid(MultlineItem));
  if (!multline) {
    multline = StackItemFactoryBindings::registerSubclass(
        runtime, u"MultlineItem", "multline", 1, typeid(MultlineItem),
        [] { return std::make_unique<MultlineItem>(); },
        [&runtime](BaseItem &raw, tree::FactoryArgs args) {
          auto *item = dynamic_cast<MultlineItem *>(&raw);
          if (!item)
            throw Exception("TypeError",
                            u"MultlineItem initializer received another class");
          item->initializeArrayItem(runtime);
          item->initializeMultlineItem(args);
        },
        [](const BaseItem *item) {
          return dynamic_cast<const MultlineItem *>(item) != nullptr;
        },
        [] { return BaseItem::baseErrors(); }, parent(u"ArrayItem"), false);
    add(u"MultlineItem", *multline);
  }
  auto flalign = existing(u"FlalignItem", typeid(FlalignItem));
  if (!flalign) {
    flalign = StackItemFactoryBindings::registerSubclass(
        runtime, u"FlalignItem", "flalign", 5, typeid(FlalignItem),
        [] { return std::make_unique<FlalignItem>(); },
        [&runtime](BaseItem &raw, tree::FactoryArgs args) {
          auto *item = dynamic_cast<FlalignItem *>(&raw);
          if (!item)
            throw Exception("TypeError",
                            u"FlalignItem initializer received another class");
          item->initializeArrayItem(runtime);
          item->initializeEqnArrayItem({});
          item->initializeFlalignItem(args);
        },
        [](const BaseItem *item) {
          return dynamic_cast<const FlalignItem *>(item) != nullptr;
        },
        [] { return BaseItem::baseErrors(); }, parent(u"EqnArrayItem"), false);
    add(u"FlalignItem", *flalign);
  }
}
} // namespace folia::math::tex
