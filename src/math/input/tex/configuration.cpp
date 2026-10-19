// MathJax 4.1.3 input/tex/Configuration.ts; Apache-2.0.
module folia.math.input.tex.parser;

import folia.math.util.option_functions;
import folia.math.util.options;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;

Key key(std::u16string_view name) { return Key(std::u16string(name)); }

Value read(Value receiver, std::u16string_view name, Context& context) {
    return context.get(std::move(receiver), key(name));
}

using ConfigurationHandle = PrioritizedListItemHandle<ConfigurationRef>;

ConfigurationRef configurationForItem(const ConfigurationHandle& record, ParserRuntime& runtime) {
    const auto item = record.readItem();
    if (const auto* configuration = std::get_if<ConfigurationRef>(&item)) return *configuration;
    if (const auto* raw = std::get_if<Value>(&item)) {
        if (const auto* identity = std::get_if<ObjectRef>(&raw->data))
            return runtime.configurationIdentities.resolve(*identity);
    }
    throw Exception("TypeError", u"Configuration priority record item is not a Configuration");
}

bool nullish(const Value& value) {
    return std::holds_alternative<Undefined>(value.data) ||
           std::holds_alternative<Null>(value.data);
}

Value orObject(Context& context, Value value) {
    return truthy(value) ? value : Value(context.object());
}

std::u16string textKey(std::string_view value) {
    std::u16string result;
    result.reserve(value.size());
    for (const unsigned char c : value) result.push_back(static_cast<char16_t>(c));
    return result;
}

Value index(Context& context, const Value& value, std::uint32_t i) {
    return context.get(value, Key(textKey(std::to_string(i))));
}

void assign(Context& context, Value destination, Value source) {
    if (std::holds_alternative<Undefined>(source.data) ||
        std::holds_alternative<Null>(source.data)) return;
    for (const Key& name : context.ownKeys(source)) {
        const auto descriptor = context.descriptor(source, name);
        if (descriptor && descriptor->enumerable)
            context.set(destination, name, context.get(source, name));
    }
}

double numberPriority(Context& context, const Value& value) {
    if (std::holds_alternative<Undefined>(value.data)) return PrioritizedList<FunctionValue>::DEFAULTPRIORITY;
    return context.toNumber(value);
}

void addProcessorProperties(Context& context, OptionFunctionBindings& bindings,
                            FunctionList& list, Value tuple) {
    // Configuration.append uses tuple[0]/tuple[1] ordinary Get, irrespective
    // of the array's iterator length.
    const Value callable = context.get(tuple, key(u"0"));
    const Value priority = context.get(tuple, key(u"1"));
    list.add(bindings.functionValue(callable), numberPriority(context, priority));
}

void addProcessorIterable(Context& context, OptionFunctionBindings& bindings,
                          FunctionList& list, Value tuple) {
    const Value destructured=context.readIterable(tuple,2);
    const Value callable=context.get(destructured,key(u"0"));
    const Value priority=context.get(destructured,key(u"1"));
    list.add(bindings.functionValue(callable), numberPriority(context, priority));
}

void addFiltersFrom(Context& context, OptionFunctionBindings& bindings,
                    FunctionList& list, Value tuples) {
    context.forEachIterable(tuples,[&](Value tuple) {
        addProcessorIterable(context, bindings, list, tuple);
    });
}

Value callMap(Context& context, Value receiver, ObjectRef callback) {
    const Value map = context.get(receiver, key(u"map"));
    const auto* function = std::get_if<ObjectRef>(&map.data);
    if (!function || context.kind(*function) != ObjectKind::function)
        throw Exception("TypeError", u"preprocessors.map is not a function");
    const std::array<Value, 1> arguments{Value(callback)};
    return context.call(*function, receiver, arguments);
}

Value callNoArgMethod(Context& context, Value receiver, std::u16string_view name) {
    const Value method = context.get(receiver, key(name));
    const auto* function = std::get_if<ObjectRef>(&method.data);
    if (!function || context.kind(*function) != ObjectKind::function)
        throw Exception("TypeError", std::u16string(name) + u" is not a function");
    return context.call(*function, receiver);
}

} // namespace

