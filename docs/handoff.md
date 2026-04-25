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
- the Stage 6 supplement parser now follows the docs-backed
  `supplement_parser_v1` contract with strict JSON validation and neutral
  fallback on invalid output
- the Home page now has a minimum in-app LLM config path for API key / URL /
  model while still preserving env var fallback
- supplement parsing UI state is now explicit enough to distinguish
  unconfigured, parsing, success, invalid response, network failure, and
  fallback-to-default
- the Stage 6 request path now explicitly sends
  `response_format = { "type": "json_object" }`
- Stage 6 has now been live-verified once against DeepSeek with real Chinese
  supplement samples, and the validation tooling can now replay raw provider
  responses through the same local C++ contract checker
- Stage 6 final local mapping is now closed enough for Stage 7 preflight:
  `budgetFlexIntent = 1.1` has a verified downstream effect for relaxed
  high-budget dinner through the real parser path, while cola/drink intent is
  locally mapped but still depends on actual beverage candidates in dish data
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
- Stage 5 management-completion UX has now moved forward on the existing pages
  instead of staying as a future cleanup:
  Meals now has recent-meal quick reuse/template actions plus faster dish-add
  behavior, and Food dish maintenance now has a merchant filter plus broader
  keyword matching
- Meals quick logging now has one more consistency fix on top of that Stage 5
  pass:
  the dish-only template-reuse path now carries dining mode in the same spirit
  as manual first-dish add, falling back to the source meal's dining mode when
  the template dish does not expose one
- One more pre-real-data high-yield polish pass has now landed without
  widening scope:
  Meals dish search ranks by keyword closeness plus recent-use count, meal
  save now rejects invalid timestamps and contradictory class-after-meal
  state, Food manager validation is stricter for enum / numeric guardrails,
  Home / Food copy is more Chinese-consistent, and recommendation tie cases
  now use deterministic ordering instead of container order
- A repo-wide UTF-8 scan has now separated terminal-side mojibake from real
  file corruption:
  the main QML pages, `RecommendationEngine`, and
  `RecommendationRecordRepository` are file-level healthy on the currently
  scanned path, and the last clearly user-visible damaged block was the
  Meals-page insight helper copy in `MealLogManager`, which has now been
  repaired

Desktop build is compiling successfully.
The local environment is now aligned to Qt `6.10.3` plus Android SDK / NDK /
JDK tooling, and Android CMake configure has also been re-verified on this
machine.
The Stage 1-6 closeout pass on `2026-04-23` found no application-code blocker
for entering Stage 7 preflight. The latest required verification was:
desktop build passed, short desktop smoke launch passed, and
`MealAdvisorValidation.exe` remained at `36/38` with only the two known
non-blocking failures.
Stage 7 preflight has now started with a first minimal frontend repair pass:
Home / Food visible mojibake was cleaned, Schedule's main English copy was
localized, and several dense action rows now wrap on phone-width screens. This
did not change schema, recommendation scoring, Stage 6 parser behavior, or LLM
scope. Desktop build, short offscreen smoke launch, and
`MealAdvisorValidation.exe` were re-run; validation remains `36/38` with the
same two known non-blocking failures.
Stage 7 preflight second pass has now repaired the highest-risk runtime-visible
copy from Home/AppState, ScheduleManager, FoodManager, MealLogManager,
RecommendationEngine, parser validation details, and default planning/schedule
seeds. Two dense Meals quick-action rows now wrap as label plus action flow.
Validation remains `36/38`; the remaining failures are still the same two
known non-blocking recommendation/feedback heuristic cases.
Stage 7 preflight third pass found and fixed a real runtime layout blocker:
Home / Schedule / Food / Meals card containers could collapse and overlap at
phone-like desktop sizes. The main QML pages now use content-height card
wrappers, normal action buttons have readable text under the desktop style, and
Food / Schedule enum selectors display Chinese labels while preserving the
original stored enum values. Desktop build, short desktop smoke launch, and
`MealAdvisorValidation.exe` were re-run; validation remains `36/38` with the
same two known non-blocking failures.
Stage 7 preflight fourth pass stayed frontend-only and focused lower down the
scroll pages: Food / Schedule / Meals now collapse more lower forms to one
column at narrow widths, long lower-card title/action rows stack instead of
squeezing, Meals feedback score controls use clearer label/value pairs, and
the top tabs plus desktop minimum width were tightened for mobile-like smoke
checks. Desktop build, short desktop smoke/screenshot launch, and
`MealAdvisorValidation.exe` were re-run; validation remains `36/38` with the
same two known non-blocking failures.
Stage 7 final desktop preflight pass found and fixed three small must-fix
usability gaps before Android packaging: Food dish `清空筛选` now clears both
search text and merchant filter, Meals dish search has an explicit
`清空搜索` action when the picker is empty/filtered, and Meals save can be
clicked with no selected dishes so the existing C++ guardrail message is
reachable. Desktop build, a real 5-second desktop process smoke launch, and
`MealAdvisorValidation.exe` were re-run; validation remains `36/38` with the
same two known non-blocking failures. Direct native-window GUI click automation
was not available in this shell, so the next confidence step is Android APK
packaging plus on-device or emulator touch validation.

