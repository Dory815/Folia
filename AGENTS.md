# Repository instructions

## Mermaid 重构执行规则（仅本 fork）

本节是给执行 Mermaid C++ 重构、测试和审阅的 agent 的工作指令。
开始工作、委派任务和恢复上下文时先读本节，再读 `docs/mermaid-cpp/HANDOFF.md`。
本节按用户 2026-10-23 的要求确定范围与优先级；旧交接、计划、测试清单中要求
追求全面兼容或穷尽边界的内容，不得覆盖本节。范围仅限 Mermaid。

### 先完成项目功能，再做分级修复

严格按以下顺序选择工作，不得以低优先级事项阻塞高优先级主线：

1. **Folia 实际应用下的功能适配。** 优先把项目需要的解析、配置、布局、输出及
   应用调用链贯通，推进所有必要阶段达到可用、可验证的状态。
2. **常见情况下的防御与修复。** 在所需功能链完成后，处理项目常见输入、
   常见编辑状态和已知使用方式下的问题；直接阻塞当前功能验收的实际缺陷随当前步骤修复。
3. **罕见情况下的问题与修复。** 在前两层完成后，再处理项目范围内有实际依据的
   低频问题。纯假设、无法关联到项目使用方式的极端情形，不构成阻塞项。
4. **项目范围之外的功能不做。** 不移植、不扩展、不为其设计兼容层、防御、
   测试矩阵或修复计划，也不把它们列为“完成重构”必须清空的待办。

优先级由实际调用、项目需求、代表性输入或可复现问题决定。“理论上可能发生”、
“上游支持”、“更通用”或“将来可能用到”均不能单独成为新增工作的理由。
已复现且影响项目当前使用的崩溃或错误属于当前功能问题，不能借此规则掩盖。

### 范围由 Folia 的需要决定

- 官方 Mermaid JS/TS 是**所需功能的行为参照**，不是必须完整翻译的功能清单。
  只阅读和移植完成当前功能所必需的源码与依赖，不启动无关的全量审计。
- 对上游扩展、插件、宿主回调、浏览器集成、配置组合及 JS 动态对象语义，
  先确认 Folia 是否实际需要。项目不使用的部分不实现，也不做预防性修复。
  不为了不需要的原型操作、运行时注入或任意外部调用重建 JS 运行机制。
- 现有图族和行为清单用于确认 Folia 的功能需求，不能自动扩展为上游全部 API、
  扩展能力和极端输入的兼容承诺；也不能仅因某个图族不在当前样例中就擅自删掉
  项目已有的功能要求。
- 不扩大到其他库或其他模块的重构。发现无关事项时继续当前 Mermaid 任务。

### 阶段必须有退出条件

- 每批开始时简短写清：要交付的项目功能、范围、通过条件和下一阶段。
  使用现有交接文件记录，不另建重复的进度体系。
- 当前阶段达到所需功能和对应验证条件后，提交该批并立即推进下一阶段。
  不得把 A 反复从 90% 打磨到 99%、99.9%、99.99%，让 B/C/D/E 始终未开始。
- 阶段完成指项目约定范围内的功能完成，不要求罕见边界全部消除。
  项目范围内已知但非阻塞的低频问题可简短记录后延后，不持续扩充假设性问题清单。
- 已通过的阶段仅因相关代码变化、实际回归或新的明确项目需求而重开；
  不因换 agent、上下文恢复或“再稳妥一点”重复宽泛检查。
- 报告进度以可用功能、已贯通阶段和真实阻塞为准，不以测试数量、
  防御分支数量或处理了多少极端输入代替主线进度。

### 验证、审阅与委派同样受限

- 验证覆盖本批改动和项目实际使用路径：代表性成功案例、必要的常见失败案例，
  以及受影响的既有回归检查。官方行为对照也只围绕当前所需功能建立。
- 对应检查通过后停止扩张验证。没有新失败、相关改动或实际使用证据时，
  不主动追加无界模糊测试、极端规模、组合穷举、全协议语义或通用防御框架。
- 审阅提出的问题先判断是否在项目范围内、是否阻塞本批验收；
  非阻塞的低频问题不得自行升级为新一轮修复任务。
- 委派给子 agent 时一并传达本节，并明确功能目标、允许范围、退出条件和
  必要验证。不得委派“全面加固”“检查所有边界”这类没有停止条件的任务。
- 保留与项目实际功能有关的必要错误处理及已有保护。本规则不是删除现有校验、
  跳过已失败测试或宣称未验证功能完成的理由。

### 仅 fork 保存，不进入上游贡献

- 本节保存在 `zhao0349/Folia` 的 `fork/mermaid-development` 分支。
  这是本 fork 的执行工作分支；不要直接向上游提交或合并整个分支。
- 本节的指南提交必须独立于功能提交。向上游贡献时使用不含本节的贡献分支，
  仅挑选需要交付的功能、测试和公共文档提交，排除本 fork 的执行指南。
- 当前 `feat/mermaid-cpp-port` 是上游 PR 的来源分支，不向其中加入本节。
  交付前检查 PR 实际差异，确认没有携带本节或引用仅 fork 存在的指南。
