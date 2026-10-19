module folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mn;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.mml_nodes.mspace;
import folia.math.core.mml_tree.mml_nodes.ms;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mfrac;
import folia.math.core.mml_tree.mml_nodes.msqrt;
import folia.math.core.mml_tree.mml_nodes.mroot;
import folia.math.core.mml_tree.mml_nodes.mstyle;
import folia.math.core.mml_tree.mml_nodes.merror;
import folia.math.core.mml_tree.mml_nodes.mpadded;
import folia.math.core.mml_tree.mml_nodes.mphantom;
import folia.math.core.mml_tree.mml_nodes.mfenced;
import folia.math.core.mml_tree.mml_nodes.menclose;
import folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.core.mml_tree.mml_nodes.mmultiscripts;
import folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.mml_nodes.mtd;
import folia.math.core.mml_tree.mml_nodes.maligngroup;
import folia.math.core.mml_tree.mml_nodes.malignmark;
import folia.math.core.mml_tree.mml_nodes.mglyph;
import folia.math.core.mml_tree.mml_nodes.semantics;
import folia.math.core.mml_tree.mml_nodes.tex_atom;
import folia.math.core.mml_tree.mml_nodes.mathchoice;
import folia.math.core.mml_tree.mml_nodes.html_node;
import std;

namespace folia::math::mml {
namespace {
void validateChildren(const MmlDocument &document, std::span<MmlNode *const> children) {
    for (std::size_t i = 0; i < children.size(); ++i) {
        auto *const child = children[i];
        if (!child) {
            throw std::invalid_argument("MML factory child cannot be null");
        }
        if (&child->document() != &document) {
            throw std::invalid_argument("MML factory child belongs to another document");
        }
        for (std::size_t j = 0; j < i; ++j) {
            auto *const previous = children[j];
            if (previous == child) {
                throw std::invalid_argument("MML factory children cannot contain duplicates");
            }
            for (auto *ancestor = previous; ancestor; ancestor = ancestor->parent()) {
                if (ancestor == child) {
                    throw std::invalid_argument("MML factory children cannot overlap by ancestry");
                }
            }
            for (auto *ancestor = child; ancestor; ancestor = ancestor->parent()) {
                if (ancestor == previous) {
                    throw std::invalid_argument("MML factory children cannot overlap by ancestry");
                }
            }
        }
    }
}

} // namespace

MmlFactory::MmlFactory(MmlDocument &document) : document_(document) {
    const auto registerClass = [this]<class Node>(NodeKind kind) {
        setNodeClass(kind, {[](MmlCreationArguments const &arguments) -> std::unique_ptr<MmlNode> {
                                return std::make_unique<Node>(arguments.document);
                            },
                            [](MmlNode const &node) {
                                return dynamic_cast<Node const *>(&node) != nullptr;
                            }});
    };
    registerClass.operator()<MmlMath>(NodeKind::math);
    registerClass.operator()<MmlMi>(NodeKind::mi);
    registerClass.operator()<MmlMn>(NodeKind::mn);
    registerClass.operator()<MmlMo>(NodeKind::mo);
    registerClass.operator()<MmlMtext>(NodeKind::mtext);
    registerClass.operator()<MmlMspace>(NodeKind::mspace);
    registerClass.operator()<MmlMs>(NodeKind::ms);
    registerClass.operator()<MmlMrow>(NodeKind::mrow);
    registerClass.operator()<MmlInferredMrow>(NodeKind::inferred_mrow);
    registerClass.operator()<MmlMfrac>(NodeKind::mfrac);
    registerClass.operator()<MmlMsqrt>(NodeKind::msqrt);
    registerClass.operator()<MmlMroot>(NodeKind::mroot);
    registerClass.operator()<MmlMstyle>(NodeKind::mstyle);
    registerClass.operator()<MmlMerror>(NodeKind::merror);
    registerClass.operator()<MmlMpadded>(NodeKind::mpadded);
    registerClass.operator()<MmlMphantom>(NodeKind::mphantom);
    registerClass.operator()<MmlMfenced>(NodeKind::mfenced);
    registerClass.operator()<MmlMenclose>(NodeKind::menclose);
    registerClass.operator()<MmlMaction>(NodeKind::maction);
    registerClass.operator()<MmlMsub>(NodeKind::msub);
    registerClass.operator()<MmlMsup>(NodeKind::msup);
    registerClass.operator()<MmlMsubsup>(NodeKind::msubsup);
    registerClass.operator()<MmlMunder>(NodeKind::munder);
    registerClass.operator()<MmlMover>(NodeKind::mover);
    registerClass.operator()<MmlMunderover>(NodeKind::munderover);
    registerClass.operator()<MmlMmultiscripts>(NodeKind::mmultiscripts);
    registerClass.operator()<MmlMprescripts>(NodeKind::mprescripts);
    registerClass.operator()<MmlNone>(NodeKind::none);
    registerClass.operator()<MmlMtable>(NodeKind::mtable);
    registerClass.operator()<MmlMlabeledtr>(NodeKind::mlabeledtr);
    registerClass.operator()<MmlMtr>(NodeKind::mtr);
    registerClass.operator()<MmlMtd>(NodeKind::mtd);
    registerClass.operator()<MmlMaligngroup>(NodeKind::maligngroup);
    registerClass.operator()<MmlMalignmark>(NodeKind::malignmark);
    registerClass.operator()<MmlMglyph>(NodeKind::mglyph);
    registerClass.operator()<MmlSemantics>(NodeKind::semantics);
    registerClass.operator()<MmlAnnotation>(NodeKind::annotation);
    registerClass.operator()<MmlAnnotationXML>(NodeKind::annotation_xml);
    registerClass.operator()<MmlTeXAtom>(NodeKind::tex_atom);
    registerClass.operator()<MathChoice>(NodeKind::math_choice);
    registerClass.operator()<TextNode>(NodeKind::text);
    registerClass.operator()<XMLNode>(NodeKind::xml);
    registerClass.operator()<HtmlNode>(NodeKind::html);
}

MmlNode &MmlFactory::adopt(std::unique_ptr<MmlNode> node) {
    return document_.adopt(std::move(node));
}

MmlNode &MmlFactory::allocateRaw(NodeKind kind) {
    return create(kind, MmlCreationArguments{document_});
}

MmlNode &MmlFactory::make(NodeKind kind, AttributeSet attributes,
                          std::span<MmlNode *const> children) {
    if (nodeKindName(kind).empty())
        throw std::invalid_argument("MML factory requires a registered node kind");
    validateChildren(document_, children);
    auto &node = allocateRaw(kind);
    node.initializeMml(std::move(attributes), children);
    return node;
}
} // namespace folia::math::mml