ProcessorValue Configuration::makeProcessor(ParserRuntime& runtime, Value func, Value priority) {
    Context& context = runtime.context();
    if (const auto* object = std::get_if<ObjectRef>(&func.data);
        object && context.kind(*object) == ObjectKind::array) return func;
    const ObjectRef tuple = context.array(2);
    context.define(tuple, key(u"0"), Descriptor::data(std::move(func)));
    context.define(tuple, key(u"1"), Descriptor::data(std::move(priority)));
    return Value(tuple);
}

ConfigurationRef Configuration::_create(ParserRuntime& runtime, Value receiver, Value name, Value config) {
    Context& context = runtime.context();
    if (std::holds_alternative<Undefined>(config.data)) config = Value(context.object());

    Value priority = read(config, u"priority", context);
    if (nullish(priority)) priority = Value(PrioritizedList<FunctionValue>::DEFAULTPRIORITY);

    auto callMakeProcessor = [&](std::u16string_view fieldName) {
        const Value method = context.get(receiver, key(u"makeProcessor"));
        const auto* function = std::get_if<ObjectRef>(&method.data);
        if (!function || context.kind(*function) != ObjectKind::function)
            throw Exception("TypeError", u"this.makeProcessor is not a function");
        const Value value = read(config, fieldName, context);
        const std::array<Value, 2> arguments{std::move(value), priority};
        return context.call(*function, receiver, arguments);
    };
    Value init = read(config, u"init", context);
    init = truthy(init) ? callMakeProcessor(u"init") : Value(Null{});
    Value conf = read(config, u"config", context);
    conf = truthy(conf) ? callMakeProcessor(u"config") : Value(Null{});

    Value pre = read(config, u"preprocessors", context);
    if (!truthy(pre)) pre = Value(context.array());
    const ObjectRef mapper = context.function(u"Configuration.makeProcessor",
        [receiver, priority](Context& active, Value, std::span<const Value> args) {
            const Value item = args.empty() ? Value(Undefined{}) : args.front();
            const Value method = active.get(receiver, key(u"makeProcessor"));
            const auto* function = std::get_if<ObjectRef>(&method.data);
            if (!function || active.kind(*function) != ObjectKind::function)
                throw Exception("TypeError", u"this.makeProcessor is not a function");
            const std::array<Value, 2> callArgs{item, priority};
            return active.call(*function, receiver, callArgs);
        });
    const Value preprocessors = callMap(context, pre, mapper);

    Value post = read(config, u"postprocessors", context);
    if (!truthy(post)) post = Value(context.array());
    const Value postprocessors = callMap(context, post, mapper);

    Value parser = read(config, u"parser", context);
    if (!truthy(parser)) parser = Value(u"tex");
    Value handler = orObject(context, read(config, u"handler", context));
    Value fallback = orObject(context, read(config, u"fallback", context));
    Value items = orObject(context, read(config, u"items", context));
    Value tags = orObject(context, read(config, u"tags", context));
    Value options = orObject(context, read(config, u"options", context));
    Value nodes = orObject(context, read(config, u"nodes", context));

    auto result = ConfigurationRef(new Configuration(runtime));
    const auto* constructor = std::get_if<ObjectRef>(&runtime.configurationClass.data);
    if (!constructor) throw Exception("TypeError", u"Configuration class metadata is not initialized");
    const ObjectRef identity = runtime.configurationIdentities.instance(result, *constructor);
    result->initializeConfiguration(identity, std::move(name), std::move(handler), std::move(fallback),
        std::move(items), std::move(tags), std::move(options), std::move(nodes),
        std::move(preprocessors), std::move(postprocessors), std::move(init), std::move(conf),
        std::move(priority), std::move(parser));
    return result;
}

Configuration::Configuration(ParserRuntime& runtime) : runtime_(runtime) {}

