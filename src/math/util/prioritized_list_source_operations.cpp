module folia.math.util.prioritized_list_source_operations;
import folia.math.util.js_number;
import std;

namespace folia::math {
using namespace options;

namespace {
Value readItems(Context& context, const Value& receiver) {
    return context.get(receiver, Key(u"items"));
}

Key sourceIndexKey(double index) {
    // Source evaluates this.items[i], so i undergoes numeric ToPropertyKey.
    // js_number::toString is the repository's shared ECMAScript number
    // conversion; it preserves NaN, infinities, -0, and fractional keys.
    const auto text = js_number::toString(index);
    return Key(std::u16string(text.begin(), text.end()));
}

bool sourceLessThan(Context& context, const Value& left, const Value& right) {
    // The relational operator obtains both operands before converting either.
    const Value leftPrimitive = context.toPrimitive(left, PrimitiveHint::numberHint);
    const Value rightPrimitive = context.toPrimitive(right, PrimitiveHint::numberHint);
    const auto* leftString = std::get_if<std::u16string>(&leftPrimitive.data);
    const auto* rightString = std::get_if<std::u16string>(&rightPrimitive.data);
    if (leftString && rightString) return *leftString < *rightString;

    const double leftNumber = context.toNumber(leftPrimitive);
    const double rightNumber = context.toNumber(rightPrimitive);
    return !std::isnan(leftNumber) && !std::isnan(rightNumber) && leftNumber < rightNumber;
}
}

Value PrioritizedListSourceOperations::add(Context& context, Value receiver,
    Value item, Value priority, const RecordFactory& makeRecord) {
    if (!makeRecord) throw std::invalid_argument("PrioritizedList add requires a record factory");

    // `let i = this.items.length` and its do/while are deliberately performed
    // as raw Number operations: no ceil, clamp, or integer normalization.
    const Value initialItems = readItems(context, receiver);
    double index = context.toNumber(context.get(initialItems, Key(u"length")));
    do {
        index -= 1;
        if (!(index >= 0)) break;

        // Every textual `this.items` in PrioritizedList.ts is a new property
        // read. In particular, an earlier getter may have replaced this.items.
        const Value currentItems = readItems(context, receiver);
        const Value currentRecord = context.get(currentItems, sourceIndexKey(index));
        const Value currentPriority = context.get(currentRecord, Key(u"priority"));
        if (!sourceLessThan(context, priority, currentPriority)) break;
    } while (true);

    // In `this.items.splice(...)`, evaluate this.items and its splice getter
    // before evaluating the record object literal. Keep that exact array as
    // both the call receiver and the record factory's owner hint.
    const Value targetItems = readItems(context, receiver);
    const Value splice = context.get(targetItems, Key(u"splice"));
    const Value insertion(static_cast<double>(index + 1));
    const Value record = makeRecord(targetItems, item, priority);
    const std::array<Value, 3> arguments{insertion, Value(0), record};
    (void)context.callMember(splice, targetItems, arguments, u"this.items.splice");
    return item;
}

Value PrioritizedListSourceOperations::remove(Context& context, Value receiver, Value item) {
    const Value initialItems = readItems(context, receiver);
    double index = context.toNumber(context.get(initialItems, Key(u"length")));
    do {
        index -= 1;
        if (!(index >= 0)) break;

        const Value currentItems = readItems(context, receiver);
        const Value currentRecord = context.get(currentItems, sourceIndexKey(index));
        const Value currentItem = context.get(currentRecord, Key(u"item"));
        if (currentItem == item) break; // Value equality is Source strict equality; NaN is unequal.
    } while (true);

    if (index >= 0) {
        const Value targetItems = readItems(context, receiver);
        const Value splice = context.get(targetItems, Key(u"splice"));
        const std::array<Value, 2> arguments{Value(index), Value(1)};
        (void)context.callMember(splice, targetItems, arguments, u"this.items.splice");
    }
    return receiver;
}
}