- 不为排除指南而擅自重写、强推或重置已有提交；遵循用户对上游 PR 的合并控制。

## Math migration continuity

When working on the native MathJax migration, read `docs/math-port/HANDOFF.md`
at the start of a session and immediately after context compaction, before
planning, delegation, edits, or repeating an investigation. It records accepted
decisions, completed checks, known rejected approaches, and the exact next step.
Update that same file after a material decision, verification, or commit and
before handing off; do not create successive competing progress notebooks.
Use the record to resume work, then inspect only changes that could invalidate
it. Do not repeat completed broad audits or reinterpret settled user decisions
merely because context was compacted. This record is continuity information,
not permission to reset Git, overwrite unrelated work, or claim integration.
Actually read the file contents after compaction; a summary mentioning its path
does not satisfy this step. Before reopening a settled issue, identify the code
change or new evidence that invalidates the recorded conclusion. Keep a short
current-state and next-action checkpoint at the top of the same handoff file.
Before context compaction or ending a work session, save the exact stopping
point, live agent/build handles, candidate paths, verification limits, and next
concrete action there. On recovery, use the latest checkpoint rather than old
agent completion messages; search ignored candidate directories with
`rg --no-ignore` before declaring a delivered file missing.
Keep one authoritative current checkpoint at the top; label superseded status
and next-action sections as historical. An instruction to read the notebook is
not enough: update stale entries before handoff and actually read it on recovery.

## Build and test

- Use C++23 modules and `import std;`.
- Core tests: `cmd /c build_test.bat 2>&1`.
- Deterministic Windows editor tests: `cmd /c build_platform_test.bat 2>&1`.
- Core-only builds may target `folia_core` with CMake.
- Restore WinUI/NuGet and native SVG dependencies with `powershell -ExecutionPolicy Bypass -File .\setup.ps1` before building the WinUI app.
- Keep generated output in ignored build directories; do not commit build products or restored packages.
- Windows console tests/probes must link `folia_console_test_runtime`; manual builds must include `tools/test-support/windows_console_runtime.cpp`. CRT failures and `abort()` reports go to stderr without desktop dialogs. Stop a broader evaluation after an abnormal process exit and diagnose it with a bounded focused run.

## Code and platform boundaries

- Core modules under `src/core` are platform-independent and must not include Windows, WinRT, Direct2D, DirectWrite, DXGI, or TSF headers.
- Windows and rendering APIs belong in `src/platform` or the WinUI application layer.
- The application shell remains WinUI 3 with C++/WinRT; do not introduce a Win32 WndProc shell.
- Tests use the repository's forked Boost.UT C++23 module through `tests/support/folia_test.hpp`, which performs `import boost.ut;`. Test translation units deliberately do not `import std;`; they include the shared test header and then import the required `folia.core.*` or `folia.platform.*` modules. Do not replace this with the header-only `<boost/ut.hpp>` path. `tests/main.cpp` provides the core runner and `tests/platform/main.cpp` provides the Windows editor runner; both pass `argc`/`argv` to `boost::ut::cfg<>.run()`. Use a positional pattern such as `FoliaTests.exe "*name*"` to filter tests (`-t` lists tags).
- Prefer semantic fixes and focused tests. Do not add source-text special cases to make one example pass.

## Document model — block tree plus block-local lossless source

The authoritative editing state is a block-level structure tree. Each editable content node (Paragraph, Heading, TableCell, and the paragraph/heading bodies inside list items, task items, block quotes, callouts, and footnotes) owns the full, character-exact Markdown source of its inline content plus a lossless editable inline CST over that source.

- `InlineDocument.source` is the authoritative inline character content of its node: complete, unnormalized, including every marker, escape, entity, whitespace, and link-writing detail.
- `InlineDocument.tree` is a lossless editable CST over `source`. Concatenating its leaf token text in order must equal `source` exactly, character for character — not Markdown-semantically equal.
- The CST must preserve the original marker glyphs (`*` vs `_`, `**` vs `__`), escapes, entities, link/url spellings, quote forms, and whitespace, and must represent unclosed, ambiguous, and temporarily invalid editing states as error/incomplete nodes rather than collapsing them to plain text.
- Saving serializes the unchanged `source`; it must not re-derive or re-select markers from semantic fields for content the user did not edit.

## In-block coordinate system

Positions inside an editable content node are expressed as a container node id plus a source offset into that node's `InlineDocument.source`, with a text affinity. The offset counts every source character, including markers.

There is one in-block coordinate. Do not maintain parallel position schemes — logical text offsets, inline node ids, opening/content/closing marker-part offsets, or offsets into the serialized full document. `NodeId` may be returned by hit-testing or queries but is not an authoritative component of selection; CST nodes provide source ranges and structure only and do not define a second coordinate.

## Inline editing is source editing

All inline operations — character input, IME composition, Backspace, Delete, selection replacement, paste, drag-drop text, auto-pairing, bold, italic, strikethrough, inline code, links (insert/edit/remove), inline math — produce block-local source edits applied to the current node's `InlineDocument.source`. The flow is:

