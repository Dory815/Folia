/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 MmlNodes/ms.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.ms;
import std;

namespace folia::math::mml {
void MmlMs::initializeClassFields() {
    initializeTexClass(static_cast<double>(TEXCLASS.ORD));
}

std::string MmlMs::kind() const {
    return "ms";
}
}
