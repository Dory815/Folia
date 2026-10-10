# Offline MathJax bundle

The QuickJS runtime uses MathJax with NewCM and the SVG font extensions for
`bboldx`, `bbm`, and `dsfont`. The TeX packages define commands; `entry.cjs`
attaches the corresponding font data to the SVG output explicitly. The font
extensions are embedded in the bundle, while NewCM's dynamic ranges are loaded
from the local `Assets/mathjax/font/` directory.

From this directory, restore and rebuild with `npm ci` and `npm run bundle`.
Run `npm test` to compare rendered outlines for every extension font variant
against the official font data in both inline and display formulas. The test
also rejects font warnings, errors, and text fallbacks.

From the repository root, run `setup.ps1`, build the WinUI app in Release, then
run `cmd /c tools\mathjax-bundle\quickjs-smoke.bat 2>&1`. This checks 500 formulas
in QuickJS with the app's 24 MiB memory limit and 8 MiB stack limit, passes their
SVG fragments through the native normalizer, and validates them with Direct2D.
Test binaries and failure artifacts stay in `build/tools/mathjax-bundle/`.
