# Root P18c — complete MML registry and MmlFactory source acceptance

Authority: MathJax 4.1.3 MML.ts SHA256
c64bcfdf65452559995a92c13db75133b1500ef59efc2be5c9489ceacc4d4964,
MmlFactory.ts SHA256
5e7338cd3eed5a4002243b7371dc5098c6433ff4ba4345cb2d50cf0385b2abdb.
Root read both complete source files and dependencies, froze interfaces,
ownership and acceptance before Luna implementation, then independently
reviewed both implementation units. All three source members and all 43
constructor bindings are mapped in the P18 inventory and implementation report.

The registry is one const shared binding to a mutable ordered map of all 43
actual compiled constructors. Defaults/class services and RTTI predicates use
the already accepted source class hierarchy; no string-labelled generic class,
omitted kind or test-only creator is substituted. Constructor identities are
shared across factories; creator objects/functions belong to individual factories.
MmlFactory defaultNodes is independently rebindable and initially aliases MML.
Explicit maps, including empty, bypass defaults. Nullopt selects an injected
typed subclass default or the current default handle; a selected null handle
throws rather than quietly using generic defaults. The constructor delegates
all existing map normalization/seeding/dispatch/ownership to the accepted generic
factory. The MML getter returns its same live creator object, distinct from
nodeMap; no second registry or factory algorithm is introduced.

Actual pinned CJS observations cover ordered names, 172 complete construction
states (four forms for every entry), and the full 43×43 instanceof matrix:
1,849 type observations including inherited script/layout/token/XML classes.
Construction snapshots include exact defaults, class/previous fields, token/
empty/inferred/notParent/arity properties, ordered raw explicit/inherited values,
whole children and four semantic projections with error categories. Inputs
include raw nested values, signed zero, NaN, text and MML children. All Text,
XML and Html bindings are included. Native suite: 175 tests, 176 assertions,
387 source values. --check reproduces its unchanged source observations.

A full live protocol compares initial aliasing, per-factory creator identity,
saved creators after class replacement/deletion, direct creator-view mutations
independent of nodeMap, current constructor lookup failures, live global map
removal/replacement/addition, existing versus future factories, default handle
rebinding, explicit/injected defaults and null defaults. A saved creator retains
its class and factory even when subsequent MML initialization fails due to a
deleted registration. All global writes are restored. Test views are held in
stable native handles before taking iterators; this also lets deliberately copied
getter results fail assertions without dangling-iterator crashes.

All 14 isolated mutants compile and fail unchanged assertions with exit 1:
omitted action, incorrect inferred/subscript/annotation/HTML classes, incorrect
TeXAtom key, registry order, replaced global handle, broken default alias,
copied creator view, ignored explicit/injected maps, silent null fallback and
empty-map substitution. Working source/fixture bytes are checked afterward;
results are p18c-mml-registry-mutations.json. Root full core build/test passed
(build/p18-final-core.log, exit 0), including all existing tests and the action
packet. Source inherited constructor lookup uses the established typed injected
defaults seam; arbitrary JS constructors/proxies are outside that declared host.
Together with P18b, accepted source coverage is 50/246. Complete TeX parser,
Common layout/fonts, SVG output and Folia integration remain unaccepted.
