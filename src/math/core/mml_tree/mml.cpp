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
#define FOLIA_MML_BIND(key, type) result->emplace_back(key, classBinding<type>())
        FOLIA_MML_BIND("math", MmlMath);

        FOLIA_MML_BIND("mi", MmlMi);
        FOLIA_MML_BIND("mn", MmlMn);
        FOLIA_MML_BIND("mo", MmlMo);
        FOLIA_MML_BIND("mtext", MmlMtext);
        FOLIA_MML_BIND("mspace", MmlMspace);
        FOLIA_MML_BIND("ms", MmlMs);

        FOLIA_MML_BIND("mrow", MmlMrow);
        FOLIA_MML_BIND("inferredMrow", MmlInferredMrow);
        FOLIA_MML_BIND("mfrac", MmlMfrac);
        FOLIA_MML_BIND("msqrt", MmlMsqrt);
        FOLIA_MML_BIND("mroot", MmlMroot);
        FOLIA_MML_BIND("mstyle", MmlMstyle);
        FOLIA_MML_BIND("merror", MmlMerror);
        FOLIA_MML_BIND("mpadded", MmlMpadded);
        FOLIA_MML_BIND("mphantom", MmlMphantom);
        FOLIA_MML_BIND("mfenced", MmlMfenced);
        FOLIA_MML_BIND("menclose", MmlMenclose);

        FOLIA_MML_BIND("maction", MmlMaction);

        FOLIA_MML_BIND("msub", MmlMsub);
        FOLIA_MML_BIND("msup", MmlMsup);
        FOLIA_MML_BIND("msubsup", MmlMsubsup);
        FOLIA_MML_BIND("munder", MmlMunder);
        FOLIA_MML_BIND("mover", MmlMover);
        FOLIA_MML_BIND("munderover", MmlMunderover);
        FOLIA_MML_BIND("mmultiscripts", MmlMmultiscripts);
        FOLIA_MML_BIND("mprescripts", MmlMprescripts);
        FOLIA_MML_BIND("none", MmlNone);

        FOLIA_MML_BIND("mtable", MmlMtable);
        FOLIA_MML_BIND("mlabeledtr", MmlMlabeledtr);
        FOLIA_MML_BIND("mtr", MmlMtr);
        FOLIA_MML_BIND("mtd", MmlMtd);
        FOLIA_MML_BIND("maligngroup", MmlMaligngroup);
        FOLIA_MML_BIND("malignmark", MmlMalignmark);

        FOLIA_MML_BIND("mglyph", MmlMglyph);

        FOLIA_MML_BIND("semantics", MmlSemantics);
        FOLIA_MML_BIND("annotation", MmlAnnotation);
        FOLIA_MML_BIND("annotation-xml", MmlAnnotationXML);

        FOLIA_MML_BIND("TeXAtom", MmlTeXAtom);
        FOLIA_MML_BIND("MathChoice", MathChoice);

        FOLIA_MML_BIND("text", TextNode);
        FOLIA_MML_BIND("XML", XMLNode);
        FOLIA_MML_BIND("html", HtmlNode);
#undef FOLIA_MML_BIND
        return result;
    }();
    return registry;
}
}
