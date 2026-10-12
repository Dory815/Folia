# P53 actual parser declaration graph

The existing P45/P46 ownership decisions are now expressed by the production
`folia.math.input.tex.parser` module. Its interface partitions form a DAG;
physical files retain upstream Configuration, Tags, ColumnParser, ParseOptions,
TexParser, ParseUtil, TeX and BaseItems responsibilities. All 26 actual BaseItems
derive from the existing canonical BaseItem. No parser algorithm is accepted
by this stage and no new parser facade is introduced.

ParserRuntime holds module-lifetime configuration/tag registries and class
metadata. ParseOptions owns one option Context graph, real factories and the
actual ColumnParser. TexParser owns one UTF-16 source/cursor and borrows the
same ParseOptions. ArrayItem and ColumnState refer to the same actual parser
and array owners. Ordinary Source records/arrays remain Context values;
typed native references retain actual classes. Source allocation and field
initialization phases remain distinct, including Missing and Null values.

Three independent Luna Source audits covered the complete class families.
Root corrected ArrayItem.getEntry to its four-element tuple, GetDelimiter to
its nullable result, TagInfo.tagFormat to an empty string and currentCS to its
empty-string initial value. Module registries must survive formula sessions.
TagsFactory and ParserRuntime cannot be copied into per-formula registries.

`tests/math/parser_declaration_contract.cpp` compiles in the normal core target
and checks canonical inheritance, all 26 items, references, field types and
selected signatures. `build/p53-parser-graph-and-map.log` records a successful
build and all 12 original Map observations, exit 0. The string-key Map host
retains insertion order, replacement position, live appended iterator entries,
sticky iterator completion, stored Undefined and aliased owner lifetime.
Its oracle is `tools/math-port/string-map-oracle.cjs`; original observations
are frozen in p53-string-map-source.json. This is its required caller surface,
not acceptance of a general JavaScript Map implementation.

Unicode normalization is a separate portable host edge. Its native body and
original comparisons are the next stage. No Configuration/Tags/ParseOptions/
TexParser/ParseUtil/TeX or derived BaseItems method bodies are supplied here.
Package parsing, mathematical fonts, common output layout, SVG and Folia
integration all remain incomplete.
