// Native MathtoolsTags dynamic subclass owner from MathtoolsTags.ts.
module folia.math.input.tex.mathtools.tags;
import folia.math.input.tex.tex_error;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
void requireRuntimeActive(const std::weak_ptr<std::atomic_bool> &lease) {
  const auto active = lease.lock();
  if (!active || !active->load(std::memory_order_acquire))
    throw Exception("TypeError", u"MathtoolsTags runtime is no longer active");
}
Key key(std::u16string_view name) { return Key(std::u16string(name)); }
Value makeTriple(Context &cx, Value first, Value second, Value third) {
  const ObjectRef result = cx.array(3);
  cx.set(Value(result), key(u"0"), std::move(first));
  cx.set(Value(result), key(u"1"), std::move(second));
  cx.set(Value(result), key(u"2"), std::move(third));
  return Value(result);
}
ObjectRef objectField(Context &cx, Tags &tags, std::u16string_view name) {
  const Value value = cx.get(Value(tags.sourceIdentity()), key(name));
  if (const auto *object = std::get_if<ObjectRef>(&value.data))
    return *object;
  throw Exception("TypeError", u"MathtoolsTags Context field is not an object");
}
} // namespace
Property
MathtoolsTags::moduleExports(ParserRuntime &runtime) {
  if (!runtime.mathtoolsTagModuleExports.missing())
    return runtime.mathtoolsTagModuleExports;
  auto tagId = runtime.mathtoolsTagId;
  if (!tagId)
    tagId = runtime.mathtoolsTagId = std::make_shared<std::uint64_t>(0);
  const auto lease = runtime.executionLifetime();
  auto body = std::make_shared<Function>();
  body->call = [&runtime, lease, tagId](const FunctionReceiver &,
                                 FunctionArgs args) -> std::any {
    requireRuntimeActive(lease);
    if (args.size() < 2)
      throw Exception(
          "TypeError",
          u"MathtoolsTagFormat requires ParserConfiguration and TeX");
    const auto *configuration = std::any_cast<ParserConfigurationRef>(&args[0]);
    const auto *jax = std::any_cast<TeX *>(&args[1]);
    if (!configuration || !*configuration || !jax || !*jax)
      throw Exception("TypeError", u"MathtoolsTagFormat arguments are invalid");
    registerTagFormat(runtime, **configuration, **jax, tagId);
    return std::monostate{};
  };
  OptionCallBinding binding;
  binding.receiver = [](Context &, Value) {
    return FunctionReceiver{Missing{}};
  };
  binding.arguments = [&runtime, lease](Context &, std::span<const Value> args) {
    requireRuntimeActive(lease);
    if (args.size() < 2)
      throw Exception("TypeError",
                      u"MathtoolsTagFormat requires (config, jax)");
    const auto *config = std::get_if<ObjectRef>(&args[0].data);
    const auto *jax = std::get_if<ObjectRef>(&args[1].data);
    if (!config || !jax)
      throw Exception("TypeError",
                      u"MathtoolsTagFormat identities are invalid");
    return std::vector<std::any>{
        std::any(runtime.parserConfigurationIdentities.resolve(*config)),
        std::any(runtime.texIdentities.resolve(*jax).get())};
  };
  binding.result = [](Context &, const std::any &) {
    return Value(Undefined{});
  };
  const FunctionRef function = body;
  (void)runtime.functions().bind(function, std::move(binding),
                                 u"MathtoolsTagFormat");
  if (const auto identity = runtime.functions().find(function))
    runtime.context().define(*identity, key(u"length"),
                             Descriptor::data(Value(2), false, false, true));
  const PropertyList exports{{"MathtoolsTagFormat", Property(function)}};
  runtime.mathtoolsTagModuleExports = Property(exports);
  return runtime.mathtoolsTagModuleExports;
}
void MathtoolsTags::registerTagFormat(
    ParserRuntime &runtime, ParserConfiguration &configuration, TeX &jax,
    const std::shared_ptr<std::uint64_t> &tagId) {
  if (!tagId)
    throw Exception("TypeError", u"Mathtools tag ID state is unavailable");
  const auto parsedValue = jax.parseOptions();
  const auto *parsed = std::get_if<ParseOptionsRef>(&parsedValue);
  if (!parsed || !*parsed)
    property_host::typeError(
        "Cannot read properties of null or undefined (reading 'options')");
  Context &cx = runtime.context();
  const Value selected = cx.get((*parsed)->options, key(u"tags"));
  const auto *selectedName = std::get_if<std::u16string>(&selected.data);
  if (!selectedName)
    throw Exception("TypeError",
                    u"Mathtools selected tag name is not a string");
  const Value configuredTags = configuration.sourceField(u"tags");
  if (*selectedName != u"base" &&
      cx.hasOwn(configuredTags, key(*selectedName))) {
    const Value constructor = cx.get(configuredTags, key(*selectedName));
    const auto *identity = std::get_if<ObjectRef>(&constructor.data);
    if (!identity)
      throw Exception("TypeError", u"Selected Mathtools tag class is invalid");
    const TagsClassRef actual = runtime.tagClassIdentities.resolve(*identity);
    if (!actual)
      throw Exception("TypeError",
                      u"Selected Mathtools tag class is unavailable");
    runtime.tagsFactory.add(*selectedName, actual);
  }
  const TagsRef parent = runtime.tagsFactory.create(*selectedName);
  const Value parentValue =
      cx.get(Value(parent->sourceIdentity()), key(u"constructor"));
  const auto *parentConstructor = std::get_if<ObjectRef>(&parentValue.data);
  if (!parentConstructor)
    throw Exception("TypeError",
                    u"Selected Mathtools tag constructor is unavailable");
  const Value mathtools = cx.get((*parsed)->options, key(u"mathtools"));
  const Value forms = cx.get(mathtools, key(u"tagforms"));
  std::u16string className;
  const TagsClassRef derived = initializeModule(runtime, *parentConstructor,
                                                forms, ++*tagId, &className);
  runtime.tagsFactory.add(className, derived);
  cx.set((*parsed)->options, key(u"tags"), Value(std::move(className)));
}
TagsClassRef MathtoolsTags::initializeModule(ParserRuntime &runtime,
                                             ObjectRef parentConstructor,
                                             Value forms, std::uint64_t tagId,
                                             std::u16string *outputName) {
  Context &cx = runtime.context();
  const auto parentPrototypeValue =
      cx.get(Value(parentConstructor), key(u"prototype"));
  const auto *parentPrototype =
      std::get_if<ObjectRef>(&parentPrototypeValue.data);
  if (!parentPrototype)
    throw Exception("TypeError", u"Selected Tags constructor has no prototype");
  const std::string digits = std::to_string(tagId);
  const std::u16string className =
      u"MathtoolsTags-" + std::u16string(digits.begin(), digits.end());
  if (outputName)
    *outputName = className;
  const auto tagClass = TagsFactory::registerSourceSubclass(
      runtime, u"TagFormat", parentConstructor,
      [&runtime, forms, lease = runtime.executionLifetime()](Tags &tags) {
        requireRuntimeActive(lease);
        Context &context = runtime.context();
        const auto storage = runtime.maps.createStorage();
        const ObjectRef mapObject = runtime.maps.bind(storage);
        context.set(Value(tags.sourceIdentity()), key(u"mtFormats"),
                    Value(mapObject));
        context.set(Value(tags.sourceIdentity()), key(u"mtCurrent"),
                    Value(Null{}));
        for (const auto &name : context.enumerableStringKeys(forms)) {
          const Value form = context.get(forms, key(name));
          const auto *array = std::get_if<ObjectRef>(&form.data);
          if (!array || context.kind(*array) != ObjectKind::array ||
              context.arrayLength(*array) != 3)
            throw TexError(
                u"InvalidTagFormDef",
                u"The tag form definition for \"%1\" should be an array of "
                u"three strings",
                std::array<Property, 1>{property_host::debugText(name)});
          storage->set(Value(name), form);
        }
      },
      [&runtime, lease = runtime.executionLifetime()](ObjectRef childPrototype) {
        requireRuntimeActive(lease);
        Context &context = runtime.context();
        const ObjectRef method = context.function(
            u"formatTag",
            [&runtime, lease,
             childPrototype](Context &cx, Value receiver,
                             std::span<const Value> arguments) -> Value {
              requireRuntimeActive(lease);
              const Value current = cx.get(receiver, key(u"mtCurrent"));
              if (!options::truthy(current)) {
                const auto parent = cx.prototype(childPrototype);
                if (!parent)
                  throw Exception(
                      "TypeError",
                      u"MathtoolsTags parent prototype is unavailable");
                const Value parentMethodValue =
                    cx.get(Value(*parent), key(u"formatTag"));
                const auto *parentMethod =
                    std::get_if<ObjectRef>(&parentMethodValue.data);
                if (!parentMethod ||
                    cx.kind(*parentMethod) != ObjectKind::function)
                  throw Exception(
                      "TypeError",
                      u"MathtoolsTags parent formatTag is not callable");
                return cx.call(*parentMethod, std::move(receiver), arguments);
              }
              const Value left = cx.get(current, key(u"0"));
              const Value right = cx.get(current, key(u"1"));
              const Value format = cx.get(current, key(u"2"));
              const Value tag =
                  arguments.empty() ? Value(Undefined{}) : arguments[0];
              const std::u16string tagText =
                  property_host::string(runtime.properties.from(
                      cx.toPrimitive(tag, PrimitiveHint::stringHint)));
              std::u16string rendered = tagText;
              if (options::truthy(format)) {
                const std::u16string formatText =
                    property_host::string(runtime.properties.from(
                        cx.toPrimitive(format, PrimitiveHint::stringHint)));
                rendered = formatText + u"{" + tagText + u"}";
              }
              return makeTriple(cx, left, Value(std::move(rendered)), right);
            });
        context.define(method, key(u"length"),
                       Descriptor::data(Value(1), false, false, true));
        context.define(childPrototype, key(u"formatTag"),
                       Descriptor::data(Value(method), true, false, true));
      });
  return tagClass;
}
bool MathtoolsTags::hasFormat(ParserRuntime &runtime, Tags &tags,
                              std::u16string_view name) {
  return runtime.maps.has(objectField(runtime.context(), tags, u"mtFormats"),
                          Value(std::u16string(name)));
}
Value MathtoolsTags::format(ParserRuntime &runtime, Tags &tags,
                            std::u16string_view name) {
  return runtime.maps.get(objectField(runtime.context(), tags, u"mtFormats"),
                          Value(std::u16string(name)));
}
void MathtoolsTags::setFormat(ParserRuntime &runtime, Tags &tags,
                              std::u16string_view name, Value value) {
  runtime.maps.set(objectField(runtime.context(), tags, u"mtFormats"),
                   Value(std::u16string(name)), std::move(value));
}
Value MathtoolsTags::current(ParserRuntime &runtime, Tags &tags) {
  return runtime.context().get(Value(tags.sourceIdentity()), key(u"mtCurrent"));
}
void MathtoolsTags::setCurrent(ParserRuntime &runtime, Tags &tags,
                               Value value) {
  runtime.context().set(Value(tags.sourceIdentity()), key(u"mtCurrent"),
                        std::move(value));
}
} // namespace folia::math::tex
