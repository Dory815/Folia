// Focused native tests for the Source-mapped common style services.
#include "support/folia_test.hpp"
import folia.math.util.styles;
import folia.math.util.style_json;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
namespace styles = folia::math::styles;
namespace style_json = folia::math::style_json;
using folia::math::Property;
using folia::math::PropertyList;
bool same16(std::u16string_view actual, std::u16string_view expected) {
    if (actual.size() != expected.size()) return false;
    for (std::size_t i = 0; i < actual.size(); ++i)
        if (actual[i] != expected[i]) return false;
    return true;
}

struct StyleProbe : styles::Styles {
    using Styles::Styles;
    using Styles::normalizeName;
    using Styles::parentName;
    using Styles::childName;
    using Styles::sanitizeValue;
};

struct DerivedStyles : styles::Styles {
    static inline styles::StylesPatterns derivedPatterns{
        std::make_shared<folia::math::regexp_host::RegExp>(u"['\";]"),
        std::make_shared<folia::math::regexp_host::RegExp>(u"^([^;]*)(?:;|$)"),
        std::make_shared<folia::math::regexp_host::RegExp>(u"([a-z]+)\\s*=\\s*([^;]*);?", u"g"),
        std::make_shared<folia::math::regexp_host::RegExp>(u"/\\*[^]*?\\*/", u"g")};
    explicit DerivedStyles(std::u16string_view css) : Styles(css, derivedPatterns) {}
};
}

suite math_port_common_styles = [] {
    "styles_parse_preserves_source_shorthands_and_insertion_order"_test = [] {
        styles::Styles style(u"color: red; padding: 1em 2em; border-top: thin dashed red;");
        expect(same16(style.get(u"paddingTop"), u"1em"));
        expect(same16(style.get(u"padding-right"), u"2em"));
        expect(same16(style.get(u"borderTopWidth"), u"thin"));
        expect(same16(style.get(u"border-top-style"), u"dashed"));
        expect(style.cssText().find(u"color: red;") != std::u16string::npos);
        expect(style.cssText().find(u"padding: 1em 2em;") != std::u16string::npos);
    };
    "styles_set_propagates_and_recombines_border_children"_test = [] {
        styles::Styles style;
        style.set(u"borderTop", styles::StyleValue(std::u16string(u"1px solid red")));
        style.set(u"borderRight", styles::StyleValue(std::u16string(u"1px solid red")));
        style.set(u"borderBottom", styles::StyleValue(std::u16string(u"1px solid red")));
        style.set(u"borderLeft", styles::StyleValue(std::u16string(u"1px solid red")));
        expect(same16(style.get(u"border"), u"1px solid red"));
        style.set(u"borderLeftColor", styles::StyleValue(std::u16string{}));
        expect(style.get(u"border").empty());
    };
    "styles_sanitize_and_normalize_utf16_names"_test = [] {
        StyleProbe style;
        expect(same16(style.sanitizeValue(u"red; blue"), u"red"));
        expect(same16(style.sanitizeValue(u"\"unclosed"), u"\"unclosed\""));
        expect(eq(style.normalizeName(u"x😀Y"), std::string("x😀-y")));
        expect(eq(style.parentName("border-top-width"), std::string("border-top")));
        expect(eq(style.childName("border-width", "top"), std::string("border-top-width")));
    };
    "styles_derived_constructor_uses_class_pattern_before_parse"_test = [] {
        DerivedStyles style(u"padding=1em 2em;color=red;");
        expect(same16(style.get(u"padding"), u"1em 2em"));
        expect(same16(style.get(u"color"), u"red"));
    };
    "style_json_shallow_merges_and_retains_nested_alias"_test = [] {
        PropertyList nested{{"color", Property("red")}};
        PropertyList nestedRules{{".child", Property(nested)}};
        PropertyList originalSelector{{"color", Property("blue")},
            {"@media test", Property(nestedRules)}};
        PropertyList input{{".a", Property(originalSelector)}};
        style_json::StyleJsonSheet sheet{Property(input)};
        originalSelector.set("color", Property("green"));
        nested.set("color", Property("purple"));
        expect(same16(sheet.cssText(),
            u".a {\n  color: blue;\n  @media test {\n      .child {\n      color: purple;\n    }\n  }\n}"));
        PropertyList addedProperties;
        addedProperties.set("padding", Property("1em"));
        PropertyList addedSelectors;
        addedSelectors.set(".a", Property(addedProperties));
        sheet.addStyles(Property(addedSelectors));
        expect(sheet.cssText().find(u"padding: 1em;") != std::u16string::npos);
        const std::array<std::u16string, 1> remove{u".a"};
        sheet.removeStyles(remove);
        expect(sheet.cssText().empty());
    };
    "style_json_explicit_null_rules_throws_source_error"_test = [] {
        style_json::StyleJsonSheet sheet{};
        bool threw = false;
        try { (void)sheet.getStyleRules(Property(folia::math::PropertyNull{})); }
        catch (const std::exception&) { threw = true; }
        expect(threw);
    };
};