void Configuration::initializeConfiguration(ObjectRef identity, Value nameValue, Value handlerValue,
        Value fallbackValue, StackItemConfigValue itemsValue, TagsConfigValue tagsValue,
        Value optionsValue, Value nodesValue, ProcessorListValue preprocessorsValue,
        ProcessorListValue postprocessorsValue, ProcessorValue initValue,
        ProcessorValue configValue, Value priorityValue, Value parserValue) {
    Context& context = runtime_.context();
    identity_ = identity;
    const ObjectRef merged = context.object();
    for (const auto [field, value] : std::array<std::pair<std::u16string_view, Value>, 4>{
        std::pair{u"character", Value(context.array())},
        std::pair{u"delimiter", Value(context.array())},
        std::pair{u"macro", Value(context.array())},
        std::pair{u"environment", Value(context.array())}})
        context.set(Value(merged), key(field), value);
    assign(context, Value(merged), std::move(handlerValue));
    const std::array<std::pair<std::u16string_view, Value>, 13> fields{{
        {u"name", Value(std::move(nameValue))}, {u"handler", Value(merged)},
        {u"fallback", std::move(fallbackValue)}, {u"items", std::move(itemsValue)},
        {u"tags", std::move(tagsValue)}, {u"options", std::move(optionsValue)},
        {u"nodes", std::move(nodesValue)}, {u"preprocessors", std::move(preprocessorsValue)},
        {u"postprocessors", std::move(postprocessorsValue)}, {u"initMethod", std::move(initValue)},
        {u"configMethod", std::move(configValue)}, {u"priority", std::move(priorityValue)},
        {u"parser", std::move(parserValue)}}};
    for (const auto& [fieldName, value] : fields)
        context.define(identity_, key(fieldName), Descriptor::data(value));
}

ConfigurationRef Configuration::construct(ParserRuntime& runtime, std::span<const Value> arguments) {
    Context& context = runtime.context();
    auto arg = [&](std::size_t i) {
        return i < arguments.size() ? arguments[i] : Value(Undefined{});
    };
    const Value name = arg(0);
    Value handler = arg(1); if (std::holds_alternative<Undefined>(handler.data)) handler = Value(context.object());
    Value fallback = arg(2); if (std::holds_alternative<Undefined>(fallback.data)) fallback = Value(context.object());
    Value items = arg(3); if (std::holds_alternative<Undefined>(items.data)) items = Value(context.object());
    Value tags = arg(4); if (std::holds_alternative<Undefined>(tags.data)) tags = Value(context.object());
    Value options = arg(5); if (std::holds_alternative<Undefined>(options.data)) options = Value(context.object());
    Value nodes = arg(6); if (std::holds_alternative<Undefined>(nodes.data)) nodes = Value(context.object());
    Value preprocessors = arg(7); if (std::holds_alternative<Undefined>(preprocessors.data)) preprocessors = Value(context.array());
    Value postprocessors = arg(8); if (std::holds_alternative<Undefined>(postprocessors.data)) postprocessors = Value(context.array());
    Value init = arg(9); if (std::holds_alternative<Undefined>(init.data)) init = Value(Null{});
    Value config = arg(10); if (std::holds_alternative<Undefined>(config.data)) config = Value(Null{});
    const Value priority = arg(11);
    const Value parser = arg(12);
    auto result = ConfigurationRef(new Configuration(runtime));
    const auto* constructor = std::get_if<ObjectRef>(&runtime.configurationClass.data);
    if (!constructor) throw Exception("TypeError", u"Configuration class metadata is not initialized");
    const ObjectRef identity = runtime.configurationIdentities.instance(result, *constructor);
    result->initializeConfiguration(identity, name, handler, fallback, items, tags, options, nodes,
        preprocessors, postprocessors, init, config, priority, parser);
    return result;
}

ParserRuntime& Configuration::runtime() const { return runtime_; }

Value Configuration::field(std::u16string_view fieldName) const {
    return runtime_.context().get(Value(identity_), key(fieldName));
}

std::u16string Configuration::name() const {
    const Value value = field(u"name");
    if (const auto* text = std::get_if<std::u16string>(&value.data)) return *text;
    throw Exception("TypeError", u"Configuration.name is not a string");
}

ConfigurationRef Configuration::create(ParserRuntime& runtime, std::u16string name, Value config) {
    ConfigurationRef result = _create(runtime, runtime.configurationClass, Value(name), std::move(config));
    runtime.configurationHandler.set(std::move(name), result);
    return result;
}

ConfigurationRef Configuration::local(ParserRuntime& runtime, Value config) {
    return _create(runtime, runtime.configurationClass, Value(u""), std::move(config));
}

Value Configuration::init() const {
    const Value tuple = field(u"initMethod");
    if (!truthy(tuple)) return Value(Null{});
    return index(runtime_.context(), tuple, 0);
}

