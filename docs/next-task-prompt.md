# Next Task Prompt

Use the prompt below in the next new Codex window.

```text
The project path is D:\Codex\2026-04-18-qt-c-app.

Before editing, read these files in order:
1. docs/memory.md
2. docs/next-task-prompt.md
3. docs/handoff.md
4. docs/v3-plan.md
5. docs/llm-supplement-prompt.md
6. docs/recommendation-metrics-table.md
7. docs/product-rules.md
8. src/recommendation/recommendationengine.h
9. src/recommendation/recommendationengine.cpp
10. tools/validation/main.cpp

Current status:
- The app is Qt Quick/QML + C++ + SQLite.
- Local rule-based recommendation remains the final authority.
- Initial V3 budget-gate recommendation logic is implemented inside the
  existing RecommendationEngine.
- Budget is no longer a default weighted scoring group.
- The supplement parser accepts budgetMode and budgetLimitYuan while still
  accepting older 13-field responses for compatibility.
- Explicit strict/relaxed budget modes apply a fixed -40 over-budget gate
  penalty and keep over-line candidates as fallbacks.
- The three parser system prompts in RecommendationEngine now include food and
  scenario guardrails:
  - supplement parsing distinguishes refined/staple carb avoidance from
    protein/beef preference and avoids inventing stay-awake pressure from dish
    names alone
  - feedback parsing records the user's actual post-meal experience instead of
    inferring sleepiness from macros or dish names
  - dish input parsing treats dish fields as static attributes, separates
    sleepinessRiskLevel from carbLevel, handles beef hotpot as high-protein /
    low-carb unless carb add-ons are present, and distinguishes refined carbs
    from high-fiber carbs
- docs/llm-supplement-prompt.md has the matching supplement guardrails.

Latest verification:
1. Desktop build passed:
   cmake --build build\desktop-debug --target MealAdvisor MealAdvisorValidation
2. MealAdvisorValidation.exe passed 51/51.
3. 5-second desktop smoke launch had empty stderr.

Recommended next goal:
- Do not rewrite the recommendation engine unless a concrete regression is
  found.
- The highest-value next step is Android/runtime validation when a device or AVD
  is available:
  - main recommendation send flow
  - V3 budget wording through the supplement parser
  - strict budget vs relaxed budget examples
  - dish input examples such as "牛肉火锅单人套餐，不加饭不加面" and
    "全麦/粗粮主食"
  - feedback examples such as "吃完不困但很撑" and "没睡好所以饭后犯困"
  - warning/breakdown display for over-budget candidates
  - Drawer LLM connection test
  - placeholder/keyboard behavior and horizontal-scroll regression checks

Optional focused follow-up before Android:
- Add local parser prompt request-shape assertions only if future prompt edits
  become risky; current validation already proves the request path and strict
  JSON contracts still work.
- Keep any UI polish QML-only unless runtime evidence points to C++ behavior.

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
