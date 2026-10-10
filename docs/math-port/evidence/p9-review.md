# Independent P9 source review

This is a read-only review of the submitted P9 implementations and root-owned
interfaces. It is not a build, test, or acceptance report. I reviewed the ten
concrete TypeScript files in the pinned MathJax 4.1.3 source tree, the matching
resolved CJS modules, `docs/math-port/leaf-contracts.md`,
`docs/math-port/mml-contracts.md`, `leaf_defaults.ixx`, `leaf_host.ixx`,
`class_binding.ixx`, the ten MML interfaces, and their implementation files.
The source/runtime fingerprints in `evidence/p9-source-lock.json` are the
fingerprints used for this review.

## Findings

I found no remaining concrete source-order, coercion, identity, null-access, or
post-super field-initialization defect in the reviewed implementations. The
earlier `TEXCLASS::ORD` spelling in `merror.cpp` and `mphantom.cpp` has been
corrected to the frozen `TEXCLASS.ORD` object member. The mstyle implementation
uses `host::methodString` for explicit `.toString()`, `host::relativeLevel`,
`host::integer`, and `host::add` for the source's parse and primitive `+=`
rules. It applies strict boolean `displaystyle`, retains raw `data-cramped`,
and only dispatches to child zero. The row loops capture or reread child-array
handles according to the corresponding TypeScript loop form; the indexed
reverse scan and post-traversal first-child lookup reread current child storage.
The token methods retain UTF-16 length/pattern input, inherited attribute
ordering, explicit-presence checks, and strict comparisons. Math retains the
source's truthiness and strict-string tests, its live captured-constructor
defaults lookup, the original prime argument, and the base-verification-before-
nested-tag-error order.

Static defaults, mutable `NONSPACELIKE` collections, `MmlMi` compiled pattern
callbacks, and actual constructor-class binding remain root-owned. The
`classBinding` implementation runs `initializeClassFields()` after
`initializeMml()`, and the MML interfaces expose the direct `initializeTexClass`
field write needed by all six classes with TeX-class initializers. `TEXCLASS`
is an object, not a C++ type. I found no missing host primitive or declaration
blocking the reviewed bodies.

## Bounded host adaptations and pending gates

`Mstyle.notParent` returns a C++ `bool`; the source getter can produce an
undefined/null falsey value when child zero is absent/null. This matches its
truthiness-only callers and is the explicitly frozen adaptation. Null outer or
nested child-array access still raises TypeError; a null/empty child zero maps
to false. Native TypeError messages are operation-specific and need not match
engine wording byte-for-byte. `Pattern` is a native compiled matcher rather
than a replaceable JavaScript `RegExp`, and `methodString` does not execute
arbitrary user callbacks. Those host-value differences are bounded by the
frozen raw-TeX contract; they would need separate work before exposing these
nodes to arbitrary JavaScript values.

The frozen interfaces and CJS-derived fixture remain root-owned. The root
reported the oracle at 644 assertions across 19 cases; I did not execute it or
build the native target. Consequently native parity, the fixture's actual
current results, compile status, and mutation sensitivity remain pending root
validation. The full MML factory registration and downstream renderer remain
separate gates.

## Reviewed member coverage

| Classes | Implemented source members reviewed | Root-owned members/interfaces |
|---|---|---|
| `MmlMrow`, `MmlInferredMrow` | `kind`, `isSpacelike`, `isEmbellished`, `core`, `coreMO`, `nonSpaceLength`, `firstNonSpace`, `lastNonSpace`, `setTeXclass`, `isInferred`, `notParent`, `toString`, and the `_core` field phase | `defaults()` bindings and class metadata |
| `MmlMath` | `kind`, `linebreakContainer`, `linebreakAlign`, `setChildInheritedAttributes`, `verifyTree` | `defaults()` binding and class metadata |
| `MmlMi` | `kind`, `setInheritedAttributes`, `setTeXclass`, and the `texclass` field phase | `defaults()`, live pattern callbacks, class metadata |
| `MmlMn` | `kind` and the `texclass` field phase | `defaults()` and class metadata |
| `MmlMtext` | `kind`, `isSpacelike`, and the `texclass` field phase | `defaults()`, live `NONSPACELIKE()`, class metadata |
| `MmlMs` | `kind` and the `texclass` field phase | `defaults()` and class metadata |
| `MmlMspace` | `kind`, `arity`, `isSpacelike`, `hasNewline`, `canBreak`, `setTeXclass`, and the `texclass` field phase | `defaults()`, live `NONSPACELIKE()`, class metadata |
| `MmlMstyle` | `kind`, `notParent`, `setInheritedAttributes`, `setChildInheritedAttributes` | `defaults()` and class metadata |
| `MmlMerror` | `kind`, `arity`, `linebreakContainer`, and the `texclass` field phase | `defaults()` and class metadata |
| `MmlMphantom` | `kind` and the `texclass` field phase | `defaults()` and class metadata |

Inherited base members are outside this packet and are reviewed in the prior
MML evidence. No unimplemented source member was found among the ten assigned
classes; this source review does not substitute for the root's acceptance gate.
