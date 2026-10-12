# P51 actual FindTeX next declaration boundary

Root has read the complete pinned FindTeX.ts, core/FindMath.ts and protoItem
producer. A readonly Luna audit inventories every field and method. This records
the next complete class boundary; actual FindTeX declarations and bodies have
not yet been frozen or accepted. Do not replace it with a smaller custom scanner.

FindTeX extends the accepted core AbstractFindMath and has no parser-family cycle.
It belongs in input/tex/find_tex.ixx and corresponding implementation files,
separate from the mutually dependent parser-family partitions. The constructor
requires its supplied option value unchanged. The actual native factory allocates
the real derived instance, executes the existing base initializer, then invokes
the derived getPatterns virtually. Derived fields are initially undefined.

Complete original responsibility inventory:

| Source member | Native declaration requirement |
| --- | --- |
| static OPTIONS | Actual Context constructor metadata: nested inline/display delimiter arrays and three switches; no C++ duplicate defaults |
| start | One shared actual RegExp identity, initially undefined; reused mutable global cursor |
| end | One ordinary Context object, initially undefined, containing live [close, display, RegExp] tuples; no unordered_map mirror |
| hasPatterns, env, sub | Initially undefined, then exact boolean/numeric values in Source phase |
| constructor(options) | Existing real FindMath base phase, followed by virtual getPatterns |
| getPatterns() | Entire original option iteration, fresh local arrays/end object, sorting, capture numbering, pattern compilation and final field publication |
| addPattern(starts, delims, display) | Append every escaped start; assign the tuple to the ordinary opener-keyed object, preserving duplicate/prototype-key behavior |
| endPattern(end, endp?) | Global pattern using actual quotePattern; preserve the explicit null first argument at the environment call |
| findEnd(text, n, start, end) | Full original bracket/control-sequence loop; returns null or the actual core protoItem identity |
| findMathInString(math, n, text) | Original environment, substitution and delimiter branch order; mutate the same matched proto record and update the same start cursor |
| findMath(strings) | Fresh native ProtoMathItems identity, captured input length and live per-index reads; no input slot snapshot |

Actual prerequisite bindings to establish before delegating these bodies:

1. Option-array forEach must execute the real looked-up native method, capture
   its length once and perform Has/Get per slot. Existing arrayValues has live
   iterator length and is unsuitable. Declare and verify the missing native
   primitive in the existing Context owner; do not put array semantics into
   FindTeX or simulate the options with another record/vector.
2. RegExp instances in end tuples must bind the actual P50 RegExpRef using
   P49 NativeIdentityBindings. Freeze Context allocation, payload resolution and
   live lastIndex/exec caller signatures, plus their lifetime. No second compiled
   pattern, plain copied payload alias or counterfeit parser service is allowed.
3. Keep the accepted native ProtoMathItems result container as its sole output
   array identity. Internal end dictionaries/tuples use Context; do not synchronize
   a second Context array with the returned vector. Generic native N/T proto
   locations still belong to core MathItem. Scanner outputs use its real protoItem.

All substring/index/capture/delimiter operations use UTF-16 code units. The
existing Property boundary is already lossless WTF-8 through the shared
property_host::debugText/text methods, including isolated surrogates. A Luna
audit incorrectly inferred strict UTF-8 loss from std::string storage; Root
checked the actual adapter and rejects that false blocker. Do not add a new
string state, prohibit valid inputs or use external utf16::encode replacement.

Independent complete-class gold must include default and configured delimiters,
duplicate/opening-prefix order, braces, control sequences, environments, references,
escaped dollars/backslashes, disabled switches, unmatched input, ordinary object
prototype keys, surrogate halves and exact multi-string offsets. Root must freeze
the complete declaration/host graph and original gold before assigning the entire
class's method bodies to Luna. Agents may not reduce the enabled profile.

This is a declaration frontier, not a functioning finder or parser. Configuration,
ParseOptions, BaseItems, TexParser and all enabled package commands remain; fonts,
common mathematical layout, shared scene, SVG and Folia integration remain.
