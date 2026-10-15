# Gate168: working Renderer, paired Scene route and remaining differences

All 57 shared targets compile with 2,075 captured inputs unchanged. The
TextMacros CONFIG decoder restores the actual 38-package public Renderer.
The API comparison passes 9/9, all 25 package captures exit 0, and Parser,
Regexp and LinkedList suites pass. Core, Host and Foundation still fail.
The Common factory State resolver cycle regression test passes.

Fresh public Renderer capture completes 150 records in continuous and
fresh-owner groups. The clean pinned Source comparator matches 148/150.
Both differences are case 52, texhtml-explicitly-enabled: native import
throws `unsupported Lite owner field`. The importer recursively reads
children before recognizing text/comment leaves. Common layout is 74/75
for the same reason. A leaf-aware correction is prepared for Gate169;
these results do not verify that correction.

The independent paired NativeRenderSession probe also completes 150 records:
144 successful pairs have identical complete frozen Scenes, and six pairs
have equal route errors. Every Scene-only record has zero calls to
Serializer::materialize and LiteAdaptor::serializeXML. Executable hashes and
all 2,075 captured inputs are unchanged before/after this run. This proves
route equality and absence of the SVG consumer in this probe. It does not
independently establish correct geometry against MathJax or Direct2D output.

Sixteen of twenty checkpoint comparisons pass. Remaining groups are layout,
XML, next-six and font packages. XML's custom fixture now reaches an unbound
XML creator rather than the earlier namespace difference; next-six and font
observations now expose callback length and stack own-key differences.
Source records and comparers remain unchanged.

Two small diagnostics reuse stable compiled modules and the shared library;
they create no build-tree/library copies. The real Lite nested measurement
restores both texts correctly, but the original UT wrongly equates the outer
Source node with the detached provider probe. Its corrected owner-specific
checks pass in an isolated diagnostic. A filter-record diagnostic identifies
missing NativeDataField deletion callbacks when Source replaces descriptors.
Factory direct-call versus apply semantics and Promise test expectations
remain separate reviewed work packets.

Evidence in build/math-port includes the Gate168 input audits, diagnostic
exit records, 25 package provenance records, 20 comparison records,
profile75-full-svg-comparison-g168-root.json and
profile75-scene-route-audit-g168.json. Evaluation is terminal with exit 1.
Folia still uses math QuickJS; Mermaid remains paused.
