#include "support/folia_test.hpp"
import folia.math.output.common;
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.mmultiscripts;
import folia.math.fonts.newcm_font_module_provider;

namespace {
using namespace folia::math;
using namespace folia::math::output::common;
using namespace boost::ut;
namespace mml = folia::math::mml;

struct Target final : MeasurementTarget {
    ExtendedMetrics measureMetrics(MetricRequest const &request) override {
        return {.em = 16,
                .ex = 8,
                .containerWidth = 640,
                .scale = 1,
                .family = request.family,
                .display = request.display};
    }

    UnknownBBox measureUnknownText(UnknownTextRequest const &) override {
        throw std::logic_error("script fixture uses known NewCM glyphs");
    }
};

template <class T> CommonWrapperRegistration registration(mml::NodeKind kind) {
    using Args = WrapperBuildArguments;
    using Creator = tree::FactoryClass<CommonWrapper, Args>;
    return {kind,
            Creator{[](Args const &a) -> std::unique_ptr<CommonWrapper> {
                        return std::make_unique<T>(a.factory, a.node, a.parent);
                    },
                    [](CommonWrapper const &w) { return dynamic_cast<T const *>(&w) != nullptr; }},
            CommonWrapperClassData::commonDefaults()};
}

class NoBreaks final : public Linebreaks {
  public:
    using Linebreaks::Linebreaks;

    void breakToWidth(CommonWrapper &, double) override {
        throw std::logic_error("unexpected linebreak");
    }
};

class FixedToken final : public CommonWrapper {
  protected:
    void computeBBox(BBox &out, bool) override {
        const bool base = getText() == u"x";
        out.w = base ? .5 : .2;
        out.h = base ? .7 : .3;
        out.d = base ? .2 : .1;
        out.ic = base ? .08 : 0;
        out.sk = base ? .02 : 0;
    }

  public:
    using CommonWrapper::CommonWrapper;
};

CommonWrapperCatalog catalog(bool fixedTokens) {
    using K = mml::NodeKind;
    CommonWrapperCatalog c{registration<CommonTextNode>(K::text),
                           registration<CommonMsub>(K::msub),
                           registration<CommonMsup>(K::msup),
                           registration<CommonMsubsup>(K::msubsup),
                           registration<CommonMmultiscripts>(K::mmultiscripts),
                           registration<CommonWrapper>(K::mprescripts),
                           registration<CommonWrapper>(K::none)};
    if (fixedTokens) {
        c.push_back(registration<FixedToken>(K::mi));
        c.push_back(registration<FixedToken>(K::mn));
    } else {
        c.push_back(registration<CommonMi>(K::mi));
        c.push_back(registration<CommonMn>(K::mn));
        c.push_back(registration<CommonMo>(K::mo));
        c.push_back(registration<CommonMrow>(K::mrow));
        c.push_back(registration<CommonInferredMrow>(K::inferred_mrow));
    }
    return c;
}

FontRepository const &fixtureFonts() {
    static const auto repo = [] {
        FontRepositoryBuilder b;
        FontFaceData f;
        f.name = "MathJaxNewcm";
        for (auto name : {"normal", "italic", "bold", "bold-italic"}) {
            FontVariant v;
            v.name = name;
            v.cache_id = name;
            f.variants.emplace(name, std::move(v));
        }
        b.addFace(std::move(f));
        for (auto [name, value] :
             std::array<std::pair<char const *, double>, 16>{{{"sub1", .2},
                                                              {"sub2", .16},
                                                              {"sup1", .4},
                                                              {"sup2", .3},
                                                              {"sup3", .5},
                                                              {"sub_drop", .1},
                                                              {"sup_drop", .1},
                                                              {"x_height", .4},
                                                              {"rule_thickness", .04},
                                                              {"separation_factor", 5},
                                                              {"big_op_spacing1", .1},
                                                              {"big_op_spacing2", .1},
                                                              {"big_op_spacing3", .1},
                                                              {"big_op_spacing4", .1},
                                                              {"scriptspace", .05},
                                                              {"extra_ic", 0}}})
            b.addParameter("MathJaxNewcm", name, value);
        return std::move(b).freeze();
    }();
    return repo;
}

CommonOutputJax makeJax(mml::MmlDocument &d, Target &target, FontRepository const &fonts,
                        bool fixed) {
    ExtendedMetrics metrics{.em = 16,
                            .ex = 8,
                            .containerWidth = 640,
                            .scale = 1,
                            .family = "MathJaxNewcm",
                            .display = false};
    return CommonOutputJax(d, CommonOutputJaxConfig{metrics, {}, fonts, "MathJaxNewcm"}, target,
                           catalog(fixed),
                           [](CommonWrapperFactory &f) -> std::unique_ptr<Linebreaks> {
                               return std::make_unique<NoBreaks>(f);
                           });
}

mml::MmlNode &token(mml::MmlDocument &d, mml::NodeKind kind, std::u16string value) {
    auto &t = dynamic_cast<mml::TextNode &>(d.factory().make(mml::NodeKind::text));
    t.setText(std::move(value));
    mml::MmlNode *children[]{&t};
    return d.factory().make(kind, {}, children);
}

std::array<mml::MmlNode *, 2> subChildren(mml::MmlDocument &d) {
    return {&token(d, mml::NodeKind::mi, u"x"), &token(d, mml::NodeKind::mi, u"i")};
}
} // namespace

