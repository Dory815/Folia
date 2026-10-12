# P51B actual RegExp identities in Context dictionaries

RegExpBindings is a converter-only native host above P49 identities and P50
RegExp. Runtime owns one binding/constructor registration per Context and lends
it to actual callers. It is not a new parser service or regex implementation.

create compiles a fresh actual shared RegExp once and binds it to a Context
instance whose actual native constructor metadata is RegExp. bind returns the
canonical object for an existing shared identity; distinct new expressions stay
distinct even when their text is equal. The existing NativeIdentityBindings map
is only an identity index. End dictionaries and [close, display, RegExp] tuples
remain the sole Context objects; no dictionary or pattern state is copied here.

resolve returns the actual shared RegExp. lastIndex/setLastIndex and exec always
resolve and operate on that same state, using the accepted P50 numeric/global
scanner profile. Context payloads retain the shared native owner, including when
the binding index is destroyed. clear is explicit runtime teardown after Source
consumers release retained references. Bindings and Context outlive typed calls;
foreign handles, wrong payload types and stale/noncanonical handles fail rather
than creating substitute objects. Non-object supplied values are TypeError.

Constructor metadata preserves Options.copy's native-instance aliasing. This
host does not claim arbitrary JS constructor invocation, RegExp prototype
reflection, Symbol/replace methods, or user-overridden exec. FindTeX uses only
construction, retained object identity, numeric cursor writes and exec results.
Use regexp_host::RegExpRef explicitly: TokenMap's existing RegExpContext seam is
a different type and must not be aliased or cast to it.

Root owns this declaration, native allocation/reference adapter and independent
original Options/RegExp observations plus actual shared-owner tests. This gate
does not implement the FindTeX class, packages, parser, mathematical layout,
SVG or Folia integration. Those requirements and the full active goal remain.
