# Next Task Prompt

Use the prompt below in the next new Codex window.

```text
The project path is D:\Codex\2026-04-18-qt-c-app.

Please read these files first to build context:
1. README.md
2. docs/project-plan.md
3. docs/data-model.md
4. docs/product-rules.md
5. docs/memory.md
6. docs/handoff.md
7. docs/resume-prompt.md

Then read these task-specific files before coding:
8. CMakeLists.txt
9. app/qml/Main.qml
10. app/qml/FoodPage.qml
11. app/qml/MealLogPage.qml
12. app/qml/SchedulePage.qml
13. src/core/foodmanager.cpp
14. src/core/meallogmanager.cpp
15. src/core/schedulemanager.cpp
16. tools/validation/main.cpp

Current handoff assumption:
Stage 1-6 are sealed enough for Stage 7 work. Stage 7 desktop preflight has now
had five small passes:
- First pass: repaired obvious Home / Food static QML copy, localized major
  Schedule UI copy, and made several dense Home / Food / Schedule action rows
  wrap on phone-width screens.
- Second pass: localized higher-risk runtime text from AppState,
  ScheduleManager, FoodManager, MealLogManager, RecommendationEngine, parser
  validation details, and default planning/schedule seeds; updated validation
  assertions to match Chinese guardrail copy; made two dense Meals quick-action
  rows wrap.
- Third pass: fixed the QML card-height collapse/overlap issue across Home /
  Schedule / Food / Meals, added a local readable-button wrapper, repaired
  remaining Home copy, wrapped the Home LLM status/action row, and changed Food
  / Schedule enum selectors to show Chinese labels while preserving stored enum
  values.
- Fourth pass: tightened lower-page narrow-width behavior in Food / Schedule /
  Meals by collapsing lower forms at narrow widths, stacking long lower-card
  title/action rows, wrapping dense metadata labels, making the Meals feedback
  score editor clearer, and tightening the desktop minimum width/top-tab setup.
- Fifth/final desktop pass: fixed the must-fix search/save usability gaps found
  in the final review. Food dish `清空筛选` now clears both search text and
  merchant filter; Meals dish picker has an explicit `清空搜索` action; Meals
  save can be clicked with no selected dishes so the existing C++ guardrail
  message is reachable.
- No schema, recommendation scoring, Stage 6 parser capability, LLM rerank,
  OCR, dish enrichment, feedback parser, Android, or packaging changes were
  made in the fifth pass.
- Desktop build passed, real desktop 5-second smoke launch passed, and
  MealAdvisorValidation remains 36/38.

The two remaining validation failures are known non-blockers:
- non-LLM relaxed high-budget dinner does not lift hotpot into top-3
- sparse seeded feedback does not emit sleepiness-watch / stable-favorites /
  low-repeat insight types

Important validation boundary:
The fifth desktop pass used real desktop process startup, static/runtime-binding
review of the requested click paths, and the existing local mock/offline
MealAdvisorValidation target. Direct GUI click automation for the native Qt
window was not available in that shell. Treat Android on-device/touch validation
as the next real interaction confidence step.

Then continue with this concrete task:
Continue Stage 7 by running the Android packaging and first Android runtime
validation pass.

Primary goal:
Produce and verify an Android arm64 debug APK using the already aligned Qt
6.10.3 / Android SDK / NDK / JDK environment, then run the app on a real device
or emulator if available. Prioritize packaging/runtime blockers and touch/keyboard
usability over polish.

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
2. Re-run the desktop fast regression first:
   `$env:PATH='C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.10.3\mingw_64\bin;C:\Qt\Tools\Ninja;' + $env:PATH`
   `& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\desktop-debug --target MealAdvisor`
   `& '.\build\desktop-debug\MealAdvisorValidation.exe'`
3. Inspect the existing Android build directory and CMake cache before changing
   build files.
4. Build the Android arm64 debug APK using the current Qt Android kit/build
   setup. Prefer the existing `build\android-arm64-debug` configuration if it
   is still valid.
5. If the APK build fails, fix only packaging/build/runtime blockers. Avoid
   unrelated UI polish.
6. If `adb` can see a device or emulator, install and launch the APK. Check:
   - app opens without crashing
   - top tabs are reachable
   - Home generate recommendation and LLM config dialog open/close
   - Schedule reset/add validation/edit/cancel
   - Food merchant/dish add/edit/cancel, search clear, filter clear
   - Meals dish search/clear, add/remove selected dish, save empty guardrail,
     feedback save/update, and scroll through lower cards
7. If no device/emulator is available, state that clearly and keep verification
   to APK build plus desktop/local validation.
8. Do not run live external LLM provider tests unless a key is already available
   and the task specifically needs it.
9. If code or docs change, update `docs/memory.md`.
10. If handoff assumptions change, update `docs/handoff.md`.
11. Before finishing, replace this file with the next concrete prompt.

At the end, state clearly:
1. Whether the Android APK was built, and its path if successful.
2. Whether install/launch was run on a real device or emulator.
3. Which Android runtime paths were checked.
4. Which issues were must-fix and fixed.
5. Which validation paths were real runtime, mock/offline, static, or skipped.
6. Whether the two known desktop validation failures changed.
7. The recommended next Stage 7 action.
```
