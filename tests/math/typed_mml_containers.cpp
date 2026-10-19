#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mathchoice;
import folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.mml_nodes.mfenced;
import folia.math.core.mml_tree.mml_nodes.semantics;
import folia.math.core.mml_tree.mml_nodes.tex_atom;

namespace m = folia::math::mml;
using namespace boost::ut;

suite typed_mml_containers = [] {
    "math keeps source linebreak and display inheritance behavior"_test = [] {
        m::MmlDocument document;
        auto &math = static_cast<m::MmlMath &>(document.factory().make(m::NodeKind::math));
        expect(math.linebreakContainer());
        expect(math.linebreakAlign().empty());

        m::AttributeSet mathAttributes;
        mathAttributes.set(u"display", std::u16string{u"block"});
        auto &displayMath = static_cast<m::MmlMath &>(
            document.factory().make(m::NodeKind::math, std::move(mathAttributes)));
        displayMath.setInheritedAttributes({}, false, 0.0, false);
        const auto *displayStyle = displayMath.attributes().lookup(u"displaystyle");
        expect(displayStyle && std::get_if<bool>(displayStyle) && *std::get_if<bool>(displayStyle));
    };

    "mathchoice selects branch by display and script level"_test = [] {
        m::MmlDocument document;
        auto &factory = document.factory();
        auto &choiceNode = factory.make(m::NodeKind::math_choice);
        auto &choice = static_cast<m::MathChoice &>(choiceNode);
        auto &a = factory.make(m::NodeKind::mi);
        auto &b = factory.make(m::NodeKind::mi);
        auto &c = factory.make(m::NodeKind::mi);
        auto &d = factory.make(m::NodeKind::mi);
        m::MmlNode *branches[]{&a, &b, &c, &d};
        choice.setChildren(branches);
        auto &root = factory.make(m::NodeKind::mrow);
        root.appendChild(choice);
        choice.setInheritedAttributes({}, false, 1.0, false);
        expect(root.children().size() == 1_u);
        expect(root.children()[0] == &c);
        expect(c.parent() == &root);

        auto &fractionalNode = factory.make(m::NodeKind::math_choice);
        auto &fractional = static_cast<m::MathChoice &>(fractionalNode);
        auto &f0 = factory.make(m::NodeKind::mi);
        auto &f1 = factory.make(m::NodeKind::mi);
        auto &f2 = factory.make(m::NodeKind::mi);
        auto &f3 = factory.make(m::NodeKind::mi);
        m::MmlNode *fractionalBranches[]{&f0, &f1, &f2, &f3};
        fractional.setChildren(fractionalBranches);
        auto &fractionalParent = factory.make(m::NodeKind::mrow);
        fractionalParent.appendChild(fractional);
        fractional.setInheritedAttributes({}, false, 0.5, false);
        expect(fractionalParent.children()[0]->kind() == m::NodeKind::mrow);
    };

    "maction selection clamps, toggles and delegates selected projections"_test = [] {
        m::MmlDocument document;
        auto &factory = document.factory();
        auto &actionNode = factory.make(m::NodeKind::maction);
        auto &action = static_cast<m::MmlMaction &>(actionNode);
        auto &first = factory.make(m::NodeKind::mi);
        auto &second = factory.make(m::NodeKind::mo);
        m::MmlNode *children[]{&first, &second};
        action.setChildren(children);
        expect(&action.selected() == &first);
        action.nextToggleSelection();
        expect(&action.selected() == &second);
        action.nextToggleSelection();
        expect(&action.selected() == &first);
        auto &third = factory.make(m::NodeKind::mrow);
        m::MmlNode *threeChildren[]{&first, &second, &third};
        action.setChildren(threeChildren);
        action.attributes().set(u"selection", std::u16string{u"\u00A00x2"});
        expect(&action.selected() == &second);
        action.nextToggleSelection();
        expect(std::get<double>(*action.attributes().own(u"selection")) == 3.0);
        expect(&action.selected() == &third);

        action.attributes().set(u"selection", std::u16string{u"1e21"});
        expect(&action.selected() == &third);
        action.nextToggleSelection();
        expect(std::get<double>(*action.attributes().own(u"selection")) == 2.0);

        action.attributes().set(u"selection", 1.0e21);
        action.nextToggleSelection();
        expect(std::get<double>(*action.attributes().own(u"selection")) == 2.0);

        action.attributes().set(u"selection", 1.5);
        expect(action.selected().kind() == m::NodeKind::mrow);

        action.attributes().set(u"actiontype", std::u16string{u"tooltip"});
        action.setInheritedAttributes({}, true, 2.0, false);
        const auto *firstDisplay = first.attributes().lookup(u"displaystyle");
        const auto *secondDisplay = second.attributes().lookup(u"displaystyle");
        expect(firstDisplay && std::get_if<bool>(firstDisplay) && *std::get_if<bool>(firstDisplay));
        expect(secondDisplay && std::get_if<bool>(secondDisplay) &&
               !*std::get_if<bool>(secondDisplay));

        action.attributes().set(u"actiontype", std::u16string{u"statusline"});
        m::VerifyOptions verifyOptions;
        verifyOptions.checkArity = false;
        verifyOptions.checkAttributes = true;
        verifyOptions.checkMathvariants = false;
        verifyOptions.fixMmultiscripts = false;
        verifyOptions.fixMtables = false;
        action.verifyTree(verifyOptions);
        expect(action.attributes().own(u"selection") == nullptr);
    };

    "semantics and annotation retain typed defaults and character data state"_test = [] {
        m::MmlDocument document;
        auto &factory = document.factory();
        auto &semantics = static_cast<m::MmlSemantics &>(factory.make(m::NodeKind::semantics));
        auto &annotation = static_cast<m::MmlAnnotation &>(factory.make(m::NodeKind::annotation));
        auto &payload = factory.make(m::NodeKind::mi);
        auto &annotationXml =
            static_cast<m::MmlAnnotationXML &>(factory.make(m::NodeKind::annotation_xml));
        m::MmlNode *payloadChildren[]{&payload};
        annotationXml.setChildren(payloadChildren);
        annotationXml.setInheritedAttributes({}, false, 3.0, false);
        expect(semantics.arity() == 1.0);
        expect(semantics.notParent());
        expect(annotation.properties().annotation.character_data.has_value());
        expect(*annotation.properties().annotation.character_data);
        expect(annotation.kind() == m::NodeKind::annotation);
        expect(annotationXml.kind() == m::NodeKind::annotation_xml);
        const auto *payloadLevel = payload.attributes().lookup(u"scriptlevel");
        expect(payloadLevel && std::get_if<double>(payloadLevel) &&
               *std::get_if<double>(payloadLevel) == 0.0);
    };

    "TeXAtom owns ORD intrinsic state and retains infinite arity"_test = [] {
        m::MmlDocument document;
        auto &atom = static_cast<m::MmlTeXAtom &>(document.factory().make(m::NodeKind::tex_atom));
        expect(atom.arity() == -1.0);
        expect(atom.notParent());
        expect(atom.state().working_tex_class == m::TexClass::ord);
    };

    "mfenced fake operators are auxiliary children with the same document parent"_test = [] {
        m::MmlDocument document;
        auto &factory = document.factory();
        auto &fenced = static_cast<m::MmlMfenced &>(factory.make(m::NodeKind::mfenced));
        auto &one = factory.make(m::NodeKind::mi);
        auto &two = factory.make(m::NodeKind::mi);
        m::MmlNode *fencedChildren[]{&one, &two};
        fenced.setChildren(fencedChildren);
        auto &root = factory.make(m::NodeKind::math);
        root.appendChild(fenced);
        fenced.setInheritedAttributes({}, true, 0.0, false);
        expect(fenced.children().size() == 2_u);
        expect(fenced.open != nullptr);
        expect(fenced.close != nullptr);
        expect(fenced.separators.size() == 1_u);
        expect(fenced.open->parent() == &fenced);
        expect(fenced.open->state().working_tex_class == m::TexClass::open);
        expect(fenced.open->attributes().lookup(u"fence") &&
               std::get_if<bool>(fenced.open->attributes().lookup(u"fence")) &&
               *std::get_if<bool>(fenced.open->attributes().lookup(u"fence")));
        expect(fenced.open->children().size() == 1_u);
        expect(fenced.open->children()[0]->kind() == m::NodeKind::text);
        expect(fenced.close->state().working_tex_class == m::TexClass::close);
        expect(fenced.state().working_tex_class == m::TexClass::inner);
    };
};
