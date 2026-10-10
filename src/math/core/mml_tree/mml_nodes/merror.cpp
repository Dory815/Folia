/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/Merror.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.merror;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {

void MmlMerror::initializeClassFields() {
    initializeTexClass(TEXCLASS.ORD);
}

std::string MmlMerror::kind() const { return "merror"; }
double MmlMerror::arity() const { return -1; }
bool MmlMerror::linebreakContainer() const { return true; }

}
