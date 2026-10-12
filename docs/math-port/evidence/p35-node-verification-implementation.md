# P35 node verification implementation

Implemented only the four assigned `verifyTree` bodies: `MmlMath`, `HtmlNode`, `MmlMprescripts`, and `MmlNone`.

`MmlMath` applies its own `options.missing()`-to-null default before its qualified base call and uses that local options value for the derived error callback. The HTML override retains its raw options value, parent short-circuit, second parent read, token check, error callback, and return. The two multiscript markers retain raw options through their qualified base call and use the original parent kind check and error message. Each body uses the shared `TreeCallGuard`; parent dispatch uses the current tree parent and tagged `requireNode` access.

No tests or builds were run, per the assigned packet. Root owns independent acceptance against the frozen Source records, options witnesses, and injected faults.

Uncertainty: this report records implementation scope only. It does not claim Source-record parity, mutation coverage, or broader math-stack acceptance; those checks remain with Root.
