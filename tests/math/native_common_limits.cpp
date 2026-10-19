#include "support/folia_test.hpp"
import folia.math.output.common;
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.fonts.newcm_font_module_provider;

namespace {
using namespace folia::math;
using namespace folia::math::output::common;
using namespace boost::ut;
namespace mml = folia::math::mml;
static_assert(std::is_base_of_v<CommonScriptbase, CommonMunder>);
static_assert(std::is_base_of_v<CommonScriptbase, CommonMover>);
static_assert(std::is_base_of_v<CommonScriptbase, CommonMunderover>);
static_assert(!std::is_base_of_v<CommonMsub, CommonMunder>);
static_assert(!std::is_base_of_v<CommonMsup, CommonMover>);
static_assert(!std::is_base_of_v<CommonMsubsup, CommonMunderover>);

struct Target final : MeasurementTarget {
    ExtendedMetrics measureMetrics(MetricRequest const &request) override {
        return {.em = 16,
                .ex = 8,
                .containerWidth = 640,
                .scale = 1,
                .family = request.family,
                .display = false};
    }

    UnknownBBox measureUnknownText(UnknownTextRequest const &) override {
        throw std::logic_error("limits fixture uses known glyphs");
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
                           registration<CommonMath>(K::math),
                           registration<CommonInferredMrow>(K::inferred_mrow),
                           registration<CommonMunder>(K::munder),
                           registration<CommonMover>(K::mover),
                           registration<CommonMunderover>(K::munderover),
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
             std::array<std::pair<char const *, double>, 17>{{{"sub1", .2},
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
                                                              {"big_op_spacing5", .15},
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

mml::MmlNode &makeNode(mml::MmlDocument &d, mml::NodeKind kind,
                       std::span<mml::MmlNode *const> children, std::u16string attr = {},
                       mml::AttributeValue value = {}) {
    auto &node = d.factory().make(kind, {}, children);
    if (!attr.empty())
        node.attributes().set(std::move(attr), std::move(value));
    return node;
}

std::array<double, 3> box(CommonWrapper &wrapper) {
    const auto b = wrapper.getOuterBBox();
    return {b.w, b.h, b.d};
}

CommonWrapper &wrapInMath(CommonOutputJax &jax, mml::MmlNode &node) {
    auto &document = node.document();
    const std::array<mml::MmlNode *, 1> children{&node};
    auto &math = document.factory().make(mml::NodeKind::math, {}, children);
    document.setRoot(math);
    (void)jax.factory().wrap(math);
    return *jax.wrapperFor(node);
}
} // namespace

suite native_common_limits_factory = [] {
    "common_munder_matches_pinned_source_bbox_and_child_slot"_test = [] {
        Target target;
        mml::MmlDocument d;
        auto jax = makeJax(d, target, fixtureFonts(), true);
        auto &base = token(d, mml::NodeKind::mi, u"x");
        auto &under = token(d, mml::NodeKind::mi, u"i");
        mml::MmlNode *kids[]{&base, &under};
        auto &node = makeNode(d, mml::NodeKind::munder, kids);
        d.setRoot(node);
        auto &w = dynamic_cast<CommonMunder &>(wrapInMath(jax, node));
        const auto b = box(w);
        expect(w.scriptChild() == w.childNodes[1]);
        expect(dynamic_cast<CommonScriptbase *>(&w) != nullptr);
        expect(eq(b[0], .5_d));
        expect(eq(b[1], .7_d));
        expect(eq(b[2], .8500000000000001));
    };
    "common_mover_accent_matches_pinned_source_bbox"_test = [] {
        Target target;
        mml::MmlDocument d;
        auto jax = makeJax(d, target, fixtureFonts(), true);
        auto &base = token(d, mml::NodeKind::mi, u"x");
        auto &over = token(d, mml::NodeKind::mi, u"2");
        mml::MmlNode *kids[]{&base, &over};
        auto &node = makeNode(d, mml::NodeKind::mover, kids, u"accent", true);
        d.setRoot(node);
        auto &w = dynamic_cast<CommonMover &>(wrapInMath(jax, node));
        const auto b = box(w);
        expect(w.scriptChild() == w.childNodes[1]);
        expect(dynamic_cast<CommonScriptbase *>(&w) != nullptr);
        expect(eq(b[0], .5_d));
        expect(eq(b[1], 1.4499999999999997));
        expect(eq(b[2], .2_d));
    };
    "common_munderover_places_both_limits_and_matches_pinned_source"_test = [] {
        Target target;
        mml::MmlDocument d;
        auto jax = makeJax(d, target, fixtureFonts(), true);
        auto &base = token(d, mml::NodeKind::mi, u"x");
        auto &under = token(d, mml::NodeKind::mi, u"i");
        auto &over = token(d, mml::NodeKind::mi, u"2");
        mml::MmlNode *kids[]{&base, &under, &over};
        auto &node = makeNode(d, mml::NodeKind::munderover, kids);
        d.setRoot(node);
        auto &w = dynamic_cast<CommonMunderover &>(wrapInMath(jax, node));
        const auto b = box(w);
        expect(w.underChild() == w.childNodes[1]);
        expect(w.overChild() == w.childNodes[2]);
        expect(w.scriptChild() == w.underChild());
        expect(dynamic_cast<CommonScriptbase *>(&w) != nullptr);
        expect(eq(b[0], .5_d));
        expect(eq(b[1], 1.3499999999999999));
        expect(eq(b[2], .8500000000000001));
    };
    "common_munderover_movable_limits_use_source_scriptbase_flow"_test = [] {
        const auto bundle = folia::math::fonts::makeNewcmFontData();
        Target target;
        mml::MmlDocument d;
        auto jax = makeJax(d, target, bundle.fonts, false);
        auto &integral = token(d, mml::NodeKind::mo, u"∫");
        integral.attributes().set(u"movablelimits", true);
        auto &under = token(d, mml::NodeKind::mi, u"i");
        auto &over = token(d, mml::NodeKind::mi, u"2");
        mml::MmlNode *kids[]{&integral, &under, &over};
        auto &node = makeNode(d, mml::NodeKind::munderover, kids, u"displaystyle", false);
        node.setInheritedAttributes();
        node.setTeXclass(nullptr);
        d.setRoot(node);
        auto &w = dynamic_cast<CommonMunderover &>(wrapInMath(jax, node));
        expect(w.hasMovableLimits());
        const auto result = box(w);
        const auto base = w.baseChild()->getOuterBBox();
        const auto script = w.scriptChild()->getOuterBBox();
        const double expectedW =
            std::max(base.rscale * (base.w + base.L + base.R),
                     w.getBaseWidth() + script.rscale * (script.w + script.L + script.R)) +
            jax.face().parameters.required("scriptspace");
        expect(eq(result[0], expectedW));
        expect(eq(result[1], std::max(base.rscale * base.h, script.rscale * script.h)));
        expect(eq(result[2], std::max(base.rscale * base.d, script.rscale * script.d)));
    };
    "common_munder_movable_limits_keep_scriptbase_zero_offset"_test = [] {
        const auto bundle = folia::math::fonts::makeNewcmFontData();
        Target target;
        mml::MmlDocument d;
        auto jax = makeJax(d, target, bundle.fonts, false);
        auto &integral = token(d, mml::NodeKind::mo, u"∫");
        integral.attributes().set(u"movablelimits", true);
        auto &under = token(d, mml::NodeKind::mi, u"i");
        mml::MmlNode *kids[]{&integral, &under};
        auto &node = makeNode(d, mml::NodeKind::munder, kids, u"displaystyle", false);
        node.setInheritedAttributes();
        node.setTeXclass(nullptr);
        d.setRoot(node);
        auto &w = dynamic_cast<CommonMunder &>(wrapInMath(jax, node));
        expect(w.hasMovableLimits());
        expect(w.getOffset()[0] == 0.0_d);
        expect(w.getOffset()[1] == 0.0_d);
        const auto result = box(w);
        const auto base = w.baseChild()->getOuterBBox();
        const auto script = w.scriptChild()->getOuterBBox();
        expect(eq(result[0],
                  std::max(base.rscale * (base.w + base.L + base.R),
                           w.getBaseWidth() + script.rscale * (script.w + script.L + script.R)) +
                      jax.face().parameters.required("scriptspace")));
    };
    "common_mover_movable_limits_keep_scriptbase_zero_offset"_test = [] {
        const auto bundle = folia::math::fonts::makeNewcmFontData();
        Target target;
        mml::MmlDocument d;
        auto jax = makeJax(d, target, bundle.fonts, false);
        auto &integral = token(d, mml::NodeKind::mo, u"∫");
        integral.attributes().set(u"movablelimits", true);
        auto &over = token(d, mml::NodeKind::mi, u"2");
        mml::MmlNode *kids[]{&integral, &over};
        auto &node = makeNode(d, mml::NodeKind::mover, kids, u"displaystyle", false);
        node.setInheritedAttributes();
        node.setTeXclass(nullptr);
        d.setRoot(node);
        auto &w = dynamic_cast<CommonMover &>(wrapInMath(jax, node));
        expect(w.hasMovableLimits());
        expect(w.getOffset()[0] == 0.0_d);
        expect(w.getOffset()[1] == 0.0_d);
        const auto result = box(w);
        const auto base = w.baseChild()->getOuterBBox();
        const auto script = w.scriptChild()->getOuterBBox();
        expect(eq(result[0],
                  std::max(base.rscale * (base.w + base.L + base.R),
                           w.getBaseWidth() + script.rscale * (script.w + script.L + script.R)) +
                      jax.face().parameters.required("scriptspace")));
    };
};