suite native_common_scripts_factory = [] {
    "common_factory_msub_matches_pinned_source_numeric_offset"_test = [] {
        Target target;
        mml::MmlDocument d;
        auto jax = makeJax(d, target, fixtureFonts(), true);
        auto kids = subChildren(d);
        auto &node = d.factory().make(mml::NodeKind::msub, {}, kids);
        d.setRoot(node);
        auto &w = dynamic_cast<CommonMsub &>(jax.factory().wrap(node));
        const auto offset = w.getOffset();
        expect(w.baseCore == w.childNodes[0]);
        expect(w.scriptChild() == w.childNodes[1]);
        expect(eq(offset[0], 0.0_d));
        expect(eq(offset[1], -.2_d));
        expect(eq(w.getDelta(*w.scriptChild()), .06_d));
    };

    "common_factory_msup_matches_pinned_source_numeric_offset"_test = [] {
        Target target;
        mml::MmlDocument d;
        auto jax = makeJax(d, target, fixtureFonts(), true);
        auto kids = subChildren(d);
        auto &node = d.factory().make(mml::NodeKind::msup, {}, kids);
        d.setRoot(node);
        auto &w = dynamic_cast<CommonMsup &>(jax.factory().wrap(node));
        const auto offset = w.getOffset();
        expect(w.scriptChild() == w.childNodes[1]);
        expect(eq(offset[0], .054000000000000006));
        expect(eq(offset[1], .3_d));
    };

    "common_factory_msubsup_matches_pinned_source_uvq"_test = [] {
        Target target;
        mml::MmlDocument d;
        auto jax = makeJax(d, target, fixtureFonts(), true);
        auto &base = token(d, mml::NodeKind::mi, u"x");
        auto &sub = token(d, mml::NodeKind::mi, u"i");
        auto &sup = token(d, mml::NodeKind::mi, u"2");
        mml::MmlNode *kids[]{&base, &sub, &sup};
        auto &node = d.factory().make(mml::NodeKind::msubsup, {}, kids);
        d.setRoot(node);
        auto &w = dynamic_cast<CommonMsubsup &>(jax.factory().wrap(node));
        const auto uvq = w.getUVQ();
        expect(w.subChild() == w.childNodes[1]);
        expect(w.supChild() == w.childNodes[2]);
        expect(eq(uvq[0], .42000000000000004));
        expect(eq(uvq[1], -.09999999999999992));
        expect(eq(uvq[2], .12_d));
        const auto cached = w.getUVQ();
        expect(eq(cached[0], uvq[0]));
        expect(eq(cached[1], uvq[1]));
        expect(eq(cached[2], uvq[2]));
    };

    "common_factory_mmultiscripts_preserves_pinned_list_counts_and_uvq"_test = [] {
        Target target;
        mml::MmlDocument d;
        auto jax = makeJax(d, target, fixtureFonts(), true);
        auto &base = token(d, mml::NodeKind::mi, u"x");
        auto &sub = token(d, mml::NodeKind::mi, u"i");
        auto &sup = token(d, mml::NodeKind::mi, u"2");
        auto &marker = d.factory().make(mml::NodeKind::mprescripts);
        auto &psub = token(d, mml::NodeKind::mi, u"j");
        auto &psup = token(d, mml::NodeKind::mi, u"3");
        mml::MmlNode *kids[]{&base, &sub, &sup, &marker, &psub, &psup};
        auto &node = d.factory().make(mml::NodeKind::mmultiscripts, {}, kids);
        d.setRoot(node);
        auto &w = dynamic_cast<CommonMmultiscripts &>(jax.factory().wrap(node));
        const auto &data = w.scriptsData();
        expect(data.numScripts == 1_u);
        expect(data.numPrescripts == 1_u);
        expect(w.firstPrescript() == 4_u);
        const auto uvq = w.getUVQ();
        expect(eq(uvq[0], .42000000000000004));
        expect(eq(uvq[1], -.09999999999999992));
        expect(eq(uvq[2], .12_d));
    };

    "real_newcm_provider_runs_independent_source_script_trees"_test = [] {
        const auto bundle = folia::math::fonts::makeNewcmFontData();
        Target target;
        {
            mml::MmlDocument d;
            auto jax = makeJax(d, target, bundle.fonts, false);
            auto kids = subChildren(d);
            auto &node = d.factory().make(mml::NodeKind::msub, {}, kids);
            node.setInheritedAttributes();
            node.setTeXclass(nullptr);
            d.setRoot(node);
            auto &w = dynamic_cast<CommonMsub &>(jax.factory().wrap(node));
            const auto offset = w.getOffset();
            const auto &params = jax.face().parameters;
            const auto base = w.baseCore->getOuterBBox();
            const auto script = w.scriptChild()->getOuterBBox();
            const double expected = std::max(
                {w.baseCharZero(w.baseCore->getLineBBox(w.baseCore->breakCount())->d * w.baseScale +
                                params.required("sub_drop") * script.rscale),
                 params.required("sub1"),
                 script.h * script.rscale - .8 * params.required("x_height")});
            expect(eq(offset[1], -expected));
            expect(base.rscale == 1.0_d);
        }
        {
            mml::MmlDocument d;
            auto jax = makeJax(d, target, bundle.fonts, false);
            auto &baseNode = token(d, mml::NodeKind::mi, u"x");
            auto &supNode = token(d, mml::NodeKind::mi, u"2");
            mml::MmlNode *kids[]{&baseNode, &supNode};
            auto &node = d.factory().make(mml::NodeKind::msup, {}, kids);
            node.setInheritedAttributes();
            node.setTeXclass(nullptr);
            d.setRoot(node);
            auto &w = dynamic_cast<CommonMsup &>(jax.factory().wrap(node));
            const auto offset = w.getOffset();
            const auto &params = jax.face().parameters;
            const auto script = w.scriptChild()->getOuterBBox();
            const auto line = w.baseCore->getLineBBox(w.baseCore->breakCount());
            const double expected =
                std::max({w.baseCharZero(line->h * w.baseScale -
                                         params.required("sup_drop") * script.rscale),
                          params.required("sup2"),
                          script.d * script.rscale + .25 * params.required("x_height")});
            expect(eq(offset[0], w.getAdjustedIc() - (w.baseRemoveIc ? 0 : w.baseIc)));
            expect(eq(offset[1], expected));
        }
        {
            mml::MmlDocument d;
            auto jax = makeJax(d, target, bundle.fonts, false);
            auto &baseNode = token(d, mml::NodeKind::mi, u"x");
            auto &subNode = token(d, mml::NodeKind::mi, u"i");
            auto &supNode = token(d, mml::NodeKind::mi, u"2");
            mml::MmlNode *kids[]{&baseNode, &subNode, &supNode};
            auto &node = d.factory().make(mml::NodeKind::msubsup, {}, kids);
            node.setInheritedAttributes();
            node.setTeXclass(nullptr);
            d.setRoot(node);
            auto &w = dynamic_cast<CommonMsubsup &>(jax.factory().wrap(node));
            const auto &params = jax.face().parameters;
            const auto sub = w.subChild()->getOuterBBox();
            const auto sup = w.supChild()->getOuterBBox();
            const auto line = w.baseCore->getLineBBox(w.baseCore->breakCount());
            double u = w.getU();
            double v = std::max(
                w.baseCharZero(line->d * w.baseScale + params.required("sub_drop") * sub.rscale),
                params.required("sub2"));
            const double supDepth = sup.d * sup.rscale, subHeight = sub.h * sub.rscale;
            double q = u - supDepth - (subHeight - v);
            const double clearance = 3 * params.required("rule_thickness");
            if (q < clearance) {
                v += clearance - q;
                const double correction = .8 * params.required("x_height") - (u - supDepth);
                if (correction > 0) {
                    u += correction;
                    v -= correction;
                }
            }
            q = u - supDepth - (subHeight - v);
            const auto uvq = w.getUVQ();
            expect(eq(uvq[0], u));
            expect(eq(uvq[1], -v));
            expect(eq(uvq[2], q));
        }
        {
            mml::MmlDocument d;
            auto jax = makeJax(d, target, bundle.fonts, false);
            auto &base = token(d, mml::NodeKind::mi, u"x");
            auto &sub = token(d, mml::NodeKind::mi, u"i");
            auto &sup = token(d, mml::NodeKind::mi, u"2");
            auto &marker = d.factory().make(mml::NodeKind::mprescripts);
            auto &psub = token(d, mml::NodeKind::mi, u"j");
            auto &psup = token(d, mml::NodeKind::mi, u"3");
            mml::MmlNode *nodes[]{&base, &sub, &sup, &marker, &psub, &psup};
            auto &node = d.factory().make(mml::NodeKind::mmultiscripts, {}, nodes);
            node.setInheritedAttributes();
            node.setTeXclass(nullptr);
            d.setRoot(node);
            auto &w = dynamic_cast<CommonMmultiscripts &>(jax.factory().wrap(node));
            expect(w.scriptsData().numScripts == 1_u);
            expect(w.scriptsData().numPrescripts == 1_u);
            expect(std::isfinite(w.getUVQ()[0]));
            expect(std::isfinite(w.getUVQ()[1]));
        }
    };
};
