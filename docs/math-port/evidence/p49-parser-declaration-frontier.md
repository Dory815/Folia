# P49 parser declaration frontier after actual InputJax

P47 and P48 are accepted prerequisites, not a native TeX parser. Root has read
the pinned ParseOptions, Configuration, ColumnParser, TexParser, TeX and StackItem
sources and obtained three bounded readonly Luna declaration audits. No parser
body assignment or new parser declaration has been accepted from those audits.

The named-module/physical-file ownership remains P45's parser-family decision.
P46's single-runtime option graph and Source global registries remain unchanged.
P48 provides the actual AbstractInputJax superclass; there is no excuse for a
fake TeX base. Root must freeze the actual declarations and compile their entire
partition graph before delegating bodies.

## Root's declaration constraints from Source

- Configuration descriptors retain actual fallback/items/tags/options/nodes
  identities. Handler defaults are four fresh arrays merged into one new record.
  Processor scalar functions become tuples, but already supplied tuples retain
  their identity. ParserConfiguration owns its distinct merged records, real
  SubHandlers, two FunctionLists and prioritized configuration descriptors.
  ConfigurationHandler remains the one module-level Map, with replacement by
  name preserving the exact descriptor value.
- ParseOptions owns fresh Context options, actual node/item factories, a complete
  by-value ColumnParser, packageData Map, parser array and nodeLists object.
  handlers aliases ParserConfiguration.handlers. Factories' configuration edges
  point to this very ParseOptions via the existing two service interfaces. Tags,
  mathStyle and mathItem begin absent; root is null. Constructor merges its
  option-list entries, then configuration.options, before selecting mathStyle.
  clear replaces parsers/root/nodeLists/error and calls tags.resetTag; it keeps
  packageData/options/factories/tags/ColumnParser. TextMacros aliases packageData
  across sessions; it must have an identity-bearing Map owner.
- The actual 26 BaseItems classes must be declared together. EqnArrayItem derives
  from ArrayItem and MstyleItem from BeginItem; the other 24 derive from BaseItem.
  Reuse the existing actual factory registry, constructor metadata, BaseItem and
  CheckType. Source BaseItem phase precedes derived fields, even though virtual
  isOpen is called during that phase. Existing explicit adoption/initialization
  already expresses that rule; do not pre-initialize derived fields.
- ArrayItem has separate fresh table/row/frame/hfill/cstart/cend/cextra/ralign
  arrays, arraydef and breakAlign records, atEnd=false, templateSubs=0, and absent
  parser until assigned by BaseMethods.Array. EqnArray maxrow=0. Mstyle attrList
  retains its supplied PropertyList identity. ColumnState owns fresh cwidth/
  calign/cspace/clines, but borrows the exact ArrayItem cstart/cend/ralign/cextra
  arrays. No value-copied vector or second parser/array state is allowed.
- TexParser owns one UTF-16 input, i/saveI/macroCount, currentCS and actual Stack;
  configuration borrows the actual ParseOptions. The existing ParserContext
  getter/setter must expose the same currentCS field. Source getters forward live
  configuration.options/itemFactory/tags. Its constructor mutates the supplied
  env isInner key, copies remaining own keys into a new ENV, registers this parser,
  creates Stack, parses, pushes stop and sets stack.env in that order. Nullable
  results (e.g. delimiter/argument/MML) stay distinct from bracket Undefined.
- TeX's native factory must enforce separateOptions -> actual InputJax base
  phase -> derived fields -> FindTeX/configuration/ParseOptions -> config -> tags
  -> actual seven postfilters. The base-phase factory alone does not implement
  the eventual derived Source phase. Actual FindTeX/FindMath are still absent.

## Actual binding gates before declaring the whole graph

Source dictionaries holding nodes or constructor descriptors need a typed native
identity binding to the existing Context graph. Property currently cannot contain
MML nodes or FactoryClassRefs; a naive unordered_map would also lose ordinary
object key/prototype behavior. Configuration item/tag/node records and nodeLists
must retain their real Source lookup/enumeration semantics and actual owners.
Context payloads can hold those real typed references. They must not clone MML
trees, construct new factory/class registries, mirror dictionaries, or maintain
synchronized Context-array/native-array copies. Root must freeze the concrete
binding and its caller signatures before Luna body work.

NodeLists is an ordinary object, not a Map: addNode/getList/removeFromList observe
inherited keys and current array identity; getList replaces the filtered array,
while removeFromList mutates the retrieved array. MML ChildArray remains the
canonical MML graph collection; a node-list array is its own Source array and must
not be confused with childNodes, MmlStack nodes, ArrayItem row, or table.

The native language host must cover required UTF-16 string/index/regexp behavior
and proper portable Unicode NFD (French mathStyle and ParseUtil). Existing
dimension patterns and map regexp hosts are not a complete dynamic regex engine.
The vendored SRELL source is a candidate to audit, not yet an accepted binding.
Windows editor ICU tests do not establish a portable math Unicode dependency.

Root corrections to agent audit wording: EnvList is Source's ordinary scalar
record (`StackItem.ts` EnvProp/EnvList); the accepted Property graph is suitable
for that boundary. It does not require another env owner. Actual TexError already
exists and is accepted by P40; do not reimplement or replace it.

This frontier records constraints and unresolved concrete types, not completion
of the declaration graph or parser. Complete enabled-package TeX -> MML, native
fonts/common mathematical layout, SVG comparison and Folia integration remain
required by the active goal. Mermaid stays paused with its existing checkpoint.
