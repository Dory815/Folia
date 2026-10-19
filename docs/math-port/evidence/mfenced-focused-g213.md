# Gate213 mfenced isolation

The full Core run exits with `0xC0000005` after about 827 seconds. Its last
captured, incomplete progress line names `math_port_mfenced_direct_null-text_212`.
Buffered progress is not a faulting stack and does not identify the crash site.

Root separately ran the same built executable with
`FoliaTests.exe "*mfenced_direct_null-text*"`. All six selected tests completed:
36 assertions, 30 passing and six failing, process exit 1. This group does not
independently reproduce the access violation. Its assertion differences still
need diagnosis; they are not accepted or discarded as fixture errors.

- Executable SHA256: `8db60b45f3a296b770c2e6590b7416f4830c19cc62a81a1435a43a44576133de`.
- Focused log: `build/math-port/mfenced-null-text-g213-focused.log`.
- Log SHA256: `2e08f17e40b088a18ecea86d97121efc9bc8b35ce1b5d06a51e8d9205fc4d3b4`.
- The executable hash still matches the full Gate213 Core run.

The pinned CJS-only `mfenced-oracle.cjs --check` verifies 225 cases and 984
observations. Review of the visible null-text path finds a local factory
returning null followed by an ordinary caught TypeError; no recursion or
Context callback occurs on that path. A native fault address/stack or a smaller
reproducing group is required before assigning a production fix.
