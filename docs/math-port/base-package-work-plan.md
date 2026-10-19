# Base parser package responsibilities

Root owns runtime hooks, shared graph interfaces, CMake, builds, native runs and
acceptance. The compiled parser/host checkpoint is not full TeX/MML acceptance.

* `/root/mml_row_mi`: all 26 real BaseItems classes and class descriptors in
  `base_items.ixx`, `base_items.cpp`, `base_item_bindings.cpp`, corresponding
  original observations, native fixtures and Source-method coverage.
* `/root/math_layout`: the complete base package, including BaseConfiguration,
  BaseMappings, BaseMethods and character/symbol responsibilities. Preserve
  upstream file ownership in distinct files; do not put a whole package into
  the registration module. It also owns the pending Tags/ColumnParser packet.
* `/root/mml_verify_nodes`: complete TeX input jax in `tex_jax.ixx/.cpp`, its
  original/native coverage, and the pending TexParser/ParseUtil packet.

Frozen BaseItems integration contract:

```cpp
struct BaseItems {
    static void initializeModule(ParserRuntime&);
};
// ParserRuntime::baseItemsModule is options::Value, initially Undefined.
```

The hook stores one ordinary module export record in `baseItemsModule`, keyed by
the upstream exported class names (`StartItem`, `StopItem`, etc.), and binds
each real constructor to its actual descriptor via `itemClassIdentities`.
BaseConfiguration reads each exported constructor's actual `prototype.kind`
and builds its own Source `items` configuration. The exports and the package
configuration are distinct Source objects, not interchangeable registries.

The actual factory adopts each product, obtains its owning `StackItemRef`, binds
`stackItemIdentities` to the genuine constructor, performs BaseItem's Source
phase and then derived phases. ArrayItem sets its borrowed runtime before its
Source fields. `StackItemFactory::DefaultStackItems` stays the separate upstream
DummyItem table. Do not replace generic factory defaults with the base package.

`StackItemClassInfo::errorsBinding` and `BaseItem::getErrors` retain their one
canonical Property graph. Do not create a second Context error-table snapshot.
Context reflection of that native graph is a required adapter dependency, not
evidence of full reflection parity.

Package parsing produces the existing actual MML graph; SVG or Direct2D layout
does not belong in these modules. Missing peers must be implemented, never
replaced by fake parsers, factories, reduced MathML or example-specific rules.
Required validation includes pinned real TeX to MML, malformed input and partial
effects, actual factory/class/array identities and relevant semantic faults.
