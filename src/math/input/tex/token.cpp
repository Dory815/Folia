/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of input/tex/Token.ts::Token (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA-256: 6b4c0e42f277daebe8e676afc3b4d3b6083ebc87939ef8203672a831be1595bf
 *************************************************************/
module folia.math.input.tex.token;
import std;

namespace folia::math::tex {

Token::Token(std::u16string spelling, std::optional<std::u16string> translated,
             std::optional<mml::AttributeSet> values)
    : token(std::move(spelling)), character(std::move(translated)),
      attributes(std::move(values)) {}

} // namespace folia::math::tex
