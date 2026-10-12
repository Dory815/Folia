# Actual MML constructor identity checkpoint

The native MML node now retains the actual `FactoryClassRef<MmlNode>` that
allocated it. `classBinding<T>` supplies this descriptor before Source MML
initialization and virtual derived-field initialization, including the empty
TextNode branch. Replacing or deleting the current kind registration does not
change an existing node's allocation identity. A different descriptor cannot
rebind that internal identity. This is not a new Source-visible own field.

The descriptor's construct callback holds a weak reference to its descriptor;
the actual factory creator and allocated node retain their real class strongly.
This avoids a descriptor/callback ownership cycle. A saved actual creator keeps
the original class after registry replacement. Source MML attribute defaults
still consult the live factory registry: calling that saved creator after
deleting the kind raises the original defaults-read TypeError. The internal
allocation identity does not replace that live registry lookup.

`tools/math-port/mml-constructor-identity-oracle.cjs` executes the pinned
MathJax CJS classes over a complete real MmlFactory. It verifies the cloned TS
hashes and normalized equality with the installed 4.1.3 TS before running.
`mml-constructor-identity-source.json` freezes all consumed TS/CJS hashes and
four original observations: registry replacement/deletion, initialization,
saved creator/live-default lookup and empty text. `--check` reproduces them.

The actual native fixture is `tests/math/mml_constructor_identity.cpp`.
The integrated host gate (`build/math-frontend-gate-6.log`) passes all six
identity tests with 16 assertions and builds the actual production library.
Additional assertions verify the real descriptor lifetime and reject rebinding
the internal allocation identity. Those two native ownership checks are not
claims about arbitrary JavaScript `.constructor` writes.

Generic Context exposure of MML nodes, actual class/prototype reflection and
graph lifetime bindings remain separate unfinished dependencies. Parser
candidate tests still fail; common layout, SVG parity and Folia integration
are not accepted by this checkpoint.
