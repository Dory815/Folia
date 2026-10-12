# P56 Configuration host and module initialization frontier

Root's next Source consumer is the complete actual Configuration class and
ConfigurationHandler in input/tex/Configuration.ts. ParserConfiguration keeps
its own responsibilities and will follow its actual TeX/ParseOptions peers.
No fake parser/jax or replacement configuration registry may satisfy this gate.

Independent Luna audits confirm these requirements:

- makeProcessor returns an existing array unchanged, otherwise a fresh pair.
- _create Gets priority once with nullish default, init/config twice only when
  truthy, then maps pre/post lists, Gets parser, and evaluates the six record
  arguments in order. Constructor Object.assign follows argument evaluation.
- Array map captures length, preserves holes, performs live Has/Get/callback,
  and defines own result data properties without prototype setter dispatch.
  Callback validation precedes result allocation and constructor lookup.
- Object.assign snapshots all own keys including symbols, tests the current
  descriptor before each Get, then Sets the fresh target. Handler defaults are
  four independent fresh arrays per instance; nested source arrays stay aliased.
- create registers after full construction, local never registers, and registry
  overwrite/iteration use the existing actual SourceStringMap owner.
- A ParserRuntime represents one loaded module environment, never one formula
  or jax. ConfigurationHandler, TagsFactory and variantMap have module lifetime.
  The initial supported route is serialized in one runtime, as fixed in P46.
- TagsFactory's allocation constructor must not register built-in classes before
  runtime identity buckets exist. Module initialization happens after actual
  constructor identities are materialized. Its mutable OPTIONS object and class
  registry are allocated once; no fake Tags instance bootstraps registration.

The existing Context lacks Array.prototype.map. Root will add its ordinary-array
native host method to the existing Context, with a retained prototype callable,
arity 1 and normal writable/non-enumerable/configurable descriptor. The callback
receives value/index/original array and the specified thisArg. Default native
Array construction, undefined constructor and live constructor getters belong
to this profile. Custom species, generic non-array receivers, proxies and JS
evaluation require their own host implementation; do not silently bypass them.
Known out-of-profile species must fail explicitly, not return a default array.

Root owns declaration/registration stubs, pinned-original oracle and gold,
compilation, faults and acceptance. Luna's exclusive assignment will be the
arrayMap body in util/option_array_iteration.cpp and the map callable in
util/option_array.cpp, with no changes to forEach/filter/indexOf algorithms,
interfaces, tests, oracle, data, CMake or other Source classes. A second Luna
reviews the stable body against the frozen operation-order observations.

This frontier records actual dependencies and preserves the full TeX/package,
font, common layout, SVG and Folia integration objective. Passing Array.map
alone will not accept Configuration, runtime metadata or mathematical output.
