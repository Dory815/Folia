# P51A ordinary option-array forEach binding

This is a native language primitive in the existing Context owner, required by
the actual FindTeX getPatterns method. It does not implement delimiter scanning
or create a second options graph. Existing arrayValues has live iterator length;
the Source forEach operation captures the initial length instead.

Root adds the real method identity to Context's Array prototype with arity 1 and
the existing nonenumerable method descriptors. Calling that looked-up method
uses the original array receiver, supplied callback value and optional thisArg.
Other actual native callbacks may override the looked-up method. The host remains
the existing ordinary Context-array profile; generic borrowed forEach on arbitrary
array-like objects and JavaScript callable evaluation are not claimed.

arrayForEach(ObjectRef array, Value callback, Value thisArgument) must validate
the same Context/array, capture its array length, then validate a native function
callback even for an empty array. For each original index it performs Has, then
Get only if present (including inherited entries), then calls the actual callback
with the supplied thisArgument and exactly [value, numeric index, original array].
The callback's return is ignored; native exceptions propagate without catches.
Future additions beyond the captured length are ignored, deleted slots skipped,
and edits to not-yet-read slots observed. Replacing the caller's array variable
does not replace the captured receiver. The looked-up method returns Undefined.

Root owns declarations, intrinsic registration wrapper and independent original
observations. Luna may implement only Context::arrayForEach in the sole body
file option_array_iteration.cpp after the complete stub graph compiles. No test,
gold, Context field, native function registry, build or commit changes by agents.
This host prerequisite does not accept FindTeX or the TeX parser.
