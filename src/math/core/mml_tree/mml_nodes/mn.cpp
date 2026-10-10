/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/mn.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.mn;
import std;

namespace folia::math::mml {
void MmlMn::initializeClassFields() {
    initializeTexClass(static_cast<double>(TEXCLASS.ORD));
}

std::string MmlMn::kind() const {
    return "mn";
}
}
