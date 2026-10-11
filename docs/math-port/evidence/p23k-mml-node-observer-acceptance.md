# Root acceptance: repaired native observer binding, full-core gate still open

Root reviewed the C++ emitter changes without modifying production APIs. The
observer uses the sole ChildArray/NodeValue graph and parent accessors. Native
callbacks and returned identities use current tagged signatures; original JS
classes, stimuli, methods and expected values remain frozen.

Root rejected the first delivery: changing notParent's eq helper to pp changed
the frozen observation category, and the generated append check referenced an
undeclared c. The agent restored eq with an explicit bool Property projection
and generated the same original c before append. Neither failed gate counts as
acceptance. Root then independently reproduced both the generated fixture and
the unchanged actual Source profile: **1,084 assertions / 43 cases** and
**1,051 helper observations**, SHA
`05b8ce98c2e6fa93edac57aac32deba966404e39e240fac042ff55cbf3841a03`.
Direct emitted expectations and the seven native resource-budget checks remain
in the fixture; no assertion is removed or skipped. Profile diagnostics now
report only the first changed record while retaining the same strict byte gate.

The isolated **90-module/all-unit graph compiles the repaired fixture**. No
semantic test is executed by this compile-only gate. Result file:
build/math-port-mml-node-observer-compile/compile-result.json, SHA
`a88045b12120b7ffeae48c605d2b711e9b7a34b55fba2e194d681a6ad256b5c1`.
Working production and the fixture remain byte-identical during the gate.

The repository `cmd /c build_test.bat 2>&1` now compiles reference_mml_node.cpp
and folia_math, but stops on seven other stale native observers:
reference_html_node.cpp, reference_script.cpp, reference_leaf.cpp,
reference_mathchoice.cpp, reference_arithmetic.cpp, reference_maction.cpp and
reference_arithmetic_order.cpp. Their dense/raw-parent/pre-tagged bindings must
be repaired at the test boundary; production compatibility fields are forbidden.
No repository core test has run in this check and the full-core gate is not green.
Log: build/p23k-full-core-check-repaired.log, SHA
`ca374dabe4161af8d16c42a3465fa7eccab7b6ed63417412efc45086dd4dd396`.
The rejected first compile log SHA is
`e44da3d6f1be69aa9390209bdc6777bd4e5ebaa9678b5bb7e63376494acb7e2e`.

This accepts only repair of this observer binding. It does not add MML algorithm
or whole-file acceptance; **54/246** remains unchanged. Remaining core observer
repairs, MML/TeX/Common/fonts/SVG and Folia are still mandatory.
