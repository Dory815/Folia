# Typed MML implementation assignments

2026-10-20. Root accepts the shared declaration shape in
`tools/math-port/drafts/native-raii/typed-mml-base-contract/mml_node.ixx`
for implementation, not as compiled or behaviorally accepted code.
The canonical destination is the existing `mml_document:node` partition.
`texClass()` remains a non-const virtual method without `noexcept`: Mo's actual
getter can perform operator lookup and install inherited attributes. Its old
algorithm cannot satisfy a const, non-throwing declaration.

## Shared boundaries

- Reuse current C++ methods, with `f4d04f25` as the pre-foundation base reference.
  Pinned MathJax 4.1.3 is the behavioral reference, not a reason to rewrite classes.
- Tree base owns the only links. Document owns actual family objects in one
  `unique_ptr` arena and one factory. Factory uses the existing typed Tree registry.
- `doc.make` is raw allocation; factory `make` initializes defaults and children
  by virtual `initializeMml` after adoption. Abstract MML methods own inferred-row
  normalization. Exact document clone and Source virtual copy remain distinct.
- Family modules export-import `folia.math.core.mml_tree.mml_document`.
  Implementation units for base/branches/empty belong to that primary module.
- Node pointers are borrowed; attributes are `AttributeSet` values, inheritance
  is `{name, origin, AttributeValue}`, and semantic properties use existing typed
  records. No Context, Property, callback receivers, generic coercion framework,
  second registry, detached semantic node, or placeholder family implementation.
- Concrete constructors accept `MmlDocument&`. Derived families may pass their
  kind through a protected `(MmlDocument&, NodeKind)` base-family constructor.
  Keep actual hierarchy and existing virtual methods.
- Each static family defaults method returns `AttributeSet`; shared materialization
  is `nodeDefaultAttributes(NodeKind)` in the existing `node_defaults` module.
  This helper installs immutable kind/global defaults; base initialization merges
  supplied layers afterward. Base/token static defaults preserve their own tables.
- Mfenced keeps actual generated Mo/Text objects, document owned and borrowed
  through its open/close/separators fields. `document.attachAuxiliary(parent, child)`
  checks ownership/cycles and sets the existing Tree parent link on a detached node
  without inserting it into normal children. The older suggestion to replace them
  with independent operator records belonged to the withdrawn structural-node
  design; it would now duplicate the actual Mo algorithms and is withdrawn.
- `AttributeList` entries already represent Source's `[origin, value]` tuples.
  Mtr's `data-vertical-align = [this.kind, alignment]` uses that same entry shape;
  it does not require an array alternative in `AttributeValue`.

## File ownership

| Packet | Existing files and responsibilities |
| --- | --- |
| base | mml_node.cpp: base accessors, constructor/defaults setup, inferred-row child methods, inheritance, verification, copy, TeX spacing |
| branches | mml_branches.cpp, mml_empty.cpp, html_adaptor and html_node: token/layout/base, empty/Text/XML/HTML behavior |
| row and tokens | mrow, mi, mn, mtext, ms, mspace, mglyph: original class semantics and text/row methods |
| arithmetic and layout | mfrac, msqrt, mroot, mstyle, merror, mpadded, mphantom, menclose: original family methods |
| scripts | msubsup, munderover, mmultiscripts: original script/limit inheritance, indexing and validation |
| table | mtable, mtr, mtd, maligngroup, malignmark: table propagation, validation, row/cell semantics |
| selection and containers | math, mathchoice, maction, mfenced, semantics, tex_atom: original orchestration and selection semantics |
| operator data | operator_dictionary, operator_dictionary_data, mo_patterns/mo_pattern_data: closed native dictionary/range/spacing records and existing finite patterns |
| operator behavior | mo_types, mo, mo_lookup, mo_classes, mo_scripts, operator_class: actual Mo methods and shared TeX class adjustment |
| Root integration | document/factory/NodeFactory, shared defaults helper, canonical promotion, CMake, native execution, review and commits |

Agents work in separate ignored packets and provide mirrored destination files,
focused Boost.UT cases, a method disposition list, and reproducible Source evidence.
They do not edit canonical files, build, run native executables, or commit.
Cross-packet API changes require Root's decision; report missing semantics instead
of adding a fallback. Existing binding-only files are not copied into the new path.

## Acceptance before production switch

1. All 43 kinds allocate actual concrete family objects through one registry;
   registered subtype queries work. No generic substitute for an unfinished kind.
2. Existing ownership, defaults, typed properties and exact clone tests remain;
   add family virtual dispatch, Source copy and inferred-row cases.
3. Focused tests cover each migrated method family and compare semantic results
   against pinned Source, including attributes/origins, text, class/spacing,
   script levels, malformed arity handling and explicit/omitted verify options.
4. Build one process at a time, total parallelism 4, math compilation pool 2.
   Native probes link console runtime. Stop a broader run on abnormal exit.
5. Root reviews preserved responsibilities and dependencies, builds and runs the
   integrated cohort. Uncompiled drafts do not count as completed components.
   Component completion does not establish TeX-to-SVG or Folia integration.
