# BaseItems native candidate coverage

The pinned original oracle is `tools/math-port/base-items-original-oracle.cjs`; it writes `build/math-port/base-items-original.json`. Its direct method observations have matching native Boost.UT assertions in `tools/math-port/parser-family-reference/base-items-native-tests.cpp`:

| Original observation | Native candidate | Compared behavior |
|---|---|---|
| `class-metadata` and `constructors` (all 26 exported constructors) | `all_twenty_six_base_items_use_the_registered_source_classes`; `base_item_constructor_callbacks_preserve_source_arguments_and_defaults` | Builds every registered product through the live Base package StackItemFactory and checks exact virtual kind/predicates, constructor length/name/prototype kind getter, and representative supplied/default constructor state against the oracle's per-class argument table. Full class reflection remains pending the generic Context class adapter. |
| `constructorIdentity` (fresh per-ArrayItem records, default parser/atEnd/templateSubs, Mstyle PropertyList identity) | `base-items-constructor-identity-native-tests.cpp` | Independent direct-source/native matrix verifies each ArrayItem gets distinct table, row, frame, hfill, arraydef, cstart/cend/cextra/ralign/breakAlign identities; mutating one breakAlign/frame leaves the second unchanged; and MstyleItem retains the caller's live attributes handle. This TU awaits Root CMake integration and native execution. |
| `base-item-checkitem-fallbacks`; `break-item-default-inserts-mspace` | `base_item_checkitem_fallbacks_match_source_return_shapes` | Exercises ordinary fallback, shared-success and fresh tuple returns, Function/Not/Dots/Subsup short-circuits, and BreakItem's actual inserted `mspace` plus retained item through the real factory and MML factory. |
| `mstyle-matching-end-wraps-content`; `equation-item-stop-throws-missing-end` | `mstyle_and_equation_items_preserve_their_environment_branches` | Calls both real environment-specific item methods; verifies matching Mstyle wrapping and EquationItem's live missing-name substitution (`Missing \\end{}`). |
| `array-rowspacing-and-create-mml-side-effects` | `array_rowspacing_and_create_mml_preserve_source_side_effects` | Verifies addRowSpacing’s dimensional update, createMml’s scriptlevel deletion and mstyle wrapper, and real mtable smallmatrix/break-align/array-padding attributes. |
| `array-handle-frame-branches` | `array_handle_frame_covers_solid_and_mixed_source_paths` | Covers handleFrame’s partial-solid menclose, four-side solid frame, and mixed-style data-frame-styles branches using actual frame arrays and MML factory nodes. |
| `eqnarray-endentry-missing-columnalign` | `eqnarray_endentry_missing_columnalign_throws_before_base_reduction` | Verifies EqnArrayItem's own `.split` failure occurs before ArrayItem reduction, preserving empty row and stack state. |
| `prime-single`, `prime-double`, `script-subscript`, `not-operator`, `dots-ldots-cdots`, `named-function`, `style-display`, `position-raise-lower`, `nonscript-space`, `left-middle-right`, `over-array-boundary`, `array-basic`, `array-column-frame`, `array-width-and-spacing` | `base_items_classes_are_reached_by_real_texparser_inputs` | Runs those same pinned CJS input cases through the actual native TexParser and checks MML root kinds, reaching the Prime/Subsup/Not/Dots/Fn/Style/Position/Nonscript/Left-Middle-Right/Over and Array createMml/frame paths via the package's handlers. |
| `stack-item-factory-metadata` | `stack_item_factory_binds_the_actual_parse_options_service` | ParseOptions’ actual shared factory is bound to one Context instance with the real class prototype and static `DefaultStackItems.base` constructor identity. The inherited `AbstractFactory.prototype` chain/method reflection and canonical own data fields remain pending their generic runtime adapters. |
| `null-item-constructor-call-on-prototype-object` | `null_item_constructor_call_initializes_the_supplied_receiver` | Emitted CJS constructor call returns the caller-owned receiver, initializes a real NullItem owner, exposes the expected virtual kind/state, and repeated calls preserve that same owner identity. |
| `open-stop-inherited-class-error` | `open_stop_uses_derived_class_error_record` | OpenItem resolves its own `stop` error from the canonical derived class error record. |
| Additive StackItemFactory argument witness (`tools/math-port/base-items-factory-args-original.cjs`) | `base_item_factory_args_preserve_native_scalar_and_nullish_slots` | Direct `start` construction distinguishes omitted/undefined/null, preserves bool/number/string/UTF-16 values and Context object identity, and rejects opaque values without coercion. This new row is not part of the frozen original corpus and awaits the next native gate. |
| `subsup-error-splice-extra-arguments` | `subsup_error_spreads_trailing_class_error_arguments` | Mutating the canonical class error record verifies that `SubsupItem.checkItem` splices and forwards message substitutions after the first two entries, and mutates the same error tuple as Source does. |
| `begin-end-name-mismatch` | `begin_end_name_mismatch_uses_source_environment_error` | Actual BeginItem/EndItem names produce the pinned `EnvBadEnd` identifier and formatted message. |
| `open-close-wraps-texatom` | `open_close_wraps_actual_content_in_tex_atom` | Actual Open/Close stack items produce one MmlItem containing a TeXAtom whose child is an inferred row containing the pushed MML node. |
| `over-double-over` | `over_double_over_uses_real_items_and_error` | Actual factory-owned OverItems, default name/close state, and `AmbiguousUseOf` error. |
| `array-default-storage` | `array_item_initial_state_keeps_actual_typed_storage` | Real ArrayItem storage kinds/defaults and canonical class/instance Context identities. |
| `array-end-entry-empty` | `array_end_entry_empty_uses_real_mml_factory_and_clears_stack` | Actual MML `mtd` creation, row append, stack clearing, and fresh empty hfill. |
| `array-end-entry-ralign` | `array_end_entry_ralign_builds_mpadded_and_preserves_source_values` | The source ralign tuple drives a real `mpadded` node, attributes, `vbox` property, moved children, and stack clearing. |
| `array-end-entry-null-hfill-partial` | `array_end_entry_null_hfill_throws_before_row_or_stack_mutation` | Exact null-length TypeError and unchanged row/stack after the failing read. |
| `array-end-row-labeled-break-align` | `array_end_row_consumes_label_and_break_alignment` | Actual labeled row kind, break-align attribute, fresh row, label reset, and atEnd reset. |
| `array-check-lines-padding` | `array_check_lines_updates_frame_and_row_spacing_records` | Source rowlines becomes a bottom frame pair and rowspacing is padded from the current property. |
| `array-check-lines-live-rowlines-get` | `array_check_lines_reloads_rowlines_before_split` | A getter returns a truthy first value and undefined on the second read; Source performs both Gets then throws before changing the frame. |
| `array-start-entry-cextra-short-circuit` | `array_start_entry_preserves_cextra_short_circuit_and_live_reads` | Accessor-backed cextra slots verify the source's short-circuit plus later fresh slot reads (`0,1,0`) without early `cextra[n+1]` access. |
| `over-array-boundary` | `over_array_boundary_uses_real_parser_and_mml_factory` | Actual TeX parser and MML factory produce the TeXAtom/inferred-row/fraction tree. |

