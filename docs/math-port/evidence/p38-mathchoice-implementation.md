# P38 MathChoice implementation report

Implemented only `MathChoice::inheritAttributes` in `mathchoice.cpp`, mapped to
the complete pinned MathJax 4.1.3 method at
`ts/core/MmlTree/MmlNodes/mathchoice.ts:74–84` (SHA-256
`e1f9fdf92e345c7bc09e3333c4f436ae8f2d1e4c8fa9a2280877590cf9025d76`).

The method enters `TreeCallGuard`, tests raw `display` truthiness, and otherwise
computes `Math.max(0, Math.min(Number(level), 2)) + 1` through Root's NaN- and
signed-zero-aware helpers. It performs the tagged numeric child lookup without
truncating fractional or NaN indices. A falsey selected slot triggers exactly
one zero-argument `mrow` factory call. The selected `NodeValue` is retained
across factory and parent callbacks. It then reads the current raw parent,
invokes `replaceChild(child, this)` and ignores the result, and only after that
returns calls inherited-attribute dispatch on the retained child with the
original four raw values/handles. The method does not normalize defaults, call
the base inheritance implementation, or modify any other node state itself.

No build or test was run for this delivery. Root owns independent review and
acceptance; this report makes no acceptance claim.
