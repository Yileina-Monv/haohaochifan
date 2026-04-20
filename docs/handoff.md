# Handoff

## Project

- Name: `MealAdvisor`
- Root path: `D:\Codex\2026-04-18-qt-c-app`
- Stack: `Qt Quick/QML + C++ + SQLite`
- Target: Android-first, desktop build used for local verification

## Current Status

The project has moved beyond a static scaffold and now includes:

- Qt/CMake project setup
- SQLite initialization
- schema creation and lightweight migrations
- seeded class periods
- seeded planning policy and recommendation profiles
- basic repositories for planning, schedule, merchants, dishes, and meal logs
- `AppState` for basic live app summary
- `ScheduleManager` plus a working schedule page
- `FoodManager` plus a working merchant/dish management page
- `MealLogManager` plus a working meal logging page with multi-dish support
- an upgraded V2 recommendation engine on the home page
- natural-language supplement parsing that can turn free text into temporary
  scoring weights through an OpenAI-compatible API
- recommendation output persistence into `recommendation_records`
- a first persisted feedback loop through `meal_feedback`
- Meals-page feedback insights that now support drill-down into supporting
  meals and dishes plus readable manual weight-adjustment suggestions
- Meals-page feedback drill-down has now been polished for better day-to-day
  use:
  representative supporting meals and dishes are prioritized first, copy is
  more Chinese-readable, and linked meals can show a lightweight historical
  recommendation snapshot inline
- Meals-page feedback drill-down now also has a quick-scan layer:
  each insight can surface a short priority headline, concise next-step text,
  and evidence-count summary before the full detail body
- Lightweight recommendation-history inspect is now easier to skim:
  linked meals can show a short context headline, whether the actual meal was
  top-1 or a lower-ranked candidate, and compact selected-vs-top1 reason text
- Meals-page feedback detail now also exposes ordered scan steps:
  the detail panel can tell the user which action / dish sample / meal sample
  to inspect first before reading the longer paragraph
- Lightweight recommendation-history inspect now supports a stronger inline
  compare path:
  linked meals can show score-gap summary, a short compare guide, and compact
  top-candidate vs actual-choice score snippets without changing schema
- Meals-page feedback detail now also highlights the first scan step before the
  rest of the list:
  the detail panel can surface a dedicated first-look block and push the longer
  explanation text into a later `补充说明` section
- Supporting meals / dishes now use stronger tie-breakers under equal
  priority:
  compare richness, score-gap availability, representative feedback density,
  and top supporting-meal quality now help keep real-data ordering more stable
- Lightweight recommendation-history inspect now also exposes a clearer first
  compare anchor:
  `comparePriorityHeadline`, numeric score-gap value, and candidate badge /
  score-gap tags are now available inline without changing schema
- A repo-wide UTF-8 scan has now separated terminal-side mojibake from real
  file corruption:
  the main QML pages, `RecommendationEngine`, and
  `RecommendationRecordRepository` are file-level healthy on the currently
  scanned path, and the last clearly user-visible damaged block was the
  Meals-page insight helper copy in `MealLogManager`, which has now been
  repaired

Desktop build is compiling successfully.

## Pages Implemented

- `Home`
- `Schedule`
- `Food`
- `Meals`

## Primary Memory File

- `docs/memory.md`

This file must be updated every time code is changed. It is the first document
to read for a new window after the core product docs.

## Important Current Files

- `README.md`
- `docs/project-plan.md`
- `docs/data-model.md`
- `docs/product-rules.md`
- `docs/memory.md`
- `data/schema.sql`
- `src/data/databasemanager.*`
- `src/core/appstate.*`
- `src/core/schedulemanager.*`
- `src/core/foodmanager.*`
- `src/core/meallogmanager.*`
- `src/recommendation/recommendationengine.*`
- `src/data/recommendationrecordrepository.*`
- `app/qml/Main.qml`
- `app/qml/SchedulePage.qml`
- `app/qml/FoodPage.qml`
- `app/qml/MealLogPage.qml`

## What Has Been Confirmed

