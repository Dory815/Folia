# Typed TeX lexical and expansion wave

This wave ports parser building blocks, not a second TeX parser. The eventual
single TexParser owns a TexScanner and calls ParseUtil; package handlers and
the stack consume the same request-owned MML document. No lexical component
constructs Scene or SVG, and no macro utility stores a renderer/request pointer.

## Locked files and APIs

| Owner | Canonical files | Contract |
|---|---|---|
| Error Luna | `input/tex/tex_error.ixx/.cpp` | `folia.math.input.tex.tex_error`, `tex::TexError`, owned error ID and UTF-16 formatted message, `std::exception::what()` text; typed string arguments only |
| Scanner Luna | `input/tex/tex_scanner.ixx/.cpp` | `folia.math.input.tex.tex_scanner`, `tex::TexScanner`, owns UTF-16 input/cursor/current control sequence; Source lexical operations only |
| Expansion Luna | `input/tex/parse_util.ixx/.cpp` | `folia.math.input.tex.parse_util`, `tex::parse_util`, Source `addArgs`/`substituteArgs` and explicit expansion limits; no MML/string formula shortcut |

`TexError(std::string id, std::u16string format,
std::span<std::u16string const> arguments = {})` owns the result. Accessors are
`id() -> std::string_view`, `message() -> std::u16string_view`, and `what()`.
Numeric package error parameters are formatted explicitly at their domain
boundary. Implement Source percent/substitution behavior, including malformed
formats, from pinned `TexError.ts`; no universal JS conversion API is introduced.

`TexScanner` supplies `source()`, `position()`, `setPosition(size_t)`,
`replaceSource(u16string)`, `currentControlSequence()` and its setter. The parser
owns when cursor/source are reset during expansion. Lexical methods are
`getCodePoint`, `nextIsSpace`, `getNext`, `getCS`, `getArgument`, `getBrackets`,
`getUpTo`, `getStar`. Missing optional arguments use `optional<u16string>`;
required missing arguments throw typed TexError. Names, delimiter strings and
source slices retain UTF-16. Registry-based delimiter recognition, dimension
matching, nested MML parsing and package dispatch remain the eventual parser's
responsibility. Do not implement them with temporary fake registries.

`parse_util::addArgs(u16string_view left, u16string_view right, size_t maxBuffer)`
and `substituteArgs(span<u16string const>, u16string_view replacement,
size_t maxBuffer)` return owned UTF-16 text. Preserve escaped characters,
parameter errors, `##`, control-sequence boundary spacing and buffer failures.
Expansion counters use an explicit request-owned `ExpansionBudget` with
`maxMacros`, `maxBuffer` and `macroCount`; checkMaxMacros preserves Source's
increment/error order and macro-versus-environment error IDs. Persistent macro
definition data remains in session; argument scanning remains in TexScanner.

## Acceptance and dependency rules

Use actual pinned Source method observations with the expected return, UTF-16
cursor and error ID/message. Pair successful/optional/error cases, escaped
braces/backslashes, nested brackets, whitespace and astral characters with
native UT assertions. For expansion, compare concatenation boundaries, escaped
hashes, missing/extra parameters, length limits and counter side effects.

These are native components, not evidence of complete TeX/package support.
Unsupported future parser operations cannot be no-op success. Import only std,
the native error module and already-reviewed native utilities. No Context,
generic Value/Property, function coercion, prototype, erased payload or object
identity graph is allowed. C++23 modules and the repository console/test rules
remain mandatory. Root owns canonical promotion, source registration, native
execution and commits; Luna packets are draft-only until reviewed.