Value Configuration::config() const {
    const Value tuple = field(u"configMethod");
    if (!truthy(tuple)) return Value(Null{});
    return index(runtime_.context(), tuple, 0);
}

void ConfigurationHandler::set(std::u16string name, ConfigurationRef configuration) {
    maps_.set(std::move(name), std::move(configuration));
}

ParserShared<Configuration> ConfigurationHandler::get(std::u16string_view name) const {
    if (auto value = maps_.get(name)) return *value;
    return Missing{};
}

SourceStringMap<ConfigurationRef>::KeyIterator ConfigurationHandler::keys() const {
    return maps_.keys();
}

void ConfigurationHandler::initializeModule(ParserRuntime& runtime) {
    runtime.configurationHandlerObject = Value(createContextObject(runtime.context(),runtime.configurationIdentities));
}

ObjectRef ConfigurationHandler::createContextObject(Context& context,
        NativeIdentityBindings<ConfigurationRef>& identities) {
    const ObjectRef handler = context.object();
    auto defineMethod = [&](std::u16string name, double length, NativeCall body) {
        const ObjectRef method = context.function(name, std::move(body));
        context.define(method, key(u"length"), Descriptor::data(Value(length), false, false, true));
        context.define(handler, key(name), Descriptor::data(Value(method), true, true, true));
    };
    defineMethod(u"set", 2, [this, &identities](Context&, Value,
            std::span<const Value> arguments) -> Value {
        const Value nameValue = arguments.empty() ? Value(Undefined{}) : arguments[0];
        const auto* name = std::get_if<std::u16string>(&nameValue.data);
        if (!name) throw Exception("Unimplemented", u"ConfigurationHandler Map key is not a string");
        const Value configurationValue = arguments.size() < 2 ? Value(Undefined{}) : arguments[1];
        const auto* object = std::get_if<ObjectRef>(&configurationValue.data);
        if (!object) throw Exception("Unimplemented", u"ConfigurationHandler value has no Configuration identity");
        set(*name, identities.resolve(*object));
        return Value(Undefined{});
    });
    defineMethod(u"get", 1, [this, &identities](Context&, Value,
            std::span<const Value> arguments) -> Value {
        const Value nameValue = arguments.empty() ? Value(Undefined{}) : arguments[0];
        const auto* name = std::get_if<std::u16string>(&nameValue.data);
        if (!name) throw Exception("Unimplemented", u"ConfigurationHandler Map key is not a string");
        const auto found = get(*name);
        const auto* configuration = std::get_if<ConfigurationRef>(&found);
        if (!configuration || !*configuration) return Value(Undefined{});
        const auto identity = identities.find(*configuration);
        if (!identity) throw Exception("TypeError", u"Configuration has no bound Context identity");
        return Value(*identity);
    });
    Context* ownerContext = &context;
    defineMethod(u"keys", 0, [this, ownerContext](Context&, Value,
            std::span<const Value>) -> Value {
        using Iterator = SourceStringMap<ConfigurationRef>::KeyIterator;
        auto state = std::make_shared<Iterator>(keys());
        const ObjectRef iterator = ownerContext->nativeIterator([state](Context& active) -> Value {
            const auto value = state->next();
            const ObjectRef result = active.object();
            active.define(result, key(u"value"), Descriptor::data(
                value ? Value(*value) : Value(Undefined{})));
            active.define(result, key(u"done"), Descriptor::data(Value(!value)));
            return Value(result);
        });
        return Value(iterator);
    });
    return handler;
}

ParserConfiguration::ParserConfiguration(ParserRuntime& runtime) : runtime_(runtime) {
}

Value ParserConfiguration::field(std::u16string_view name) const {
    if (!identity_.context) throw Exception("TypeError", u"ParserConfiguration has no Context identity");
    return runtime_.context().get(Value(identity_), key(name));
}

