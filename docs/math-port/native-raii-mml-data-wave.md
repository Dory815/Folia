# MML data wave

The structural foundation is not complete MML semantics. Before family
inheritance, operator classes and CommonWrapper ports, implement explicit
mathematical attribute defaults and define the required typed node properties.

## Attributes and defaults packet

Canonical scope: `core/mml_tree/attributes.ixx/.cpp`,
`node_defaults.ixx/.cpp`, and the default initialization call in `mml_factory.cpp`.
The node-default module exports `nodeDefaults(NodeKind)` and `globalDefaults()`
as immutable views over ordered mathematical attribute entries. Data comes
from all 43 pinned Source classes; offline extraction is allowed, native JS
execution is not. Unknown default value types are an explicit incomplete gate.

`AttributeValue` stays the closed null/bool/double/UTF-16-string variant.
Source `INHERIT` is the mathematical string sentinel `_inherit_`, not a generic
object marker. `AttributeSet` owns four ordered data layers: explicit values,
inherited values, node defaults and global defaults. Its existing per-layer
accessors remain raw. Add `setGlobalDefault` and `globalDefault` as the fourth
raw layer. `lookup` searches the layers in that order and resolves INHERIT to
the global default. Provide named effective inherited/default lookups where
Source needs those chains, and `isSet`/`hasDefault` with their mathematical
meaning. Do not add property descriptors, prototype objects or callable hooks.

MmlFactory installs node/global defaults as domain data before applying caller
explicit/inherited attributes. No existing caller-supplied layers may be lost.
The first implementation may copy finite tables into request values; immutable
sharing may be introduced only with a concrete safe owner and measured benefit.
Test own/inherited/default/global precedence, null versus absent, INHERIT,
ordered updates/removal and actual defaults for representative families against
the pinned Source. Whole Source default-table equality is required, not just a
few selected values or the number of attributes.

## Typed node-property inventory

Read the Source MML, all 38 TeX packages and common/SVG wrappers for every
mathematically used `getProperty`, `setProperty`, `setProperties` and direct
node state field. Record literal and dynamic keys with producers, consumers,
value types, request/session lifetime and family responsibility. Exclude JS
reflection/identity mutation from the native API, but retain mathematical uses.

Root will choose concrete field records and family-owned types from this
inventory before implementation. Do not build a string-keyed universal property
bag, std::any payload, function graph, side registry or wrapper compatibility
layer. Node properties live on request-owned MmlNode values, and algorithms
stay in their Source family modules. This audit does not authorize implementing
a second MML representation or moving family behavior into the Renderer facade.
