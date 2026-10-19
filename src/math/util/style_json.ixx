// MathJax 4.1.3 util/StyleJson.ts; Apache-2.0.
export module folia.math.util.style_json;
export import folia.math.core.tree.property;
import std;

export namespace folia::math::style_json {
using StyleJson = PropertyList;

class StyleJsonSheet {
protected:
    StyleJson styles_;

public:
    explicit StyleJsonSheet(Property styles = Missing{});
    virtual ~StyleJsonSheet() = default;
    StyleJsonSheet(const StyleJsonSheet&) = delete;
    StyleJsonSheet& operator=(const StyleJsonSheet&) = delete;

    virtual std::u16string cssText() const;
    virtual void addStyles(const Property& styles);
    virtual void removeStyles(std::span<const std::u16string> selectors);
    virtual void clear();
    virtual std::u16string getStyleString() const;
    virtual std::vector<std::u16string> getStyleRules(
        Property styles = Missing{}, std::u16string_view spaces = {}) const;
    virtual std::u16string getStyleDefString(
        const Property& styles, std::u16string_view spaces) const;
};
}
