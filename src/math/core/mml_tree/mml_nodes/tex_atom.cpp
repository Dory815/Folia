/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MmlNodes/TeXAtom.ts (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA-256: bad114224392b94852456a4636b85ad90a5a2c96b63e6c7911d155db299f7028
 *************************************************************/
module folia.math.core.mml_tree.mml_nodes.tex_atom;
import folia.math.core.mml_tree.arithmetic_host;
import folia.math.core.mml_tree.operator_class;
import std;

namespace folia::math::mml {

MmlNode* MmlTeXAtom::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    host::arithmeticChild(*this, 0).setTeXclass(nullptr);
    return adjustTeXclass(previous);
}

MmlNode* MmlTeXAtom::adjustTeXclass(MmlNode* previous) {
    return adjustOperatorTeXclass(*this, previous);
}

} // namespace folia::math::mml
