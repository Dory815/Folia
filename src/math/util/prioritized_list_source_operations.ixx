// Candidate shared implementation of the pinned PrioritizedList Source loops.
export module folia.math.util.prioritized_list_source_operations;
export import folia.math.util.option_types;
import std;

export namespace folia::math {
class PrioritizedListSourceOperations {
public:
    // Called after Source has read this.items.splice and evaluated i + 1,
    // immediately before the object-literal argument is passed to splice.
    // targetItems is the exact receiver whose splice property was read.
    using RecordFactory = std::function<options::Value(
        options::Value targetItems, options::Value item, options::Value priority)>;

    // Source equivalent of PrioritizedList.add. The caller resolves the
    // default-priority argument and owns the method's return-value policy.
    static options::Value add(options::Context& context, options::Value receiver,
        options::Value item, options::Value priority, const RecordFactory& makeRecord);

    // Source equivalent of PrioritizedList.remove. Returns the same receiver,
    // matching the source method's chaining result.
    static options::Value remove(options::Context& context, options::Value receiver,
        options::Value item);
};
}