void ParserConfiguration::setNativeField(std::u16string_view name, std::any payload) {
    Context& context = runtime_.context();
    if (const auto* list = std::any_cast<std::shared_ptr<PrioritizedList<ConfigurationRef>>>(&payload);
        list && *list) {
        const ObjectRef identity = bindConfigurationPriorityList(runtime_, *list);
        context.define(identity_, key(name), Descriptor::data(Value(identity)));
        return;
    }
    // FunctionList's registered Context identity is the Source instance: it
    // owns the live `items` array view and the inherited PrioritizedList API.
    // Keep the native owner as that object's payload so typed execution and
    // Source add/remove/iteration all operate on the same replaceable storage.
    if (const auto* list = std::any_cast<std::shared_ptr<FunctionList>>(&payload);
        list && *list) {
        const ObjectRef identity = runtime_.functionLists.bind(*list);
        context.define(identity_, key(name), Descriptor::data(Value(identity)));
        return;
    }
    const ObjectRef carrier = context.object();
    context.setPayload(carrier,std::move(payload));
    context.define(identity_, key(name), Descriptor::data(Value(carrier)));
}

template<class T>
std::shared_ptr<T> parserNativePayload(Context& context, Value value, std::u16string_view fieldName) {
    const auto* object = std::get_if<ObjectRef>(&value.data);
    if (!object) throw Exception("TypeError", std::u16string(u"ParserConfiguration.") + std::u16string(fieldName) + u" is not a native list");
    const auto& payload = std::as_const(context).payload(*object);
    if (const auto* pointer = std::any_cast<std::shared_ptr<T>>(&payload); pointer && *pointer) return *pointer;
    throw Exception("TypeError", std::u16string(u"ParserConfiguration.") + std::u16string(fieldName) + u" is not a native list");
}

std::shared_ptr<FunctionList> ParserConfiguration::functionList(std::u16string_view name) const {
    return parserNativePayload<FunctionList>(runtime_.context(), field(name), name);
}
std::shared_ptr<PrioritizedList<ConfigurationRef>> ParserConfiguration::configurationList() const {
    return parserNativePayload<PrioritizedList<ConfigurationRef>>(runtime_.context(), field(u"configurations"), u"configurations");
}
std::shared_ptr<SubHandlers> ParserConfiguration::handlerList() const {
    return parserNativePayload<SubHandlers>(runtime_.context(), field(u"handlers"), u"handlers");
}

void ParserConfiguration::initializeParserConfiguration(Value packages, Value parserNames) {
    Context& context = runtime_.context();
    if (std::holds_alternative<Undefined>(parserNames.data)) {
        const ObjectRef defaults = context.array(1);
        context.define(defaults, key(u"0"), Descriptor::data(Value(u"tex")));
        parserNames = Value(defaults);
    }
    if (const auto* constructor = std::get_if<ObjectRef>(&runtime_.parserConfigurationClass.data))
        (void)runtime_.parserConfigurationIdentities.instance(shared_from_this(), *constructor);
    const auto identity = runtime_.parserConfigurationIdentities.find(shared_from_this());
    if (!identity) throw Exception("TypeError", u"ParserConfiguration identity was not bound");
    identity_ = *identity;
    setNativeField(u"initMethod", std::make_shared<FunctionList>());
    setNativeField(u"configMethod", std::make_shared<FunctionList>());
    setNativeField(u"configurations", std::make_shared<PrioritizedList<ConfigurationRef>>());
    context.define(identity_, key(u"parsers"), Descriptor::data(Value(context.array())));
    setNativeField(u"handlers", std::make_shared<SubHandlers>(runtime_.mapHandler));
    context.define(identity_, key(u"items"), Descriptor::data(Value(context.object())));
    context.define(identity_, key(u"tags"), Descriptor::data(Value(context.object())));
    context.define(identity_, key(u"options"), Descriptor::data(Value(context.object())));
    context.define(identity_, key(u"nodes"), Descriptor::data(Value(context.object())));
    context.set(Value(identity_), key(u"parsers"), parserNames);

    // Preserve the Source property Gets and call receivers; these are actual
    // Array prototype methods, not a native shortcut around user overrides.
    const Value sliced = callNoArgMethod(context, packages, u"slice");
    const Value snapshot = callNoArgMethod(context, sliced, u"reverse");
    context.forEachIterable(snapshot,[&](Value package) {
        addPackage(package);
    });
    const auto configurations = configurationList();
    auto current = configurations->begin();
    for (;;) {
        auto step = current.next();
        if (step.done) break;
        const auto& record = step.value;
        append(configurationForItem(record, runtime_), record.readPriorityValue());
    }
}

