// Candidate UT for the Source-mapped CommonMo family implementation.
#include "support/folia_test.hpp"
import folia.math.output.common;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mrow;

namespace {
using namespace boost::ut;
namespace mml = folia::math::mml;
namespace common = folia::math::output::common;

struct RejectUnexpectedMeasurement final : common::MeasurementTarget {
    common::ExtendedMetrics measureMetrics(common::MetricRequest const &) override {
        throw std::logic_error("CommonMo focused test did not request metrics");
    }

    common::UnknownBBox measureUnknownText(common::UnknownTextRequest const &) override {
        throw std::logic_error("CommonMo focused test did not measure unknown text");
    }
};

struct RejectUnexpectedLinebreaks final : common::Linebreaks {
    using Linebreaks::Linebreaks;

    void breakToWidth(common::CommonWrapper &, double) override {
        throw std::logic_error("CommonMo focused test did not request algorithmic line breaking");
    }
};

std::unique_ptr<common::Linebreaks> makeLinebreaks(common::CommonWrapperFactory &factory) {
    return std::make_unique<RejectUnexpectedLinebreaks>(factory);
}

template <class Wrapper> common::CommonWrapperRegistration registration(mml::NodeKind kind) {
    return {
        kind,
        {[](common::WrapperBuildArguments const &args) -> std::unique_ptr<common::CommonWrapper> {
             return std::make_unique<Wrapper>(args.factory, args.node, args.parent);
         },
         [](common::CommonWrapper const &wrapper) {
             return dynamic_cast<Wrapper const *>(&wrapper) != nullptr;
         }},
        common::CommonWrapperClassData::commonDefaults()};
}

common::CommonWrapperCatalog catalog() {
    return {registration<common::CommonMo>(mml::NodeKind::mo),
            registration<common::CommonWrapper>(mml::NodeKind::text),
            registration<common::CommonWrapper>(mml::NodeKind::math),
            registration<common::CommonInferredMrow>(mml::NodeKind::inferred_mrow),
            registration<common::CommonWrapper>(mml::NodeKind::mrow)};
}

auto makeRepository() {
    common::FontRepositoryBuilder builder;
    common::FontFaceData face;
    face.name = "test-face";
    face.cssFamilyPrefix = "MathJax_";
    face.cssFontMap.emplace("normal", common::CssFontData{"MathJax_Main", false, false});
    face.size_variants = {"size-0", "size-1", "size-2"};
    face.stretch_variants = {"stretch-0"};
    face.parameters.values = {{"x_height", .5},         {"axis_height", .25},
                              {"delimiterfactor", 900}, {"delimitershortfall", .3},
                              {"extender_factor", .5},  {"nulldelimiterspace", .12}};
    common::FontVariant normal;
    normal.name = "normal";
    normal.cache_id = "test-cache";
    face.variants.emplace("normal", std::move(normal));
    face.operatorRemap.emplace(0x2B, u"⊕");
    face.accentRemap.emplace(0x5E, u"ˆ");
    builder.addFace(std::move(face));

    common::DelimiterRecipe recipe;
    recipe.direction = common::StretchDirection::vertical;
    recipe.sizes = {.5, 1, 1.5};
    recipe.variants = {0, 1, 2};
    recipe.schar = {0x239A, 0x239B, 0x239B};
    recipe.stretch = {0x239B, 0x239C, 0x239D};
    recipe.stretchv = {0, 0, 0};
    recipe.c = 0x28;
    recipe.ext = 1;
    recipe.min = 0;
    recipe.heightDepth = std::array<double, 2>{.75, .25};
    recipe.heightDepthWidth = std::array<double, 3>{.75, .25, .8};
    recipe.fullExtent = std::array<double, 2>{1, 2};
    builder.addDelimiter("test-face", 0x28, std::move(recipe));
    return std::move(builder).freeze();
}

mml::MmlMo &makeMo(mml::MmlDocument &document, std::u16string text) {
    auto &mo = dynamic_cast<mml::MmlMo &>(document.factory().make(mml::NodeKind::mo));
    auto &token = dynamic_cast<mml::TextNode &>(document.factory().make(mml::NodeKind::text));
    token.setText(std::move(text));
    mo.appendChild(token);
    return mo;
}
} // namespace