1. apply one or more `TextEdit`s to the current node's source;
2. update the selection's source offset;
3. re-parse the current node's inline CST only;
4. reconcile stable node ids where ranges are untouched or merely shifted;
5. update the affected layout and rendering.

Inline editing must not re-serialize the whole document, rebuild the full document AST, restore the cursor through a full-document source map, synchronize a legacy selection against the serialized document, or mutate semantic inline nodes directly (insert/erase into node text, split/merge inline vectors, reassign `children`). NodeId reconciliation is an optimization for keeping UI state on interactive links/code/math spans, not a precondition for cursor restoration.

## Block-level structure editing

Structure commands — split, join, insert, delete, set block type, indent/outdent list item, wrap/unwrap block quote, edit callout/footnote, edit table structure — modify the block tree. Enter inside a content node splits the node: cut the source at the current offset, create two nodes, parse each CST, move the caret to offset 0 of the second node. Cross-block Backspace/Delete joins nodes; it must not delete a newline in a serialized full-document string and re-parse. Keep block-text commands and block-structure commands as distinct layers; do not intermix them.

## Unified block tree

All structure nodes — document, block quote, list, list item, task list item, table, table row, table cell, callout, footnote, paragraph, heading, code block, math block, thematic break — share one tree accessed through uniform parent/children/insert/remove/replace/move/walk interfaces. The special side containers and their bespoke recursive entry points (`quote_children`, `list_items`, `task_items`, `table_header`, `table_rows`) are replaced by this single tree. Block-editing algorithms must not have separate hand-written recursions per node kind.

## Selection and history

Selection is container id plus source offset (and affinity) — one selection, no legacy/double-tracking. Undo/Redo records reversible transactions of `TextEdit`s and tree edits plus the selection before/after, and restores block state and selection exactly. It must not depend on re-parsing the full document. Complete snapshots may exist only as a debug/disaster fallback, never as the primary history mechanism for normal edits.

## Responsibilities

```text
Parser:       Markdown -> block tree + per-node InlineDocument (source + lossless CST)
Inline CST:   source -> token/CST (lossless, every character classified; one-node re-parse on edit)
Serializer:   block tree -> Markdown (inline = node.source, verbatim; markers never re-derived)
Editor:       TextEdit on node source -> re-parse one node -> reconcile ids -> render one node
Renderer:     node CST + selection -> visual representation; hit-test returns TextPosition
```

Markdown syntax recognition belongs to the inline CST parser, Markdown formatting belongs to the serializer, and keyboard interaction belongs to block-tree/source editing.

## Module boundaries and architectural discipline

Source editing, CST parsing, selection, and history remain separate, narrowly-scoped modules. `src/core/document_edit.ixx` is a thin facade and must not accumulate feature-specific editing logic. There is one source of truth: do not introduce an old/new dual track, a compatibility path, a feature flag selecting a prior model, or a fallback to legacy behavior. Reintroducing any of these is an architectural regression, not an incremental migration step.

## Invariants and testing

The CST must satisfy, continuously: `flatten_tokens(parse(source)) == source` and `serialize_lossless(parse(source)) == source`, both character-exact, preserving `*`/`_`, `**`/`__`, `[t](url)`/`[t](<url>)`, quote styles, escapes, and entities. Tests must exercise per-character input, deletion, selection, Enter, cross-block merge, format toggles, copy/paste, and undo/redo on representative inputs (`abc`, `*abc*`, `_abc_`, `**abc**`, `__abc__`, `**`, `**abc`, `a***b***c`, `~~abc~~`, `~~abc`, `` `abc` ``, `` `abc ``, `[title](url)`, `[title](<url>)`, `[title](url "name")`, `[title](`, `![alt](url)`, `$abc$`, `$abc`, `\*abc\*`, `&amp;`, `a\**b*`) across paragraph, heading, list/task item, quote, table cell, callout, footnote, and at document start/end/empty/adjacent-empty/nested contexts — checking source, CST flatten, block tree, selection, rendered text, save, reload, and undo/redo each time. Add random property tests asserting flatten == source, selection in range, save/reload losslessness, and exact undo/redo restoration. Normal-edit paths must perform zero full-document parses and zero full-document serializations; Unicode offsets must be one consistent unit across the core/render boundary, with explicit surrogate/combining/emoji conversion at the WinUI/DirectWrite edge.

## Agent workflow

Before changing editing interaction, understand the existing block tree, inline source/CST, document positions, transactions, normalization, selection, history, and serializer design. Treat the model in this document as the established architecture, not as an active migration. If implementation conflicts with these principles, call out the architectural regression and fix the responsible boundary when the task requires it. Do not add compatibility layers or revive legacy full-document/string-driven editing paths.

Keep changes scoped, preserve unrelated user work, and verify with the narrowest relevant tests plus a build when practical. Commit in small, reviewable batches — never one dump of hundreds of files. Do not use destructive Git operations unless the user explicitly authorizes them.
