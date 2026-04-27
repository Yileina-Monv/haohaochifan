# Next Task Prompt

Use the prompt below in the next new Codex window.

```text
The project path is D:\Codex\2026-04-18-qt-c-app.

Use skill:codex-design and, if a device/emulator is available, use the
test-android-apps Android emulator QA workflow.

Before editing, read these files in order:
1. docs/memory.md
2. docs/handoff.md
3. docs/android-frontend-design-optimization-plan.md
4. docs/chat-style-frontend-refactor-plan.md
5. docs/product-rules.md
6. app/qml/Theme.qml
7. app/qml/StyledButton.qml
8. app/qml/StyledTextField.qml
9. app/qml/StyledTextArea.qml
10. app/qml/StyledComboBox.qml
11. app/qml/SectionCard.qml
12. app/qml/Main.qml
13. app/qml/FoodPage.qml
14. app/qml/MealLogPage.qml
15. app/qml/SchedulePage.qml
16. CMakeLists.txt

Current status:
- The app is Qt Quick/QML + C++ + SQLite.
- It is Android-first; desktop narrow windows are used for local visual
  verification when no Android device or AVD is available.
- Round 1 of docs/android-frontend-design-optimization-plan.md has landed:
  shared Theme.qml tokens, SectionCard, StyledButton, StyledTextField,
  StyledTextArea, StyledComboBox, CMake registration, and shared controls
  across Main / Food / Schedule / Meals.
- Round 2 has landed as a focused QML-only pass:
  - Main header, task preview, current recommendation, recommendation cards,
    Drawer shell, Drawer LLM 调试, and Drawer 饭后反馈 now use Theme tokens and
    SectionCard more consistently.
  - Main's shared shell surfaces no longer carry page-local hard-coded hex
    colors.
  - Food / Schedule / Meals styled text fields and text areas now use the
    shared hintText API instead of direct placeholderText overrides.
  - No C++ logic, parser contract, schema, persistence, or recommendation
    scoring was changed.
- A targeted Android screenshot feedback fix has landed for Drawer
  `反馈与记录` / Meals `餐次信息`:
  - the form now uses labeled controls and tighter mobile margins
  - the old full ISO timestamp field was replaced with date + hour + minute
    controls
  - quick time buttons (`现在` / `午餐` / `晚餐`) fill common meal times
  - saving still composes the original timestamp string before calling the
    existing `mealLogManager.saveMealLog(...)` path
- Latest local validation passed 51/51.
- Latest desktop smoke launch had empty stderr.
- Android arm64 debug APK was rebuilt and copied to
  C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk.
- Latest APK SHA256:
  7A2AF1CADD692360E2464FC966DD40B5B97FF5E2F7B54B403F68A9A6A9150B65
- APK size is 71845058 bytes, timestamp 2026-04-27 17:44:25.
- aapt badging still reports package org.qtproject.example.MealAdvisor,
  minSdkVersion 28, targetSdkVersion 36, launchable Qt activity, launcher
  icons, and native-code arm64-v8a.
- Last known adb / emulator check still had no attached device and no
  configured AVD, so Android install/launch/touch validation is still pending.

Recommended next goal:
1. First, check Android availability:
   - adb devices -l
   - emulator -list-avds
2. If a device or AVD is available, run Android runtime validation before doing
   more polish:
   - install the latest APK
   - launch org.qtproject.example.MealAdvisor
   - capture screenshots with adb
   - validate the main recommendation page at portrait and landscape widths
   - validate the 推荐 / 反馈 / 菜品 / 日常 segmented composer modes
   - validate StyledTextField / StyledTextArea keyboard behavior and placeholder
     hiding after the hintText cleanup
   - validate StyledComboBox popups on Android
   - validate Drawer open/close, Drawer section buttons, LLM 调试, 饭后反馈,
     Schedule, Food, and Meals scrolling
   - specifically retest Drawer 反馈与记录 -> 餐次信息: date/hour/minute controls,
     quick time buttons, and save behavior
   - retest Drawer LLM 调试 -> 测试连接 if a provider key is available
3. If Android is still unavailable, keep the next frontend pass small and
   screenshot-led:
   - inspect Drawer Schedule / Food / Meals at narrow desktop widths
   - reduce only the most obvious remaining page-local color/radius literals in
     embedded management pages
   - do not start another broad redesign before real Android touch evidence
   - keep the pass QML-only unless a concrete runtime bug requires C++

Visual direction:
- Product type: practical Android-first meal decision/logging tool.
- Tone: calm, dense, scannable, and tool-like.
- Green: primary action / success / positive.
- Warm neutral: food context, light hints, page background.
- Blue-gray: records / comparison / system information.
- Red: errors / risk / warnings.
- Avoid one-note beige/brown palettes, large rounded marketing cards, and mixed
  custom/default controls.
- Target sizes:
  - large card radius: 16
  - small card radius: 12
  - button/input radius: 10-12
  - page title: 24-26
  - section title: 19-21
  - body text: 15
  - label text: 13
  - mobile touch target: at least 44 where practical

Validation commands:
1. Desktop build:
   & 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\desktop-debug --target MealAdvisor MealAdvisorValidation
2. Validation:
   & '.\build\desktop-debug\MealAdvisorValidation.exe'
3. Android APK build:
   & 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\android-arm64-debug --target MealAdvisor_make_apk --config Debug
4. Android availability check:
   & "$env:LOCALAPPDATA\Android\Sdk\platform-tools\adb.exe" devices -l
   & "$env:LOCALAPPDATA\Android\Sdk\emulator\emulator.exe" -list-avds

Boundaries:
- Do not rewrite the recommendation engine for frontend polish.
- Do not add OCR/image parsing, cloud sync, LLM reranking, full nutrition lookup,
  or direct LLM SQLite writes.
- Keep local recommendation scoring as the final authority.
- Keep write actions behind local managers and explicit user confirmation.
- Do not claim Android runtime validation passed without a real device/emulator
  session.
- Use UTF-8-safe tooling for bulk QML rewrites; PowerShell console output can
  misdisplay Chinese, and Windows PowerShell Set-Content without explicit UTF-8
  can corrupt QML Chinese strings.

Before finishing:
- Update docs/memory.md with what changed and validation results.
- Update docs/handoff.md only if project direction, architecture, or major UI
  behavior changes.
- Replace docs/next-task-prompt.md with the next concrete prompt.
- Report any Android validation gap clearly.
```
