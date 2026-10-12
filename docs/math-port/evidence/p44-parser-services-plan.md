# P44 parser service ownership plan after P43

Superseded before implementation by p45-parser-module-ownership.md. State-owner
and lifecycle observations below remain Source evidence; the broad service
expansion/import graph below is not the active implementation architecture.

This is Root's next architecture boundary, not accepted implementation. Source
signatures and original gold must still be frozen before Luna body assignments.
Do not forward-declare a class in one named module and define the same class in
another module. Each real class has exactly one canonical declaration owner.

## State owners

ParserConfiguration owns the package registry merge, handlers, configured item
and tag classes, node creators, options, filters and init/config callbacks.
ParseOptions owns one session: handlers handle, options, real NodeFactory and
StackItemFactory, Tags, ColumnParser, packageData, current parser list, mathItem,
root, nodeLists and error. TexParser owns the actual mutable UTF-16 string/index,
currentCS/saveI/macroCount and actual Stack. TagsFactory is one static registry;
each session gets one actual Tags instance. No owner stores a mirror of another
owner's state.

## Interface import boundaries to freeze

1. Keep types as token/ParseResult/StackItemContext/value identities. Move the
   entire ParserContext, ParseInput and parser-dependent callable declarations
   together into one new tex_parser_context owner; update parse_callable.cpp
   module ownership and direct consumers. Do not keep an old/new context pair.
2. StackItem/Stack stay independent of parser context, token maps and ParseOptions.
   The context imports Stack and value types, exposes only real TexParser-backed
   operations, and does not import MapHandler or ParseOptions. That prevents the
   current types -> StackItem -> types cycle if parser methods need Stack.
3. TokenMap/MapHandler import the context. Context handler dispatch operations
   are implemented by the actual TexParser; no second handler registry lives
   in the interface.
4. BaseItems imports Stack, the parser context and low factory/tag services.
   ArrayItem retains a borrowed actual parser, reading/writing its same string/i.
   ColumnParser imports the complete BaseItems declaration; BaseItems does not
   import ColumnParser. ColumnParser algorithms are in implementation units.
5. Independent tag service interfaces use MML/property values only. Tags owns
   its full classes and borrows the one session for options/nodeFactory/nested
   parsing. Its implementation may import actual TexParser to makeTag; a borrowed
   parseNested service must construct that same real TexParser, never a shortcut.
6. Low StackItemFactoryConfiguration remains below StackItem/context. Extend it
   only with required option/tag/node-list services; it must not import high
   parser session, ColumnParser or handler interfaces. ParseOptions implements
   the service and keeps its sole state. Existing NodeFactoryConfiguration is
   the addNode back-edge to the same object.
7. Higher ParseSessionServices composes context/handlers, low stack services,
   actual ColumnParser and tag services. TexParser's full declaration depends on
   this service, not ParseOptions. ParseOptions's full declaration imports the
   canonical Configuration/factory/tag/column interfaces and implements session
   services. Implementation units may import full peer modules without a cycle
   in the interface import graph.
8. Configuration declarations own their real classes and depend only on handler,
   item/tag class descriptors, options, priority/filter utilities and a borrowed
   TeX configuration target. They do not import ParseOptions or TexParser.

All Source service signatures and typed identity references must be enumerated
from complete enabled callers before these interfaces are coded. In particular
do not convert packageData/parser-stack/nodeLists into unrelated Property copies.

## Initialization and lifecycle

Before ParseOptions constructor body, Source initializes options={}, ColumnParser,
packageData Map, parsers=[], root=null, nodeLists={} and error=false. Then retain
configuration.handlers; allocate NodeFactory, bind configuration=this, setCreators
from actual configuration.nodes; allocate StackItemFactory from configuration.items
and bind this; apply caller defaultOptions first and configuration defaults second;
select current mathStyle or TeX fallback. This constructor does not create tags.

Actual TeX setup applies user options/configuration callbacks, registers tag
classes and selects/creates Tags, then binds tags.configuration to this session.
clear replaces parsers/root/nodeLists/error and calls tags.resetTag; factories,
options/tags/packageData/ColumnParser persist. Dynamic Configuration.add must retain
the Source registration/init -> creators/items/tags -> defaults/user options ->
filters -> config callback order.

Start.finalize/Equation need actual Tags. Array/EqnArray need actual parser input/i,
ColumnParser, ParseUtil and UnitUtil. A small reducer packet may have independent
method gold, but the configured base registry must include all 26 Source kinds;
test-only P43 Probe classes do not supply it. TeX -> MML package parity must precede
font/common layout, SVG and Folia acceptance. Mermaid stays paused.
