# Gate255: canonical font observer and additional SVG sizes

The provider observer now uses the same canonical class/instance host protocol
as the accepted full-font observer: real RegExp and Map services, class identity
before Source static initialization, live default option fields, then
construction through the Context constructor. No font data, layout algorithm,
expected output or comparison tolerance was changed.

The strict provider comparison passed all 40 modules, 80 requests and 24,646
loaded glyphs. Font instance/extension comparison passed 3/3. The normalized
provider output SHA-256 is
`f505835ebe43d980f884ff4de85eba0cab1a810a5b718a11232564036f5bbb16`;
the provider executable SHA-256 is
`64f220d25ddc332bd39790ff9b1c0b37c4e3175228ed4238e00858b3e2686a26`.

An additional 12 fixed-profile formulas were captured at em 12, 16 and 32.
Each Source case used a fresh CJS realm and the pinned 38-package passive
event host; every em16 Source SVG was checked against the prior G213 fresh
capture. The native observer matched all 36 raw SVG byte strings and request
metadata exactly. Three zero-area texhtml outputs are explicitly recorded as
Source-expected blank outputs using BBox and foreignObject dimensions, not
silently removed from the corpus.

| Artifact | SHA-256 |
| --- | --- |
| Source capture | `ea3ba8c81492d14427db61ff653083053e7f5b05bb737799ceffb21dffd5cb65` |
| Native full-SVG executable | `aad931c1090a83266ccaaaf8bb87bf358ddb2f74d9c8a1c081896badbcc8c451` |
| Native input | `fa324049d9df248636e122fc033fb6f54d1965926eea27a78921715876ff0ab4` |
| Native output | `d8a758448dafc97c708132539c0cbb553108ad350d74d1b23412500fe8a27e48` |

Local artifacts: `build/math-port/font-provider-g255.json`, its provenance,
`build/math-port/g6-native-g255/native.json`, and
`build/math-port/g6-native-g255/comparison.json`. Source fixtures and their
producer remain under the ignored G252 raster-gate draft directory pending
promotion into the actual platform test.

This is SVG-byte evidence for this finite corpus. It does not establish G6
Direct2D raster acceptance, the whole current profile, whole-Core acceptance,
or the Folia runtime switch.
