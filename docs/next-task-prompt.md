# Next Task Prompt

Use the prompt below in the next new Codex window.

```text
The project path is D:\Codex\2026-04-18-qt-c-app.

Before editing, read these files in order:
1. docs/memory.md
2. docs/next-task-prompt.md
3. docs/llm-natural-language-business-plan.md
4. docs/handoff.md
5. docs/project-plan.md
6. docs/data-model.md
7. docs/product-rules.md

Then inspect the task-specific files:
8. app/qml/Main.qml
9. app/qml/FoodPage.qml
10. app/qml/MealLogPage.qml
11. app/qml/SchedulePage.qml
12. src/recommendation/recommendationengine.h
13. src/recommendation/recommendationengine.cpp
14. src/core/foodmanager.h
15. src/core/foodmanager.cpp
16. src/core/meallogmanager.h
17. src/core/meallogmanager.cpp
18. data/schema.sql
19. tools/validation/main.cpp

Current status:
- The app is Qt Quick/QML + C++ + SQLite.
- Main page is recommendation-first and now has a compact natural-language
  mode switch for `推荐 / 反馈 / 菜品 / 日常`.
- The shared main-page composer uses a local preview shape:
  `intent`, `summary`, `actions`, `requiresConfirmation`, `missingFields`,
  and `state`.
- `推荐` reuses `recommendationEngine.parseSupplement(text)` when text is
  provided, then refreshes deterministic local recommendations with
  `runDecision()`. LLM output does not rerank candidates.
- `反馈` targets the selected/latest recent meal, reuses
  `parseFeedback(text, mealSummary)`, fills local feedback fields, and only
  writes after `确认保存` through `mealLogManager.saveMealFeedback(...)`.
- Drawer feedback was also changed from parse-and-save to parse-preview-confirm.
- `菜品` uses an existing merchant selector, reuses
  `parseDishInput(text, merchantName)`, shows a structured preview, and only
  imports after confirmation through `FoodManager::addDish(...)`.
- `日常` currently provides preview/no-op scaffolding only. No
  `temporary_events` table, parser, manager, or recommendation-context merge
  exists yet.
- FoodPage still has `LLM 菜品输入`; it fills the dish form for review instead
  of saving directly.
- Existing LLM request paths remain OpenAI Chat Completions-compatible:
  `testLlmConnection(...)`, `parseSupplement(...)`, `parseFeedback(...)`, and
  `parseDishInput(...)`.
- Latest local checks:
  - desktop `MealAdvisor` + `MealAdvisorValidation` build passed
  - `MealAdvisorValidation.exe` exits 0 and reports `48/50`
  - 5-second desktop smoke launch produced empty captured stderr
- Latest follow-up fix: the Home recommendation result `ScrollView` now pins
  `contentWidth` to `availableWidth` and disables horizontal scrolling after an
  Android screenshot showed that area could still slide left/right.
- Latest desktop APK:
  `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`
  - SHA256:
    `6031CC8FBD3D330CD08C646730E07CB2435EF97CE8617669CFCC38DD77EACA0D`
  - size: `71644274` bytes
  - timestamp: `2026-04-26 19:25:49`
  - package: `org.qtproject.example.MealAdvisor`
  - minSdkVersion `28`, targetSdkVersion `36`, native-code `arm64-v8a`
- The two validation failures are known non-blocking baseline cases:
  - non-LLM relaxed high-budget dinner does not lift hotpot into top-3
  - sparse seeded feedback does not emit sleepiness-watch / stable-favorites /
    low-repeat insight types
- Android runtime validation is still blocked in this shell when no adb target
  or AVD is available. Do not claim Android runtime validation passed unless an
  actual device/emulator session was run.

New goal:
Run the next confidence pass for the main-page natural-language task layer.

Preferred next work:
1. If an Android device or AVD is available, use the Test Android Apps
   `android-emulator-qa` workflow to install/launch the APK and validate:
   - main-page `推荐 / 反馈 / 菜品 / 日常` mode switching
   - shared composer placeholder and soft-keyboard behavior
   - structured preview card visibility and wrapping
   - Home recommendation result area cannot be dragged left/right and shows no
     horizontal scrollbar
   - `反馈` parse -> preview -> confirm save, plus manual fallback
   - `菜品` merchant selector -> parse -> preview -> confirm import
   - Drawer open/close, section switching, and LLM `测试连接`
   - portrait/landscape scrolling and no horizontal drag regression
2. If Android is still unavailable, keep validation local and implement only a
   small targeted follow-up:
   - either extract the QML-local preview/action logic toward a C++ natural
     language task service, or
   - implement the first temporary routine persistence slice
     (`temporary_events` schema/repository/manager) without recommendation
     integration until validated.

Boundaries:
- Do not add OCR/image parsing, cloud sync, LLM reranking, full nutrition
  lookup, or direct LLM SQLite writes.
- Keep local recommendation scoring as the final authority.
- Keep write actions behind local managers and explicit user confirmation.
- If adding temporary routine persistence, keep it deterministic and validate
  schema/repository behavior before merging events into recommendation context.

Validation commands:
1. Desktop build:
   $env:PATH='C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.10.3\mingw_64\bin;C:\Qt\Tools\Ninja;' + $env:PATH
   & 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\desktop-debug --target MealAdvisor MealAdvisorValidation
2. Validation:
   $env:PATH='D:\Codex\2026-04-18-qt-c-app\build\desktop-debug;C:\Qt\6.10.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;' + $env:PATH
   & '.\build\desktop-debug\MealAdvisorValidation.exe'
3. Desktop smoke:
   launch `build\desktop-debug\MealAdvisor.exe` for 5 seconds and confirm
   captured stderr is empty.
4. If Android packaging is affected or runtime validation is possible:
   & 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\android-arm64-debug --target MealAdvisor_make_apk
   Copy-Item build\android-arm64-debug\android-build\build\outputs\apk\debug\android-build-debug.apk C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk -Force
   & 'C:\Users\Administrator\AppData\Local\Android\Sdk\build-tools\36.0.0\aapt.exe' dump badging build\android-arm64-debug\android-build\build\outputs\apk\debug\android-build-debug.apk

Before finishing:
- Update docs/memory.md.
- Update docs/handoff.md if schema, project direction, or major behavior
  changes.
- Replace docs/next-task-prompt.md with the next concrete prompt.
- Report validation failures clearly, especially if the two known baseline
  failures remain.
```