## Pages Implemented

- `Home`
- `Schedule`
- `Food`
- `Meals`

## Primary Memory File

- `docs/memory.md`
- `docs/next-task-prompt.md`

This file must be updated every time code is changed. It is the first document
to read for a new window after the core product docs.
`docs/next-task-prompt.md` should also be refreshed at the end of each task so
the next window can continue with a concrete prompt instead of rebuilding the
next step from scratch.

## Important Current Files

- `README.md`
- `docs/project-plan.md`
- `docs/data-model.md`
- `docs/product-rules.md`
- `docs/memory.md`
- `docs/handoff.md`
- `docs/resume-prompt.md`
- `docs/next-task-prompt.md`
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
- The local machine is now aligned to:
  Qt `6.10.3`, Android SDK command-line tools, `platform-tools`,
  `build-tools;36.0.0`, `platforms;android-35`, `platforms;android-36`,
  NDK `27.2.12479018`, and Temurin JDK `21`
- Qt-related user env vars now point at `C:\Qt\6.10.3\mingw_64`, and Android
  env vars now point at
  `C:\Users\35378\AppData\Local\Android\Sdk`
- Recommendation V2 now uses:
  - structured group/sub-metric weights
  - longer non-breakfast multi-meal compensation
  - post-meal sleep-plan modifiers
  - Chinese-ready reasons and warnings
- Recommendation runs are now persisted into `recommendation_records`
- Meal feedback is now persisted and minimally editable from the Meals page
- Dish-level historical feedback now feeds back into recommendation scoring
- The Meals form now supports a quicker real-world logging path:
  recent meals can be reused as templates or copied as dish sets, dish search
  can add the first hit on Enter, and the first added dish can carry its
  default dining mode into the meal form
- The dish-only template-reuse path now also preserves dining-mode carry-over
  through template dish metadata, instead of only the manual-add path doing so
- Food dish management now supports merchant-level filtering in addition to
  broader text search across operational dish fields such as burden,
  sleepiness, flavor, odor, and dining mode
- Supplement parsing uses `Qt6::Network` and an OpenAI-compatible chat
  completions API
- Supplement parsing now prefers in-app saved config first, then falls back to
  `MEALADVISOR_LLM_API_KEY`, `MEALADVISOR_LLM_API_URL`,
  `MEALADVISOR_LLM_MODEL`, and finally `OPENAI_API_KEY`
- A Clang-only vexing-parse issue in the supplement-parser network request path
  has now been fixed in `RecommendationEngine`, which matters for Android
  builds more than the earlier desktop-only verification path
- The parser request path now uses docs-backed `system` + `user` prompts,
  OpenAI Chat Completions style payloads, and `temperature = 0`
- The parser response path now enforces:
  - strict JSON only
  - `version = supplement_parser_v1`
  - exact 13-field `result`
  - allowed fixed-step values / enums / nap-minute set
  - neutral fallback on invalid output
- `MealAdvisorValidation` now covers supplement parser fallback / success cases
  for:
  unconfigured, malformed JSON, invalid structure, invalid fixed-set values,
  network failure, timeout, and valid structured output
- Android CMake configure and arm64 APK packaging have now been re-verified
  with the installed Qt Android kit
- There is now a desktop-only local validation target:
  `MealAdvisorValidation` seeds the current fake-data set and checks Meals /
  Food / Recommendation / Feedback / Schedule behavior without changing schema
- `RecommendationEngine` now supports a fixed-now env override
  (`MEALADVISOR_FIXED_NOW`) for deterministic local validation scenarios
- `MealLogManager` now enforces strict ISO meal timestamps, so malformed values
  like `2026/04/22 12:30` no longer slip through as valid saves
- `MealAdvisorValidation` is intentionally excluded from Android builds so the
  Android packaging flow only targets the real app
