// Root-frozen ordinary array slice binding, no parser/map algorithm.
module folia.math.core.tree.property;
import std;
namespace folia::math {
PropertyArray PropertyArray::slice(std::size_t first) const {
    const std::size_t length = size();
    const std::size_t start = std::min(first, length);
    const std::size_t resultLength = length - start;

    PropertyArray result;
    result.resize(resultLength);

    const PropertyList source = record();
    for (std::size_t sourceIndex = start; sourceIndex < length; ++sourceIndex) {
        const std::string key = std::to_string(sourceIndex);
        if (!source.has(key)) continue;
        const Property value = source.get(key);
        result.set(sourceIndex - start, value);
    }
    return result;
}
}
