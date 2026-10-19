# Owned TeX session keys and environment values

Macro keys now use UTF-16 strings throughout TexSessionState. The ASCII-only
API spelling and narrow-key map are removed. The pinned Source witness covers
four scanner cases and nine definition cases: newcommand accepts a single BMP
control symbol but rejects a two-code-unit astral name, while def can store that
astral control sequence. The session dictionary owns names without imposing
the individual parser command's grammar; each handler remains responsible for
its Source syntax rules. No parallel key map or compatibility overload exists.

TexEnvironment owns its twenty explicitly typed optional fields. Its overlay
copies present false/zero/empty values in the Source operand order; seed
resolution owns returned data and defaults absent isInner to true. Actual
pinned Stack/Parser observations cover nested open environments, array reset,
pop restoration and the isInner cases. These support the data contract, not
a claim that the complete native parser/stack has been implemented.

Native compilation and all focused runs passed: session **6 tests / 42
assertions**, environment **6 / 24**. Logs are ignored under
build/math-port/native-raii-component-5.log through component-16.log.
Complete renderer acceptance is still blocked by the independently diagnosed
font code-generation stack frame, with its bounded initializer rewrite pending.
