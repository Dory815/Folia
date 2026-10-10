# P2 BBox port evidence

Status: implementation submitted for independent review; not accepted.

Upstream source: `ts/util/BBox.ts`, MathJax 4.1.3, SHA256
`a4232fac0fce35a15ad05a4dbb4af0999bf04f90ec3f0fe0c594b22c5da70b5b`.
The port is `src/math/util/bbox.ixx`, module
`folia.math.util.bbox`. `BIGDIMEN` is imported from the frozen
`folia.math.util.constants` module.

## Method and data ledger

| Upstream member | Native member | Ported behavior |
| --- | --- | --- |
| `fullWidth` | `BBox::fullWidth` | Constant string view `"100%"`. |
| `boxSides` | `BBox::boxSides` | Ordered records `(name,index,dimension)`: `Top,0,h`; `Right,1,w`; `Bottom,2,d`; `Left,3,w`. |
| `zero()` | `BBox::zero()` | Constructs all three dimensions as zero; scale metadata receives constructor defaults. |
| static `empty()` | `BBox::makeEmpty()` | Constructs width zero and height/depth `-BIGDIMEN`. Renamed because C++ cannot distinguish it from the instance `empty()` by staticness alone. |
| constructor | `BBox()` / `explicit BBox(BBoxData)` | Omitted width follows JS `w || 0`; omitted height/depth become `-BIGDIMEN`; present height/depth are retained, including signed zero and NaN. All ancillary fields receive upstream zero/one/empty-string initial values. |
| instance `empty()` | `BBox& BBox::empty()` | Sets only `w=0`, `h=d=-BIGDIMEN`, then returns this object for chaining. |
| `clean()` | `clean()` | Replaces only exact `-BIGDIMEN` sentinels in `w/h/d` with zero. |
| `rescale(scale)` | `rescale(double)` | Multiplies only `w/h/d`, in upstream order. |
| `combine(cbox,x,y)` | `combine(const BBox&,double,double)` | Uses child `rscale`, child width plus `L/R`, and the source conditional `>` updates. Locals are read before writes, preserving self-aliasing. |
| `append(cbox)` | `append(const BBox&)` | Adds scaled child width plus `L/R`; conditionally raises height/depth using source `>` comparisons. It does not copy child metadata. |
| `updateFrom(cbox)` | `updateFrom(const BBox&)` | Writes only `h,d,w`; copies `pwidth` only when the source string is nonempty. Other destination metadata is preserved. |
| `copy()` | `BBox copy() const` | Returns an independent value copy of all fields. |

`BBoxData` uses optional dimensions so an omitted key remains distinct from a
present numeric value. The constructor explicitly models JavaScript number
truthiness for width: NaN, `+0`, and `-0` become zero; nonzero finite values
and infinities are retained. Height and depth use presence, not truthiness.
Comparisons in `combine`, `append`, and `clean` remain explicit conditionals;
they are not replaced with `std::max`.

## Fixture coverage

The root-owned `math_port_bbox_reference` fixture in
`tests/math/reference_primitives.cpp` covers default/omitted fields, zero,
signed zero, NaN and infinity construction; sentinel creation and cleaning;
all update methods and metadata preservation; negative and positive rescaling;
copy independence; `append(box)` and `combine(box,...)` self-aliasing; and each
`boxSides` record plus `fullWidth`. The fixture and oracle were not modified.

No builds were run for this packet; root owns the shared build and acceptance.
No contract gap was encountered. The only API adaptation is the specified
static `empty()` to `makeEmpty()` rename. This evidence records the ported
surface and fixture coverage; it does not claim parity acceptance.
