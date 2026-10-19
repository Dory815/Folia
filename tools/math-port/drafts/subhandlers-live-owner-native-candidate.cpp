// Focused native observation of the live SubHandlers/SubHandler owner graph.
// All reported values are read from actual Context objects and native owners.
import std;
import folia.math.input.tex.handler_context_bindings;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
using namespace folia::math::options;
namespace tex = folia::math::tex;

struct Parser final : tex::ParserContext {
    Property current = Missing{};
    Property currentCS() const override { return current; }
    void setCurrentCS(Property value) override { current = std::move(value); }
};

struct Regex final : tex::RegExpContext {
    bool test(const Property&) override { return false; }
};

Value call(Context& context, Value receiver, std::u16string_view method,
           std::span<const Value> arguments = {}) {
    return context.callMember(context.get(receiver, Key(std::u16string(method))),
                              receiver, arguments, method);
}

Value member(Context& context, Value receiver, std::u16string_view name) {
    return context.get(receiver, Key(std::u16string(name)));
}

std::optional<Value> iteratorNext(Context& context, ObjectRef iterator) {
    const Value step = call(context, Value(iterator), u"next");
    if (std::get<bool>(member(context, step, u"done").data)) return {};
    return member(context, step, u"value");
}

}

int main() {
    try {
        Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context, functions);
        MapContextBindings maps(context);
        FunctionListContextBindings functionLists(context, functions);
        tex::MapHandler registry;
        Parser parser;
        Regex regex;
        auto executionLease = std::make_shared<std::atomic_bool>(true);
        const SymbolRef fallbackSymbol = context.symbol(u"fallback");

        tex::TokenMapContextHooks hooks;
        hooks.constructMap = [&](tex::TokenMapClass, Context&, std::span<const Value>) {
            return tex::makeAbstractTokenMap(registry, Property("observer-map"),
                                             tex::ParseMethod{Property(PropertyNull{})});
        };
        hooks.constructToken = [](Context&, std::span<const Value>) -> tex::TokenRef {
            throw Exception("Unimplemented", u"Observer does not construct Token values");
        };
        hooks.constructMacro = [](Context&, std::span<const Value>) -> tex::MacroRef {
            throw Exception("Unimplemented", u"Observer does not construct Macro values");
        };
        hooks.parseMethodFromValue = [&](Context&, Value value) -> tex::ParseMethod {
            return functions.functionValue(std::move(value));
        };
        hooks.parseMethodToValue = [&](Context&, const tex::ParseMethod& method) -> Value {
            if (const auto* property = std::get_if<Property>(&method)) return properties.to(*property);
            const auto object = functions.find(std::get<FunctionRef>(method));
            if (!object) throw Exception("Unimplemented", u"ParseMethod has no Context identity");
            return Value(*object);
        };
        hooks.regexpFromValue = [](Context&, Value) -> tex::RegExpRef { return {}; };
        hooks.regexpToValue = [](Context&, const tex::RegExpRef&) -> Value { return Value(Null{}); };
        hooks.parseInputFromValue = [&](Context&, Value value) -> tex::ParseInput {
            return {parser, properties.from(std::move(value))};
        };
        hooks.parseResultToValue = [&](Context&, tex::ParseResult result) -> Value {
            if (const auto* property = std::get_if<Property>(&result)) return properties.to(*property);
            if (std::holds_alternative<tex::StackItemRef>(result)) return Value(Null{});
            const auto symbol = std::get<tex::ParseSymbolRef>(std::move(result));
            return Value(symbol && symbol->description == u"fallback"
                ? fallbackSymbol : context.symbol(symbol ? symbol->description : u""));
        };

        tex::TokenMapContextBindings tokenMaps(
            context, functions, properties, maps, std::move(hooks), executionLease);
        auto bindings = std::make_unique<tex::SubHandlerContextBindings>(
            context, properties, maps, tokenMaps, functionLists, functions, registry);

        const ObjectRef handlersObject = context.construct(bindings->subHandlersConstructor());
        const Value handlers(handlersObject);
        const ObjectRef firstHandler = context.construct(bindings->subHandlerConstructor());
        const ObjectRef secondHandler = context.construct(bindings->subHandlerConstructor());

        const auto handlerPrototypeKeys=context.ownKeys(Value(bindings->subHandlerPrototype()));
        const std::vector<Key> expectedHandlerPrototypeKeys{
            Key(u"constructor"),Key(u"add"),Key(u"remove"),Key(u"parse"),Key(u"lookup"),
            Key(u"contains"),Key(u"toString"),Key(u"applicable"),Key(u"retrieve"),Key(u"warn")};
        const auto warnDescriptor=context.descriptor(Value(bindings->subHandlerPrototype()),Key(u"warn"));
        const bool warnDescriptorMatches=warnDescriptor&&warnDescriptor->writable&&
            warnDescriptor->enumerable&&warnDescriptor->configurable&&!warnDescriptor->accessor;
        const Value warnValue=context.get(Value(bindings->subHandlerPrototype()),Key(u"warn"));
        const auto* warnFunction=std::get_if<ObjectRef>(&warnValue.data);
        const bool warnFunctionMatches=warnFunction&&context.kind(*warnFunction)==ObjectKind::function&&
            context.get(warnValue,Key(u"name"))==Value(u"")&&
            context.get(warnValue,Key(u"length"))==Value(1);
        const std::array<std::pair<std::u16string_view,double>,9> expectedMethods{{
            {u"add",3},{u"remove",2},{u"parse",1},{u"lookup",1},{u"contains",1},
            {u"toString",0},{u"applicable",1},{u"retrieve",1},{u"warn",1}}};
        bool completeDescriptorSurface=true;
        for(const auto& [name,length]:expectedMethods){
            const Key key{std::u16string(name)};
            const auto descriptor=context.descriptor(Value(bindings->subHandlerPrototype()),key);
            if(!descriptor||descriptor->accessor||!descriptor->writable||!descriptor->enumerable||
               !descriptor->configurable){completeDescriptorSurface=false;break;}
            const auto* function=std::get_if<ObjectRef>(&descriptor->value.data);
            if(!function||context.kind(*function)!=ObjectKind::function||
               context.get(Value(*function),Key(u"name"))!=Value(u"")||
               context.get(Value(*function),Key(u"length"))!=Value(length)){
                completeDescriptorSurface=false;break;
            }
        }
        const bool completeSubHandlerPrototype=handlerPrototypeKeys==expectedHandlerPrototypeKeys&&
            warnDescriptorMatches&&warnFunctionMatches&&completeDescriptorSurface;
        if(!completeSubHandlerPrototype){
            if(handlerPrototypeKeys!=expectedHandlerPrototypeKeys){
                const auto count=std::min(handlerPrototypeKeys.size(),expectedHandlerPrototypeKeys.size());
                std::size_t index=0;
                while(index<count&&handlerPrototypeKeys[index]==expectedHandlerPrototypeKeys[index])++index;
                std::cerr<<"SubHandler prototype key mismatch at "<<index;
                if(index<expectedHandlerPrototypeKeys.size())
                    std::cerr<<" expected="<<property_host::debugText(context.keyString(expectedHandlerPrototypeKeys[index]));
                if(index<handlerPrototypeKeys.size())
                    std::cerr<<" actual="<<property_host::debugText(context.keyString(handlerPrototypeKeys[index]));
                std::cerr<<'\n';
            }
            for(const auto& [name,length]:expectedMethods){
                const Key key{std::u16string(name)};
                const auto descriptor=context.descriptor(Value(bindings->subHandlerPrototype()),key);
                if(!descriptor){std::cerr<<"SubHandler method "<<property_host::debugText(name)<<" descriptor missing\n";break;}
                const auto* function=std::get_if<ObjectRef>(&descriptor->value.data);
                if(!function){std::cerr<<"SubHandler method "<<property_host::debugText(name)<<" value is not function\n";break;}
                const Value actualName=context.get(Value(*function),Key(u"name"));
                const Value actualLength=context.get(Value(*function),Key(u"length"));
                const auto* actualNameText=std::get_if<std::u16string>(&actualName.data);
                const auto* actualLengthNumber=std::get_if<double>(&actualLength.data);
                const bool matches=descriptor->writable&&descriptor->enumerable&&descriptor->configurable&&
                    !descriptor->accessor&&context.kind(*function)==ObjectKind::function&&
                    actualNameText&&actualNameText->empty()&&actualLengthNumber&&*actualLengthNumber==length;
                if(!matches){
                    std::cerr<<"SubHandler method "<<property_host::debugText(name)
                        <<" expected name='' length="<<length<<" flags=111; actual name='"
                        <<(actualNameText?property_host::debugText(*actualNameText):"<non-string>")
                        <<"' length="<<(actualLengthNumber?*actualLengthNumber:-1)
                        <<" flags="<<descriptor->writable<<descriptor->enumerable<<descriptor->configurable
                        <<" accessor="<<descriptor->accessor<<'\n';
                    break;
                }
            }
            throw Exception("AssertionError",u"SubHandler Source prototype surface mismatch");
        }
        std::u16string observedWarning;
        const auto warningOwner=std::make_shared<tex::SubHandler>(registry,
            [&](const std::u16string& value){observedWarning=value;});
        const Value warningReceiver(bindings->bind(warningOwner));
        const std::array<Value,1> warningArgument{Value(u"probe")};
        (void)call(context,warningReceiver,u"warn",warningArgument);
        const bool warnDelegatesToNative=observedWarning==u"TexParser Warning: probe";
        if(!warnDelegatesToNative)
            throw Exception("AssertionError",u"SubHandler.warn did not use the native warning path");
        const auto warningObject=context.object();
        context.set(warningObject,Key(u"valueOf"),Value(context.function(u"valueOf",[](Context&,Value,std::span<const Value>){return Value(u"value");})));
        context.set(warningObject,Key(u"toString"),Value(context.function(u"toString",[](Context&,Value,std::span<const Value>){return Value(u"text");})));
        const std::array<Value,1> objectWarningArgument{Value(warningObject)};
        (void)call(context,warningReceiver,u"warn",objectWarningArgument);
        const bool warnDefaultPrimitiveHint=observedWarning==u"TexParser Warning: value";
        if(!warnDefaultPrimitiveHint)
            throw Exception("AssertionError",u"SubHandler.warn did not preserve concatenation conversion order");

        const ObjectRef objectKey = context.object();
        const std::array<std::pair<Value, Value>, 3> entries{
            std::pair{Value(7), Value(firstHandler)},
            std::pair{Value(u"7"), Value(secondHandler)},
            std::pair{Value(objectKey), Value(firstHandler)}};
        for (const auto& [key, handler] : entries) {
            const std::array<Value, 2> arguments{key, handler};
            (void)call(context, handlers, u"set", arguments);
        }
        const bool numericIdentity = call(context, handlers, u"get", std::span<const Value>(&entries[0].first, 1)) == entries[0].second;
        const bool stringIdentity = call(context, handlers, u"get", std::span<const Value>(&entries[1].first, 1)) == entries[1].second;
        const bool objectIdentity = call(context, handlers, u"get", std::span<const Value>(&entries[2].first, 1)) == entries[2].second;
        const bool numericStringDistinct = entries[0].second != entries[1].second;

        const Value mapValue = member(context, handlers, u"map");
        const auto* mapObject = std::get_if<ObjectRef>(&mapValue.data);
        if (!mapObject) throw Exception("AssertionError", u"SubHandlers.map is not a Context Map object");
        const bool mapOwnerIdentity = maps.get(*mapObject, entries[0].first) == entries[0].second;

        const Value keyIteratorValue = call(context, handlers, u"keys");
        const ObjectRef keyIterator = std::get<ObjectRef>(keyIteratorValue.data);
        const auto firstYield = iteratorNext(context, keyIterator);
        const ObjectRef lateHandler = context.construct(bindings->subHandlerConstructor());
        const std::array<Value, 2> lateEntry{Value(u"late"), Value(lateHandler)};
        (void)call(context, handlers, u"set", lateEntry);
        std::vector<Value> remainingKeys;
        while (const auto key = iteratorNext(context, keyIterator)) remainingKeys.push_back(*key);

        const ObjectRef firstHandlerObject = firstHandler;
        const Value firstHandlerValue(firstHandlerObject);
        const Value originalConfiguration = member(context, firstHandlerValue, u"_configuration");
        const Value secondConfiguration = member(context, Value(secondHandler), u"_configuration");
        const tex::TokenMapRef nativeMap = tex::makeAbstractTokenMap(
            registry, Property("owner-record"), tex::ParseMethod{Property(PropertyNull{})});
        const Value nativeMapObject(tokenMaps.bind(nativeMap));
        const std::array<Value, 2> addMap{nativeMapObject, Value(3)};
        (void)call(context, originalConfiguration, u"add", addMap);
        const Value originalItems = member(context, originalConfiguration, u"items");
        const Value originalRecord = context.get(originalItems, Key(u"0"));
        context.set(firstHandlerValue, Key(u"_configuration"), secondConfiguration);
        const bool replacementUsesNewOwner = member(context, firstHandlerValue, u"_configuration") == secondConfiguration;
        const bool oldRecordStillBound = context.get(originalItems, Key(u"0")) == originalRecord;
        context.set(firstHandlerValue, Key(u"_configuration"), originalConfiguration);
        const bool originalOwnerRestored = member(context, firstHandlerValue, u"_configuration") == originalConfiguration;

        const Value originalFallback = member(context, firstHandlerValue, u"_fallback");
        const ObjectRef replacementFallbackNative = functionLists.bind(std::make_shared<FunctionList>());
        const Value replacementFallback(replacementFallbackNative);
        context.set(firstHandlerValue, Key(u"_fallback"), replacementFallback);
        const bool fallbackReplacementUsesOwner = member(context, firstHandlerValue, u"_fallback") == replacementFallback;
        context.set(firstHandlerValue, Key(u"_fallback"), originalFallback);
        const bool fallbackOwnerRestored = member(context, firstHandlerValue, u"_fallback") == originalFallback;

        const std::array<Value, 7> required{
            Value(numericIdentity), Value(stringIdentity), Value(objectIdentity),
            Value(numericStringDistinct), Value(mapOwnerIdentity), Value(replacementUsesNewOwner),
            Value(oldRecordStillBound && originalOwnerRestored && fallbackReplacementUsesOwner && fallbackOwnerRestored)};
        if (std::any_of(required.begin(), required.end(), [](const Value& value) {
                return !std::get<bool>(value.data);
            })) {
            throw Exception("AssertionError", u"SubHandlers live owner identity witness failed");
        }

        // Standalone bindings must own a lease that invalidates retained
        // Context callbacks when the binding wrapper is destroyed.
        bindings.reset();
        bool standaloneLeaseExpired = false;
        try {
            (void)member(context, firstHandlerValue, u"_configuration");
        } catch (const Exception& error) {
            standaloneLeaseExpired = error.name == "TypeError";
        }
        if (!standaloneLeaseExpired)
            throw Exception("AssertionError", u"SubHandler standalone lease did not expire");

        std::cout << "{\"schema\":1,\"nativeChecks\":{";
        std::cout << "\"numericKeyIdentity\":" << (numericIdentity ? "true" : "false")
                  << ",\"stringKeyIdentity\":" << (stringIdentity ? "true" : "false")
                  << ",\"objectKeyIdentity\":" << (objectIdentity ? "true" : "false")
                  << ",\"numericStringDistinct\":" << (numericStringDistinct ? "true" : "false")
                  << ",\"mapProjectionIdentity\":" << (mapOwnerIdentity ? "true" : "false")
                  << ",\"firstYieldType\":\""
                  << (std::holds_alternative<double>(firstYield->data) ? "number" : "other")
                  << "\",\"lateKeySeenByLiveIterator\":"
                  << (std::any_of(remainingKeys.begin(), remainingKeys.end(), [&](const Value& key) {
                        return key == Value(u"late");
                     }) ? "true" : "false")
                  << ",\"configurationReplacement\":" << (replacementUsesNewOwner ? "true" : "false")
                  << ",\"oldRecordStillBound\":" << (oldRecordStillBound ? "true" : "false")
                  << ",\"configurationRestored\":" << (originalOwnerRestored ? "true" : "false")
                  << ",\"fallbackReplacement\":" << (fallbackReplacementUsesOwner ? "true" : "false")
                  << ",\"fallbackRestored\":" << (fallbackOwnerRestored ? "true" : "false")
                  << ",\"standaloneLeaseExpired\":" << (standaloneLeaseExpired ? "true" : "false")
                  << ",\"completeSubHandlerPrototype\":" << (completeSubHandlerPrototype ? "true" : "false")
                  << ",\"warnFunctionMetadata\":" << (warnFunctionMatches ? "true" : "false")
                  << ",\"warnDescriptor\":" << (warnDescriptorMatches ? "true" : "false")
                  << ",\"completeDescriptorSurface\":" << (completeDescriptorSurface ? "true" : "false")
                  << ",\"warnDelegatesToNative\":" << (warnDelegatesToNative ? "true" : "false")
                  << ",\"warnDefaultPrimitiveHint\":" << (warnDefaultPrimitiveHint ? "true" : "false")
                  << "}}\n";
        return 0;
    } catch (const Exception& error) {
        std::cerr << error.name << ": " << property_host::debugText(error.message) << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "std::exception: " << error.what() << '\n';
        return 1;
    }
}
