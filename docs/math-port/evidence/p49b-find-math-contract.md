# P49B actual FindMath base before FindTeX declarations

FindTeX's actual Source superclass is core/FindMath.ts, not core/InputJax.ts.
Root freezes its complete interface/AbstractFindMath declarations and host class
registration/allocation, before delegating its single constructor algorithm.
Source ProtoItem aliases move to their original core/MathItem.ts owner; InputJax
and FindMath import the same aliases. No parallel proto result shape is created.

Source hash: 6beae5addf391086272ffdcf7050d85e7bb42457842d12b7ec3af991583720d8.

The abstract base owns only its canonical Context options plus native allocation
bindings to that Context/instance. OPTIONS lives solely on the actual constructor
object, registered once per runtime by FindMathModule. There is no cached class
options table. findMath remains abstract; the native array finder will be the
actual FindTeX. Generic DOM inputs remain opaque complete native inputs.

Luna owns only src/math/core/find_math.cpp::initializeFindMath. The Source supplied
options parameter has no default {} initializer: pass it unchanged to userOptions.
Capture current instance.constructor once, allocate fresh default destination
before reading that CLASS.OPTIONS, run actual defaultOptions then actual
userOptions, and publish options_ only after both complete. No catch, dictionary
mirror, fake finder, superclass substitution, snapshot static options or duplicate
function body is allowed. Full allocation precedes this Source phase, with
derived FindTeX fields and getPatterns deferred until after it.

Root owns declarations/host, actual Source oracle, native fixture, compile/red
baseline, review/build/fault checks and acceptance. Luna cannot edit those files,
build, test, commit or delegate. This prerequisite does not accept FindTeX,
regexp behavior, complete parser declarations, TeX/SVG or Folia integration.
