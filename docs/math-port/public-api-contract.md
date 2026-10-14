# Public native math interface

Status: implementation candidate. An isolated compile and nine-case Source
comparison pass; the complete shared build and renderer acceptance are pending.
This interface does not authorize switching Folia's current math worker.

`src/math/math.ixx` exports `folia.math`. `Renderer::render` accepts UTF-8 or
UTF-16 TeX and display, em and container-width parameters. Its result owns an
SVG string and the immutable Scene completed by that same conversion.
`toSvg` is a convenience call over `render`, with no separate rendering path.

`src/math/math.cpp` owns application-profile assembly only. It constructs the
existing native render session and HTMLMathDocument, calls its real `convert`,
serializes the completed SVG output, and returns the completed Scene. TeX,
MML, measurement, placement, font lookup and SVG serialization remain in
their established modules. The facade does not implement any math command,
compute geometry, interpret SVG, or introduce another parser or layout tree.

One Renderer instance belongs to one calling thread and retains one Source
document/session across calls. Macro and document state therefore follow the
same lifetime as Folia's reference entry. Returned Scene trees and SVG strings
remain usable after later conversions and destruction of that Renderer.
The future Direct2D consumer takes this completed Scene and performs no
mathematical measurement or placement of its own.

The profile is the existing 38-package Folia entry with NewCM and the bboldx,
bbm and dsfont extensions. It uses fontCache none, exFactor 0.5, display line
breaking and inline fragments. Zero/NaN em and width take the reference entry's
defaults; other supplied numeric values are passed to the existing pipeline.
Source exceptions retain their names and messages in owned RenderError values.
Font loading uses the native session's synchronous font module provider.

## Verification

The public-interface candidate imports only `std` and `folia.math`. Nine ordered
cases cover plain math, fractions and roots, blackboard font extensions,
display scripts, a narrow container, labeled alignment, incomplete input,
UTF-8 Greek text and reuse after earlier conversions. It traverses every
returned immutable Scene after the Renderer has been destroyed.

The corresponding original observer loads the pinned MathJax 4.1.3 profile,
verifies its dependency hashes and converts the same ordered inputs. The
complete original observation SHA-256 is
`4fe64e87ccaf386e19f67defd1d940d5332b247151d4d0f41b90274a500f443c`.
The comparison checks every SVG/error record exactly and separately requires
the completed Scene lifetime observations.

Passing these nine cases accepts only this public entry's bounded behavior.
Complete package registration, the full rendering observations and the other
Source comparisons remain required before Folia integration. An isolated
compile against a previously completed library is an early diagnostic only;
the new interface must also pass the complete shared build and fresh runners.

The bounded isolated observations are in
`evidence/public-api-isolated-g101.md`.
