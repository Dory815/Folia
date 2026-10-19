#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.operator_class;
import folia.math.core.mml_tree.operator_dictionary;
import folia.math.core.mml_tree.mo_patterns;

using namespace boost::ut;
using namespace folia::math::mml;

namespace {
MmlMo &makeMo(MmlDocument &document, std::u16string text) {
    auto &node = document.factory().make(NodeKind::mo);
    auto &textNode = dynamic_cast<TextNode &>(document.factory().make(NodeKind::text));
    textNode.setText(text);
    node.appendChild(textNode);
    return dynamic_cast<MmlMo &>(node);
}
} // namespace

suite typed_mml_mo = [] {
    "mo raw and factory initialization preserve Source class-field phases"_test = [] {
        MmlDocument document;
        auto &rawNode = document.make(NodeKind::mo);
        auto *raw = dynamic_cast<MmlMo *>(&rawNode);
        expect(raw != nullptr);
        expect(raw->texClassMode() == MoTexClassMode::unset);
        expect(!raw->lspace.has_value());
        expect(!raw->rspace.has_value());
        expect(!raw->texClass().has_value());

        auto &initialized = makeMo(document, u"+");
        expect(initialized.texClassMode() == MoTexClassMode::cleared);
        expect(initialized.lspace == 5.0 / 18.0);
        expect(initialized.rspace == 5.0 / 18.0);
        expect(initialized.texClass() == TexClass::bin);
        // A getter resolves the dictionary record but Source does not run
        // checkOperatorTable here, so class-field spacing stays at 5/18.
        expect(initialized.lspace == 5.0 / 18.0);
        expect(initialized.rspace == 5.0 / 18.0);
        initialized.setTexClass(TexClass::rel);
        expect(initialized.texClassMode() == MoTexClassMode::explicit_value);
        expect(initialized.state().working_tex_class == TexClass::rel);
        initialized.setTexClass(std::nullopt);
        expect(initialized.texClassMode() == MoTexClassMode::cleared);
        expect(!initialized.state().working_tex_class.has_value());
        expect(initialized.texClass() == TexClass::bin);
    };

    "mo lookup uses ordered forms and typed operator records"_test = [] {
        MmlDocument document;
        auto &mo = makeMo(document, u"+");
        const auto forms = mo.getForms();
        expect(forms.size() == 3_u);
        expect(forms[0] == u"infix");
        auto *definition = operator_dict::findOperatorDefinition(forms[0], u"+");
        expect(definition != nullptr);
        expect(definition->texClass == TexClass::bin);
        expect(definition->left == 4.0);
        expect(definition->right == 4.0);
        const auto resolved = mo.texClass();
        expect(resolved == TexClass::bin);
        expect(mo.lspace == 5.0 / 18.0);
        expect(mo.rspace == 5.0 / 18.0);

        // The source lookup getter has no spacing side effects. Inheritance
        // dispatch invokes checkOperatorTable and applies the typed metrics.
        mo.setInheritedAttributes();
        expect(mo.lspace == definition->left / 18.0);
        expect(mo.rspace == definition->right / 18.0);

        mo.attributes().set(u"form", std::u16string(u"prefix"));
        const auto explicitForms = mo.getForms();
        expect(explicitForms.front() == u"infix");
        mo.setTexClass(std::nullopt);
        expect(mo.texClass() == TexClass::bin);
    };

    "mo explicit spacing attributes and defaults remain distinct"_test = [] {
        MmlDocument document;
        auto &node = document.factory().make(NodeKind::mo);
        auto &mo = dynamic_cast<MmlMo &>(node);
        expect(!mo.hasSpacingAttributes());
        mo.attributes().set(u"lspace", std::u16string(u"0em"));
        expect(mo.hasSpacingAttributes());
        const auto defaults = MmlMo::defaults();
        expect(std::get<std::u16string>(*defaults.defaultValue(u"form")) == u"infix");
        expect(std::get<bool>(*defaults.defaultValue(u"fence")) == false);
    };

    "mo pattern and prime remap services are closed typed data"_test = [] {
        expect(matchMoPattern(MmlMo::pseudoScripts(), u"'²"));
        expect(!matchMoPattern(MmlMo::pseudoScripts(), u"abc"));
        expect(matchMoPattern(MmlMo::opPattern(), u"abc"));
        expect(matchMoPattern(MmlMo::mathaccents(), u"^"));
        expect(MmlMo::remapPrimes().size() == 10_u);

        MmlDocument document;
        auto &mo = makeMo(document, u"''");
        mo.setInheritedAttributes();
        expect(mo.properties().tex.primes == u"′′");
        expect(mo.properties().tex.pseudo_script == true);

        auto &superscriptNode = document.factory().make(NodeKind::msup);
        auto &superscript = dynamic_cast<MmlMsubsup &>(superscriptNode);
        auto &base = document.factory().make(NodeKind::mi);
        auto &scriptedMo = makeMo(document, u"'²");
        superscript.appendChild(base);
        superscript.appendChild(scriptedMo);
        scriptedMo.setInheritedAttributes();
        expect(superscript.isKind(NodeKind::msubsup));
        expect(!superscript.isKind(NodeKind::msub));
        expect(scriptedMo.properties().tex.pseudo_script == false);
    };

    "shared TeX-class adjustment applies binary endpoint and relation rules"_test = [] {
        MmlDocument document;
        auto &rowNode = document.factory().make(NodeKind::mrow);
        auto &row = dynamic_cast<MmlMrow &>(rowNode);
        auto &left = makeMo(document, u"x");
        auto &op = makeMo(document, u"+");
        auto &right = makeMo(document, u"x");
        row.appendChild(left);
        row.appendChild(op);
        row.appendChild(right);
        left.setTexClass(TexClass::ord);
        op.setTexClass(TexClass::bin);
        expect(adjustOperatorTeXclass(op, &left) == &op);
        expect(op.prevClass() == TexClass::ord);
        expect(op.texClass() == TexClass::bin);

        op.setTexClass(TexClass::rel);
        left.setTexClass(TexClass::bin);
        expect(adjustOperatorTeXclass(op, &left) == &op);
        expect(op.prevClass() == TexClass::ord);
        expect(left.texClass() == TexClass::ord);
    };
};