ParserRuntime& ParserConfiguration::runtime() const { return runtime_; }

void ParserConfiguration::init() {
    const auto initMethod = functionList(u"initMethod");
    const std::array<std::any, 1> arguments{std::any(shared_from_this())};
    initMethod->execute(arguments);
}

void ParserConfiguration::config(TeX& jax) {
    const auto configMethod = functionList(u"configMethod");
    const std::array<std::any, 2> arguments{std::any(shared_from_this()), std::any(&jax)};
    configMethod->execute(arguments);
    const auto configurations = configurationList();
    auto current = configurations->begin();
    for (;;) {
        auto step = current.next();
        if (step.done) break;
        addFilters(jax, configurationForItem(step.value, runtime_));
    }
}

ParserShared<Configuration> ParserConfiguration::getPackage(std::u16string_view name) {
    Context& context = runtime_.context();
    const Value handler = runtime_.configurationHandlerObject;
    const Value getMethod = context.get(handler, key(u"get"));
    const auto* getFunction = std::get_if<ObjectRef>(&getMethod.data);
    if (!getFunction || context.kind(*getFunction) != ObjectKind::function)
        throw Exception("TypeError", u"ConfigurationHandler.get is not a function");
    const std::array<Value, 1> getArguments{Value(std::u16string(name))};
    const Value rawFound = context.call(*getFunction, handler, getArguments);
    ParserShared<Configuration> found = Missing{};
    if (truthy(rawFound)) {
        const auto* identity = std::get_if<ObjectRef>(&rawFound.data);
        if (!identity)
            throw Exception("TypeError", u"ConfigurationHandler.get returned a non-configuration value");
        found = runtime_.configurationIdentities.resolve(*identity);
    }
    if (const auto* configuration = std::get_if<ConfigurationRef>(&found); configuration && *configuration) {
        const Value parsers = field(u"parsers");
        const Value includesMethod = context.get(parsers, key(u"includes"));
        const auto* function = std::get_if<ObjectRef>(&includesMethod.data);
        if (!function || context.kind(*function) != ObjectKind::function)
            throw Exception("TypeError", u"this.parsers.includes is not a function");
        const Value target = (*configuration)->field(u"parser");
        const std::array<Value, 1> arguments{target};
        if (!truthy(context.call(*function, parsers, arguments))) {
            throw Exception("Error", u"Package '" + std::u16string(name) + u"' doesn't target the proper parser");
        }
    } else {
        std::u16string message = u"Package '" + std::u16string(name) + u"' not found.  Omitted.";
        warn(std::move(message));
    }
    return found;
}

void ParserConfiguration::addPackage(Value package) {
    Context& context = runtime_.context();
    std::u16string name;
    const bool stringPackage = std::holds_alternative<std::u16string>(package.data);
    if (stringPackage) name = std::get<std::u16string>(package.data);
    else name = std::get<std::u16string>(index(context, package, 0).data);
    const ParserShared<Configuration> config = getPackage(name);
    if (const auto* found = std::get_if<ConfigurationRef>(&config); found && *found) {
        Value priority = stringPackage ? (*found)->field(u"priority") : index(context, package, 1);
        configurationList()->add(*found, numberPriority(context, priority));
    }
}

void ParserConfiguration::append(ConfigurationRef configuration, Value priorityValue) {
    if (!configuration) throw Exception("TypeError", u"Cannot read properties of null (reading 'priority')");
    if (!truthy(priorityValue)) priorityValue = configuration->field(u"priority");
    if (truthy(configuration->field(u"initMethod"))) {
        const auto list = functionList(u"initMethod");
        const Value tuple = configuration->field(u"initMethod");
        addProcessorProperties(runtime_.context(), runtime_.functions(), *list, tuple);
    }
    if (truthy(configuration->field(u"configMethod"))) {
        const auto list = functionList(u"configMethod");
        const Value tuple = configuration->field(u"configMethod");
        addProcessorProperties(runtime_.context(), runtime_.functions(), *list, tuple);
    }
    const auto handlers = handlerList();
    const Value handlerConfiguration = configuration->field(u"handler");
    const Value fallbackConfiguration = configuration->field(u"fallback");
    const double priority = numberPriority(runtime_.context(), priorityValue);
    handlers->add(runtime_.context(), runtime_.functions(), handlerConfiguration,
                  fallbackConfiguration, priority);
    const Value targetItems = field(u"items");
    assign(runtime_.context(), targetItems, configuration->field(u"items"));
    const Value targetTags = field(u"tags");
    assign(runtime_.context(), targetTags, configuration->field(u"tags"));
    const Value targetOptions = field(u"options");
    const std::array<Value, 1> optionDefinitions{configuration->field(u"options")};
    (void)options::defaultOptions(runtime_.context(), targetOptions, optionDefinitions);
    const Value targetNodes = field(u"nodes");
    assign(runtime_.context(), targetNodes, configuration->field(u"nodes"));
}

