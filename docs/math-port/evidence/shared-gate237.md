# Shared gate 237: prototype binding and property identity diagnosis

The 61-target build completed with exit 0, at most four compile jobs, and
2,197 unchanged registered inputs through evaluation. Native captures terminated
normally; no test process remained live when the inputs were released.

Fresh native factory products now register their canonical payload and install
the actual class prototype before descriptor-sensitive field binding. Existing
Source receivers retain their supplied prototypes. The factory accessor test
passes, including checks that no own factory field hides the inherited accessor.
The pinned Source witnesses are tracked under
`tools/math-port/regressions/g237-mml-binding/` and were run from that location.

The complete `FoliaMathHostTests` runner exits 1 with one failing test and two
assertions: `foreignObject` loses identity when converted from the MML module's
typed Property through the parser's separate ContextPropertyBindings adapter;
the two independently created typed wrappers also compare unequal. The new
raw Context assertion passes, locating the problem after finite-child copying,
at the typed adaptation boundary. Both adapters use the same Context.

The pinned JavaScript witness confirms that finite-child conversion must retain
the exact object-valued property. The remaining assertions are preserved for a
subsequent boundary fix. Evidence is in `build/math-port/host-g237.txt.*` and
`g237-property.txt.*`; this gate does not claim host-suite acceptance or validate
SVG after the prototype change.