The candidate fixture initializes one real `ParserRuntime`, Base package, TeX/ParseOptions, StackItemFactory, and MmlFactory. It contains no substitute parser, factory, or MML nodes. Reflection of the native class `errors` data remains pending Root's canonical adapter; the Subsup native candidate mutates the existing canonical C++ error record directly to test the Source spread behavior.

## Direct BaseItem reduction additions

`base-items-original-oracle.cjs` now includes `base-item-checkitem-reduction-errors-and-order` and `position-item-close-missing-box`. The independently integrable `base-items-reductions-native-tests.cpp` covers those native-reachable branches using actual package-created items: `open` + `over` records the exact pushed MML node in the over item's `num` property before clearing the open stack; a truthy cell `linebreak` returns the shared fail tuple without clearing or otherwise changing the open stack; a false linebreak throws `Misplaced` with the cell's current name; a final `mml` item transfers its first node to the receiver and returns the shared fail tuple; `OpenItem` + `StopItem` resolves the derived inherited error entry and throws the exact `ExtraOpenMissingClose` message; `LeftItem` + `RightItem` produces one real `mml` item with the fenced `mrow` properties; and `PositionItem` + `StopItem` throws the exact blank-substitution `MissingBoxFor` message while preserving its `move` property and stack. The oracle's accessor trace confirms `linebreak` is read before `name`. The test translation unit is separate from the existing BaseItems candidate so it can be integrated without changing that in-flight candidate source again.