void ParserConfiguration::addFilters(TeX& jax, ConfigurationRef configuration) {
    if (!configuration) throw Exception("TypeError", u"Cannot read properties of null (reading 'preprocessors')");
    addFiltersFrom(runtime_.context(), runtime_.functions(), *jax.preFilters,
                   configuration->field(u"preprocessors"));
    addFiltersFrom(runtime_.context(), runtime_.functions(), *jax.postFilters,
                   configuration->field(u"postprocessors"));
}

void ParserConfiguration::warn(std::u16string message) {
    std::u16string output = u"MathJax Warning: " + message;
    if (options::OPTIONS.warningSink) options::OPTIONS.warningSink(std::move(output));
}

void ParserConfiguration::add(std::u16string name, TeX& jax, Value userOptions) {
    const ParserShared<Configuration> loaded = getPackage(name);
    const auto* found = std::get_if<ConfigurationRef>(&loaded);
    if (!found || !*found)
        throw Exception("TypeError", u"Cannot read properties of undefined (reading 'priority')");
    const ConfigurationRef configuration = *found;
    append(configuration);
    const auto configurations = configurationList();
    const Value priority = configuration->field(u"priority");
    configurations->add(configuration, numberPriority(runtime_.context(), priority));
    init();

    ParserShared<ParseOptions> parserValue = jax.parseOptions();
    const auto* parser = std::get_if<ParseOptionsRef>(&parserValue);
    if (!parser || !*parser) throw Exception("TypeError", u"Cannot read properties of null (reading 'nodeFactory')");
    (*parser)->nodeFactoryOwner().setCreators(runtime_.context(), runtime_.properties, configuration->field(u"nodes"));

    for (const auto& kind : runtime_.context().enumerableStringKeys(configuration->field(u"items"))) {
        const Key field(kind);
        const Value classValue = runtime_.context().get(configuration->field(u"items"), field);
        const auto* object = std::get_if<ObjectRef>(&classValue.data);
        if (!object) throw Exception("Unimplemented", u"StackItem class value requires its canonical native identity");
        const StackItemClassRef itemClass = runtime_.itemClassIdentities.resolve(*object);
        const auto* itemFactory = std::get_if<std::shared_ptr<StackItemFactory>>(&(*parser)->itemFactory);
        if (!itemFactory || !*itemFactory) throw Exception("TypeError", u"Cannot read properties of null (reading 'setNodeClass')");
        (*itemFactory)->setNodeClass(property_host::debugText(kind), itemClass);
    }
    runtime_.tagsFactory.addTags(configuration->field(u"tags"));
    const std::array<Value, 1> defaults{configuration->field(u"options")};
    (void)options::defaultOptions(runtime_.context(), (*parser)->options, defaults);
    const std::array<Value, 1> supplied{std::move(userOptions)};
    (void)options::userOptions(runtime_.context(), (*parser)->options, supplied);
    addFilters(jax, configuration);

    const Value callback = configuration->config();
    if (truthy(callback)) {
        const FunctionValue callable = runtime_.functions().functionValue(callback);
        const FunctionRef function = functions::reference(callable);
        if (!function || !function->call) throw Exception("TypeError", u"config.config is not callable");
        const auto self = shared_from_this();
        const std::array<std::any, 2> args{std::any(self),
                                           std::any(static_cast<TeX*>(&jax))};
        const FunctionReceiver receiver(std::static_pointer_cast<NativeObject>(self));
        (void)function->call(receiver, args);
    }
}

} // namespace folia::math::tex
