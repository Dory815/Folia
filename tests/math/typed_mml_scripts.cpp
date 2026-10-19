#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.core.mml_tree.mml_nodes.mmultiscripts;

namespace {
using namespace boost::ut;
using namespace folia::math::mml;

suite typed_mml_script_families = [] {
    "script_family_kinds_arities_and_child_slots"_test = [] {
        MmlDocument document;
        auto &subsup = document.factory().make(NodeKind::msubsup);
        auto &sub = document.factory().make(NodeKind::msub);
        auto &sup = document.factory().make(NodeKind::msup);
        auto &munderover = document.factory().make(NodeKind::munderover);
        auto &under = document.factory().make(NodeKind::munder);
        auto &over = document.factory().make(NodeKind::mover);
        auto &multi = document.factory().make(NodeKind::mmultiscripts);
        auto &prescripts = document.factory().make(NodeKind::mprescripts);
        auto &none = document.factory().make(NodeKind::none);

        expect(subsup.kind() == NodeKind::msubsup && subsup.arity() == 3);
        expect(sub.kind() == NodeKind::msub && sub.arity() == 2);
        expect(sup.kind() == NodeKind::msup && sup.arity() == 2);
        expect(eq(dynamic_cast<MmlMsubsup &>(sub).base(), std::size_t{0}));
        expect(eq(dynamic_cast<MmlMsubsup &>(sub).sub(), std::size_t{1}));
        expect(eq(dynamic_cast<MmlMsubsup &>(sub).sup(), std::size_t{2}));
        expect(eq(dynamic_cast<MmlMsubsup &>(sup).sup(), std::size_t{1}));
        expect(eq(dynamic_cast<MmlMsubsup &>(sup).sub(), std::size_t{2}));

        expect(munderover.kind() == NodeKind::munderover && munderover.arity() == 3);
        expect(under.kind() == NodeKind::munder && under.arity() == 2);
        expect(over.kind() == NodeKind::mover && over.arity() == 2);
        expect(dynamic_cast<MmlMunderover &>(munderover).linebreakContainer());
        expect(eq(dynamic_cast<MmlMunderover &>(over).over(), std::size_t{1}));
        expect(eq(dynamic_cast<MmlMunderover &>(over).under(), std::size_t{2}));
        expect(multi.kind() == NodeKind::mmultiscripts && multi.arity() == 1);
        expect(prescripts.kind() == NodeKind::mprescripts && prescripts.arity() == 0);
        expect(none.kind() == NodeKind::none && none.arity() == 0);

        expect(subsup.attributes().defaultValue(u"subscriptshift") != nullptr);
        expect(munderover.attributes().defaultValue(u"accent") != nullptr);
        expect(under.attributes().defaultValue(u"accentunder") != nullptr);
    };
};

double inheritedScriptLevel(MmlNode const &node) {
    const auto *value = node.attributes().lookup(u"scriptlevel");
    if (!value)
        return -1;
    const auto *level = std::get_if<double>(value);
    return level ? *level : -1;
}

suite typed_mml_script_inheritance = [] {
    "msup_inherits_into_its_physical_script_slot_without_a_subscript"_test = [] {
        MmlDocument document;
        auto &base = document.factory().make(NodeKind::none);
        auto &script = document.factory().make(NodeKind::none);
        std::array<MmlNode *, 2> children{&base, &script};
        auto &node = document.factory().make(NodeKind::msup, {}, children);
        node.setInheritedAttributes({}, true, 2, false);
        expect(node.children().size() == 2_u);
        expect(inheritedScriptLevel(base) == 2.0);
        expect(inheritedScriptLevel(script) == 3.0);
        expect(!base.properties().tex.prime_style.value_or(false));
        expect(!script.properties().tex.prime_style.value_or(false));
        node.setInheritedAttributes({}, false, 4, true);
        expect(inheritedScriptLevel(script) == 5.0);
        expect(script.properties().tex.prime_style.value_or(false));
    };

    "subsup_increases_script_level_only_for_script_children"_test = [] {
        MmlDocument document;
        auto &base = document.factory().make(NodeKind::none);
        auto &sub = document.factory().make(NodeKind::none);
        auto &sup = document.factory().make(NodeKind::none);
        std::array<MmlNode *, 3> children{&base, &sub, &sup};
        auto &node = document.factory().make(NodeKind::msubsup, {}, children);
        node.setInheritedAttributes({}, true, 2, false);
        expect(eq(inheritedScriptLevel(base), 2.0));
        expect(eq(inheritedScriptLevel(sub), 3.0));
        expect(eq(inheritedScriptLevel(sup), 3.0));
    };

    "mover_uses_over_slot_and_explicit_accent_script_level"_test = [] {
        MmlDocument document;
        auto &base = document.factory().make(NodeKind::none);
        auto &over = document.factory().make(NodeKind::none);
        std::array<MmlNode *, 2> children{&base, &over};
        auto &node = document.factory().make(NodeKind::mover, {}, children);
        node.attributes().set(u"accent", true);
        node.setInheritedAttributes({}, true, 4, false);
        expect(eq(inheritedScriptLevel(base), 4.0));
        expect(eq(inheritedScriptLevel(over), 4.0));
    };

    "multiscripts_adds_owned_none_for_unpaired_postscript"_test = [] {
        MmlDocument document;
        auto &base = document.factory().make(NodeKind::none);
        auto &sub = document.factory().make(NodeKind::none);
        std::array<MmlNode *, 2> children{&base, &sub};
        auto &node = document.factory().make(NodeKind::mmultiscripts, {}, children);
        node.setInheritedAttributes({}, false, 1, true);
        expect(eq(node.children().size(), std::size_t{3}));
        expect(node.children().back()->kind() == NodeKind::none);
        expect(eq(inheritedScriptLevel(*node.children().back()), 2.0));
    };
};
} // namespace
