#include "support/folia_test.hpp"
import folia.math.output.common;

namespace {
using namespace folia::math;
using namespace folia::math::output::common;
using namespace boost::ut;

struct TestMeasurementTarget final : MeasurementTarget {
    ExtendedMetrics measureMetrics(MetricRequest const &request) override {
        return {.em = 16,
                .ex = 8,
                .containerWidth = 1280,
                .scale = 1,
                .family = request.family,
                .display = request.display};
    }

    UnknownBBox measureUnknownText(UnknownTextRequest const &) override {
        return {};
    }
};

template <class Wrapper> tree::FactoryClass<CommonWrapper, WrapperBuildArguments> wrapperClass() {
    return {[](WrapperBuildArguments const &args) -> std::unique_ptr<CommonWrapper> {
                return std::make_unique<Wrapper>(args.factory, args.node, args.parent);
            },
            [](CommonWrapper const &wrapper) {
                return dynamic_cast<Wrapper const *>(&wrapper) != nullptr;
            }};
}

class UnexpectedBreaks final : public Linebreaks {
  public:
    using Linebreaks::Linebreaks;

    void breakToWidth(CommonWrapper &, double) override {
        throw std::logic_error("unexpected linebreak request in wrapper unit test");
    }
};

CommonWrapperCatalog testCatalog() {
    CommonWrapperCatalog catalog;
    const auto add = [&](mml::NodeKind kind, auto creator) {
        catalog.push_back({kind, std::move(creator), CommonWrapperClassData::commonDefaults()});
    };
    add(mml::NodeKind::text, wrapperClass<CommonTextNode>());
    add(mml::NodeKind::mi, wrapperClass<CommonMi>());
    add(mml::NodeKind::mn, wrapperClass<CommonMn>());
    add(mml::NodeKind::mo, wrapperClass<CommonMo>());
    add(mml::NodeKind::mrow, wrapperClass<CommonMrow>());
    add(mml::NodeKind::inferred_mrow, wrapperClass<CommonInferredMrow>());
    add(mml::NodeKind::math, wrapperClass<CommonMath>());
    return catalog;
}

FontRepository const &testFonts() {
    static const FontRepository repository = [] {
        FontRepositoryBuilder builder;
        FontFaceData face;
        face.name = "MathJaxNewcm";
        for (const auto *name : {"normal", "italic", "bold", "bold-italic"}) {
            FontVariant variant;
            variant.name = name;
            variant.cache_id = name;
            face.variants.emplace(name, std::move(variant));
        }
        builder.addFace(std::move(face));
        builder.addParameter("MathJaxNewcm", "x_height", .442);
        builder.addParameter("MathJaxNewcm", "rule_thickness", .04);
        return std::move(builder).freeze();
    }();
    return repository;
}

mml::MmlNode &token(mml::MmlDocument &document, mml::NodeKind kind, std::u16string value) {
    auto &text = dynamic_cast<mml::TextNode &>(document.factory().make(mml::NodeKind::text));
    text.setText(std::move(value));
    std::array<mml::MmlNode *, 1> children{&text};
    return document.factory().make(kind, {}, children);
}

CommonOutputJaxConfig wrapperConfig(CommonOptions options = {},
                                    InlineBreakPolicy policy = InlineBreakPolicy::opportunistic) {
    ExtendedMetrics metrics{.em = 16,
                            .ex = 8,
                            .containerWidth = 1280,
                            .scale = 1,
                            .family = "MathJaxNewcm",
                            .display = false};
    return {metrics, std::move(options), testFonts(), "MathJaxNewcm", policy};
}

std::unique_ptr<Linebreaks> makeUnexpectedBreaks(CommonWrapperFactory &factory) {
    return std::make_unique<UnexpectedBreaks>(factory);
}

CommonOutputJax makeJax(mml::MmlDocument &document, TestMeasurementTarget &target,
                        CommonOptions options = {}) {
    return CommonOutputJax(document, wrapperConfig(std::move(options)), target, testCatalog(),
                           &makeUnexpectedBreaks);
}

struct WrapperProbe final : CommonWrapper {
    using CommonWrapper::CommonWrapper;

    std::shared_ptr<LineBBox> lineFrom(CommonWrapper &child, std::size_t index) {
        return getChildLineBBox(child, index);
    }
};
} // namespace

