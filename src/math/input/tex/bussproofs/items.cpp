// Native port of MathJax 4.1.3 input/tex/bussproofs/BussproofsItems.ts.
module folia.math.input.tex.bussproofs.items;
import folia.math.input.tex.parser;
import folia.math.input.tex.stack;
import folia.math.input.tex.node_util;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
using namespace options;
namespace {
using namespace options;
void setProof(mml::NodeValue node) {
  NodeUtil::instance().setProperty(node, "bspr_proof", true);
}
CheckType complete(ProofTreeItem &item, mml::NodeValue node,
                   BaseItem *endItem) {
  const auto source = item.sourceFactory();
  const auto *service = std::get_if<StackItemFactoryService *>(&source);
  if (!service || !*service)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'create')");
  const std::any argument(node);
  BaseItem *raw =
      (*service)->itemFactory().create("mml", tree::FactoryArgs(&argument, 1));
  if (!raw)
    property_host::typeError(
        "Cannot read properties of null (reading 'isKind')");
  auto values = std::make_shared<std::vector<StackEntry>>();
  values->emplace_back((*service)->reference(raw));
  values->emplace_back((*service)->reference(endItem));
  auto tuple = std::make_shared<CheckTuple>();
  tuple->items = std::move(values);
  tuple->success = true;
  return tuple;
}
} // namespace
Stack &ProofTreeItem::innerStack(std::string_view member) {
  if (auto *owner = std::get_if<StackRef>(&innerStack_)) {
    if (*owner)
      return **owner;
    property_host::typeError("Cannot read properties of null (reading '" +
                             std::string(member) + "')");
  }
  if (const auto *raw = std::get_if<Value>(&innerStack_)) {
    if (std::holds_alternative<Undefined>(raw->data) ||
        std::holds_alternative<Null>(raw->data))
      property_host::typeError(
          std::string("Cannot read properties of ") +
          (std::holds_alternative<Undefined>(raw->data) ? "undefined" :
                                                         "null") +
          " (reading '" + std::string(member) + "')");
    property_host::typeError("this.innerStack." + std::string(member) +
                             " is not a function");
  }
  {
    const auto source = sourceFactory();
    const auto *service = std::get_if<StackItemFactoryService *>(&source);
    if (!service || !*service)
      property_host::typeError(
          "Cannot read properties of undefined (reading 'create')");
    innerStack_ = std::make_shared<Stack>(source, PropertyList{}, true);
  }
  return *std::get<StackRef>(innerStack_);
}
Value ProofTreeItem::innerStackSourceValue(ParserRuntime &runtime) {
  if (std::holds_alternative<std::monostate>(innerStack_))
    (void)innerStack();
  if (const auto *owner = std::get_if<StackRef>(&innerStack_))
    return Value(runtime.stackBindings.bind(*owner));
  return std::get<Value>(innerStack_);
}
void ProofTreeItem::setInnerStackSourceValue(ParserRuntime &runtime,
                                             Value value) {
  if (const auto *identity = std::get_if<ObjectRef>(&value.data)) {
    try {
      innerStack_ = runtime.resolveStack(*identity);
      return;
    } catch (const Exception &) {
      // Source's public field assignment is an ordinary write. Keep a foreign
      // value intact and let the later member access produce the error.
    }
  }
  innerStack_ = std::move(value);
}
std::string ProofTreeItem::kind() const { return "proofTree"; }
CheckType ProofTreeItem::checkItem(BaseItem *item) {
  if (item && item->isKind("end") &&
      property_host::string(stack_property_host::scalar(item->getName())) ==
          u"prooftree") {
    auto node = toMml();
    setProof(node);
    return complete(*this, node, item);
  }
  if (item && item->isKind("stop")) {
    const Property name = stack_property_host::scalar(getName());
    throw TexError(u"EnvMissingEnd", u"Missing \\end{%1}",
                   std::array<Property, 1>{name});
  }
  const auto source = sourceFactory();
  const auto *service = std::get_if<StackItemFactoryService *>(&source);
  if (!service || !*service)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'create')");
  const std::array<StackEntry, 1> values{
      item ? StackEntry((*service)->reference(item))
           : StackEntry(Property(Missing{}))};
  innerStack("Push").Push(values);
  return BaseItem::fail;
}
mml::NodeValue ProofTreeItem::toMml(Property inferred, Property forceRow) {
  auto tree = BaseItem::toMml(std::move(inferred), std::move(forceRow));
  const StackEntry start = innerStack("Top").Top();
  const auto *item = std::get_if<StackItemRef>(&start);
  auto *startItem =
      item && *item ? dynamic_cast<BaseItem *>(item->get()) : nullptr;
  if (startItem && startItem->isKind("start") && startItem->Size() == 0)
    return tree;
  const auto source = sourceFactory();
  const auto *service = std::get_if<StackItemFactoryService *>(&source);
  if (!service || !*service)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'create')");
  BaseItem *stop = (*service)->itemFactory().create("stop", {});
  if (!stop)
    property_host::typeError("StopItem constructor is not registered");
  const std::array<StackEntry, 1> entry{(*service)->reference(stop)};
  innerStack("Push").Push(entry);
  const StackEntry prefixEntry = innerStack("Top").Top();
  const auto *prefixItem = std::get_if<StackItemRef>(&prefixEntry);
  if (!prefixItem || !*prefixItem)
    property_host::typeError("Proof tree prefix is not a stack item");
  auto *prefixBase = dynamic_cast<BaseItem *>(prefixItem->get());
  if (!prefixBase)
    property_host::typeError("Proof tree prefix is not a BaseItem");
  const mml::NodeValue prefix = prefixBase->toMml();
  const std::array<mml::NodeValue, 2> children{prefix, tree};
  auto childArray = std::make_shared<tree::ChildArray<mml::MmlNode>>();
  for (const auto &child : children)
    childArray->push_back(child);
  const std::array<std::any, 3> args{std::any(Property("mrow")),
                                     std::any(childArray),
                                     std::any(Property(PropertyList{}))};
  return create("node", FunctionArgs(args));
}
tree::FactoryClassRef<BaseItem>
ProofTreeItem::initializeModule(ParserRuntime &runtime) {
  if (std::holds_alternative<Undefined>(runtime.stackItemModule.data))
    StackItemFactoryBindings::initializeModule(runtime);
  const auto *module = std::get_if<ObjectRef>(&runtime.stackItemModule.data);
  if (!module)
    throw Exception("TypeError", u"StackItem module exports are unavailable");
  const auto baseValue = runtime.context().get(Value(*module), Key(u"BaseItem"));
  const auto *base = std::get_if<ObjectRef>(&baseValue.data);
  if (!base)
    throw Exception("TypeError", u"BaseItem constructor is unavailable");
  return StackItemFactoryBindings::registerSubclass(
      runtime, u"ProofTreeItem", "proofTree", 0, typeid(ProofTreeItem),
      [] { return std::make_unique<ProofTreeItem>(); },
      [](BaseItem &, tree::FactoryArgs) {},
      [](const BaseItem *value) {
        return dynamic_cast<const ProofTreeItem *>(value) != nullptr;
      },
      [] { return BaseItem::baseErrors(); }, *base, true,
      StackItemSourceSurface{
          true,
          {{u"checkItem", 1}, {u"toMml", 0}},
          {{u"leftLabel", Value(Null{}),
            [](BaseItem &item) {
              return dynamic_cast<ProofTreeItem &>(item).leftLabel;
            },
            [](BaseItem &item, Value value) {
              dynamic_cast<ProofTreeItem &>(item).leftLabel = std::move(value);
            }},
           {u"rigthLabel", Value(Null{}),
            [](BaseItem &item) {
              return dynamic_cast<ProofTreeItem &>(item).rigthLabel;
            },
            [](BaseItem &item, Value value) {
              dynamic_cast<ProofTreeItem &>(item).rigthLabel = std::move(value);
            }},
           {u"innerStack", Value(Undefined{}),
            [&runtime](BaseItem &item) {
              return dynamic_cast<ProofTreeItem &>(item).innerStackSourceValue(runtime);
            },
            [&runtime](BaseItem &item, Value value) {
              dynamic_cast<ProofTreeItem &>(item).setInnerStackSourceValue(runtime,std::move(value));
            },
            [&runtime](BaseItem &item) -> Value {
              auto stack=std::make_shared<Stack>(
                  item.sourceFactory(),PropertyList{},true);
              return Value(runtime.bindStack(stack));
            }}}});
}
tree::FactoryClassRef<BaseItem>
BussproofsItems::initializeModule(ParserRuntime &runtime) {
  return ProofTreeItem::initializeModule(runtime);
}
} // namespace folia::math::tex
