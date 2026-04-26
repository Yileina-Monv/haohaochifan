# Next Task Prompt

Use the prompt below in the next new Codex window.

```text
The project path is D:\Codex\2026-04-18-qt-c-app.

Before editing, read these files in order:
1. docs/memory.md
2. docs/next-task-prompt.md
3. docs/v3-plan.md
4. docs/llm-supplement-prompt.md
5. docs/recommendation-metrics-table.md
6. docs/product-rules.md
7. docs/handoff.md
8. src/recommendation/recommendationengine.h
9. src/recommendation/recommendationengine.cpp
10. tools/validation/main.cpp

Current status:
- The app is Qt Quick/QML + C++ + SQLite.
- Local rule-based recommendation remains the final authority.
- Initial V3 budget-gate recommendation logic has been implemented inside the
  existing RecommendationEngine.
- Budget is no longer a default weighted scoring group.
- The supplement parser now accepts budgetMode and budgetLimitYuan while still
  accepting the older 13-field response for compatibility.
- Explicit strict/relaxed budget modes apply a fixed -40 over-budget gate
  penalty and keep over-line candidates as fallbacks.
- Acquisition / long-meal cost is handled through scene fit so class-day lunch
  still avoids slow heavy options without relying on price.
- Latest verification:
  1. Desktop build passed:
     cmake --build build\desktop-debug --target MealAdvisor MealAdvisorValidation
  2. MealAdvisorValidation.exe passed 51/51.
  3. 5-second desktop smoke launch had empty stderr.

Recommended next goal:
- Do not rewrite the recommendation engine again unless a concrete regression is
  found.
- The highest-value next step is Android/runtime validation when a device or AVD
  is available:
  - main recommendation send flow
  - V3 budget wording through the supplement parser
  - strict budget vs relaxed budget examples
  - warning/breakdown display for over-budget candidates
  - Drawer LLM connection test
  - placeholder/keyboard behavior and horizontal-scroll regression checks

Optional focused follow-up before Android:
- Add a small UI polish pass only if the budget-gate warning/breakdown copy is
  hard to read in the existing recommendation cards.
- Keep it QML-only unless runtime evidence points to C++ behavior.

Validation commands:
1. Desktop build:
   $env:PATH='C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.10.3\mingw_64\bin;C:\Qt\Tools\Ninja;' + $env:PATH
   & 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build build\desktop-debug --target MealAdvisor MealAdvisorValidation
2. Validation:
   $env:PATH='D:\Codex\2026-04-18-qt-c-app\build\desktop-debug;C:\Qt\6.10.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;' + $env:PATH
   & '.\build\desktop-debug\MealAdvisorValidation.exe'
3. Desktop smoke:
   launch build\desktop-debug\MealAdvisor.exe for 5 seconds and confirm captured stderr is empty.

Boundaries:
- Do not add OCR/image parsing, cloud sync, LLM reranking, full nutrition lookup,
  or direct LLM SQLite writes.
- Keep local recommendation scoring as the final authority.
- Keep write actions behind local managers and explicit user confirmation.
- Do not claim Android runtime validation passed unless an actual device/emulator
  session was run.

Before finishing any future implementation turn:
- Update docs/memory.md.
- Update docs/handoff.md if schema, project direction, or major behavior changes.
- Replace docs/next-task-prompt.md with the next concrete prompt.
- Report validation failures clearly.
```
