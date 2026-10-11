/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of input/tex/Token.ts::Macro (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA-256: 6b4c0e42f277daebe8e676afc3b4d3b6083ebc87939ef8203672a831be1595bf
 *************************************************************/
module folia.math.input.tex.macro;
import std;

namespace folia::math::tex {

Macro::Macro(Property token, ParseMethod func, Property args)
    : token_(std::move(token)), func_(std::move(func)),
      args_(args.missing() ? Property(PropertyArray{}) : std::move(args)) {}

Property Macro::token() const { return token_; }
ParseMethod Macro::func() const { return func_; }
Property Macro::args() const { return args_; }

} // namespace folia::math::tex
