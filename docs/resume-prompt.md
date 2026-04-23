# Resume Prompt

Use the generic template below in a new Codex window.

Prefer `docs/next-task-prompt.md` for the immediate next handoff task.
Keep this file as the stable reusable template, and keep
`docs/next-task-prompt.md` as the concrete next-window prompt that should be
updated at the end of every completed task.

## Fixed Template

```text
The project path is D:\Codex\haohaochifan.

Please read these files first to build context:
1. README.md
2. docs/project-plan.md
3. docs/data-model.md
4. docs/product-rules.md
5. docs/memory.md
6. docs/handoff.md

After reading them, summarize the current project status, confirmed business
rules, completed features, and unfinished features in 5-10 high-signal bullet
points.

Then continue with this concrete task:
[replace this with the current task]

Optional task-specific files to read before coding:
[replace this with any extra files for the current task, or delete this block]

Requirements:
- Do not re-plan the whole product from scratch. Continue from the current solution.
- Check the current code and docs first. Do not assume the project is empty.
- Reuse the existing Qt/QML, C++, and SQLite structure whenever possible.
- If docs and code disagree, point out the difference first, then continue based
  on the actual code state.
- Preserve existing uncommitted work and avoid reverting unrelated changes.
- If code or docs are changed during the task, update docs/memory.md before
  finishing.
- If the task changes project direction, priorities, or handoff assumptions,
  update docs/handoff.md before finishing.
- Before finishing, write the next concrete new-window handoff prompt into
  docs/next-task-prompt.md so the following task can continue from updated
  memory without rebuilding context manually.
```

## Suggested Examples

Example 1:

```text
The project path is D:\Codex\haohaochifan.

Please read these files first to build context:
1. README.md
2. docs/project-plan.md
3. docs/data-model.md
4. docs/product-rules.md
5. docs/memory.md
6. docs/handoff.md

After reading them, summarize the current state and then continue wiring
recommendation_records persistence into the app.
```

Example 2:

```text
The project path is D:\Codex\haohaochifan.

Please read these files first to build context:
1. README.md
2. docs/project-plan.md
3. docs/data-model.md
4. docs/product-rules.md
5. docs/memory.md
6. docs/handoff.md

After reading them, summarize the current state and then continue building the
API settings page for the supplement parser.
```
