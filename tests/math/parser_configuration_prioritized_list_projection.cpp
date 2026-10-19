#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.util.option_functions;
import folia.math.util.regexp_host;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o = folia::math::options;
namespace t = folia::math::tex;

suite parser_configuration_prioritized_list_projection = [] {
  "configurations_projects_the_actual_prioritized_list_owner"_test = [] {
    o::Context context;
    OptionFunctionBindings functions(context);
    regexp_host::RegExpBindings regexp(context);
    Unicode16Normalization unicode;
    core::InputJaxModule input(context);
    core::FindMathModule findMath(context);
    t::FindTeXModule findTeX(context, findMath, regexp);
    t::ParserRuntime runtime(context, functions, regexp, unicode, input, findTeX);
    runtime.initializeParserRuntime();

    const auto constructor = std::get<o::ObjectRef>(runtime.parserConfigurationClass.data);
    const auto packageNames = context.array();
    const auto parserObject = context.construct(constructor, std::array<o::Value, 1>{o::Value(packageNames)});
    const auto parser = runtime.parserConfigurationIdentities.resolve(parserObject);
    auto registered = t::Configuration::create(runtime, u"projection-test", o::Value(context.object()));
    parser->addPackage(o::Value(u"projection-test"));

    const auto list = parser->sourceField(u"configurations");
    expect(std::ranges::equal(context.ownKeys(list), std::array<o::Key, 1>{o::Key(u"items")}));
    const auto listPrototype = context.prototype(std::get<o::ObjectRef>(list.data));
    expect(listPrototype.has_value());
    expect(*listPrototype == runtime.functionLists.prioritizedLists().prototype());
    const auto listItemsDescriptor = context.descriptor(list, o::Key(u"items"));
    expect(listItemsDescriptor.has_value());
    expect(listItemsDescriptor->writable && listItemsDescriptor->enumerable && listItemsDescriptor->configurable);

    const auto itemsBefore = context.get(list, o::Key(u"items"));
    const auto itemsIdentity = std::get<o::ObjectRef>(itemsBefore.data);
    expect(context.toNumber(context.get(itemsBefore, o::Key(u"length"))) == 1.0);
    const auto first = context.get(itemsBefore, o::Key(u"0"));
    expect(std::ranges::equal(context.ownKeys(first),
        std::array<o::Key, 2>{o::Key(u"item"), o::Key(u"priority")}));
    const auto registeredIdentity = runtime.configurationIdentities.find(registered);
    expect(registeredIdentity.has_value());
    expect(context.get(first, o::Key(u"item")) == o::Value(*registeredIdentity));
    expect(context.toNumber(context.get(first, o::Key(u"priority"))) == 5.0);

    const auto local = t::Configuration::local(runtime, o::Value(context.object()));
    const auto localIdentity = runtime.configurationIdentities.find(local);
    expect(localIdentity.has_value());
    const auto add = context.get(list, o::Key(u"add"));
    (void)context.callMember(add, list,
        std::array<o::Value, 2>{o::Value(*localIdentity), o::Value(2.0)});
    expect(context.get(list, o::Key(u"items")) == itemsBefore);
    expect(context.toNumber(context.get(itemsBefore, o::Key(u"length"))) == 2.0);
    const auto firstAfterAdd = context.get(itemsBefore, o::Key(u"0"));
    expect(context.get(firstAfterAdd, o::Key(u"item")) == o::Value(*localIdentity));
    expect(context.toNumber(context.get(firstAfterAdd, o::Key(u"priority"))) == 2.0);
    const auto iteratorMethod = context.get(list, o::Key(context.intrinsics().symbolIterator));
    const auto iterator = context.callMember(iteratorMethod, list);
    const auto step = context.callMember(context.get(iterator, o::Key(u"next")), o::Value(iterator));
    expect(context.get(step, o::Key(u"value")) == firstAfterAdd);
    expect(!std::get<bool>(context.get(step, o::Key(u"done")).data));

    (void)context.callMember(add, list,
        std::array<o::Value, 2>{o::Value(*registeredIdentity), o::Value(true)});
    expect(context.toNumber(context.get(itemsBefore, o::Key(u"length"))) == 3.0);
    const auto booleanPriorityRecord = context.get(itemsBefore, o::Key(u"0"));
    expect(context.get(booleanPriorityRecord, o::Key(u"priority")) == o::Value(true));
    context.set(firstAfterAdd, o::Key(u"priority"), o::Value(true));
    expect(context.get(firstAfterAdd, o::Key(u"priority")) == o::Value(true));

    const auto typedOwner = std::make_shared<PrioritizedList<t::ConfigurationRef>>();
    const auto firstProjection = t::bindConfigurationPriorityList(runtime, typedOwner);
    const auto firstArray = context.get(o::Value(firstProjection), o::Key(u"items"));
    typedOwner->add(registered, 3.0);
    const auto recordBeforeRebind = context.get(firstArray, o::Key(u"0"));
    const auto secondProjection = t::bindConfigurationPriorityList(runtime, typedOwner);
    const auto secondArray = context.get(o::Value(secondProjection), o::Key(u"items"));
    const auto recordAfterRebind = context.get(secondArray, o::Key(u"0"));
    expect(firstProjection == secondProjection);
    expect(firstArray == secondArray);
    expect(recordBeforeRebind == recordAfterRebind);
    typedOwner->add(local, 1.0);
    expect(context.toNumber(context.get(firstArray, o::Key(u"length"))) == 2.0);
    expect(context.get(context.get(firstArray, o::Key(u"0")), o::Key(u"item")) == o::Value(*localIdentity));

    context.define(std::get<o::ObjectRef>(firstArray.data), o::Key(u"length"),
        o::Descriptor::data(o::Value(2.0), false, false, false));
    const auto lockedLength = context.descriptor(firstArray, o::Key(u"length"));
    expect(lockedLength.has_value() && !lockedLength->writable);
    bool growthBlocked = false;
    try {
      typedOwner->add(registered, 0.0);
    } catch (const o::Exception& error) {
      growthBlocked = error.name == "TypeError";
    }
    expect(growthBlocked);
    expect(context.toNumber(context.get(firstArray, o::Key(u"length"))) == 2.0);

    // Source can rebind items from the old array-like's length getter while
    // add() is still constructing its record for the new array's splice.
    const auto listObject = std::get<o::ObjectRef>(list.data);
    const auto nativeList = std::any_cast<std::shared_ptr<PrioritizedList<t::ConfigurationRef>>>(
        context.payload(listObject));
    const auto sourceItems = context.object();
    const auto replacementItems = context.object();
    context.set(replacementItems, o::Key(u"length"), o::Value(0.0));
    std::optional<o::Value> spliceReceiver;
    std::vector<o::Value> spliceArguments;
    const auto splice = context.function(u"splice", [&](o::Context&, o::Value receiver,
        std::span<const o::Value> arguments) {
      spliceReceiver = std::move(receiver);
      spliceArguments.assign(arguments.begin(), arguments.end());
      return o::Value(o::Undefined{});
    });
    context.set(replacementItems, o::Key(u"splice"), o::Value(splice));
    context.set(list, o::Key(u"items"), o::Value(sourceItems));
    const std::weak_ptr<PrioritizedListItemsAccess<t::ConfigurationRef>> activeItemsAccess =
        nativeList->itemsAccess();
    bool accessRetainedDuringGetter = false;
    const auto lengthGetter = context.function(u"length", [&](o::Context& cx, o::Value,
        std::span<const o::Value>) {
      cx.set(list, o::Key(u"items"), o::Value(replacementItems));
      accessRetainedDuringGetter = !activeItemsAccess.expired();
      return o::Value(0.0);
    });
    context.define(sourceItems, o::Key(u"length"),
        o::Descriptor::access(lengthGetter, {}, true, true));
    (void)context.callMember(add, list,
        std::array<o::Value, 2>{o::Value(*localIdentity), o::Value(7.0)});
    expect(accessRetainedDuringGetter);
    expect(activeItemsAccess.expired());
    expect(context.get(list, o::Key(u"items")) == o::Value(replacementItems));
    expect(spliceReceiver == std::optional<o::Value>(o::Value(replacementItems)));
    expect(spliceArguments.size() == 3);
    expect(spliceArguments[0] == o::Value(0.0) && spliceArguments[1] == o::Value(0.0));
    expect(context.get(spliceArguments[2], o::Key(u"item")) == o::Value(*localIdentity));
    expect(context.get(spliceArguments[2], o::Key(u"priority")) == o::Value(7.0));
  };
};
} // namespace
