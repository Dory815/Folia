# Native node records and ownership

The node-state component uses closed value-owned records for TeX, proof,
layout, table and annotation properties. It has no runtime string-key access.
The 47 in-node Source keys are accounted for; in-lists belongs to the still
missing typed ParseOptions worklist/copyMembership service. isChars is also
represented from the annotation constructor. fnOP has no scoped use and remains
an explicit audit gap. Complete package annotation projection remains pending.

Pinned Source MmlFactory constructor observations cover all 43 kinds, including
raw operator class storage separately from its dictionary getter. The Source
copy observations preserve present false/zero/empty values. Bussproofs' local
move helper is not exported; its three-field semantics were inspected directly
at ts/input/tex/bussproofs/BussproofsUtil.ts:296-305 rather than inferred from the
withdrawn C++ implementation.

Native compilation passed. All four focused node-state tests pass with 89
assertions, including whole-record equality after copy/merge, constructor
defaults, proof moves, fractional previous script levels and deep independent
clones. Root corrected the draft clone test's mistaken assumption that math's
first child is a direct token; the generic clone case now uses a valid mrow.
Root also added whole-record value comparisons so assigning every field alone
cannot stand in for checking every field. Logs: ignored
build/math-port/native-raii-component-1.log through component-4.log.

The complete math run is not accepted: font-profile initialization separately
exposed an oversized generated native stack frame. That issue is being fixed
at its generator boundary; no executable stack reserve is increased.
