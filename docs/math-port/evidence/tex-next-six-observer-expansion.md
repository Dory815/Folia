# Next-six observer coverage expansion

The next-six oracle still emits schema 1 and the same 20 cases. Its `maps`
projection now also includes the actual `empheq-env` and `empheq-macros` maps.
Empheq is part of this packet and is initialized for the Cases implementation;
omitting its maps left two owned exports outside the complete record. Package
load order, the pinned Source files, and the original case inputs are unchanged.

The expanded Source record SHA-256 is
`53969b6b3aca84c360e1df8004c60c602c9e3ea3326cd7e7eae3c0e14386b621`.
Removing those two map entries from every record reproduces the prior complete
Source record byte-for-byte, SHA-256
`86dbd573ef9c0b2c3d4ca16ad891c9a16bd9b8b1a5d4f887d9e5e3de036c8aa0`.
The retained projection is `build/math-port/tex-next-six-old-projection.json`.
The 20 per-case difference rows and all 93 grouped paths against the retained
G101 native observation are also identical after that projection.

The oracle continues to verify all 246 pinned Source locks from `scope.json`
and its profile hash before loading package code. These observations describe
the expanded capture surface; they do not establish native parity.
