# Next Task Prompt

Use the prompt below in the next new Codex window.

```text
The project path is D:\Codex\haohaochifan.

Please read these files first to build context:
1. README.md
2. docs/project-plan.md
3. docs/data-model.md
4. docs/product-rules.md
5. docs/memory.md
6. docs/handoff.md
7. docs/resume-prompt.md

Then read these task-specific files before coding:
8. app/qml/Main.qml
9. app/qml/FoodPage.qml
10. app/qml/MealLogPage.qml
11. app/qml/SchedulePage.qml
12. src/recommendation/recommendationengine.h
13. src/recommendation/recommendationengine.cpp
14. tools/validation/main.cpp

Current handoff assumption:
Stage 1-6 are sealed enough for Stage 7 work. The first Stage 7 preflight pass
has started and made only minimal frontend changes:
- Home / Food visible static QML mojibake was repaired.
- Schedule's main English UI copy was localized to Chinese.
- A few dense Home / Food / Schedule action rows now wrap on phone-width
  screens.
- No schema, recommendation-core, Stage 6 parser, or LLM capability changes
  were made.
- Desktop build passed, short offscreen desktop smoke launch passed, and
  MealAdvisorValidation remains 36/38.

The two remaining validation failures are known non-blockers:
- non-LLM relaxed high-budget dinner does not lift hotpot into top-3
- sparse seeded feedback does not emit sleepiness-watch / stable-favorites /
  low-repeat insight types

Then continue with this concrete task:
Continue Stage 7 preflight, second small frontend pass only.

Primary goal:
Inspect the remaining high-risk runtime frontend surfaces and fix only
must-fix usability or visible-copy issues. Prioritize real user-visible
problems over polish.

Scope limits:
- Do not expand Stage 6.
- Do not add LLM sub-capabilities.
- Do not add dish enrichment, OCR, feedback parser, LLM rerank, or a
  recommendation-core rewrite.
- Do not change schema direction unless a tiny migration is strictly required
  for a discovered blocker.
- Preserve existing uncommitted work.
- Do not do a broad visual redesign.

Suggested order:
1. Confirm current git status and preserve existing uncommitted work.
2. Re-scan runtime-visible copy for Home / Meals / Food / Schedule, including
   dynamic text coming from C++ managers or seeded data.
3. Focus on Meals long-card density and Home dynamic recommendation output;
   Food / Schedule only need follow-up if form wrapping or copy is still
   obviously bad.
4. If you make QML changes, run:
   `cmake --build build/desktop-debug-6103 --target MealAdvisor`
5. Run a short desktop smoke launch.
6. Run:
   `build/desktop-debug-6103/MealAdvisorValidation.exe`
7. Do not run Android APK packaging unless shared build / packaging / Android
   logic changes. If needed, use:
   `cmake --build build/android-arm64-debug-6103 --target MealAdvisor_make_apk`
8. If code or docs change, update `docs/memory.md`.
9. If handoff assumptions change, update `docs/handoff.md`.
10. Before finishing, replace this file with the next concrete prompt.

At the end, state clearly:
1. Which pages were checked.
2. Which issues were must-fix and fixed.
3. Which issues are safe follow-up polish.
4. Which validation paths were real runtime, mock/offline, or static.
5. Whether Android APK was run; if not, why.
6. The recommended next Stage 7 action.
```
