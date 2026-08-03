# Table interaction contract

Folia treats a Markdown table as a rectangular structure whose cells own
ordinary block-local Markdown source. This document defines which gestures
edit text and which gestures edit table structure so the two cannot be
silently confused.

## Selection model

There is one authoritative `TextSelection`: anchor and active
`TextPosition`s, each expressed as an editable container id plus a source
offset and affinity. Tables do not introduce cell-selection coordinates or a
parallel selection state.

- Dragging through cell content creates a row-major text range.
- The first and last cells are selected only over their endpoint source
  ranges. Cells strictly between them are selected in full.
- Reversing anchor and active produces the same selected content.
- Visual highlighting must use those exact ranges; it must not paint an
  entire cell merely because the range intersects it.
- Copying a same-cell range returns that exact Markdown source substring.
- Copying a cross-cell range returns cell source separated by tabs within a
  row and newlines between rows. It does not silently copy the whole table.

This linear rule deliberately handles ranges such as the final cell of one
row through the first cell of the next. Such a range is not forced into an
artificial rectangle.

## Text deletion

Backspace, Delete, Cut, and selection replacement are text operations.

- Within one cell they edit only that cell's source.
- Across cells in one table they erase the selected source slice in each cell
  and preserve every row and cell node.
- Empty cells remain valid cells. Text deletion never removes a row, column,
  or table.
- Undo/Redo restores all edited cell sources and the selection as one
  transaction.

Selections that cross a table boundary must treat the table as a structure
barrier; implementations must not remove individual `TableCell` nodes or
leave rows with unequal column counts. Structural deletion requires an
explicit table command.

## Structural controls

Insertion, deletion, and reordering of rows or columns are structural
operations. They are invoked only by visible table controls or an explicit
command. A table command has a valid target only when both text-selection
endpoints resolve to the same cell. Pointer controls collapse the selection
to their source cell before issuing the command.

Consequences:

- A cross-cell text selection is never interpreted from its active endpoint
  as "delete this row" or "move this column".
- Row and column drags are mutually exclusive with text-selection drags.
- Dropping is valid only inside the source table.
- Delete removes content; a row/column delete control removes structure.

## Keyboard behavior

- `Tab` moves to the next cell.
- `Tab` in the final cell appends one body row and moves to its first cell.
- `Shift+Tab` moves to the previous cell; in the first cell it does nothing.
- Table navigation that does not change structure creates no history entry.
  Appending the final row is a reversible structure transaction.
- Enter and soft-break behavior remains a cell-local Markdown edit; it does
  not navigate or reshape the table.

## Markdown capability boundary

Markdown tables remain rectangular and do not support merged cells,
`rowspan`, or `colspan`. Safe HTML tables may render those HTML features, but
they are not converted into Markdown-table structure commands. Their source
is edited as HTML/source content according to the existing HTML rules.

The contract follows familiar desktop table conventions where they fit
Markdown, while keeping Folia's source-backed block tree authoritative.