suite common_mo_native_impl = [] {
    "CommonMo factory creation preserves Source delimiter decisions"_test = [] {
        auto fonts = makeRepository();
        RejectUnexpectedMeasurement measurement;
        mml::MmlDocument document;
        auto &source = makeMo(document, u"(");
        source.attributes().set(u"stretchy", true);
        source.attributes().set(u"symmetric", false);
        source.attributes().set(u"linebreak", u"newline");
        source.attributes().set(u"linebreakstyle", u"before");
        document.setRoot(source);

        common::CommonOptions options;
        common::ExtendedMetrics metrics;
        common::CommonOutputJax jax(document, {metrics, options, fonts, "test-face"}, measurement,
                                    catalog(), &makeLinebreaks);
        auto *moPointer = dynamic_cast<common::CommonMo *>(&jax.factory().wrap(source));
        expect(moPointer != nullptr);
        if (!moPointer)
            return;
        auto &mo = *moPointer;
        expect(mo.breakStyle == "before");
        expect(mo.breakCount() == 1_u);
        expect(mo.canStretch(common::StretchDirection::vertical));
        expect(std::holds_alternative<common::ActiveStretch>(mo.stretch));

        const std::array<double, 1> one{1.25};
        const std::array<double, 2> pair{2, 1};
        expect(mo.getWH(std::span<double const>{}) == 0.0);
        expect(mo.getWH(one) == 1.25);
        expect(mo.getWH(pair) == 3.0);
        source.attributes().set(u"symmetric", true);
        expect(mo.getWH(pair) == 3.5);
        source.attributes().set(u"symmetric", false);

        const std::array<double, 1> fixedRequest{1.2};
        mo.childNodes.front()->bboxComputed = true;
        mo.getStretchedVariant(fixedRequest, true);
        expect(mo.size.has_value());
        expect(std::get<common::FixedStretchSize>(*mo.size).index == 2_u);
        expect(mo.variant == "size-2");
        expect(std::get<common::ActiveStretch>(mo.stretch).selectedCodepoint ==
               std::uint32_t{0x239B});
        expect(!mo.childNodes.front()->bboxComputed);

        mo.size.reset();
        mo.stretch = common::ActiveStretch{0x28, {}};
        const std::array<double, 1> zeroSizeRequest{.2};
        mo.getStretchedVariant(zeroSizeRequest, true);
        expect(std::get<common::FixedStretchSize>(*mo.size).index == 0_u);
        expect(std::get<common::ActiveStretch>(mo.stretch).selectedCodepoint ==
               std::uint32_t{0x239A});
        const std::array<double, 1> zeroSizeRepeat{.1};
        mo.getStretchedVariant(zeroSizeRepeat, true);
        expect(std::get<common::ActiveStretch>(mo.stretch).selectedCodepoint ==
               std::uint32_t{0x239A});

        mo.size.reset();
        mo.stretch = common::ActiveStretch{0x28, {}};
        mo.bbox = folia::math::BBox::zero();
        const std::array<double, 2> assemblyRequest{3, 2};
        mo.getStretchedVariant(assemblyRequest);
        expect(std::holds_alternative<common::AssemblyStretchSize>(*mo.size));
        expect(mo.bbox.h == 3.0);
        expect(mo.bbox.d == 2.0);
        expect(mo.bbox.w == .8);
        expect(mo.checkExtendedHeight(4.1, *fonts.delimiter("test-face", 0x28)) == 5.0);
    };

    "CommonMo baseline uses supplied dimensions and face axis height"_test = [] {
        auto fonts = makeRepository();
        RejectUnexpectedMeasurement measurement;
        mml::MmlDocument document;
        auto &source = makeMo(document, u"(");
        source.attributes().set(u"symmetric", false);
        document.setRoot(source);
        common::CommonOptions options;
        common::ExtendedMetrics metrics;
        common::CommonOutputJax jax(document, {metrics, options, fonts, "test-face"}, measurement,
                                    catalog(), &makeLinebreaks);
        auto *moPointer = dynamic_cast<common::CommonMo *>(&jax.factory().wrap(source));
        expect(moPointer != nullptr);
        if (!moPointer)
            return;
        auto &mo = *moPointer;
        auto const *recipe = fonts.delimiter("test-face", 0x28);
        expect(recipe != nullptr);
        expect(mo.getBaseline(std::array<double, 2>{3, 1}, 4, *recipe) ==
               std::array<double, 2>{3, 1});
        source.attributes().set(u"symmetric", true);
        expect(mo.getBaseline(std::array<double, 2>{3, 1}, 4, *recipe) ==
               std::array<double, 2>{3, 2.5});
        source.attributes().set(u"symmetric", false);
        expect(mo.getBaseline({}, 4, *recipe) == std::array<double, 2>{3, 1});
    };

    "CommonMo non-delimiter and typed MML remap outcomes remain distinct"_test = [] {
        auto fonts = makeRepository();
        RejectUnexpectedMeasurement measurement;
        mml::MmlDocument document;
        auto &math = dynamic_cast<mml::MmlMath &>(document.factory().make(mml::NodeKind::math));
        auto &row = dynamic_cast<mml::MmlMrow &>(document.factory().make(mml::NodeKind::mrow));
        auto &source = makeMo(document, u"x");
        math.appendChild(row);
        row.appendChild(source);
        document.setRoot(math);
        common::CommonOptions options;
        common::ExtendedMetrics metrics;
        common::CommonOutputJax jax(document, {metrics, options, fonts, "test-face"}, measurement,
                                    catalog(), &makeLinebreaks);
        (void)jax.factory().wrap(math);
        auto *mo = dynamic_cast<common::CommonMo *>(jax.wrapperFor(source));
        expect(mo != nullptr);
        if (!mo)
            return;
        expect(!mo->canStretch(common::StretchDirection::vertical));
        expect(std::holds_alternative<common::NoStretch>(mo->stretch));
        const std::array<std::uint32_t, 1> plus{0x2B};
        expect(mo->remapChars(plus) == std::vector<std::uint32_t>{0x2295});
        source.properties().tex.primes = u"′′";
        expect(mo->remapChars(plus) == std::vector<std::uint32_t>{0x2032, 0x2032});
    };
};
