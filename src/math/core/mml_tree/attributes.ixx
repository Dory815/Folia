export module folia.math.core.mml_tree.attributes;
import std;

export namespace folia::math::mml {
struct AttributeNull final {
    friend constexpr bool operator==(AttributeNull, AttributeNull) = default;
};

using AttributeValue = std::variant<AttributeNull, bool, double, std::u16string>;

class AttributeSet final {
public:
    using Entry = std::pair<std::u16string, AttributeValue>;

private:
    std::vector<Entry> own_;
    std::vector<Entry> inherited_;
    std::vector<Entry> defaults_;
    std::vector<Entry> globalDefaults_;

    static const AttributeValue* find(
        const std::vector<Entry>& entries,
        std::u16string_view name) noexcept;
    static void put(
        std::vector<Entry>& entries,
        std::u16string name,
        AttributeValue value);

public:
    void set(std::u16string name, AttributeValue value);
    void setInherited(std::u16string name, AttributeValue value);
    void setDefault(std::u16string name, AttributeValue value);
    void setGlobalDefault(std::u16string name, AttributeValue value);

    bool erase(std::u16string_view name);
    const AttributeValue* own(std::u16string_view name) const noexcept;
    const AttributeValue* inherited(std::u16string_view name) const noexcept;
    const AttributeValue* defaultValue(std::u16string_view name) const noexcept;
    const AttributeValue* globalDefault(std::u16string_view name) const noexcept;

    const AttributeValue* effectiveInherited(std::u16string_view name) const noexcept;
    const AttributeValue* effectiveDefault(std::u16string_view name) const noexcept;
    const AttributeValue* lookup(std::u16string_view name) const noexcept;
    bool isSet(std::u16string_view name) const noexcept;
    bool hasDefault(std::u16string_view name) const noexcept;

    const std::vector<Entry>& ownEntries() const noexcept { return own_; }
    const std::vector<Entry>& inheritedEntries() const noexcept { return inherited_; }
    const std::vector<Entry>& defaultEntries() const noexcept { return defaults_; }
    const std::vector<Entry>& globalDefaultEntries() const noexcept { return globalDefaults_; }
};
}
