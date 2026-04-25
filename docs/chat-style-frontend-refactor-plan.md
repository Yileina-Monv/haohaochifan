# Chat-Style Frontend Refactor Plan

## Summary

Refactor the app from `TabBar + SwipeView` into an LLM-chat-style daily-use
surface. The main page contains mode buttons, a chat/result area, a bottom input
box, and a send button. The right-side expand button opens a management drawer
for LLM debugging, schedule config, food config, and feedback/meal records.

Updated visual direction:

- Right drawer uses a half-width layout when space allows, but keeps a practical
  minimum width so forms remain usable.
- Main page uses a light frosted-glass style: translucent panels, subtle
  blur/tint, readable text, and restrained shadows.

## Key Changes

- Rework `app/qml/Main.qml`:
  - remove visible top `TabBar`
  - add top toolbar with app title/status and right-side expand button
  - add mode switch: `需求推荐`, `饭后反馈`
  - add central chat-like content area for current result, recommendation
    cards, parser status, and feedback state
  - add bottom composer with multiline input and send button
  - apply light frosted-glass styling to main content panels and composer
- Drawer behavior:
  - open from the right
  - width rule: prefer about half the window width, but keep a usable minimum
    around `360px` and avoid unusable narrow forms on small screens
  - use section switcher inside drawer:
    - `LLM 调试`
    - `课表配置`
    - `餐食配置`
    - `反馈与记录`
  - embed existing `SchedulePage`, `FoodPage`, and `MealLogPage` rather than
    rewriting their internals in this pass
- `需求推荐` mode:
  - input text defaults to LLM supplement parsing
  - send parses user demand with `recommendationEngine.parseSupplement(text)`
    when configured
  - then runs or exposes the existing local recommendation decision path
  - display top 3 candidates as assistant-style result cards with reasons,
    warnings, and score details
- `饭后反馈` mode:
  - select a recent meal, defaulting to the latest available meal
  - input text becomes feedback free text
  - structured fields reuse existing feedback ratings and
    `mealLogManager.saveMealFeedback(...)`
  - no new LLM feedback parser in this pass

## Public Interfaces / Types

- No schema change.
- No recommendation-core rewrite.
- No new feedback-parser contract.
- Prefer QML-only changes; add a small C++ helper only if needed for reliable
  parser-then-recommend sequencing.
- Existing exposed objects remain source of truth:
  `recommendationEngine`, `mealLogManager`, `scheduleManager`, `foodManager`,
  `appConfig`, and `appState`.

## Test Plan

- Desktop build:
  `cmake --build build\desktop-debug --target MealAdvisor`
- Existing validation:
  `.\build\desktop-debug\MealAdvisorValidation.exe`
- Smoke checks:
  - app opens without QML errors
  - no old top tabs visible
  - drawer opens/closes from right
  - drawer width follows the minimum-width-first half-width rule
  - frosted panels remain readable
  - Schedule/Food/Meals still function inside drawer
  - recommendation input sends through existing parser/recommendation path
  - feedback mode saves structured feedback for a selected recent meal
- Narrow-width checks:
  - composer stays reachable
  - drawer does not make forms unusably narrow
  - main glass overlay does not obscure text or controls

## Assumptions

- "半宽" means half-width when feasible, with minimum usable width taking
  priority.
- Frosted glass should be light and readable, not dark or heavily blurred.
- Existing management pages can be embedded with minimal adaptation.
- After implementation, update `docs/memory.md`, `docs/handoff.md` if handoff
  assumptions change, and `docs/next-task-prompt.md`.
