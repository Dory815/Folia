# Typed TeX environment packet

Luna owns `input/tex/environment.ixx/.cpp`, module
`folia.math.input.tex.environment`, namespace `folia::math::tex`. Depend on std
only. This is one native data contract used by the eventual parser and stack;
it is not an alternative parser or a generic environment object.

`TexEnvironment` owns optional<bool> display, in_root, operator_letters,
is_global, bold_symbol, vector_star; optional<double> script_level; optional
owned UTF-16 strings font, italic_font, style, size, color, horizontal_size,
closing_environment, cd_min_width, cd_min_height, vector_font, math_variant,
math_size, math_color. These correspond to the exact 38-package environment
fields enumerated in `tex-parser-architecture/closed-inventory.md`. Do not
merge lexical TeX dimensions with normalized layout lengths or guess finite
variant/style spellings. Empty strings and explicit false/zero stay present.

`braketItem` becomes a Braket stack-frame depth member later, not a field in
this shared environment. `isInner` belongs to a parser seed/global record, not
the environment. Add `ParserEnvironmentSeed { TexEnvironment environment;
optional<bool> is_inner; }` and an owned resolved seed result with bool inner
defaulting true when absent. Splitting a seed consumes a native value and
must not retain pointers to caller state.

Expose named `overlayEnvironment(destination, source)` that copies every
present source field, leaving absent source fields untouched. Stack.Push uses
this operation with the exact Source operand order: a new open-frame env is
the destination and the current active env is the source. Source active values
therefore override conflicting initial frame values; other initial frame
fields survive. Copy/reset of a whole environment uses ordinary value copy or
assignment to an empty record. Non-open frames will borrow the active env;
the later Stack packet owns scope switching and borrowing, not this module.

Do not add a string-key API, generic value variant, shared mutable map, dynamic
property, object identity, callback, parser pointer or session reference. Do not
add StackGlobal fields yet: its per-field Source value domains are a separate
lock. This packet does not change session macros or the persistent package
state boundary.

Acceptance: obtain actual Source Stack.Push/Pop environment observations for
open copy versus array reset, the above overlay direction, empty/absent/false
and zero, restoration of the previous environment and independent nested value
copies. Capture TexParser's isInner default/present-false handling. Generate
native tests checking every field, copy independence and owned seed lifetime;
separate data-helper acceptance from pending complete native Stack behavior.
Write only an ignored complete packet; Root promotes, builds and tests.
