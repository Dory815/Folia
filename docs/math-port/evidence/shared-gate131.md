# Gates130–131: factory calls and raw font family

Gate130 captures 1,963 unchanged inputs and ends with two compile failures.
The Lite fontFamilyProperty declaration was placed on LiteBase instead of
LiteAdaptor. A registered Bussproofs session witness also prematurely used
the still-unregistered Stack API. The declaration is corrected, and the
witness is restored exactly to its Gate129 hash, without removing its prior
assertions. The new Stack checks remain in the separate candidate packet.

Gate131 builds all 51 targets with unchanged captured inputs. Complete
rendering matches 73/75 records with 37/38 registered packages and one
Bussproofs warning. Public SVG/error API remains 9/9 and texttip's full SVG
stays exact. Both TexHtml examples and valid Empheq now match. Remaining
renderer differences are tooltip's missing Context processMath method and
the unregistered Bussproofs configuration. The Source tooltip ends with
`node.addEventListener is not a function`; the native path must reach the
same operations and error, not manufacture event support.

The actual production changes preserve raw metrics.family through common
layout and apply Source truthiness at XML font selection; previously Missing
became the truthy string `undefined`. Lite font-family Context calls likewise
retain false, zero, null and undefined. The captured class and instance options
both contain Times, so there is no evidence of a missing default-options merge.
The relation post-filter now reads raw parent instead of logical Parent(),
so adjacent relation tokens under inferred rows combine. EmpheqMO now passes
its empty definition as Property through the existing Boldsymbol token creator.

Common factory Context create/wrap use canonical MML/wrapper owners and the
existing product adoption path. wrap reads create, then apply, then node.kind
in Source order. Shared factory creator selection is captured before argument
conversion. The standard apply intrinsic handles array-like indexed reads.
MML kind and Tags method metadata are projected from actual native owners.
This does not complete all factory fields or arbitrary object replacement;
their owner packets remain separate required work.

All processes are terminal and comparison input audits pass. This evaluation
is not accepted: complete Core reports four failing new tests, including two
parser fixture exceptions and two metadata/shape assertion groups. Host,
Parser, Regexp and LinkedList pass. Foundation fails its new XML font-family
fallback test with a native option exception. The standalone XML capture exits
with `Expected a Lite DOM node`: its fallback passes a null parent to a Source
method that ignores that argument, while the native binder validates it early.
These checks remain in place and require fixes. The earlier standalone XML
namespace differences also remain pending in the creator-path packet.

Fonts remain 13/16 and next-six 18/20. Their first differences have advanced
past Tags arity to Source standalone Jax.adaptor null versus a native adaptor
object, plus the existing parser/Stack factory alias. Other previously passing
package, identity, map, host/layout, Attributes, Mhchem and texttip comparisons
remain passing. No Source expected record or comparison tolerance is changed.

Ignored evidence under build/math-port includes gate-131-inputs-before.json,
gate-131-input-changes.json, diagnostic-exits-131-root.json,
common-layout-native/comparison-g131.json, package-captures-g131.json and
checkpoint-comparisons-g131.json. Full native observation SHA-256 is
`d00e0165499c022ef199764e39c92f603d23c286c42e80fa5b1a54fa32cf54f0`;
the full Source hash remains
`9f58d09411512b4682b1f0049e40d6a47844ddffb9794eda52e2700998e0d740`.

Folia still uses its current math QuickJS worker. Mermaid remains paused.
