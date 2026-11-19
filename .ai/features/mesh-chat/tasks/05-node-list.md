# Task 05: node-list

## Objective

Tab on the Mesh screen opens the Node list: the nodes heard, newest first, with names, signal strength and time since last heard.

## Context

**Satisfies**: REQ-13

Tells the user who is around and whether they are in coverage before writing. Nodes are already tracked by Task 01 for short names.

## Depends on

- Task 01

## Expected Behavior

- Tab on the Mesh screen opens the Node list; `;` and `.` scroll it; `` ` `` goes back to the Mesh screen.
- Each node shows long name, short name, last signal strength and the time since it was last heard, the most recent first.
- Any packet from a node, text or not, moves it to the top and updates its signal and time.

## Acceptance Criteria

- [ ] Tab opens the Node list, `` ` `` goes back to the Mesh screen (REQ-13)
- [ ] Each row shows long and short name, signal and time since last heard (REQ-13)
- [ ] Nodes are ordered by last heard, newest first, updated as packets arrive (REQ-13)

## Edge Cases

- No node heard yet → "No nodes heard yet" (REQ-13)
- Node heard but not yet announced → its ID-based short name, long name empty (REQ-13)
- iPhone link lost while on the Node list → it stays on top, as the Mesh screen (REQ-5)
