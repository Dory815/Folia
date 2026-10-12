# P34 row and identifier implementation

Implemented the seven authorized method bodies from the pinned MathJax 4.1.3
sources, using the existing tagged `ChildArray`/`NodeValue` model and virtual
MML APIs.

Changed methods:

- `MmlMrow::nonSpaceLength`
- `MmlMrow::firstNonSpace`
- `MmlMrow::lastNonSpace`
- `MmlMrow::setTeXclass`
- `MmlInferredMrow::toString`
- `MmlMi::setTeXclass`
- `MmlMstyle::setChildInheritedAttributes` (added by Root's P34 extension)

The row scans preserve captured-array/live-index behavior, including the
backward scan's per-index re-read of `childNodes`. TeX class assignment keeps
the delimited-row branch, raw virtual class access, and child return threading.
The inferred row joins the captured array with hole/nullish empty fields and
the shared array identity recursion guard. Identifier class assignment uses
the registered UTF-16 operator-name callback, exact `Missing` checks for
undefined properties, and virtual class/property APIs. The style child
attribute method preserves explicit nullish checks, raw property values,
relative script-level addition, fresh virtual attribute reads, and the single
current `childNodes[0]` dispatch.

No build or tests were run, per the task boundary. Acceptance and parity remain
unverified here; Root owns the reference-leaf verification. No missing API
boundary required a substitute in these seven methods.