- Planning days are Tuesday to Friday
- Commute days are included
- Saturday to Monday are not actively planned
- Breakfast is logged but not actively recommended by default
- Merchant management should track price, dine-in/takeaway/delivery, delivery ETA, distance, queue time, and availability notes
- Dish nutrition indicators should use `low / medium / high`
- One dish does not belong to multiple merchants
- Combo meals should be split when possible
- Drinks and snacks should be separate dishes with reduced impact weight when needed
- GI is folded into carb judgment instead of being stored separately
- Flavor and odor are explicit labels, not only free-text notes

## Technical Notes

- The app database is created through bundled `schema.sql`
- Migrations are currently lightweight `ALTER TABLE ADD COLUMN` checks in `DatabaseManager`
- Existing local DBs should survive schema growth better than before
- The desktop build needs MinGW in `PATH` during manual command-line builds
- The current recommendation trigger is manual from the home page
- Recommendation V2 now uses:
  - structured group/sub-metric weights
  - longer non-breakfast multi-meal compensation
  - post-meal sleep-plan modifiers
  - Chinese-ready reasons and warnings
- Recommendation runs are now persisted into `recommendation_records`
- Meal feedback is now persisted and minimally editable from the Meals page
- Dish-level historical feedback now feeds back into recommendation scoring
- Supplement parsing uses `Qt6::Network` and an OpenAI-compatible chat
  completions API
- API config is currently environment-variable-based:
  `MEALADVISOR_LLM_API_KEY`, `MEALADVISOR_LLM_API_URL`,
  `MEALADVISOR_LLM_MODEL`, with `OPENAI_API_KEY` fallback
- Android CMake configure has been verified previously with the installed Qt Android kit

## Known Issues / Follow-ups

1. PowerShell console output in this environment can misdisplay UTF-8 Chinese as mojibake; confirm suspected encoding damage from file contents or runtime UI, not terminal echoes alone.
2. Schedule default import currently includes a placeholder note for the Wednesday late-class room because the screenshot did not fully show it.
3. `docs/recommendation-metrics-table.md` is currently readable in UTF-8 and can be used as a literal reference again; do not assume it is corrupted unless a future scan finds a real damaged segment.
4. Supplement parsing has no in-app settings UI yet and depends on external API configuration.
5. Recommendation persistence currently links `selected_dish_id` heuristically when one meal contains multiple dishes.
6. Feedback drill-down and readable weight suggestions now exist on the Meals
   page and are much more usable than before; the main Meals-page Chinese copy
   is now cleaner and the first-look path is more explicit, but sample
   ordering is still a heuristic and should be validated against more real
   usage.
7. Recommendation-history inspect is now more readable inline, but there is
    still no dedicated deeper history compare view if later debugging needs go
    beyond the current card-level summary.
8. The latest round repaired the clearly user-visible damaged Meals-page helper
   copy in `src/core/meallogmanager.cpp`, but older non-Meals helper branches
   and unscanned runtime paths may still need targeted verification.

## Recommended Next Steps

1. Continue targeted encoding verification on runtime-visible dynamic strings
   outside the newly repaired Meals helper block:
   - prioritise any older manager/repository helper copy that still reaches
     `Home`, `Food`, `Schedule`, or `Meals`
   - validate runtime UI before widening the scope of any further encoding fix
2. Polish the new Meals-page drill-down detail:
   - validate whether the new first-scan-step highlight plus reordered
     `补充说明` block actually reduce detail-reading load on real feedback data
   - keep validating the newer representative-sample tie-breakers for meals and
     dishes against more real usage
   - continue tightening small copy details only if usage exposes ambiguity,
     while staying inside the existing Meals page
   - decide whether recommendation-history inspection now needs only one more
     small inline compare pass or a deeper view later
3. Add a frontend-adjustable weight UI and settings UI for API configuration
4. Decide whether supplement inputs should also be stored

## If Starting In A New Window

Use the prompt in `docs/resume-prompt.md`.

Then ask the model to:

1. Read `README.md`
2. Read `docs/project-plan.md`
3. Read `docs/data-model.md`
4. Read `docs/product-rules.md`
5. Read `docs/memory.md`
6. Read `docs/handoff.md`
7. Continue with the next requested task without re-planning from scratch
