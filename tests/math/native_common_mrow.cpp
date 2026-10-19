// Candidate UT for the Source-mapped CommonMrow lifecycle and explicit breaks.
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
        throw std::logic_error("CommonMrow focused tests did not request metrics");
    }

    common::UnknownBBox measureUnknownText(common::UnknownTextRequest const &) override {
        throw std::logic_error("CommonMrow focused tests did not measure unknown text");
    }
};

struct RejectUnexpectedLinebreaks final : common::Linebreaks {
    using Linebreaks::Linebreaks;

    void breakToWidth(common::CommonWrapper &, double) override {
        throw std::logic_error("explicit Source newline must not invoke width breaking");
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
    return {registration<common::CommonWrapper>(mml::NodeKind::math),
            registration<common::CommonInferredMrow>(mml::NodeKind::inferred_mrow),
            registration<common::CommonMrow>(mml::NodeKind::mrow),
            registration<common::CommonMo>(mml::NodeKind::mo),
            registration<common::CommonWrapper>(mml::NodeKind::text)};
}

common::FontRepository makeRepository() {
    common::FontRepositoryBuilder builder;
    common::FontFaceData face;
    face.name = "row-fixture";
    face.cssFamilyPrefix = "MathJax_";
    face.cssFontMap.emplace("normal", common::CssFontData{"MathJax_Main", false, false});
    face.size_variants = {"size-0", "size-1", "size-2"};
    face.stretch_variants = {"stretch-0"};
    face.parameters.values = {{"x_height", .5},         {"axis_height", .25},
                              {"delimiterfactor", 900}, {"delimitershortfall", .3},
                              {"extender_factor", .5},  {"nulldelimiterspace", .12}};
    common::FontVariant normal;
    normal.name = "normal";
    normal.cache_id = "row-cache";
    face.variants.emplace("normal", std::move(normal));
    builder.addFace(std::move(face));
    return std::move(builder).freeze();
}

mml::MmlMo &appendMo(mml::MmlDocument &document, mml::MmlNode &parent, std::u16string value,
                     bool explicitBreak = false) {
    auto &mo = dynamic_cast<mml::MmlMo &>(document.factory().make(mml::NodeKind::mo));
    auto &token = dynamic_cast<mml::TextNode &>(document.factory().make(mml::NodeKind::text));
    token.setText(std::move(value));
    mo.appendChild(token);
    if (explicitBreak) {
        mo.attributes().set(u"linebreak", u"newline");
        mo.attributes().set(u"linebreakstyle", u"before");
    }
    parent.appendChild(mo);
    return mo;
}
} // namespace

suite native_common_mrow = [] {
    "empty_inferred_mrow_factory_lifecycle_and_scale_inheritance"_test = [] {
        auto fonts = makeRepository();
        RejectUnexpectedMeasurement measurement;
        mml::MmlDocument document;
        auto &math = document.factory().make(mml::NodeKind::math);
        auto &inferred = document.factory().make(mml::NodeKind::inferred_mrow);
        math.attributes().setInherited(u"mathsize", u"1.25em");
        document.append(math, inferred);
        document.setRoot(math);

        common::CommonOptions options;
        common::ExtendedMetrics metrics;
        common::CommonOutputJax jax(document, {metrics, options, fonts, "row-fixture"}, measurement,
                                    catalog(), &makeLinebreaks);
        auto &root = jax.factory().wrap(math);
        auto *row = dynamic_cast<common::CommonInferredMrow *>(root.childNodes.at(0));
        expect(row != nullptr);
        if (!row)
            return;
        expect(!row->isStack);
        expect(row->breakCount() == 0_u);
        expect(root.bbox.scale == 1.25);
        expect(row->bbox.scale == root.bbox.scale);
        expect(row->bbox.rscale == 1.0);
        auto &box = row->getBBox();
        expect(box.w == 0.0);
        expect(box.h == 0.0);
        expect(box.d == 0.0);
    };

    "Mrow_counts_an_explicit_MML_newline_from_factory_wrapped_Mo"_test = [] {
        auto fonts = makeRepository();
        RejectUnexpectedMeasurement measurement;
        mml::MmlDocument document;
        auto &math = document.factory().make(mml::NodeKind::math);
        auto &rowNode = document.factory().make(mml::NodeKind::mrow);
        document.append(math, rowNode);
        auto &newline = appendMo(document, rowNode, u"+", true);
        appendMo(document, rowNode, u"x");
        document.setRoot(math);

        common::CommonOptions options;
        common::ExtendedMetrics metrics;
        metrics.display = true;
        common::CommonOutputJax jax(document, {metrics, options, fonts, "row-fixture"}, measurement,
                                    catalog(), &makeLinebreaks);
        (void)jax.factory().wrap(math);
        auto *row = dynamic_cast<common::CommonMrow *>(jax.wrapperFor(rowNode));
        expect(row != nullptr);
        if (!row)
            return;
        auto *mo = dynamic_cast<common::CommonMo *>(row->childNodes.at(0));
        expect(mo != nullptr);
        if (!mo)
            return;
        expect(mo->node == &newline);
        expect(mo->breakStyle == "before");
        expect(row->breakCount() == 1_u);
        auto makeIndentedLine = [](double width, double initialLeft) {
            auto line = std::make_shared<common::LineBBox>(
                folia::math::BBoxData{.w = width, .h = .75, .d = .25});
            line->L = initialLeft;
            line->indentData = std::make_shared<common::IndentData>();
            const common::IndentPair indent{u"left", u"1em"};
            line->indentData->lines = {indent, indent, indent};
            return line;
        };
        row->lineBBox = {makeIndentedLine(2.0, .5), makeIndentedLine(3.0, .25)};
        for (double width : {6.0, 8.0, 10.0}) {
            expect(row->setChildPWidths(false, width));
            expect(row->lineBBox[0]->L == 1.0);
            expect(row->lineBBox[1]->L == 1.0);
        }
        // Width-driven line creation is witnessed separately. The supplied
        // line boxes isolate repeated indentation of an existing broken row.
    };
};
