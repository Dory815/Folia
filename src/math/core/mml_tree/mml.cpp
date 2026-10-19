/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium
 * Native ordered registry for core/MmlTree/MML.ts (4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml;
import folia.math.core.mml_tree.class_binding;
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
const std::shared_ptr<tree::FactoryClassMap<MmlNode>>& MML() {
    static const auto registry = [] {
        auto result = std::make_shared<tree::FactoryClassMap<MmlNode>>();
        result->reserve(43);
#define FOLIA_MML_BIND(key, type, sourceName, sourceKind, parent, arity) result->emplace_back(key, classBinding<type>(sourceName, sourceKind, parent, arity))
#define FOLIA_MML_BIND_NO_DEFAULTS(key, type, sourceName, sourceKind, parent, arity) result->emplace_back(key, classBinding<type>(sourceName, sourceKind, parent, arity, std::nullopt, std::nullopt, false))
#define FOLIA_MML_ARITY(key, type, sourceName, sourceKind, parent, ctorArity, nodeArity) result->emplace_back(key, classBinding<type>(sourceName, sourceKind, parent, ctorArity, nodeArity))
#define FOLIA_MML_ARITY_NO_DEFAULTS(key, type, sourceName, sourceKind, parent, ctorArity, nodeArity) result->emplace_back(key, classBinding<type>(sourceName, sourceKind, parent, ctorArity, nodeArity, std::nullopt, false))
#define FOLIA_MML_ARITY_ACCENTS(key, type, sourceName, sourceKind, parent, ctorArity, nodeArity) result->emplace_back(key, classBinding<type>(sourceName, sourceKind, parent, ctorArity, nodeArity, std::nullopt, true, true))
#define FOLIA_MML_INFERRED(key, type, sourceName, sourceKind, parent, ctorArity) result->emplace_back(key, classBinding<type>(sourceName, sourceKind, parent, ctorArity, std::nullopt, true))
        FOLIA_MML_BIND("math", MmlMath, u"MmlMath", u"math", u"AbstractMmlLayoutNode", 0);

        FOLIA_MML_BIND("mi", MmlMi, u"MmlMi", u"mi", u"AbstractMmlTokenNode", 0);
        FOLIA_MML_BIND("mn", MmlMn, u"MmlMn", u"mn", u"AbstractMmlTokenNode", 0);
        FOLIA_MML_BIND("mo", MmlMo, u"MmlMo", u"mo", u"AbstractMmlTokenNode", 0);
        FOLIA_MML_BIND("mtext", MmlMtext, u"MmlMtext", u"mtext", u"AbstractMmlTokenNode", 0);
        FOLIA_MML_ARITY("mspace", MmlMspace, u"MmlMspace", u"mspace", u"AbstractMmlTokenNode", 0, 0.0);
        FOLIA_MML_BIND("ms", MmlMs, u"MmlMs", u"ms", u"AbstractMmlTokenNode", 0);

        FOLIA_MML_BIND("mrow", MmlMrow, u"MmlMrow", u"mrow", u"AbstractMmlNode", 0);
        FOLIA_MML_INFERRED("inferredMrow", MmlInferredMrow, u"MmlInferredMrow", u"inferredMrow", u"MmlMrow", 0);
        FOLIA_MML_ARITY("mfrac", MmlMfrac, u"MmlMfrac", u"mfrac", u"AbstractMmlBaseNode", 0, 2.0);
        FOLIA_MML_ARITY("msqrt", MmlMsqrt, u"MmlMsqrt", u"msqrt", u"AbstractMmlNode", 0, -1.0);
        FOLIA_MML_ARITY("mroot", MmlMroot, u"MmlMroot", u"mroot", u"AbstractMmlNode", 0, 2.0);
        FOLIA_MML_BIND("mstyle", MmlMstyle, u"MmlMstyle", u"mstyle", u"AbstractMmlLayoutNode", 0);
        FOLIA_MML_ARITY("merror", MmlMerror, u"MmlMerror", u"merror", u"AbstractMmlNode", 0, -1.0);
        FOLIA_MML_BIND("mpadded", MmlMpadded, u"MmlMpadded", u"mpadded", u"AbstractMmlLayoutNode", 0);
        FOLIA_MML_BIND("mphantom", MmlMphantom, u"MmlMphantom", u"mphantom", u"AbstractMmlLayoutNode", 0);
        FOLIA_MML_BIND("mfenced", MmlMfenced, u"MmlMfenced", u"mfenced", u"AbstractMmlNode", 0);
        FOLIA_MML_ARITY("menclose", MmlMenclose, u"MmlMenclose", u"menclose", u"AbstractMmlNode", 0, -1.0);

        FOLIA_MML_ARITY("maction", MmlMaction, u"MmlMaction", u"maction", u"AbstractMmlNode", 0, 1.0);

        FOLIA_MML_ARITY("msub", MmlMsub, u"MmlMsub", u"msub", u"MmlMsubsup", 0, 2.0);
        FOLIA_MML_ARITY("msup", MmlMsup, u"MmlMsup", u"msup", u"MmlMsubsup", 0, 2.0);
        FOLIA_MML_ARITY("msubsup", MmlMsubsup, u"MmlMsubsup", u"msubsup", u"AbstractMmlBaseNode", 0, 3.0);
        FOLIA_MML_ARITY("munder", MmlMunder, u"MmlMunder", u"munder", u"MmlMunderover", 0, 2.0);
        FOLIA_MML_ARITY_ACCENTS("mover", MmlMover, u"MmlMover", u"mover", u"MmlMunderover", 0, 2.0);
        FOLIA_MML_ARITY_ACCENTS("munderover", MmlMunderover, u"MmlMunderover", u"munderover", u"AbstractMmlBaseNode", 0, 3.0);
        FOLIA_MML_ARITY("mmultiscripts", MmlMmultiscripts, u"MmlMmultiscripts", u"mmultiscripts", u"MmlMsubsup", 0, 1.0);
        FOLIA_MML_ARITY("mprescripts", MmlMprescripts, u"MmlMprescripts", u"mprescripts", u"AbstractMmlNode", 0, 0.0);
        FOLIA_MML_ARITY("none", MmlNone, u"MmlNone", u"none", u"AbstractMmlNode", 0, 0.0);

        FOLIA_MML_BIND("mtable", MmlMtable, u"MmlMtable", u"mtable", u"AbstractMmlNode", 0);
        FOLIA_MML_ARITY_NO_DEFAULTS("mlabeledtr", MmlMlabeledtr, u"MmlMlabeledtr", u"mlabeledtr", u"MmlMtr", 0, 1.0);
        FOLIA_MML_BIND("mtr", MmlMtr, u"MmlMtr", u"mtr", u"AbstractMmlNode", 0);
        FOLIA_MML_ARITY("mtd", MmlMtd, u"MmlMtd", u"mtd", u"AbstractMmlBaseNode", 0, -1.0);
        FOLIA_MML_BIND("maligngroup", MmlMaligngroup, u"MmlMaligngroup", u"maligngroup", u"AbstractMmlLayoutNode", 0);
        FOLIA_MML_ARITY("malignmark", MmlMalignmark, u"MmlMalignmark", u"malignmark", u"AbstractMmlNode", 0, 0.0);

        FOLIA_MML_BIND("mglyph", MmlMglyph, u"MmlMglyph", u"mglyph", u"AbstractMmlTokenNode", 0);

        FOLIA_MML_ARITY("semantics", MmlSemantics, u"MmlSemantics", u"semantics", u"AbstractMmlBaseNode", 0, 1.0);
        FOLIA_MML_BIND("annotation", MmlAnnotation, u"MmlAnnotation", u"annotation", u"MmlAnnotationXML", 0);
        FOLIA_MML_BIND("annotation-xml", MmlAnnotationXML, u"MmlAnnotationXML", u"annotation-xml", u"AbstractMmlNode", 0);

        FOLIA_MML_ARITY("TeXAtom", MmlTeXAtom, u"TeXAtom", u"TeXAtom", u"AbstractMmlBaseNode", 3, -1.0);
        FOLIA_MML_ARITY("MathChoice", MathChoice, u"MathChoice", u"MathChoice", u"AbstractMmlBaseNode", 0, 4.0);

        FOLIA_MML_BIND_NO_DEFAULTS("text", TextNode, u"TextNode", u"text", u"AbstractMmlEmptyNode", 0);
        FOLIA_MML_BIND_NO_DEFAULTS("XML", XMLNode, u"XMLNode", u"XML", u"AbstractMmlEmptyNode", 0);
        FOLIA_MML_BIND_NO_DEFAULTS("html", HtmlNode, u"HtmlNode", u"html", u"XMLNode", 0);
#undef FOLIA_MML_BIND
#undef FOLIA_MML_BIND_NO_DEFAULTS
#undef FOLIA_MML_ARITY
#undef FOLIA_MML_ARITY_NO_DEFAULTS
#undef FOLIA_MML_ARITY_ACCENTS
#undef FOLIA_MML_INFERRED
        return result;
    }();
    return registry;
}
}
