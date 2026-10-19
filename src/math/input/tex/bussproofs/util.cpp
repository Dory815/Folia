// Native port of MathJax 4.1.3 input/tex/bussproofs/BussproofsUtil.ts.
module folia.math.input.tex.bussproofs.util;
import folia.math.core.math_document;
import folia.math.input.tex.node_util;
import folia.math.input.tex.unit_util;
import folia.math.util.lengths;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;
namespace folia::math::tex::bussproofs_util {
using namespace options;
struct GeometryState {
  ParserRuntime *runtime;
  Context *context;
  std::optional<ObjectRef> document;
  std::optional<ObjectRef> item;
  explicit GeometryState(ParserRuntime &owner)
      : runtime(&owner), context(&owner.context()) {}
  Context &contextOwner() const { return *context; }
};
namespace {
using namespace options;
constexpr std::string_view prefix = "bspr_";
mml::NodeValue child(mml::NodeValue node, std::size_t index) {
  const auto children = NodeUtil::instance().getChildren(node);
  if (!children || index >= children->size())
    return PropertyNull{};
  return children->get(index);
}
mml::NodeValue parent(mml::NodeValue node) {
  return node.requireNode("Parent")->Parent();
}
Property nodeProperty(mml::NodeValue node, std::string_view name) {
  return NodeUtil::instance().getProperty(node, name);
}
std::size_t indexOf(mml::NodeValue node) {
  auto *raw = node.requireNode("childPosition");
  const auto position = raw->childPosition();
  return position.value_or(std::numeric_limits<std::size_t>::max());
}
double bbox(GeometryState &state, mml::NodeValue node) {
  if (!state.document || !state.item)
    property_host::typeError(
        "Cannot read properties of null (reading 'outputJax')");
  const auto identity =
      state.runtime->mmlNodeIdentities.find(node.requireNode());
  if (!identity)
    throw Exception("Unimplemented",
                    u"Bussproofs MML node has no canonical Context identity");
  state.context->set(Value(*state.item), Key(u"root"), Value(*identity));
  const ObjectRef document = *state.document;
  const Value output = state.context->get(Value(document), Key(u"outputJax"));
  const auto *outputIdentity = std::get_if<ObjectRef>(&output.data);
  if (!outputIdentity)
    property_host::typeError(
        "Cannot read properties of undefined (reading 'getBBox')");
  const Value method = state.context->get(output, Key(u"getBBox"));
  const auto *callable = std::get_if<ObjectRef>(&method.data);
  if (!callable || state.context->kind(*callable) != ObjectKind::function)
    property_host::typeError("doc.outputJax.getBBox is not a function");
  const std::array<Value, 2> arguments{Value(*state.item), Value(document)};
  const Value box = state.context->call(*callable, output, arguments);
  return state.context->toNumber(state.context->get(box, Key(u"w")));
}
mml::NodeValue ruleNode(mml::NodeValue node) {
  std::size_t i = 0;
  while (node.isNode() && !NodeUtil::instance().isType(node, "mtable")) {
    if (NodeUtil::instance().isType(node, "text"))
      return PropertyNull{};
    if (NodeUtil::instance().isType(node, "mrow")) {
      node = child(node, 0);
      i = 0;
      continue;
    }
    node = child(parent(node), i++);
  }
  return node;
}
mml::NodeValue premises(mml::NodeValue rule, std::u16string_view direction) {
  return child(
      child(child(child(child(rule, direction == u"up" ? 1 : 0), 0), 0), 0), 0);
}
mml::NodeValue premise(mml::NodeValue nodes, std::size_t n) {
  return child(child(child(nodes, n), 0), 0);
}
mml::NodeValue conclusion(mml::NodeValue rule, std::u16string_view direction) {
  return child(child(child(child(rule, direction == u"up" ? 0 : 1), 0), 0), 0);
}
mml::NodeValue column(mml::NodeValue node) {
  while (node.isNode() && !NodeUtil::instance().isType(node, "mtd"))
    node = parent(node);
  return node;
}
mml::NodeValue nextSibling(mml::NodeValue node) {
  const auto p = parent(node);
  return child(p, indexOf(node) + 1);
}
mml::NodeValue parentInference(mml::NodeValue node) {
  while (node.isNode() &&
         property_host::nullish(getProperty(node, "inference")))
    node = parent(node);
  return node;
}
double spaces(GeometryState &state, mml::NodeValue inf, mml::NodeValue rule,
              bool right = false) {
  double result = 0;
  if (inf == rule)
    return 0;
  if (inf != parent(rule)) {
    const Property margin =
        nodeProperty(inf, right ? "proof-right" : "proof-left");
    result += property_host::truthy(margin) ? property_host::number(margin) : 0;
    inf = parent(rule);
  }
  if (inf == rule)
    return result;
  const auto children = NodeUtil::instance().getChildren(inf);
  if (!children || children->empty())
    return result;
  const auto edge = children->get(right ? children->size() - 1 : 0);
  if (edge != rule)
    result += bbox(state, edge);
  return result;
}
double adjustValue(GeometryState &state, mml::NodeValue inf,
                   bool right = false) {
  const auto rule = ruleNode(inf);
  const auto direction =
      property_host::string(getProperty(rule, "inferenceRule"));
  const auto conc = conclusion(rule, direction);
  return spaces(state, inf, rule, right) +
         (bbox(state, rule) - bbox(state, conc)) / 2;
}
void moveProperties(mml::NodeValue source, mml::NodeValue target) {
  for (const auto name : {"inference", "proof", "labelledRule"}) {
    const auto value = getProperty(source, name);
    if (!property_host::nullish(value)) {
      setProperty(target, name, value);
      removeProperty(source, name);
    }
  }
}
mml::NodeValue createNode(ParseOptions &config, std::string_view kind,
                          mml::Children children,
                          Property attrs = PropertyList{}) {
  const std::array<std::any, 3> args{std::any(Property(std::string(kind))),
                                     std::any(std::move(children)),
                                     std::any(std::move(attrs))};
  return config.nodeFactoryOwner().create("node", args);
}
void addSpace(GeometryState &state, ParseOptions &config, mml::NodeValue inf,
              double amount, bool right = false) {
  if (amount == 0)
    return;
  if (property_host::truthy(getProperty(inf, "inferenceRule")) ||
      property_host::truthy(getProperty(inf, "labelledRule"))) {
    auto row = createNode(config, "mrow",
                          std::make_shared<tree::ChildArray<mml::MmlNode>>());
    const auto p = parent(inf);
    p.requireNode("replaceChild")->replaceChild(row, inf);
    row.requireNode("appendChild")->appendChild(inf);
    moveProperties(inf, row);
    inf = row;
  }
  const std::string property = right ? "proof-right" : "proof-left";
  const Property previous = nodeProperty(inf, property);
  const double total =
      (property_host::truthy(previous) ? property_host::number(previous) : 0) +
      amount;
  NodeUtil::instance().setProperty(inf, property, Property(total));
  std::string style;
  for (const auto side : {"left", "right"}) {
    const Property margin = nodeProperty(inf, std::string("proof-") + side);
    if (property_host::truthy(margin)) {
      if (!style.empty())
        style += "; ";
      style += std::string("margin-") + side + ": " +
               UnitUtil::instance().em(property_host::number(margin));
    }
  }
  NodeUtil::instance().setAttribute(
      inf, "style",
      Property(property_host::debugText(
          std::u16string(style.begin(), style.end()))));
}
} // namespace
void setProperty(mml::NodeValue node, std::string_view name, Property value) {
  NodeUtil::instance().setProperty(
      node, std::string(prefix) + std::string(name), std::move(value));
}
Property getProperty(mml::NodeValue node, std::string_view name) {
  return NodeUtil::instance().getProperty(node, std::string(prefix) +
                                                    std::string(name));
}
void removeProperty(mml::NodeValue node, std::string_view name) {
  node.requireNode("removeProperty")
      ->removeProperty(
          std::array<std::string, 1>{std::string(prefix) + std::string(name)});
}
void saveDocument(GeometryState &state, const FilterData &data) {
  const auto *document =
      std::get_if<core::MathDocumentService *>(&data.document);
  if (!document || !*document)
    property_host::typeError(
        "Cannot read properties of null (reading 'outputJax')");
  if (!data.data)
    throw Exception("TypeError", u"Bussproofs filter requires parser runtime");
  if (&data.data->runtime() != state.runtime)
    throw Exception("TypeError", u"Bussproofs module belongs to a different ParserRuntime");
  const auto identity = (*document)->sourceIdentity();
  if (&(*document)->sourceContext() != state.context)
    throw Exception("TypeError",
                    u"Bussproofs document and parser use different Contexts");
  state.document = identity;
  const Value output = state.context->get(Value(identity), Key(u"outputJax"));
  if (!state.context->has(output, Key(u"getBBox")))
    throw Exception("Error", u"The bussproofs extension requires an output jax "
                             u"with a getBBox() method");
}
void clearDocument(GeometryState &state, const FilterData &) {
  state.document.reset();
}
void makeBsprAttributes(GeometryState &, const FilterData &data) {
  if (!data.data)
    return;
  const auto root = data.data->root;
  if (!root.isNode())
    return;
  root.requireNode("walkTree")
      ->walkTree([&](mml::MmlNode *node, const std::any &) -> std::any {
        std::vector<std::string> attributes;
        for (const auto &name : node->getPropertyNames())
          if (name.starts_with(prefix)) {
            const auto value = node->getProperty(name);
            attributes.push_back(
                name + ":" +
                property_host::debugText(property_host::string(value)));
          }
        if (!attributes.empty()) {
          std::string joined;
          for (const auto &value : attributes) {
            if (!joined.empty())
              joined.push_back(';');
            joined += value;
          }
          NodeUtil::instance().setAttribute(
              mml::NodeValue(node), "semantics",
              Property(property_host::debugText(
                  std::u16string(joined.begin(), joined.end()))));
        }
        return std::any{};
      });
}
void balanceRules(GeometryState &state, const FilterData &data) {
  if (!data.data)
    return;
  ParseOptions &config = *data.data;
  if (&config.runtime() != state.runtime)
    throw Exception("TypeError", u"Bussproofs module belongs to a different ParserRuntime");
  const auto *document =
      std::get_if<core::MathDocumentService *>(&data.document);
  if (!document || !*document)
    property_host::typeError(
        "Cannot read properties of null (reading 'options')");
  Context &cx = *state.context;
  // Source constructs its scratch MathItem from arg.document on every call,
  // while getBBox later consults the module-scoped document set by
  // saveDocument. Keep those two identities independent: another Jax can
  // clear the latter without changing the former access order.
  const Value callDocument((*document)->sourceIdentity());
  const Value options = cx.get(callDocument, Key(u"options"));
  const Value constructor = cx.get(options, Key(u"MathItem"));
  const auto *math = std::get_if<core::MathItem *>(&data.math);
  if (!math || !*math)
    property_host::typeError(
        "Cannot read properties of null (reading 'display')");
  const auto *itemConstructor = std::get_if<ObjectRef>(&constructor.data);
  if (!itemConstructor)
    property_host::typeError("doc.options.MathItem is not a constructor");
  const std::array<Value, 3> itemArguments{
      Value(std::u16string{}), Value(Null{}),
      state.runtime->properties.to((*math)->display)};
  state.item = cx.construct(*itemConstructor, itemArguments);
  auto sequences = config.getList(u"sequent");
  if (sequences) {
    std::vector<mml::NodeValue> items;
    for (std::size_t i = 0; i < sequences->size(); ++i)
      items.push_back(sequences->get(i));
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
      mml::NodeValue seq = *it;
      if (property_host::truthy(getProperty(seq, "sequentProcessed"))) {
        removeProperty(seq, "sequentProcessed");
        continue;
      }
      auto inf = parentInference(seq);
      if (getProperty(inf, "inference") != Property(1.0))
        continue;
      std::vector<mml::NodeValue> collect{seq};
      while (getProperty(inf, "inference") == Property(1.0)) {
        inf = ruleNode(inf);
        const auto premiseFirst = premise(
            premises(inf,
                     property_host::string(getProperty(inf, "inferenceRule"))),
            0);
        const auto sequent =
            property_host::truthy(getProperty(premiseFirst, "inferenceRule"))
                ? conclusion(premiseFirst, property_host::string(getProperty(
                                               premiseFirst, "inferenceRule")))
                : premiseFirst;
        if (property_host::truthy(getProperty(sequent, "sequent"))) {
          seq = child(sequent, 0);
          collect.push_back(seq);
          setProperty(seq, "sequentProcessed", true);
        }
        inf = premiseFirst;
      }
      mml::NodeValue top = collect.back();
      collect.pop_back();
      while (!collect.empty()) {
        const auto bottom = collect.back();
        collect.pop_back();
        const double dl = bbox(state, child(top, 0)) -
                          bbox(state, child(bottom, 0));
        const double dr = bbox(state, child(top, 2)) -
                          bbox(state, child(bottom, 2));
        if (property_host::truthy(getProperty(parent(top), "axiom"))) {
          const auto add = [&](mml::NodeValue seq, std::size_t position,
                               std::string_view direction, double width) {
            auto space = createNode(
                config, "mspace",
                std::make_shared<tree::ChildArray<mml::MmlNode>>(),
                Property(PropertyList{
                    {"width", Property(UnitUtil::instance().em(width))}}));
            if (direction == "left") {
              auto row = child(child(seq, position), 0);
              const auto children = NodeUtil::instance().getChildren(row);
              const std::array<mml::NodeValue, 1> added{space};
              children->splice(0, 0, added);
              space.requireNode("setParent")->setParent(mml::NodeValue(row));
            } else
              child(seq, position)
                  .requireNode("appendChild")
                  ->appendChild(space);
            setProperty(parent(seq),
                        std::string("sequentAdjust_") + std::string(direction),
                        Property(width));
          };
          if (dl < 0)
            add(top, 0, "left", std::abs(dl));
          else
            add(bottom, 0, "left", std::abs(dl));
          if (dr < 0)
            add(top, 2, "right", std::abs(dr));
          else
            add(bottom, 2, "right", std::abs(dr));
        }
        top = bottom;
      }
    }
  }
  auto inferences = config.getList(u"inference");
  if (!inferences)
    return;
  double maxAdjust = 0;
  for (std::size_t i = 0; i < inferences->size(); ++i) {
    mml::NodeValue inf = inferences->get(i);
    const bool isProof = property_host::truthy(getProperty(inf, "proof"));
    const auto rule = ruleNode(inf);
    const auto direction =
        property_host::string(getProperty(rule, "inferenceRule"));
    const auto prs = premises(rule, direction);
    const auto first = premise(prs, 0);
    double leftAdjust = 0;
    if (property_host::truthy(getProperty(first, "inference"))) {
      const double adjust = adjustValue(state, first);
      if (adjust) {
        addSpace(state, config, first, -adjust);
        const double w = spaces(state, inf, rule);
        addSpace(state, config, inf, adjust - w);
        leftAdjust = adjust - w;
      }
    }
    const auto last =
        premise(prs, NodeUtil::instance().getChildren(prs)->size() - 1);
    if (getProperty(last, "inference").missing() ||
        std::holds_alternative<PropertyNull>(
            getProperty(last, "inference").value))
      continue;
    const double adjust = adjustValue(state, last, true);
    addSpace(state, config, last, -adjust, true);
    const double w = spaces(state, inf, rule, true);
    const double delta = (bbox(state, rule) - bbox(state, parent(prs))) / 2;
    addSpace(state, config, inf, delta < leftAdjust ? -delta : -leftAdjust);
    maxAdjust = std::max(0.0, std::max(0.0, maxAdjust + adjust - w) - delta);
    const auto col = column(inf);
    if (isProof || !col.isNode()) {
      const auto owner =
          property_host::truthy(getProperty(inf, "proof")) ? inf : parent(inf);
      addSpace(state, config, owner, maxAdjust, true);
      continue;
    }
    const auto sibling = nextSibling(col);
    if (sibling.isNode()) {
      const auto em = UnitUtil::instance().em(maxAdjust);
      const std::u16string width(em.begin(), em.end());
      const auto pos = createNode(
          config, "mspace", std::make_shared<tree::ChildArray<mml::MmlNode>>(),
          Property(PropertyList{
              {"width", Property(property_host::debugText(width))}}));
      sibling.requireNode("appendChild")->appendChild(pos);
      maxAdjust = 0;
      continue;
    }
    if (!parentInference(col).isNode())
      continue;
  }
}
namespace {
Value filterFunction(ParserRuntime &runtime,
                     const std::shared_ptr<GeometryState> &state,
                     std::u16string_view name,
                     void (*run)(GeometryState &, const FilterData &)) {
  auto body = std::make_shared<Function>();
  body->sourceMetadata = FunctionSourceMetadata{std::u16string(name), 1};
  body->call = [state, run](const FunctionReceiver &,
                            FunctionArgs args) -> std::any {
    if (args.empty())
      throw Exception("TypeError", u"Bussproofs filter data is missing");
    const auto *input = std::any_cast<core::InputJaxFilterDataRef>(&args[0]);
    if (!input || !*input)
      throw Exception("TypeError", u"Bussproofs filter data has the wrong type");
    const auto current=(*input)->currentData();
    const auto *data = std::any_cast<ParseOptionsRef>(&current);
    if (!data || !*data)
      throw Exception("TypeError", u"Bussproofs filter requires ParseOptions");
    const auto mathReference=(*input)->mathReference();
    core::MathItemReference math=Missing{};
    if(const auto* item=std::get_if<core::MathItem*>(&mathReference))math=*item;
    else if(std::holds_alternative<PropertyNull>(mathReference))math=PropertyNull{};
    else if(std::holds_alternative<Missing>(mathReference))math=Missing{};
    else property_host::typeError("Filter math is not a MathItem");
    const auto documentReference=(*input)->documentReference();
    core::MathDocumentReference document=Missing{};
    if(const auto* owner=std::get_if<core::MathDocumentService*>(&documentReference))document=*owner;
    else if(std::holds_alternative<PropertyNull>(documentReference))document=PropertyNull{};
    else if(std::holds_alternative<Missing>(documentReference))document=Missing{};
    else property_host::typeError("Filter document is not a MathDocument");
    run(*state, FilterData{std::move(math), std::move(document), data->get()});
    return std::monostate{};
  };
  OptionCallBinding binding;
  binding.receiver = [](Context &, Value receiver) {
    return FunctionReceiver(std::move(receiver));
  };
  binding.arguments = [](Context &context, std::span<const Value> args) {
    if (args.empty())
      throw Exception("TypeError", u"Bussproofs filter data is missing");
    const auto *identity = std::get_if<ObjectRef>(&args[0].data);
    if (!identity)
      throw Exception("TypeError", u"Bussproofs filter identity is invalid");
    const auto *input = std::any_cast<core::InputJaxFilterDataRef>(
        &context.payload(*identity));
    if (!input || !*input)
      throw Exception("TypeError", u"Bussproofs filter identity is invalid");
    return std::vector<std::any>{std::any(*input)};
  };
  binding.result = [](Context &, const std::any &) {
    return Value(Undefined{});
  };
  auto result = runtime.functions().bind(body, std::move(binding),
                                         std::u16string(name));
  const auto *identity = std::get_if<ObjectRef>(&result.data);
  if (!identity)
    throw Exception("TypeError", u"Bussproofs export has no Context identity");
  runtime.context().define(*identity, Key(u"length"),
                           Descriptor::data(Value(1.0), false, false, true));
  return result;
}
Value exportedFunction(
    ParserRuntime &runtime, std::u16string_view name, double length,
    std::function<Value(ParserRuntime &, FunctionArgs)> call) {
  auto body = std::make_shared<Function>();
  body->call = [&runtime, call = std::move(call)](
                   const FunctionReceiver &, FunctionArgs args) -> std::any {
    return std::any(call(runtime, std::move(args)));
  };
  OptionCallBinding binding;
  binding.receiver = [](Context &, Value) {
    return FunctionReceiver{Missing{}};
  };
  binding.arguments = [](Context &, std::span<const Value> args) {
    std::vector<std::any> converted;
    converted.reserve(args.size());
    for (const auto &value : args)
      converted.emplace_back(value);
    return converted;
  };
  binding.result = [](Context &, const std::any &result) {
    if (const auto *value = std::any_cast<Value>(&result))
      return *value;
    return Value(Undefined{});
  };
  const Value result = runtime.functions().bind(
      std::move(body), std::move(binding), std::u16string(name));
  const auto *identity = std::get_if<ObjectRef>(&result.data);
  if (!identity)
    throw Exception("TypeError", u"Bussproofs export has no Context identity");
  runtime.context().define(*identity, Key(u"length"),
                           Descriptor::data(Value(length), false, false, true));
  return result;
}
Value argument(const FunctionArgs &args, std::size_t index) {
  if (index >= args.size())
    throw Exception("TypeError", u"Bussproofs utility argument is missing");
  const auto *value = std::any_cast<Value>(&args[index]);
  if (!value)
    throw Exception("TypeError", u"Bussproofs utility argument is invalid");
  return *value;
}
mml::NodeValue argumentNode(ParserRuntime &runtime, const FunctionArgs &args,
                            std::size_t index) {
  const Value value = argument(args, index);
  const auto *identity = std::get_if<ObjectRef>(&value.data);
  if (!identity)
    throw Exception("TypeError", u"Bussproofs MML argument is invalid");
  return runtime.mmlNodeIdentities.resolve(*identity);
}
std::u16string argumentString(ParserRuntime &runtime, const FunctionArgs &args,
                              std::size_t index) {
  return property_host::string(runtime.properties.from(argument(args, index)));
}
} // namespace
std::shared_ptr<BussproofsUtilModule>
BussproofsUtilModule::initializeModule(ParserRuntime &runtime,
                                      util::RuntimeModules &modules) {
  if (&modules.context() != &runtime.context())
    throw Exception("TypeError", u"Bussproofs module belongs to a different Context");
  const auto owner = modules.getOrCreate<BussproofsUtilModule>(
      u"input/tex/bussproofs/BussproofsUtil", [&] {
        auto module = std::shared_ptr<BussproofsUtilModule>(
            new BussproofsUtilModule());
        module->state_ = std::make_shared<GeometryState>(runtime);
        module->setProperty_ = exportedFunction(
            runtime, u"setProperty", 3,
            [](ParserRuntime &owner, FunctionArgs args) {
              const auto name = argumentString(owner, args, 1);
              setProperty(argumentNode(owner, args, 0),
                          property_host::debugText(name),
                          owner.properties.from(argument(args, 2)));
              return Value(Undefined{});
            });
        module->getProperty_ = exportedFunction(
            runtime, u"getProperty", 2,
            [](ParserRuntime &owner, FunctionArgs args) {
              const auto name = argumentString(owner, args, 1);
              return owner.properties.to(getProperty(
                  argumentNode(owner, args, 0), property_host::debugText(name)));
            });
        module->removeProperty_ = exportedFunction(
            runtime, u"removeProperty", 2,
            [](ParserRuntime &owner, FunctionArgs args) {
              const auto name = argumentString(owner, args, 1);
              removeProperty(argumentNode(owner, args, 0),
                             property_host::debugText(name));
              return Value(Undefined{});
            });
        module->saveDocument_ = filterFunction(
            runtime, module->state_, u"saveDocument", &saveDocument);
        module->clearDocument_ = filterFunction(
            runtime, module->state_, u"clearDocument", &clearDocument);
        module->makeAttributes_ = filterFunction(
            runtime, module->state_, u"makeBsprAttributes", &makeBsprAttributes);
        module->balanceRules_ = filterFunction(
            runtime, module->state_, u"balanceRules", &balanceRules);
        return module;
      });
  if (owner->state_->runtime != &runtime)
    throw Exception("TypeError", u"Bussproofs module belongs to a different ParserRuntime");
  return owner;
}
Context &BussproofsUtilModule::context() const {
  return state_->contextOwner();
}
Value BussproofsUtilModule::function(std::u16string_view name) const {
  if (name == u"setProperty") return setProperty_;
  if (name == u"getProperty") return getProperty_;
  if (name == u"removeProperty") return removeProperty_;
  if (name == u"saveDocument") return saveDocument_;
  if (name == u"clearDocument") return clearDocument_;
  if (name == u"makeBsprAttributes") return makeAttributes_;
  if (name == u"balanceRules") return balanceRules_;
  return Value(Undefined{});
}
} // namespace folia::math::tex::bussproofs_util
