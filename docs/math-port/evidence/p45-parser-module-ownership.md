# P45 actual parser-family module ownership decision

Root replaces the unimplemented broad-service expansion in
p44-parser-services-plan.md with one named-module owner and separate interface
partitions for the mutually dependent real TeX classes. This preserves Source
file responsibilities and actual class identities without proliferating parser,
configuration, tag or column facades. It does not move accepted low modules.

## Source basis

The pinned Source has genuine cycles among TexParser, ParseOptions, Tags,
ColumnParser, BaseItems, ParseUtil and Configuration. TexParser owns string/i,
currentCS/saveI/macroCount and Stack; ParseOptions owns options/handlers,
factories/tags/columnParser/packageData/parsers/mathItem/root/nodeLists/error.
Tags.makeTag constructs the actual nested TexParser with the same ParseOptions.
ArrayItem holds the actual active parser; ColumnParser mutates the same ArrayItem.
Configuration config/add uses the actual TeX jax and its parseOptions/filters.

Cross-named-module forward declarations cannot express one class with multiple
owners. One `folia.math.input.tex.parser` owner solves declaration identity;
partitions' interface imports still form a DAG. Physical files stay aligned
with original responsibilities; algorithms stay in corresponding .cpp files.

## Declaration owners to implement next

- parser_family.ixx: sole primary interface, exports every named partition.
- parser_types.ixx / :types: same-owner forwards of real ParserConfiguration,
  Configuration, ParseOptions, TexParser, Tags/AbstractTags, ColumnParser,
  ArrayItem and TeX classes; typed lightweight aliases. No class algorithms.
- configuration.ixx / :configuration: real configuration classes, package
  registration order and descriptors; not a second registry.
- tags.ixx / :tags: actual Labels/TagInfo/Tags implementations/TagsFactory.
- base/base_items.ixx / :base_items: all 26 actual configured base item classes.
  They derive from the accepted BaseItem, whose existing owner is unchanged.
- column_parser.ixx / :column_parser: actual ColumnState/ColumnParser, accepting
  same-owner TexParser/ArrayItem references. No copied parser input or array data.
- parse_options.ixx / :parse_options: actual canonical session fields and existing
  NodeFactoryConfiguration/StackItemFactoryConfiguration back-edge implementations.
- tex_parser.ixx / :tex_parser: actual parser fields and complete typed Source
  methods. It inherits the current ParserContext for the invocation bridge;
  it does not maintain another context state.
- parse_util.ixx / :parse_util: actual helpers using the same parser/session/items.
- tex_jax.ixx / :tex_jax: actual TeX class when needed by Configuration, avoiding
  a false TeX forward declaration belonging to a separate named module.

Declaration ordering follows complete-type requirements: reference/pointer/
vector-of-pointer members can use :types forwards; by-value members and bases
import their actual defining partition. ParseOptions imports ColumnParser's
complete declaration. Interface method bodies that need peer fields move into
implementation units. No partition mutually imports another partition.
Implementation units belong to the primary module and see real peer declarations.
External code imports only the primary module, never individual partitions.

Existing standalone Stack/BaseItem/NodeFactory/StackItemFactory/token maps and
UnitUtil remain canonical. Actual BaseItems can validate/cast the existing
configuration service back-edge to the same ParseOptions in their implementation
units, rather than copying options or installing another configuration object.
Actual base/AMS callback bodies accept the current ParseMethod bridge and obtain
the same real TexParser by checked native type, using its complete typed methods.
There is no generic Property dispatch substitute for parser methods.

## Toolchain evidence and limits

Root's ignored build/p45-partition-pilot verifies the repository's installed
CMake/Ninja/MSVC with C++23/import std and explicit CXX_MODULES file sets:
same-owner forwards in :types, distinct parser/options/array/column declarations,
by-value column member, primary export-imports, implementation-unit auto visibility,
separate client import, pointer identity and one shared options state. Configure,
compile, static link, client link and executable all pass, exit 0
(build/p45-partition-pilot.log). An initial batch trailing-directory quote error
is corrected by using `%~dp0.` for the CMake source directory.

This ignored diagnostic is only a C++ toolchain/declaration proof, never a Source
parser, TeX semantic test, alternate production implementation or acceptance gate
for math output. The actual class declarations must still compile together in
folia_math before algorithm delegation. Root owns that gate and Source gold.

## Remaining real cross-owner requirements

core/MathItem.ts belongs to a separate core owner. ParseOptions and Tags refer
to its actual identity; do not define a parser-specific same-named fake MathItem.
Its necessary native interface/fields must be established against real callers
(Tags.startEquation/finishEquation and BaseMethods display), with the portable
core/MathDocument/InputJax boundaries reviewed. MathItem does not acquire a
reverse parser-module dependency. Fonts/common layout/output remain separate.

Root must enumerate complete fields/defaults/nullable results and Source
constructor phases, define the actual module partitions, freeze independent
original observations, then delegate bounded method bodies to Luna. There is
no permission to ship a small start/stop/probe registry as the base parser.
Complete base/AMS/enabled-package TeX -> MML parity, shared layout, SVG and Folia
still define success. Mermaid remains paused.
