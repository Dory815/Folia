// MathJax 4.1.3 util/StyleJson.ts; Apache-2.0.
export module folia.math.util.style_json;
import std;

export namespace folia::math::style_json {
struct StyleEntry {
    std::u16string name;
    std::variant<std::u16string, double, std::vector<StyleEntry>> value;
};

using StyleJson = std::vector<StyleEntry>;

class StyleJsonSheet {
  protected:
    StyleJson styles_;

  public:
    explicit StyleJsonSheet(StyleJson styles = {});
    virtual ~StyleJsonSheet() = default;
    StyleJsonSheet(const StyleJsonSheet &) = delete;
    StyleJsonSheet &operator=(const StyleJsonSheet &) = delete;

    virtual std::u16string cssText() const;
    virtual void addStyles(const StyleJson &styles);
    virtual void removeStyles(std::span<const std::u16string> selectors);
    virtual void clear();
    virtual std::u16string getStyleString() const;
    virtual std::vector<std::u16string> getStyleRules(std::u16string_view spaces = {}) const;
    virtual std::vector<std::u16string> getStyleRules(const StyleJson &styles,
                                                      std::u16string_view spaces = {}) const;
    virtual std::u16string getStyleDefString(const StyleJson &styles,
                                             std::u16string_view spaces) const;
};
} // namespace folia::math::style_json