The oracle additionally records `position-item-horizontal-final-expansion` and `style-item-close-wraps-content`; `base-items-position-style-native-tests.cpp` adds direct real-factory cases for both. The horizontal branch asserts left-MML/final-original/right-MML ordering and node identity. The style close branch checks the generated `mstyle` attributes, inferred row child, and retained stop item. These remain independent candidate TUs pending Root integration and native execution.

The pinned oracle now also records `prime-empty-script-wraps-two-live-stack-nodes`, `subsup-missing-open-sup-error`, and `not-item-explicit-overlay-mutates-token-in-place`. `base-items-prime-subsup-native-tests.cpp` exercises the direct `PrimeItem.checkItem` wrapping branch with two actual MML nodes (including retained input-item identity), the `SubsupItem.checkItem` position-2 missing-open error with unchanged position/empty stack, and NotItem's explicit overlay branch preserving the original token and appending the combining text node in place. This candidate is separate from the in-flight fixture and awaits Root integration/native execution.

## Per-class audit

The oracle's `constructors` object records every exported class's constructor arity, own enumerable fields, initialized values, kind and virtual flags. The native matrix directly covers those class identities, constructor lengths/names/prototype getters, and live-factory product predicates. Method-specific coverage is:

| Source class | Locked CJS behavior exercised | Native candidate behavior exercised |
|---|---|---|
| StartItem | Constructor/global, `start` and start/stop parse paths | Constructor/global and fallback return shape |
| StopItem | Close predicate and open/stop error path | All-class constructor/predicate matrix; used as real factory item in errors |
| OpenItem | Open/close wrapping and open/stop class error | Direct MML wrapping, error-record lookup, fallback |
| CloseItem | Close predicate and open/close handling | Direct MML wrapping path |
| NullItem | Constructor metadata and null-item constructor ordinary call | Same-receiver ordinary-call behavior and factory identity |
| PrimeItem | Single/double-prime parser cases | Native TexParser prime root-kind cases |
| SubsupItem | Subscript parser case, missing-script error and mutating `splice(2)` | Direct class-error mutation/spread plus parser path |
| OverItem | Double-over error and `{a\\over b}` | Direct double-over and MML parser path |
| LeftItem | `\\left...\\right` fence path | Native TexParser left/middle/right tree root |
| Middle | Constructor arguments and middle delimiter parser path | Constructor delimiter/color values and parser path |
| RightItem | Constructor arguments and right delimiter parser path | Constructor delimiter/color values and parser path |
| BreakItem | Break insertion and array row separators | Native `mspace` insertion shape and array parser paths |
| BeginItem | Name mismatch and begin/end parse paths | Direct error and ordinary fallback |
| EndItem | Close predicate and end-name mismatch | Real factory item passed to Begin/Mstyle checks |
| StyleItem | Display-style parser path | Native TexParser style path and fallback |
| PositionItem | Raise/lower and MissingBoxFor source paths | Native TexParser raise/lower path and fallback |
| CellItem | Array cell/empty-cell paths | Array parser corpus and all-class predicate matrix |
| MmlItem | Final predicate on parser outputs | All-class predicate matrix and returned MML stack items |
| FnItem | Named-function parser path | Native TexParser `\\sin x` path and open short-circuit |
| NotItem | Negation-remap parser path | Native TexParser `\\not=` and open short-circuit |
| NonscriptItem | Nonscript-space parse path | Native TexParser `\\nonscript\\,y` plus fresh tuple return |
| DotsItem | Ellipsis/operator-dots parser path | Native TexParser `\\ldots`/`\\cdots` plus open short-circuit |
| ArrayItem | EndEntry, EndRow, StartEntry, checkLines, addRowSpacing, createMml, handleFrame, array parsing | Direct real-factory MML operations plus TeX parser matrix and accessor/error cases |
| EqnArrayItem | Constructor tags state and missing-columnalign error | Constructor initialization and exact EndEntry partial-state error |
| EquationItem | Constructor tags state, stop error and equation parse cases | Constructor initialization and exact live-name stop error |
| MstyleItem | Constructor attributes/name and matching-end wrapping | Constructor values and direct matching-end MML wrapper |

Source class metadata and constructor values have a locked CJS oracle. Full reflection of native class `errors`/own data descriptors and direct JS construct/ordinary-call identity for every constructor still depends on Root's shared Context class/data-slot adapters; the matrix does not claim those boundaries as accepted parity.
