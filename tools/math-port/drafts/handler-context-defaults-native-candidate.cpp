// Focused Context witness for the shared PrioritizedList/FunctionList default.
// Every emitted value is read back from the actual bound Context records.
import std;
import folia.math.util.function_list_bindings;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
using namespace folia::math::options;

Value member(Context& context, Value receiver, std::u16string_view name) {
    return context.get(receiver, Key(std::u16string(name)));
}

double firstPriority(Context& context, Value instance) {
    const Value items = member(context, instance, u"items");
    const Value record = context.get(items, Key(u"0"));
    return context.toNumber(member(context, record, u"priority"));
}

struct Run {
    double defaultValue = 0;
    double prioritizedAdd = 0;
    double functionAdd = 0;
    double functionAddList = 0;
    double functionConstructorExplicit = 0;
    double functionConstructorDefault = 0;
};

Run observe(double defaultValue) {
    Context context;
    OptionFunctionBindings functions(context);
    FunctionListContextBindings functionLists(context, functions);
    auto& prioritized = functionLists.prioritizedLists();
    const ObjectRef priorityConstructor = prioritized.constructor();
    const ObjectRef functionConstructor = functionLists.constructor();
    context.set(Value(priorityConstructor), Key(u"DEFAULTPRIORITY"), Value(defaultValue));

    const ObjectRef list = context.construct(priorityConstructor);
    const std::array<Value, 1> addArgument{Value(u"priority-item")};
    const Value listAdd = member(context, Value(list), u"add");
    (void)context.callMember(listAdd, Value(list), addArgument, u"prioritized.add");

    const ObjectRef functionList = context.construct(functionConstructor);
    const Value functionAdd = member(context, Value(functionList), u"add");
    const std::array<Value, 1> directArgument{Value(u"function-item")};
    (void)context.callMember(functionAdd, Value(functionList), directArgument, u"functionList.add");

    const Value addList = member(context, Value(functionList), u"addList");
    const ObjectRef listDefinitions = context.array(1);
    context.define(listDefinitions, Key(u"0"), Descriptor::data(Value(u"add-list-item")));
    const std::array<Value, 1> listArgument{Value(listDefinitions)};
    (void)context.callMember(addList, Value(functionList), listArgument, u"functionList.addList");
    const Value functionItems = member(context, Value(functionList), u"items");
    const Value secondRecord = context.get(functionItems, Key(u"1"));
    const double addListPriority = context.toNumber(member(context, secondRecord, u"priority"));

    const ObjectRef tuple = context.array(2);
    context.define(tuple, Key(u"0"), Descriptor::data(Value(u"explicit-priority")));
    context.define(tuple, Key(u"1"), Descriptor::data(Value(2)));
    const ObjectRef constructorDefinitions = context.array(2);
    context.define(constructorDefinitions, Key(u"0"), Descriptor::data(Value(tuple)));
    context.define(constructorDefinitions, Key(u"1"), Descriptor::data(Value(u"constructor-default")));
    const std::array<Value, 1> constructorArgument{Value(constructorDefinitions)};
    const ObjectRef constructedList = context.construct(functionConstructor, constructorArgument);
    const Value constructedItems = member(context, Value(constructedList), u"items");
    const Value explicitRecord = context.get(constructedItems, Key(u"0"));
    const Value defaultRecord = context.get(constructedItems, Key(u"1"));

    return {
        context.toNumber(member(context, Value(priorityConstructor), u"DEFAULTPRIORITY")),
        firstPriority(context, Value(list)),
        firstPriority(context, Value(functionList)),
        addListPriority,
        context.toNumber(member(context, explicitRecord, u"priority")),
        context.toNumber(member(context, defaultRecord, u"priority"))
    };
}

bool functionListStandaloneLeaseExpires() {
    Context context;
    OptionFunctionBindings functions(context);
    ObjectRef constructor;
    {
        FunctionListContextBindings bindings(context, functions);
        constructor = bindings.constructor();
    }
    try {
        (void)context.construct(constructor);
    } catch (const Exception& error) {
        return error.name == "TypeError";
    }
    return false;
}

bool prioritizedListStandaloneLeaseExpires() {
    Context context;
    ObjectRef constructor;
    {
        PrioritizedListContextBindings bindings(context);
        constructor = bindings.constructor();
    }
    try {
        (void)context.construct(constructor);
    } catch (const Exception& error) {
        return error.name == "TypeError";
    }
    return false;
}

bool functionListExternalLeaseExpires() {
    Context context;
    OptionFunctionBindings functions(context);
    auto lease = std::make_shared<std::atomic_bool>(true);
    ObjectRef constructor;
    {
        FunctionListContextBindings bindings(context, functions, lease);
        constructor = bindings.constructor();
    }
    lease->store(false, std::memory_order_release);
    try {
        (void)context.construct(constructor);
    } catch (const Exception& error) {
        return error.name == "TypeError";
    }
    return false;
}
}

int main() {
    try {
        const auto first = observe(13);
        const auto second = observe(4);
        const bool functionListLeaseExpired = functionListStandaloneLeaseExpires();
        const bool prioritizedListLeaseExpired = prioritizedListStandaloneLeaseExpires();
        const bool functionListExternalLeaseExpired = functionListExternalLeaseExpires();
        if (first.prioritizedAdd != 13 || first.functionAdd != 13 ||
            first.functionAddList != 13 || first.functionConstructorExplicit != 2 ||
            first.functionConstructorDefault != 13 || second.prioritizedAdd != 4 ||
            second.functionAdd != 4 || second.functionAddList != 4 ||
            second.functionConstructorDefault != 4 || !functionListLeaseExpired ||
            !prioritizedListLeaseExpired || !functionListExternalLeaseExpired) {
            throw folia::math::options::Exception("AssertionError", u"per-Context PrioritizedList defaults diverged");
        }
        std::cout << "{\"schema\":1,\"nativeChecks\":{";
        std::cout << "\"functionListStandaloneLeaseExpired\":"
                  << (functionListLeaseExpired ? "true" : "false")
                  << ",\"prioritizedListStandaloneLeaseExpired\":"
                  << (prioritizedListLeaseExpired ? "true" : "false")
                  << ",\"functionListExternalLeaseExpired\":"
                  << (functionListExternalLeaseExpired ? "true" : "false")
                  << "},\"first\":{";
        std::cout << "\"default\":" << first.defaultValue
                  << ",\"prioritizedAdd\":" << first.prioritizedAdd
                  << ",\"functionAdd\":" << first.functionAdd
                  << ",\"functionAddList\":" << first.functionAddList
                  << ",\"functionConstructorExplicit\":" << first.functionConstructorExplicit
                  << ",\"functionConstructorDefault\":" << first.functionConstructorDefault
                  << "},\"second\":{";
        std::cout << "\"default\":" << second.defaultValue
                  << ",\"prioritizedAdd\":" << second.prioritizedAdd
                  << ",\"functionAdd\":" << second.functionAdd
                  << ",\"functionAddList\":" << second.functionAddList
                  << ",\"functionConstructorExplicit\":" << second.functionConstructorExplicit
                  << ",\"functionConstructorDefault\":" << second.functionConstructorDefault
                  << "}}\n";
        return 0;
    } catch (const folia::math::options::Exception& error) {
        std::cerr << error.name << ": " << folia::math::property_host::debugText(error.message) << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "std::exception: " << error.what() << '\n';
        return 1;
    }
}