suite native_common_wrapper_source_cases = [] {
    "style_removal_and_scale_match_pinned_style_case"_test = [] {
        TestMeasurementTarget target;
        mml::MmlDocument document;
        auto jax = makeJax(document, target);
        mml::AttributeSet attrs;
        attrs.set(u"style",
                  u"font-size: 150%; padding: 2px; margin-left: 3px; border: 1px solid red");
        auto &row = document.factory().make(mml::NodeKind::mrow, attrs);
        document.setRoot(row);
        auto &wrapper = jax.factory().wrap(row);
        expect(wrapper.styles.has_value());
        expect(wrapper.styleData.has_value());
        expect(wrapper.removedStyles.at("fontSize") == "150%");
        // Pinned Source case style-variant-scale-and-child-order records 1.5.
        expect(wrapper.bbox.scale == 1.5);
    };

    "tex_spacing_matches_pinned_a_plus_b_case"_test = [] {
        TestMeasurementTarget target;
        mml::MmlDocument document;
        auto &a = token(document, mml::NodeKind::mi, u"a");
        auto &plus = token(document, mml::NodeKind::mo, u"+");
        auto &b = token(document, mml::NodeKind::mi, u"b");
        std::array<mml::MmlNode *, 3> children{&a, &plus, &b};
        auto &math = document.factory().make(mml::NodeKind::math, {}, children);
        math.setInheritedAttributes();
        document.setRoot(math);
        LayoutJob job(document, wrapperConfig({}, InlineBreakPolicy::forced), target, testCatalog(),
                      &makeUnexpectedBreaks);
        auto &root = job.build();
        (void)root.getBBox();
        auto &wrapper = *root.childNodes.front();
        expect(wrapper.childNodes.size() == 3_u);
        expect(wrapper.childNodes[1]->node->kind() == mml::NodeKind::mo);
        // Values are from common-wrapper-native-cases/expected.json, pinned
        // by the accompanying Source/CJS hashes and the source case "a+b".
        expect(wrapper.childNodes[1]->bbox.L == 0.0_d);
        expect(wrapper.childNodes[2]->bbox.L == 2.0 / 9.0);
    };

    "mathml_spacing_matches_pinned_a_plus_b_case"_test = [] {
        TestMeasurementTarget target;
        CommonOptions options;
        options.mathmlSpacing = true;
        mml::MmlDocument document;
        auto &a = token(document, mml::NodeKind::mi, u"a");
        auto &plus = token(document, mml::NodeKind::mo, u"+");
        auto &b = token(document, mml::NodeKind::mi, u"b");
        std::array<mml::MmlNode *, 3> children{&a, &plus, &b};
        auto &math = document.factory().make(mml::NodeKind::math, {}, children);
        math.setInheritedAttributes();
        document.setRoot(math);
        LayoutJob job(document, wrapperConfig(std::move(options), InlineBreakPolicy::forced),
                      target, testCatalog(), &makeUnexpectedBreaks);
        auto &root = job.build();
        (void)root.getBBox();
        auto &wrapper = *root.childNodes.front();
        expect(wrapper.childNodes[1]->bbox.L == 0.0_d);
        expect(wrapper.childNodes[1]->bbox.R == 2.0 / 9.0);
        expect(wrapper.childNodes[2]->bbox.L == 0.0_d);
    };

    "invalid_explicit_mathvariant_records_source_warning_and_falls_back"_test = [] {
        TestMeasurementTarget target;
        mml::MmlDocument document;
        auto jax = makeJax(document, target);

        mml::AttributeSet invalidAttributes;
        invalidAttributes.set(u"mathvariant", u"not-a-source-variant");
        auto &invalidText =
            dynamic_cast<mml::TextNode &>(document.factory().make(mml::NodeKind::text));
        invalidText.setText(u"x");
        std::array<mml::MmlNode *, 1> invalidChildren{&invalidText};
        auto &invalid = document.factory().make(mml::NodeKind::mi, std::move(invalidAttributes),
                                                invalidChildren);
        document.setRoot(invalid);

        auto &invalidWrapper = jax.factory().wrap(invalid);
        expect(invalidWrapper.variant == "normal");
        const auto &diagnostics = jax.diagnostics();
        expect(diagnostics.size() == 1_u);
        expect(diagnostics[0].code == CommonDiagnosticCode::unknown_math_variant);
        expect(diagnostics[0].node_kind == mml::NodeKind::mi);
        expect(diagnostics[0].value == "not-a-source-variant");

        mml::AttributeSet validAttributes;
        validAttributes.set(u"mathvariant", u"normal");
        auto &validText =
            dynamic_cast<mml::TextNode &>(document.factory().make(mml::NodeKind::text));
        validText.setText(u"y");
        std::array<mml::MmlNode *, 1> validChildren{&validText};
        auto &valid =
            document.factory().make(mml::NodeKind::mi, std::move(validAttributes), validChildren);
        auto &validWrapper = jax.factory().wrap(valid);
        expect(validWrapper.variant == "normal");
        expect(jax.diagnostics().size() == 1_u);

        mml::AttributeSet nullAttributes;
        nullAttributes.set(u"mathvariant", mml::AttributeNull{});
        auto &nullText =
            dynamic_cast<mml::TextNode &>(document.factory().make(mml::NodeKind::text));
        nullText.setText(u"z");
        std::array<mml::MmlNode *, 1> nullChildren{&nullText};
        auto &explicitNull =
            document.factory().make(mml::NodeKind::mi, std::move(nullAttributes), nullChildren);
        auto &nullWrapper = jax.factory().wrap(explicitNull);
        expect(nullWrapper.variant == "normal");
        expect(jax.diagnostics().size() == 2_u);
        expect(jax.diagnostics()[1].value == "null");
    };

    "base_probe_preserves_unmodified_child_line_identity"_test = [] {
        TestMeasurementTarget target;
        mml::MmlDocument document;
        auto jax = makeJax(document, target);
        auto &a = token(document, mml::NodeKind::mi, u"a");
        std::array<mml::MmlNode *, 1> children{&a};
        auto &row = document.factory().make(mml::NodeKind::mrow, {}, children);
        document.setRoot(row);
        auto &probe = dynamic_cast<WrapperProbe &>(
            jax.factory().adopt(std::make_unique<WrapperProbe>(jax.factory(), row)));
        auto *child = probe.childNodes.front();
        auto childLine = child->getLineBBox(0);
        auto parentLine = probe.lineFrom(*child, 0);
        // This is a direct base-helper probe. The pinned Source case records
        // the same alias rule when styleData and side spacing are absent.
        expect(parentLine == childLine);
    };

    "bbox_false_returns_cached_bbox_when_already_computed"_test = [] {
        TestMeasurementTarget target;
        mml::MmlDocument document;
        auto jax = makeJax(document, target);
        auto &row = document.factory().make(mml::NodeKind::mrow);
        document.setRoot(row);
        auto &wrapper = jax.factory().wrap(row);
        const auto cached = wrapper.getBBox();
        const auto nonSaving = wrapper.getBBox(false);
        expect(nonSaving.w == cached.w);
        expect(nonSaving.h == cached.h);
        expect(nonSaving.d == cached.d);
        expect(wrapper.bboxComputed);
    };

    "bbox_false_starts_from_zero_without_mutating_unsaved_wrapper_state"_test = [] {
        TestMeasurementTarget target;
        mml::MmlDocument document;
        auto jax = makeJax(document, target);
        mml::AttributeSet attrs;
        attrs.set(u"style", u"font-size: 150%; margin-left: 3px");
        attrs.set(u"lspace", u".2em");
        attrs.set(u"rspace", u".3em");
        auto &text = dynamic_cast<mml::TextNode &>(document.factory().make(mml::NodeKind::text));
        text.setText(u"+");
        std::array<mml::MmlNode *, 1> children{&text};
        auto &mo = document.factory().make(mml::NodeKind::mo, std::move(attrs), children);
        auto &left = token(document, mml::NodeKind::mi, u"a");
        auto &right = token(document, mml::NodeKind::mi, u"b");
        std::array<mml::MmlNode *, 3> rowChildren{&left, &mo, &right};
        auto &row = document.factory().make(mml::NodeKind::mrow, {}, rowChildren);
        row.setInheritedAttributes();
        row.setTeXclass(nullptr);
        document.setRoot(row);
        (void)jax.factory().wrap(row);
        auto &wrapper = *jax.wrapperFor(mo);

        expect(wrapper.bbox.scale == 1.5);
        expect(wrapper.bbox.L == .2);
        expect(wrapper.bbox.R == .3);
        expect(!wrapper.bboxComputed);
        const auto result = wrapper.getBBox(false);
        expect(result.scale == 1.0);
        expect(result.rscale == 1.0);
        expect(result.L == 0.0);
        expect(result.R == 0.0);
        expect(wrapper.bbox.scale == 1.5);
        expect(!wrapper.bboxComputed);
    };

    "fontfamily_attribute_overrides_removed_inline_fontfamily"_test = [] {
        TestMeasurementTarget target;
        mml::MmlDocument document;
        auto jax = makeJax(document, target);
        mml::AttributeSet attributes;
        attributes.set(u"style", u"font-family: InlineMath");
        attributes.set(u"fontfamily", u"MathAttribute");
        auto &text = dynamic_cast<mml::TextNode &>(document.factory().make(mml::NodeKind::text));
        text.setText(u"x");
        std::array<mml::MmlNode *, 1> children{&text};
        auto &mi = document.factory().make(mml::NodeKind::mi, std::move(attributes), children);
        document.setRoot(mi);

        auto &wrapper = jax.factory().wrap(mi);
        expect(wrapper.variant == "-explicitFont");
        expect(wrapper.styles->get(u"fontFamily") == u"MathAttribute");
    };
};
