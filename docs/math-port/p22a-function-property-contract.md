# P22a root freeze — configuration function identity and slice binding

This is a native host prerequisite for complete TokenMap/configuration, not
acceptance of TokenMap or a general JS function-object host. Root owns type
layout, module graph, fixtures/stimuli, builds, mutations, acceptance and every
existing call-site change. Luna owns only src/math/util/function_value.cpp,
src/math/core/tree/property_array.cpp and
docs/math-port/evidence/p22a-function-property-implementation.md after dispatch.
No other edits, builds/tests, expected data, commits, delegation or integration.

## One function and one graph

Move the unique Missing marker to object_types, exported transitively to old
importers. callable_types depends only on object_types/std. Property exports
callable_types and stores the existing FunctionRef as an additional value arm.
These are binding containers for the same shared Function and same shared
FunctionBody; there is no cloned callback, second macro-array representation,
callback-name identity or compatibility route. Null raw function properties
are invalid native protocols and rejected by the Property constructor.

The frozen functions::reference(FunctionValue) returns the same active direct
FunctionRef or function-valued Property reference, otherwise an empty handle.
functions::fromProperty unwraps a stored function to the direct typed arm;
all other Property values remain unchanged. functions::toProperty wraps the
same direct reference or returns the existing Property. It uses the guarded
Property constructor for a null direct handle; no native function text or
evaluated function-property metadata is fabricated.

FunctionValue's non-template equality preserves strict source identity across
typed/raw containers: same FunctionRef is equal, different functions are not,
even with identical bodies; raw Property scalars/objects keep existing strict
Property equality (NaN unequal, zero signs equal). Two direct null handles
retain their prior equality; a null typed handle remains an invalid callable,
not permission to create a raw function Property. A direct non-function
Property and a direct function compare unequal. No function coercion occurs.

Root's existing functions::invoke and invokeParseMethod use reference, capture
the same live body as before, preserve receiver/arguments/results/throws and
keep FunctionList/PrioritizedList algorithms unchanged. Root validates both
storage spellings against actual Source and existing callable regressions.

## Ordinary array slice

PropertyArray::slice(size_t first) is a native nonnegative-index binding;
MacroMap's Source call supplies exactly 1. Snapshot original array length,
start at min(first,length), create a fresh ordinary array of length length-start,
then for every source index perform HasProperty followed by Get iff present,
writing that destination own slot. Missing holes stay holes; own undefined
and inherited undefined become present undefined; nested objects/arrays and
functions retain shared identity. Numeric prototype data is included. Do not
call spread/argument expansion or compact holes. No generic signed JS-index
coercion or custom species/proxy/getter execution is claimed by this binding.

## Domain and required later work

Actual inputs are ordinary configured map/function dictionaries and arrays
with own/prototype data values, including callable slot zero and ordinary
source Args in the remaining slots. FunctionRef is a callable identity value,
not PropertyStorage. Key enumeration of dictionaries is independent of values.
Metadata access on the function object itself explicitly reports an unbound
native host rather than returning incorrect Object.prototype metadata.
Function-object descriptors, arbitrary getters/proxies/custom iterators/species,
and coercion/metadata not required by these map-data callsites are **not**
accepted here. If a configured Source callsite requires them, root must add
the corresponding typed host contract; no silent omission/fallback is allowed.
Complete engine/TokenMap/parser/Common/scene/SVG/Folia gates remain open.

## Independent acceptance

Root produces actual Node/CJS storage, slicing, identity/closure/receiver/result
observations before implementation. Native tests check those values, function
ownership and invalid-native boundaries. Compiling semantic mutations must fail
assertions, without crashes or compilation errors counting. Root runs focused
tests plus the required complete core build/tests, preserving accepted Source
fixtures and method bodies. This host packet increases no TS file count.
