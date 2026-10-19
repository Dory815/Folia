# Common-output source dependency closure

Read-only inventory for the next common-layout packet. Reference is pinned
MathJax 4.1.3, commit `fb987178c2d279a99b0db5ea02e751691703955e`. This report
records direct source imports and hashes; it does not freeze native interfaces
or claim implementation parity. `e3-common-scene-boundary.md` remains the
method-level evidence for its selected common/SVG wrapper families.

## Common-output entry and services

| Source | SHA-256 | Direct dependency closure |
| --- | --- | --- |
| `ts/output/common.ts` | `3686a8e803e26ae59d03c096ac2e6e5b1ab1332a5c4bcaa083cb9a02c2c61096` | `core/OutputJax`, `MathDocument`, `MathItem`, MML node/mspace, `DOMAdaptor`; `Options`, `lengths`, `Styles`, `StyleJson`, `BBox`; CommonWrapper/Factory, LinebreakVisitor. This is the output-jax/lifecycle boundary, not just geometry. |
| `ts/output/common/Wrapper.ts` | `d1db6112db1cb3546658eb8868b5997a4f69e588632ecbe4e08a71949544a828` | `core/DOMAdaptor`, `MathItem.Metrics`, generic Tree Wrapper/Node; MML `mo`; `string`, `lengths`, `Styles`, `StyleJson`, `Options`, `BBox`, LineBBox, LinebreakVisitor, CommonOutputJax, wrapper factory and `mo`/`mrow`. |
| `ts/output/common/WrapperFactory.ts` | `a798803bab094ae1e72cd7f96eb37aca936ac458da145c1ebb22079a0280b6cd` | CommonOutputJax, generic Tree WrapperFactory, CommonWrapper, MML node/class; per-kind registrations are constructor dependencies. |
| `ts/output/common/LineBBox.ts` | `42646988f6914b044044c4fe6edd700e6b68e1b6b16524ae0282d5715723326a` | MML node, BBox/BBoxData, CommonWrapper string map. |
| `ts/output/common/LinebreakVisitor.ts` | `c3089b8aa548958b2f86654d9f4de7d52fa037f51e5bbb894c47021cd75c9436` | Tree Visitor; CommonOutputJax, wrapper factory/base; `mo`, `mspace`, `mfrac`, `msubsup`, `mmultiscripts`, `mfenced`, `maction`, `msqrt`, `mtext`; BBox/LineBBox; `TEXCLASS`, `OPTABLE`, MML node/TextNode. |
| `ts/output/common/Notation.ts` | `45108b1dcd73541f042e79e53bdfb7cdd6f7e56a723026eab8143c81341990a3` | CommonWrapper and `CommonMenclose`. |
| `ts/output/common/Direction.ts` | `cfa6ccfacb73e6991d373a3677a097356de2092405c742f29811337fd9e511c2` | Direction constants consumed by common delimiter and table layout. |
| `ts/output/common/FontData.ts` | `7d8eeca536fa1b7c14517590b9c5a372ad10b34667d196d09b030f88cda780b2` | MathJax loader, Options composition, StyleJson, AsyncLoad, Retries, Direction. Font methods are common layout inputs; SVG paths are not imported here. |

## All 27 common wrapper source files

Every file below also imports `CommonWrapperFactory` and `CommonOutputJax`;
specialized families additionally depend on the other wrapper/MML/util files
listed in the last column. Directory capitalization in the pinned checkout is
`Wrappers/`.

