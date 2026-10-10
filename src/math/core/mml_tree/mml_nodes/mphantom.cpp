/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native port of MathJax 4.1.3 MmlNodes/Mphantom.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mphantom;
import folia.math.core.mml_tree.leaf_host;
import std;

namespace folia::math::mml {

void MmlMphantom::initializeClassFields() {
    initializeTexClass(TEXCLASS.ORD);
}

std::string MmlMphantom::kind() const { return "mphantom"; }

}
