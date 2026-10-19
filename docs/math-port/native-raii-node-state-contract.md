# Native node state implementation contract

The 217-file pinned Source audit found 48 used node-property names and one
allowlist-only name (`fnOP`). Node properties contain primitive mathematical
data; borrowed MML pointers occur in parser stack frames, not these properties.
The audit packet is `drafts/native-raii/node-properties/typed-field-decisions.json`.
Its dynamic copy/enumeration operations become explicit typed transfers.

## Files and representation

Luna implements `core/mml_tree/node_properties.ixx/.cpp` with module
`folia.math.core.mml_tree.node_properties`, namespace `folia::math::mml`.
It depends only on std and the independent NodeKind enum. It exports owned records and closed enums, no node
pointers, callback or property-name lookup API. MmlNode owns NodeProperties
and MmlIntrinsicState by value and exposes named `properties()` and `state()`
references. Algorithms remain in their family modules. These records are data,
not another node hierarchy or an object runtime.

Use `optional<bool>` for a Source boolean property: absent differs from present
false for transfer and annotation even when a consumer uses truthiness. Use
`optional<u16string>` for delimiter/text fields (empty differs from absent).
All records support native value copy/move. There is no get/set/erase-by-string,
universal variant, reflection API or property enumeration at runtime.

`TexClass` has none=-1, ord=0, op=1, bin=2, rel=3, open=4, close=5, punct=6,
inner=7. `MmlIntrinsicState` has optional working_tex_class,
previous_tex_class and previous_script_level (optional<double>). These describe different stages
of TeX spacing. The explicit Source property `texClass` is a separate
`TexNodeProperties::explicit_tex_class`: it requests a class rather than
recording the mutable result of class adjustment. Do not maintain a duplicate
cached computed operator class. If the working class is absent for mo, its
family query derives the dictionary class without storing it.

## Exact data members

NodeProperties contains these named records. Every item below is optional
unless otherwise stated; names before parentheses are the native spelling,
names in parentheses identify Source spelling.

- TexNodeProperties boolean fields: auto_operator(autoOP), fix_bold(fixBold),
  fix_prescript(fixPrescript), fix_stretchy(fixStretchy), ignore_variant
  (ignore-variant), math_accent(mathaccent), movable_limits(movablelimits),
  move_sup_sub(movesupsub), no_dictionary_definition(noDictDef), pseudo_script
  (pseudoscript), relations_combined(relationsCombined), small_matrix
  (smallmatrix), sub_sup_allowed(subsupOK), prime_style(texprimestyle),
  variant_form(variantForm), with_delimiters(withDelims).
- TexNodeProperties owned strings: open, close, left_delimiter(ldelim),
  right_delimiter(rdelim), primes, keep_attributes(keep-attrs), vertical_box
  (vbox). Preserve vbox strings supplied by parbox; do not guess a finite enum.
  `script_alignment(scriptalign)` is optional HorizontalAlignment
  {left,center,right}; package parsing validates conversion at its boundary.
  `script_position(sub-sup)` is optional ScriptPosition {subscript,superscript}.
  `thickness` is optional `variant<double,u16string>` for its proven numeric-zero
  and lexical-dimension alternatives. `explicit_tex_class` is optional TexClass.
- ProofNodeProperties boolean fields: axiom(bspr_axiom), proof(bspr_proof),
  sequent(bspr_sequent), sequent_processed(bspr_sequentProcessed).
  `premise_count(bspr_inference)` is optional size_t; `rule_direction`
  (bspr_inferenceRule) is optional ProofRuleDirection {up,down};
  `labelled_rule(bspr_labelledRule)` is optional ProofLabels {left,right,both};
  `proof_label(bspr_prooflabel)` is optional ProofSide {left,right}.
  Optional doubles: sequent_adjust_left/right(bspr_sequentAdjust_left/right),
  left_space(proof-left), right_space(proof-right).
- LayoutNodeProperties boolean fields: breakable, force_break(forcebreak),
  inline_breaks(inline-breaks), inline_forced(inlineForced), inline_marked
  (inlineMarked), newline, process_breaks(process-breaks). Source writes both
  true and string "true" for process-breaks; all scoped consumers use truthiness
  and its serialized spelling is identical. Normalize that value once to a
  boolean at the producer boundary, not through generic coercion.
- TableNodeProperties: optional<bool> use_height(useHeight); initialized true
  only for mtable by MmlFactory. AnnotationNodeProperties: optional<bool>
  character_data, initialized true only for annotation by MmlFactory; this represents Source isChars
  even though the earlier literal-property scan missed its direct initializer.

`in-lists` is not a string field on MmlNode. ParseOptions owns typed auxiliary
worklist membership and exposes copyMembership during TeX node copying. It will
use explicit list kinds, not an identity/object registry. fix_* markers above
are semantic filter markers; the worklist service owns scheduling. `fnOP` has
no scoped writer/consumer and no native field; retain it as an explicit audit
gap if a later package call site needs it.

## Construction and transfer

MmlFactory initializes intrinsic classes with Source constructors: ord for mi,
mn,mtext,ms,msqrt,mroot,merror,mphantom,menclose,mtable,mglyph,TeXAtom; none for
mspace; inner for mfenced; absent for other kinds. Mo dictionary class is a
family query. Working mo left/right spacing will belong to its later family
state, not a duplicate tex-class property.

`copyNodeProperties(from,to)` performs value assignment of the records only,
not intrinsic class state. `mergeNodeProperties(from,to)` copies each present
property over the destination, preserving destination values for absent source
fields. It must not silently clear present false/zero/empty values. Proof's
move routine transfers and clears exactly premise_count, proof and labelled_rule
when present, matching Bussproofs.moveProperties; other proof data stays put.
No transfer is implemented by serializing to attributes or property-name maps.

MmlDocument::cloneSubtree deep-copies text, attributes, properties and intrinsic
state as its complete native clone operation. Source-style ParseUtil copy and
relation combination will use the explicitly narrower transfer functions.
Factory-created inferred/explicit rows receive constructor state through the
same factory path. Document ownership remains unchanged.

## Acceptance and scope

Implement paired Source observations for constructor classes, present/absent
false/zero/empty transfer, relation-style merge, the three-field proof move and
deep clone independence. Test native mutation/reset without a universal string
API. Check every member in the finite mapping, not a representative subset.
Record the string-true normalization, request worklist exclusion and fnOP gap.
Luna writes an ignored complete packet; Root reviews, promotes, builds and
commits. This packet implements data and ownership, not family algorithms.
