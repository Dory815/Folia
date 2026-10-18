#include "support/folia_test.hpp"
import folia.math.util.prioritized_list_source_operations;

namespace {
using namespace boost::ut;
namespace o = folia::math::options;
using folia::math::PrioritizedListSourceOperations;

o::ObjectRef dataRecord(o::Context& context, o::Value item, o::Value priority) {
    const auto record = context.object();
    context.define(record, o::Key(u"item"), o::Descriptor::data(std::move(item)));
    context.define(record, o::Key(u"priority"), o::Descriptor::data(std::move(priority)));
    return record;
}

suite prioritizedListSourceOperations = [] {
    "prioritized_list_add_rereads_items_and_observes_source_call_order"_test = [] {
        o::Context context;
        const auto receiver = context.object();
        const auto initial = context.array(2);
        const auto firstLoopArray = context.array(2);
        const auto secondLoopArray = context.array(1);
        const auto spliceArray = context.array();
        const auto firstRecord = dataRecord(context, o::Value(u"first"), o::Value(20));
        const auto secondRecord = dataRecord(context, o::Value(u"second"), o::Value(5));
        std::vector<std::string> trace;
        const auto firstPriority = context.function(u"priority getter 20",
            [&trace](o::Context&, o::Value, std::span<const o::Value>) {
                trace.emplace_back("priority-20");
                return o::Value(20);
            });
        context.define(firstRecord, o::Key(u"priority"), o::Descriptor::access(firstPriority, {}, true, true));
        const auto secondPriority = context.function(u"priority getter 5",
            [&trace](o::Context&, o::Value, std::span<const o::Value>) {
                trace.emplace_back("priority-5");
                return o::Value(5);
            });
        context.define(secondRecord, o::Key(u"priority"), o::Descriptor::access(secondPriority, {}, true, true));

        const auto firstIndex = context.function(u"index 1 getter",
            [&trace, firstRecord](o::Context&, o::Value, std::span<const o::Value>) {
                trace.emplace_back("index-1");
                return o::Value(firstRecord);
            });
        context.define(firstLoopArray, o::Key(u"1"), o::Descriptor::access(firstIndex, {}, true, true));
        const auto secondIndex = context.function(u"index 0 getter",
            [&trace, secondRecord](o::Context&, o::Value, std::span<const o::Value>) {
                trace.emplace_back("index-0");
                return o::Value(secondRecord);
            });
        context.define(secondLoopArray, o::Key(u"0"), o::Descriptor::access(secondIndex, {}, true, true));

        const std::array<o::Value, 4> arrays{
            o::Value(initial), o::Value(firstLoopArray), o::Value(secondLoopArray), o::Value(spliceArray)};
        std::size_t itemsReads = 0;
        const auto itemsGetter = context.function(u"items getter",
            [&trace, &itemsReads, &arrays, receiver](o::Context&, o::Value actualReceiver, std::span<const o::Value>) {
                expect(actualReceiver == o::Value(receiver));
                trace.emplace_back("items");
                return arrays.at(itemsReads++);
            });
        context.define(receiver, o::Key(u"items"), o::Descriptor::access(itemsGetter, {}, true, true));

        const auto arraySplice = context.get(o::Value(context.intrinsics().arrayPrototype), o::Key(u"splice"));
        const auto spliceFunction = context.function(u"observable splice",
            [&context, &trace, spliceArray, arraySplice](o::Context&, o::Value actualReceiver,
                std::span<const o::Value> arguments) {
                expect(actualReceiver == o::Value(spliceArray));
                trace.emplace_back("splice-call");
                return context.callMember(arraySplice, actualReceiver, arguments, u"array splice oracle");
            });
        const auto spliceGetter = context.function(u"splice getter",
            [&trace, spliceFunction, spliceArray](o::Context&, o::Value actualReceiver, std::span<const o::Value>) {
                expect(actualReceiver == o::Value(spliceArray));
                trace.emplace_back("splice-get");
                return o::Value(spliceFunction);
            });
        context.define(spliceArray, o::Key(u"splice"), o::Descriptor::access(spliceGetter, {}, true, true));

        const auto item = o::Value(u"inserted");
        const auto result = PrioritizedListSourceOperations::add(context, o::Value(receiver), item, o::Value(10),
            [&context, &trace](o::Value, o::Value sourceItem, o::Value sourcePriority) {
                trace.emplace_back("record");
                return o::Value(dataRecord(context, std::move(sourceItem), std::move(sourcePriority)));
            });

        expect(result == item);
        expect(itemsReads == 4_u);
        expect(context.arrayLength(spliceArray) == 1_u);
        const auto inserted = context.get(o::Value(spliceArray), o::Key(u"0"));
        expect(context.get(inserted, o::Key(u"item")) == item);
        const std::vector<std::string> expected{
            "items", "items", "index-1", "priority-20", "items", "index-0", "priority-5", "items",
            "splice-get", "record", "splice-call"};
        expect(trace == expected);
    };

    "prioritized_list_source_loops_keep_nan_and_fractional_indices_raw"_test = [] {
        o::Context context;
        const auto receiver = context.object();
        const auto arrayLike = context.object();
        context.set(arrayLike, o::Key(u"length"), o::Value(std::numeric_limits<double>::quiet_NaN()));
        context.set(receiver, o::Key(u"items"), o::Value(arrayLike));
        bool readNaNIndex = false;
        const auto nanIndexGetter = context.function(u"NaN index getter",
            [&readNaNIndex](o::Context&, o::Value, std::span<const o::Value>) {
                readNaNIndex = true;
                return o::Value(o::Undefined{});
            });
        context.define(arrayLike, o::Key(u"NaN"), o::Descriptor::access(nanIndexGetter, {}, true, true));

        double observedStart = 0;
        const auto splice = context.function(u"recording splice",
            [&observedStart, arrayLike](o::Context&, o::Value actualReceiver, std::span<const o::Value> args) {
                expect(actualReceiver == o::Value(arrayLike));
                observedStart = std::get<double>(args[0].data);
                expect(std::isnan(observedStart));
                expect(args[1] == o::Value(0));
                return o::Value(o::Undefined{});
            });
        context.set(arrayLike, o::Key(u"splice"), o::Value(splice));
        const auto added = PrioritizedListSourceOperations::add(context, o::Value(receiver), o::Value(u"x"), o::Value(3),
            [&context](o::Value, o::Value item, o::Value priority) {
                return o::Value(dataRecord(context, std::move(item), std::move(priority)));
            });
        expect(added == o::Value(u"x"));
        expect(!readNaNIndex);

        const auto fractionalReceiver = context.object();
        const auto fractionalItems = context.object();
        const auto fractionalRecord = dataRecord(context, o::Value(u"target"), o::Value(0));
        context.set(fractionalItems, o::Key(u"length"), o::Value(2.5));
        context.set(fractionalItems, o::Key(u"1.5"), o::Value(fractionalRecord));
        context.set(fractionalReceiver, o::Key(u"items"), o::Value(fractionalItems));
        double removedStart = 0;
        const auto fractionalSplice = context.function(u"fractional splice",
            [&removedStart, fractionalItems](o::Context&, o::Value actualReceiver, std::span<const o::Value> args) {
                expect(actualReceiver == o::Value(fractionalItems));
                removedStart = std::get<double>(args[0].data);
                expect(args[1] == o::Value(1));
                return o::Value(o::Undefined{});
            });
        context.set(fractionalItems, o::Key(u"splice"), o::Value(fractionalSplice));
        const auto removed = PrioritizedListSourceOperations::remove(context,
            o::Value(fractionalReceiver), o::Value(u"target"));
        expect(removed == o::Value(fractionalReceiver));
        expect(removedStart == 1.5);
    };
};
}
