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

struct RuntimeFixture {
    o::Context context;
    OptionFunctionBindings functions{context};
    regexp_host::RegExpBindings regexp{context};
    Unicode16Normalization unicode;
    core::InputJaxModule input{context};
    core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context, findMath, regexp};
    t::ParserRuntime runtime{context, functions, regexp, unicode, input, findTeX};
    RuntimeFixture() { runtime.initializeParserRuntime(); }
};

suite parser_configuration_prioritized_list_live_array = [] {
  "configuration_list_uses_replaceable_live_source_array_owner"_test = [] {
    RuntimeFixture fixture;
    auto& context = fixture.context;
    const auto constructor = std::get<o::ObjectRef>(fixture.runtime.parserConfigurationClass.data);
    const auto packageNames = context.array();
    const auto parserObject = context.construct(constructor,
        std::array<o::Value, 1>{o::Value(packageNames)});
    const auto parser = fixture.runtime.parserConfigurationIdentities.resolve(parserObject);
    auto registered = t::Configuration::create(fixture.runtime, u"array-owner-test", o::Value(context.object()));
    parser->addPackage(o::Value(u"array-owner-test"));
    const o::Value list = parser->sourceField(u"configurations");
    const auto nativeList = std::any_cast<std::shared_ptr<PrioritizedList<t::ConfigurationRef>>>(
        &context.payload(std::get<o::ObjectRef>(list.data)));
    expect(nativeList != nullptr);
    const o::Value originalArray = context.get(list, o::Key(u"items"));

    const o::Value iteratorMethod = context.get(list, o::Key(context.intrinsics().symbolIterator));
    const o::Value iterator = context.callMember(iteratorMethod, list);
    const o::Value replacement = context.array();
    context.set(list, o::Key(u"items"), replacement);
    expect(context.get(list, o::Key(u"items")) == replacement);
    expect(context.get(originalArray, o::Key(u"length")) == o::Value(1.0));

    const auto local = t::Configuration::local(fixture.runtime, o::Value(context.object()));
    const auto localIdentity = fixture.runtime.configurationIdentities.find(local);
    expect(localIdentity.has_value());
    const auto add = context.get(list, o::Key(u"add"));
    const o::Value returnValue = context.callMember(add, list,
        std::array<o::Value, 2>{o::Value(*localIdentity), o::Value(3.0)});
    expect(returnValue == o::Value(*localIdentity));
    expect(context.get(replacement, o::Key(u"length")) == o::Value(1.0));
    const o::Value inserted = context.get(replacement, o::Key(u"0"));
    expect(context.get(inserted, o::Key(u"item")) == o::Value(*localIdentity));
    expect(context.get(inserted, o::Key(u"priority")) == o::Value(3.0));
    const auto remove = context.get(list, o::Key(u"remove"));
    expect(context.callMember(remove, list, std::array<o::Value, 1>{o::Value(*localIdentity)}) == list);
    expect(context.get(replacement, o::Key(u"length")) == o::Value(0.0));

    const o::Value oldStep = context.callMember(context.get(iterator, o::Key(u"next")), iterator);
    expect(context.get(oldStep, o::Key(u"value")) == context.get(originalArray, o::Key(u"0")));

    const o::ObjectRef sparseObject = context.array(3);
    const o::ObjectRef sparseRecord = context.object();
    context.define(sparseRecord, o::Key(u"item"), o::Descriptor::data(o::Value(*localIdentity)));
    context.define(sparseRecord, o::Key(u"priority"), o::Descriptor::data(o::Value(u"raw-priority")));
    context.set(o::Value(sparseObject), o::Key(u"1"), o::Value(sparseRecord));
    context.set(list, o::Key(u"items"), o::Value(sparseObject));
    bool holeReadThrowsTypeError = false;
    try {
      (void)context.callMember(add, list,
          std::array<o::Value, 2>{o::Value(*localIdentity), o::Value(1.0)});
    } catch (const o::Exception& error) {
      holeReadThrowsTypeError = error.name == "TypeError";
    }
    expect(holeReadThrowsTypeError);
    expect(!context.hasOwn(o::Value(sparseObject), o::Key(u"0")));
    expect(context.get(o::Value(sparseObject), o::Key(u"length")) == o::Value(3.0));
    expect(!(*nativeList)->itemsAccess()->read(0).present());

    const o::ObjectRef ownUndefinedArray = context.array(1);
    context.set(o::Value(ownUndefinedArray), o::Key(u"0"), o::Value(o::Undefined{}));
    context.set(list, o::Key(u"items"), o::Value(ownUndefinedArray));
    const auto ownUndefinedSlot = (*nativeList)->itemsAccess()->read(0);
    expect(ownUndefinedSlot.present());
    expect(std::holds_alternative<o::Undefined>(context.get(o::Value(ownUndefinedArray), o::Key(u"0")).data));

    // Exercise the real ParserConfiguration consumer. Source reads slot 0,
    // whose getter sets length to zero, then reads length and stops without
    // trying to resolve the returned undefined configuration.
    std::vector<int> accessOrder;
    double liveLength = 1;
    const o::ObjectRef arrayLike = context.object();
    const auto lengthGetter = context.function(u"get length",
        [&](o::Context&, o::Value, std::span<const o::Value>) {
            accessOrder.push_back(2);
            return o::Value(liveLength);
        });
    const auto indexGetter = context.function(u"get 0",
        [&](o::Context&, o::Value, std::span<const o::Value>) {
            accessOrder.push_back(1);
            liveLength = 0;
            return o::Value(o::Undefined{});
        });
    context.define(arrayLike, o::Key(u"length"), o::Descriptor::access(lengthGetter, std::nullopt));
    context.define(arrayLike, o::Key(u"0"), o::Descriptor::access(indexGetter, std::nullopt));
    context.set(list, o::Key(u"items"), o::Value(arrayLike));
    auto jax = t::TeX::make(fixture.runtime);
    bool configSucceeded = true;
    try { parser->config(*jax); }
    catch (...) { configSucceeded = false; }
    expect(configSucceeded);
    expect(accessOrder.size() == 2);
    expect(accessOrder.size() < 2 || (accessOrder[0] == 1 && accessOrder[1] == 2));

    // The iterator's completion test must compare the incremented integer
    // directly with live JavaScript numeric length: 1 > .5, while 1 > NaN is false.
    const o::ObjectRef fractionalLength = context.object();
    const auto fractionalGetter = context.function(u"get length",
        [](o::Context&, o::Value, std::span<const o::Value>) { return o::Value(0.5); });
    context.define(fractionalLength, o::Key(u"length"), o::Descriptor::access(fractionalGetter, std::nullopt));
    context.set(list, o::Key(u"items"), o::Value(fractionalLength));
    auto fractionalStep = (*nativeList)->begin().next();
    expect(fractionalStep.done);
    const o::ObjectRef nanLength = context.object();
    const auto nanGetter = context.function(u"get length",
        [](o::Context&, o::Value, std::span<const o::Value>) { return o::Value(std::numeric_limits<double>::quiet_NaN()); });
    context.define(nanLength, o::Key(u"length"), o::Descriptor::access(nanGetter, std::nullopt));
    context.set(list, o::Key(u"items"), o::Value(nanLength));
    auto nanStep = (*nativeList)->begin().next();
    expect(!nanStep.done);

    int nanSpliceCalls = 0;
    double nanSpliceStart = 0;
    const o::ObjectRef nanOperations = context.object();
    const auto nanOperationsLength = context.function(u"get length",
        [](o::Context&, o::Value, std::span<const o::Value>) { return o::Value(std::numeric_limits<double>::quiet_NaN()); });
    const auto splice = context.function(u"splice",
        [&](o::Context&, o::Value, std::span<const o::Value> args) {
            ++nanSpliceCalls;
            nanSpliceStart = args.empty() ? 0 : context.toNumber(args[0]);
            return o::Value(o::Undefined{});
        });
    context.define(nanOperations, o::Key(u"length"), o::Descriptor::access(nanOperationsLength, std::nullopt));
    context.define(nanOperations, o::Key(u"splice"), o::Descriptor::data(o::Value(splice)));
    context.set(list, o::Key(u"items"), o::Value(nanOperations));
    (void)context.callMember(add, list,
        std::array<o::Value, 2>{o::Value(*localIdentity), o::Value(2.0)});
    expect(nanSpliceCalls == 1 && std::isnan(nanSpliceStart));
    (void)context.callMember(remove, list, std::array<o::Value, 1>{o::Value(*localIdentity)});
    expect(nanSpliceCalls == 1);

    const o::ObjectRef fractionalOperations = context.object();
    context.define(fractionalOperations, o::Key(u"length"), o::Descriptor::data(o::Value(1.5)));
    context.define(fractionalOperations, o::Key(u"splice"), o::Descriptor::data(o::Value(splice)));
    context.set(list, o::Key(u"items"), o::Value(fractionalOperations));
    bool fractionalAddTypeError = false;
    try {
      (void)context.callMember(add, list,
          std::array<o::Value, 2>{o::Value(*localIdentity), o::Value(2.0)});
    } catch (const o::Exception& error) { fractionalAddTypeError = error.name == "TypeError"; }
    expect(fractionalAddTypeError);
    bool fractionalRemoveTypeError = false;
    try {
      (void)context.callMember(remove, list, std::array<o::Value, 1>{o::Value(*localIdentity)});
    } catch (const o::Exception& error) { fractionalRemoveTypeError = error.name == "TypeError"; }
    expect(fractionalRemoveTypeError);

    const o::ObjectRef rawArray = context.array();
    context.set(list, o::Key(u"items"), o::Value(rawArray));
    (void)context.callMember(add, list,
        std::array<o::Value, 2>{o::Value(*localIdentity), o::Value(u"12")});
    const o::Value rawRecord = context.get(o::Value(rawArray), o::Key(u"0"));
    expect(context.get(rawRecord, o::Key(u"priority")) == o::Value(u"12"));
    (void)context.callMember(add, list,
        std::array<o::Value, 2>{o::Value(*localIdentity), o::Value(true)});
    expect(context.get(context.get(o::Value(rawArray), o::Key(u"0")), o::Key(u"priority")) == o::Value(true));
    expect(context.get(context.get(o::Value(rawArray), o::Key(u"1")), o::Key(u"priority")) == o::Value(u"12"));

    const auto lengthDescriptor = context.descriptor(o::Value(rawArray), o::Key(u"length"));
    expect(lengthDescriptor.has_value());
    expect(lengthDescriptor->writable && !lengthDescriptor->enumerable && !lengthDescriptor->configurable);
    context.define(rawArray, o::Key(u"length"), o::Descriptor::data(o::Value(2.0), false, false, false));
    bool lockedLengthThrowsTypeError = false;
    try {
      (void)context.callMember(add, list,
          std::array<o::Value, 2>{o::Value(*localIdentity), o::Value(4.0)});
    } catch (const o::Exception& error) {
      lockedLengthThrowsTypeError = error.name == "TypeError";
    }
    expect(lockedLengthThrowsTypeError);
    const auto lockedDescriptor = context.descriptor(o::Value(rawArray), o::Key(u"length"));
    expect(lockedDescriptor.has_value() && !lockedDescriptor->writable);
    expect(context.get(o::Value(rawArray), o::Key(u"length")) == o::Value(2.0));
  };

  "configuration_binding_preserves_preexisting_storage_and_record_cells"_test = [] {
    RuntimeFixture fixture;
    auto& context = fixture.context;
    const auto constructor = std::get<o::ObjectRef>(fixture.runtime.parserConfigurationClass.data);
    const auto parserObject = context.construct(constructor, std::array<o::Value, 1>{o::Value(context.array())});
    const auto parser = fixture.runtime.parserConfigurationIdentities.resolve(parserObject);
    const auto first = t::Configuration::local(fixture.runtime, o::Value(context.object()));
    const auto second = t::Configuration::local(fixture.runtime, o::Value(context.object()));
    const auto firstIdentity = fixture.runtime.configurationIdentities.find(first);
    const auto secondIdentity = fixture.runtime.configurationIdentities.find(second);
    expect(firstIdentity.has_value() && secondIdentity.has_value());

    auto owner = std::make_shared<PrioritizedList<t::ConfigurationRef>>();
    (void)owner->add(first, 7);
    const auto oldStorage = owner->storageRef();
    const auto oldCell = oldStorage->at(0);
    auto oldIterator = owner->begin();
    const auto listIdentity = t::bindConfigurationPriorityList(fixture.runtime, owner);
    expect(owner->storageRef() == oldStorage);
    expect(oldStorage->at(0) == oldCell);
    const o::Value items = context.get(o::Value(listIdentity), o::Key(u"items"));
    const o::Value sourceRecord = context.get(items, o::Key(u"0"));
    expect(std::holds_alternative<o::ObjectRef>(sourceRecord.data));
    expect(context.get(sourceRecord, o::Key(u"item")) == o::Value(*firstIdentity));

    oldCell->priority = 11;
    expect(context.get(sourceRecord, o::Key(u"priority")) == o::Value(11.0));
    context.set(sourceRecord, o::Key(u"priority"), o::Value(13.0));
    context.set(sourceRecord, o::Key(u"item"), o::Value(*secondIdentity));
    auto oldStep = oldIterator.next();
    expect(!oldStep.done);
    expect(oldStep.value.readPriorityValue() == o::Value(13.0));
    const auto oldItem = oldStep.value.readItem();
    expect(std::holds_alternative<o::Value>(oldItem));
    expect(std::get<o::Value>(oldItem) == o::Value(*secondIdentity));

    // Install this same owner on a real ParserConfiguration and exercise its
    // Source-facing config consumer against the shared record cell.
    context.set(o::Value(parserObject), o::Key(u"configurations"), o::Value(listIdentity));
    expect(parser->sourceField(u"configurations") == o::Value(listIdentity));
    auto jax = t::TeX::make(fixture.runtime);
    parser->config(*jax);
  };

  "configuration_context_array_rejects_access_after_execution_lease_expires"_test = [] {
    RuntimeFixture fixture;
    auto& context = fixture.context;
    auto owner = std::make_shared<PrioritizedList<t::ConfigurationRef>>();
    const auto configuration = t::Configuration::local(fixture.runtime, o::Value(context.object()));
    (void)owner->add(configuration, 4);
    owner->storageRef()->insert(owner->storageRef()->begin(), nullptr);
    const auto storage = owner->storageRef();
    const auto list = t::bindConfigurationPriorityList(fixture.runtime, owner);
    const auto array = context.get(o::Value(list), o::Key(u"items"));
    const auto originalSize = storage->size();
    auto lease = fixture.runtime.executionLifetime().lock();
    expect(lease != nullptr);
    lease->store(false, std::memory_order_release);

    const auto rejectsExpiredAccess = [](auto&& operation) {
      try { operation(); }
      catch (const o::Exception& error) { return error.name == "TypeError"; }
      return false;
    };
    expect(rejectsExpiredAccess([&] { (void)context.get(array, o::Key(u"length")); }));
    expect(rejectsExpiredAccess([&] { (void)context.get(array, o::Key(u"0")); }));
    expect(rejectsExpiredAccess([&] { (void)context.get(array, o::Key(u"1")); }));
    expect(rejectsExpiredAccess([&] { (void)context.hasOwn(array, o::Key(u"0")); }));
    expect(rejectsExpiredAccess([&] { (void)context.ownKeys(array); }));
    expect(rejectsExpiredAccess([&] { context.set(array, o::Key(u"0"), o::Value(17)); }));
    expect(rejectsExpiredAccess([&] { context.set(array, o::Key(u"length"), o::Value(0)); }));
    expect(rejectsExpiredAccess([&] { (void)context.erase(std::get<o::ObjectRef>(array.data), o::Key(u"1")); }));
    expect(storage->size() == originalSize && !storage->at(0) && storage->at(1));
  };
};
}
