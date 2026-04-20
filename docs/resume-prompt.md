# Resume Prompt

Use the following prompt in a new Codex window:

```text
The project path is D:\Codex\2026-04-18-qt-c-app.

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

Requirements:
- Do not re-plan the whole product from scratch. Continue from the current solution.
- Check the current code and docs first. Do not assume the project is empty.
- Reuse the existing Qt/QML, C++, and SQLite structure whenever possible.
- If docs and code disagree, point out the difference first, then continue based
  on the actual code state.
- If code is changed during the task, update docs/memory.md before finishing.
```

## Suggested Examples

Example 1:

```text
The project path is D:\Codex\2026-04-18-qt-c-app.

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
The project path is D:\Codex\2026-04-18-qt-c-app.

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
