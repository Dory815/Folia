export module folia.math.core.dom_node;
export import folia.math.core.tree.property;
import std;

// Opaque identity-bearing output DOM handle.  Core consumers depend only on
// this protocol; each adaptor owns a concrete store implementation.
export namespace folia::math {
struct DOMNodeRef;
class DOMNodeStore {
public:
    virtual ~DOMNodeStore() = default;
    virtual Property nodeProperty(std::uint64_t id) = 0;
    virtual std::optional<std::uint64_t> nodeId(const Property& value) const = 0;
};

struct DOMNodeRef {
    std::shared_ptr<DOMNodeStore> store;
    std::uint64_t id = 0;
    explicit operator bool() const noexcept { return store && id != 0; }
    friend bool operator==(const DOMNodeRef&, const DOMNodeRef&) = default;
};

// A node query can return an actual node, a raw Source value, explicit null,
// or undefined. The Property arm is deliberately not projected or coerced.
using DOMNodeValue = std::variant<Missing, PropertyNull, DOMNodeRef, Property>;

// The slots are the one fresh Source array returned by childNodes. `owner`
// leases the graph for as long as the typed array handle is retained; it is
// not a second node list or a copy of the slots.
struct DOMNodeArray {
    PropertyArray slots;
    std::shared_ptr<DOMNodeStore> owner;
};

class DOMNodePropertyBinding : public PropertyRecordBinding {
public:
    virtual DOMNodeRef nodeRef() const = 0;
};

inline Property nodeProperty(const DOMNodeRef& node) {
    if (!node || !node.store) throw std::invalid_argument("invalid DOM node handle");
    return node.store->nodeProperty(node.id);
}
inline DOMNodeValue propertyNode(Property value) {
    if (value.missing()) return Missing{};
    if (std::holds_alternative<PropertyNull>(value.value)) return PropertyNull{};
    if (const auto storage = std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);
        storage && *storage && (*storage)->binding) {
        if (const auto binding = std::dynamic_pointer_cast<DOMNodePropertyBinding>((*storage)->binding))
            return binding->nodeRef();
    }
    return value;
}
}