| Source | SHA-256 | Additional direct imports |
| --- | --- | --- |
| `Wrappers/maction.ts` | `7a83a9dea6f03ed6ceca9d4fd4537bf4f571023e65ca9ff3b1d614cf558a28c9` | MML node/maction, BBox, string splitting. |
| `Wrappers/math.ts` | `990a3f3b733a1d23b8001fdb1298afc3530445204b066e943983fc8085bb99d0` | BBox. |
| `Wrappers/menclose.ts` | `97a9f2fb5a9b0976b4719c7730812913f63ecf431c0a148b9fd8c71e9b38b809` | `msqrt`, Notation, BBox, MML node, string splitting. |
| `Wrappers/mfenced.ts` | `0da151ef587e065dbdfe1cd750999a41bcfee67c0d765ea0622baa206b40a40d` | Inferred mrow, MML node/mfenced, BBox. |
| `Wrappers/mfrac.ts` | `bd11ae20e3f698656add3a38a4aba4385a8595ab4f8f64f5629eab40156ace9d` | `mo`, MML node, BBox, Direction. |
| `Wrappers/mglyph.ts` | `3a4d41fe473fbb370b7a906b8631584bb8f2cee210c9a4e89041edbe7c928a1a` | TextNode wrapper, MML node/TextNode, BBox. |
| `Wrappers/mi.ts` | `5691d4c14fd92e77f63248d67f1363e7a53063544ba44cfc66f4f280df48c7ff` | BBox. |
| `Wrappers/mmultiscripts.ts` | `1dbfa29bf392f4ee668a516797bc93a01a8d72b1297c93e7c8587180415414b6` | CommonWrapper/base, `msubsup`, BBox, LineBBox. |
| `Wrappers/mn.ts` | `c08ea85f40877ad39dead45c2e18004c3e8a11072e341c08cf9bbd4d23b01503` | No additional direct import beyond the common wrapper factory/jax. |
| `Wrappers/mo.ts` | `67c7ed182bc8a38139da98e4d42d0350d2386de29a7e5f95095b4161dcab1bcf` | MML node/TEXCLASS/mo, BBox, LineBBox, Unicode utilities, Direction/NOSTRETCH. |
| `Wrappers/mpadded.ts` | `b34393bce01ad32b95b9b4f32ff2bf90616e9caacb93c25a033e8253798a435e` | BBox, Tree Property. |
| `Wrappers/mroot.ts` | `7fe799638a6282a1b75b13ed26c371c580dec7d576f9a86cf9158f6925a92b00` | CommonWrapper/base, `msqrt`, BBox. |
| `Wrappers/mrow.ts` | `7e363c64d653d9cebd18c9e70784018c6f44b29b09bc26847b921b47e696e5d9` | MML node, BBox, LineBBox, Direction. |
| `Wrappers/ms.ts` | `e0660114418cab65ae3ee590531d3c117aeb141e56be643c2fb2ada18d1f14b9` | MML node. |
| `Wrappers/mspace.ts` | `f2ee9c1d05d17ae90a9606646861639d066640b6c42072f09bec0862ee418159` | MML node/mspace, BBox, LineBBox. |
| `Wrappers/msqrt.ts` | `1af07e3067fc041346bac1a6020a49cd9841a02e26227f1f5bda90ca2b0f56f8` | `mo`, MML node, BBox, Direction. |
| `Wrappers/msubsup.ts` | `b9ef3b9eb31e7a011e0f0d8d45048fbe15cb0614f8acbd970ceb96afdeb0a0b5` | CommonWrapper/base, BBox. |
| `Wrappers/mtable.ts` | `a1e060baab84d7b842c811c560ab73f32ca91614e3a5b02f7f1712db73b0100c` | `mtr`, MML node, BBox, Direction, split/isPercent, numeric sum/max, Styles/TRBL. |
| `Wrappers/mtd.ts` | `06a87d48c77c0d167881e29c9115ce2dd50f89beac9f27dbe9ed56a99e884d97` | `mtable`, `mtr`. |
| `Wrappers/mtext.ts` | `855b7d60c3736cbb7d3368ae3399c71a704754cecdbc78dd490508a2dd3cc4e9` | MML TextNode, LinebreakVisitor index types, LineBBox. |
| `Wrappers/mtr.ts` | `7f9657da0fbf35b48523b345f3e9886fa384a0871bb3028ad06a5887a577aabb` | BBox, Direction. |
| `Wrappers/munderover.ts` | `bd77fa0e9ff51e1e2771810198e987d4b3047f68aefde5ceb64de0266fb078eb` | CommonWrapper/base, BBox. |
| `Wrappers/scriptbase.ts` | `f63e12bd993ff76bd1717d73e3a9c9f57831bdcc8cbc80f2e01b7d4de0cbb02e` | `munderover`, `mo`, MML node/msubsup/mo, BBox, LineBBox, Direction. |
| `Wrappers/semantics.ts` | `527f49600e0caede0c95ed954d246a344d4027238b471a779cc55b66ba015927` | BBox. |
| `Wrappers/TeXAtom.ts` | `4c607387e28e172e83b0cf22718e6966b99e2ce8528d288deef31bcb3ba36f8f` | BBox. |
| `Wrappers/TextNode.ts` | `cb6189cf34ea5f906e33c0ef5fa5389b05c0ffffade6a5953e4037b9674d885a` | BBox, MML TextNode/mo. |
| `Wrappers/XmlNode.ts` | `aa03369e8f9e4b36154c9e3d4b7ad9bed94fa0713c8ead00467a289e42654dea` | ExtendedMetrics/UnknownBBox, MML XMLNode, BBox, Styles/StyleJson, string splitting. |

## Closure implications for interface planning

* Common wrapper measurement is coupled to completed MML attributes and
  `TEXCLASS`/operator dictionary state. `mo`, `mfrac`, `msqrt`, scripts and
  tables also call other common wrappers, so per-file compilation alone does
  not define a runnable layout slice.
* `Wrapper.ts` and `common.ts` consume `DOMAdaptor`, computed styles, metrics,
  and MathDocument/MathItem lifecycle. The raw-TeX API may adapt this boundary
  to explicit render options and no-document defaults only after those exact
  source call paths are classified; substituting empty styles or guessed
  metrics would alter wrapper scale/BBox behavior.
* `LinebreakVisitor` is a graph dependency, not optional utility glue: it
  dispatches node-specific wrappers and operator classes and consumes
  `LineBBox`. Display line breaking in the Folia entry profile therefore
  closes over all listed wrappers plus the exact numeric/string/length/BBox
  helpers.
* Common code consumes the shared resolved font metrics, CSS/remap/parameters
  and delimiter APIs. SVG font path options are needed by SVG scene construction
  but must not become a common-wrapper dependency.
* `Wrappers/XmlNode.ts`, `mglyph.ts` and `TextNode.ts` retain XML/TextNode host
  dependencies even though ordinary TeX is the entry path. Their enabled and
  reachable profile classification remains Root-owned.

This file is dependency inventory only. It does not declare unsupported
wrappers excluded, supply a backend-neutral scene contract, or accept fonts or
common layout.
