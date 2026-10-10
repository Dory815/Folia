# Operator dictionary prerequisite audit

Status: dependency audit, not implementation or acceptance. Root verified the
installed CJS in an isolated Node process and inspected the pinned source.
Hashes and actual counts are in evidence/operator-dependency-audit.json.

OperatorDictionary.ts exports OPDEF, MO, RANGES, getRange, MMLSPACING and
OPTABLE. There are 27 named MO arrays, 50 range arrays, eight spacing rows,
137 prefix entries, 94 postfix entries and 1,021 infix entries. The 1,252 table
entries reference only 37 definition arrays: 1,242 entries alias named MO
arrays. Flattening these into separately copied records loses observable state.
Definition arrays and their property objects require distinct live identities.

getRange first reads infix, then prefix, then postfix by truthy short circuit.
Table objects are ordinary objects, so inherited names such as constructor and
toString cannot be treated as absent entries. A table hit allocates a fresh
four-slot range with its class copied from slot2 (possibly Missing for an
inherited function). Otherwise it reads the first UTF-16 code point and scans
the ordered RANGES collection, stopping at the first upper bound even when
the lower bound fails. A matched range returns its existing array identity;
the fallback allocates a fresh REL/mo tuple. Empty text is an explicit ORD
infix entry. Multi-character text uses its complete table key before examining
only its first code point. OPDEF defaults apply only to Missing/undefined.

The profile imports common output and therefore LinebreakVisitor. Its module
initialization mutates every truthy fence property object reached through
postfix definitions, adding linebreakstyle=after. Root observed 33 affected
postfix keys; aliases in other forms see the same object writes. It then
shallow-clones infix U+2061's definition array and replaces slot3 with a fresh
linebreak=nobreak property object. The clone no longer aliases MO.NONE.
This is an initialization behavior owned by LinebreakVisitor, not an extra
immutable dictionary record to bake into generated initial data.

Before delegation root must freeze identity-bearing operator/range/spacing
array handles, ordinary-object lookup semantics, live table bindings and the
native scalar/property bridge used by MmlMo, BaseConfiguration and NodeUtil.
Generation must validate every initial source record and every alias; it must
not define getRange's behavior or silently import the post-initialization state.
Fixtures must separately validate pristine dictionary state, live mutation,
and the profile's subsequent startup mutation. No substitute mo implementation
or partial MML registry is authorized by this audit.
