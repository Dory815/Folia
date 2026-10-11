# P20d root speculative-escape repair experiment

This is a bounded ignored-copy repair, not regexp engine adoption. P20b's
independent real-RegExp corpus exposed `[\\uD800\\u0000-\\u0004]/iu` and
the DBFF counterpart being rejected by the candidate although Node accepts
them. Root inspected try_escape_unicode_sequence: its failed low-surrogate
trial snapshots the cursor AFTER consuming the second backslash/u prefix.
Restoring only to that cursor loses the prefix and reparses the digits as
ordinary class atoms. The general transaction must restore before the trial.

Root freezes real Node witnesses via regexp-lookahead-oracle.cjs before editing:
high-boundary surrogates, valid/invalid/non-low/braced following escapes, plain
atoms/classes/repetitions and i/u/v controls. No character/pattern special case.
Preserve paired-surrogate success and prior unicode/legacy mode separation.

Root changes only a new ignored copy's parse.rs: snapshot input immediately
before try_consume_str("\\u"), retain that snapshot for failed trial rollback,
and omit the after-prefix snapshot. No other parse branch, flags, engine data,
production dependency or existing expected result changes. Preserve original
P20b snapshot and report. Independently replay all three frozen corpora,
measure fixes/regressions, run upstream tests and keep any unresolved differences
explicit. No source-count increase or Folia integration follows this experiment.