- The current Stage 6 supplement-parser rules and prompt template are now
  documented in:
  - `docs/llm-supplement-rules.md`
  - `docs/llm-supplement-prompt.md`
- Current LLM boundary:
  - strict JSON parser only
  - 13 fixed fields
  - strong fields use `0.4-2.5` fixed-step values
  - weak fields use `0.75-1.35` fixed-step values
  - `postMealSleepPlan` is an enum
  - `plannedNapMinutes` is a fixed integer field
  - `sleepPlanConfidence` is for governance / gating, not direct score bonus

## Known Issues / Follow-ups

1. PowerShell console output in this environment can misdisplay UTF-8 Chinese as mojibake; confirm suspected encoding damage from file contents or runtime UI, not terminal echoes alone.
2. Schedule default import currently includes a placeholder note for the Wednesday late-class room because the screenshot did not fully show it.
3. `docs/recommendation-metrics-table.md` is currently readable in UTF-8 and can be used as a literal reference again; do not assume it is corrupted unless a future scan finds a real damaged segment.
4. Supplement parsing now has a minimum in-app settings path and has one real
   DeepSeek live-provider verification pass behind it, but there has still not
   been a fully manual click-driven desktop UI live session in this workspace;
   current confidence comes from request capture, parser-path fault injection,
   direct live-provider responses, and desktop smoke launch.
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
9. The new Meals quick-template flow plus the latest search-order / guardrail
    pass still need validation against longer real-data usage and Android
    touch interaction, even though the desktop build still compiles and the
    app starts locally.
10. The current validated non-LLM recommendation baseline still keeps
    `牛肉火锅单人套餐` out of top-3 for a relaxed no-class high-budget dinner
    scenario, because that intent is not explicitly represented in the current
    non-LLM local context. The Stage 6 parsed `budgetFlexIntent` path now does
    lift that candidate, so this should not block Stage 7.
11. The current seeded 10-log validation data now surfaces
    `feedback_coverage`, `recommendation_hits`,
    `weight_adjustment_suggestions`, and `context_split`, but still does not
    emit `sleepiness_watch`, `stable_favorites`, or `low_repeat`; the current
    heuristics still prefer repeated exact-dish evidence over sparse
    pattern-level signals.
12. The Stage 6 contract, prompt wording, and final local field mapping pass
    are implemented. Budget relaxation now has a verified downstream effect;
    cola/beverage behavior is limited mainly by the current lack of cola /
    beverage candidates in local data, not by the parser contract.
13. The Stage 1-6 closeout did not change application code. Android APK
    packaging was not rerun because no shared build or packaging logic changed.
    Real provider replay was not rerun because no Stage 6 code changed and the
    current shell had no provider key environment variables.
14. The first five Stage 7 preflight passes fixed obvious static QML copy
    issues, higher-risk dynamic C++/seed/validation strings, a blocking
    phone-width card-overlap problem, normal button text readability, and the
    most obvious enum-display, lower-page narrow-width, search-clear, and
    Meals save-guardrail reachability issues. Remaining frontend work is mainly
    Android touch validation and real repeated-use checks.

## Recommended Next Steps

1. Continue Stage 7 with Android packaging and first Android runtime
   validation: build the arm64 debug APK, install/launch on a device or
   emulator if available, and focus on native touch/keyboard/scroll behavior.
2. Reuse desktop build, short smoke check, and `MealAdvisorValidation.exe` as
   the fast regression path; rerun Android arm64 packaging only if shared build
   or packaging logic changes.
3. If a real provider key is available, optionally replay the six DeepSeek
   samples once more, but do not block Stage 7 unless provider output regresses.
4. Keep the local recommendation engine as the final scorer and do not expand
   into dish enrichment, OCR, schema redesign, feedback parser, rerank, or a
   recommendation rewrite during Stage 7 preflight.

## If Starting In A New Window

Use the concrete prompt in `docs/next-task-prompt.md`.

If that file has not been refreshed for the latest task yet, fall back to the
generic template in `docs/resume-prompt.md`.

Then ask the model to:

1. Read `README.md`
2. Read `docs/project-plan.md`
3. Read `docs/data-model.md`
4. Read `docs/product-rules.md`
5. Read `docs/memory.md`
6. Read `docs/handoff.md`
7. Continue with the next requested task without re-planning from scratch
8. Update `docs/memory.md` and rewrite `docs/next-task-prompt.md` before
   finishing if the task changes code or project docs
