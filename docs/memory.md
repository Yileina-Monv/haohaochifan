# Project Memory

Last updated: `2026-04-20`

## Update Rule

This file is the persistent working memory for the project.

Every time code is modified, update this file before ending the task.

At minimum, keep these sections current:

1. `Current Snapshot`
2. `Recent Changes`
3. `Next Steps`
4. `Open Issues`

If the current task changes project direction or major behavior, also update
`docs/handoff.md`.

## Current Snapshot

- Project: `MealAdvisor`
- Root path: `D:\Codex\2026-04-18-qt-c-app`
- Stack: `Qt Quick/QML + C++ + SQLite`
- Desktop build: compiling successfully in `build/desktop-debug`
- Estimated overall progress: about `81%` to `85%`
- Main pages currently present: `Home`, `Schedule`, `Food`, `Meals`
- Data layer already includes repositories for planning, schedule, merchants,
  dishes, and meal logs
- Recommendation flow is now manual-triggered from the home page instead of
  auto-refreshing on startup
- Recommendation context is based on the current time window and schedule, not
  on the previous meal
- Home page now includes a natural-language supplement input box that can call
  an OpenAI-compatible API and convert text into temporary scoring weights
- A dedicated metrics reference document now exists at
  `docs/recommendation-metrics-table.md`
- Recommendation Engine V2 Core has now been implemented inside the existing
  `RecommendationEngine` instead of a parallel engine
- The scoring logic is no longer one large hard-coded formula:
  it now resolves group weights and sub-metric weights through a central
  weight/config structure with a reserved override interface for future UI
- Candidate output is now Chinese-ready for frontend use:
  each recommendation exposes `reason`, `reasons`, `warnings`, `warningText`,
  and score breakdown data
- Multi-meal compensation now uses a longer recent-meal window and excludes
  breakfast by default
- Recommendation outputs are now persisted into
  `recommendation_records`
- Supplement parsing now includes post-meal sleep-plan fields:
  `post_meal_sleep_plan`, `planned_nap_minutes`, `sleep_need_level`,
  `sleep_plan_confidence`
- Post-meal sleep planning now modifies carb, sleepiness, and total-time
  penalties during V2 scoring
- A first feedback loop is now connected:
  `meal_feedback` stores taste/repeat/fullness/sleepiness/comfort/focus and
  the engine reads aggregated dish-level feedback back into preference and risk
  scoring
- Feedback aggregation is no longer a raw dish-level average:
  it now applies recency decay, portion/meal-impact weighting, and lowers the
  influence of snack/beverage feedback
- Recommendation persistence linkage is now more explicit:
  new meal logs match against recommendation top-3 dishes and persist
  `selected_meal_log_id` plus `selected_candidate_rank`
- Meal feedback now also persists the linked `recommendation_record_id` when a
  selected recommendation can be resolved from the meal log
- Meals page now includes a minimal saved-feedback workflow for recent meals
- Home page now renders recommendation warnings and score breakdown data
  directly instead of only the combined `reason` string
- Meals page now surfaces recommendation linkage more clearly:
  matched recommendation record, candidate rank, selected dish, and whether the
  saved feedback is linked back to the originating recommendation
- Meals page now exposes a first feedback analytics surface:
  feedback coverage, recommendation hit quality, stable favorites,
  sleepiness-watch dishes, and low-repeat dishes
- Feedback analytics now also includes:
  class-pressure vs relaxed context split, ranking momentum, and dish
  improving/worsening direction from recent vs older feedback windows
- Meals page feedback insights are now drill-down capable:
  each insight can expose supporting dishes, supporting meals, and readable
  suggested weight changes against the existing engine keys
- Recent meal cards can now be visually tied back to the currently selected
  insight so the user can inspect the supporting samples without querying
  SQLite or building a parallel debug view
- Feedback drill-down is now more usable in the existing Meals page:
  supporting meals and supporting dishes are no longer just recent-first;
  they now prioritize more representative samples such as multi-match meals,
  feedback-complete meals, and linked recommendation hits
- Weight hints are now more Chinese-readable:
  they expose user-facing adjustment labels and copy instead of only raw
  engine-style keys, while still preserving the underlying weight key
- Meals page now includes a lightweight recommendation-history inspect path
  that reuses the existing `recommendation_records` linkage:
  linked meals can show the original context summary plus the historical top-3
  candidates inline without any schema redesign
- Meals-page insight detail is now easier to scan at a glance:
  each insight can expose a `priorityHeadline`, a short `quickLookText`, and
  compact evidence-count summary before the longer drill-down text
- Supporting sample ordering is now more stable under ties:
  supporting meals and dishes now use explicit tie-breakers instead of falling
  back almost entirely to recency
- Recommendation-history inspect now includes a lighter narrative summary:
  linked meals can show selected-vs-top1 comparison, short context headline,
  and compact reason text without changing the existing schema
- Meals-page insight detail now also includes an ordered `scanSteps` layer so
  the user can see what to inspect first before reading the longer paragraph
- Lightweight recommendation-history inspect now exposes clearer inline compare
  fields:
  score-gap summary, compare guide text, and named top/selected candidate
  score snippets without changing the existing schema
- Meals-page feedback detail now also exposes a first-look scan anchor:
  `firstScanStepTitle`, `firstScanStepBody`, and `remainingScanStepCount`
  let the detail panel highlight the first action before the longer text
- Supporting sample ordering now uses stronger tie-breakers:
  compare richness, score-gap availability, representative feedback density,
  and top supporting-meal quality now help keep dish/meal ordering stable
- Lightweight recommendation-history inspect now also exposes
  `comparePriorityHeadline`, numeric `scoreGapValue`, and candidate-level
  badge / score-gap tags without changing `recommendation_records`
- Meals-page copy has been cleaned one more round in-place:
  the main meal-log form is more Chinese, the detail panel pushes long
  explanation into a later `补充说明` block, and the linked recommendation
  compare card now tells the user what to inspect first
- A repo-wide UTF-8 scan has now confirmed that most earlier terminal-side
  mojibake was PowerShell display noise rather than file corruption:
  the main QML pages, `RecommendationEngine`, and
  `RecommendationRecordRepository` are file-level healthy, while the last
  clearly user-visible damaged block was concentrated in
  `MealLogManager` Meals-page insight helper copy
- Stage assessment:
  - Stage 1 `Foundation`: done
  - Stage 2 `Input Capability`: substantially done
  - Stage 3 `Recommendation Engine / V2 Core`: near complete
  - Stage 4 `Feedback Loop`: well underway
  - Stage 5 `Management Completion`: not started
  - Stage 6 `LLM Enhancement`: partially started
  - Stage 7 `Android Polish/Release`: not started

## Roadmap Framework

- Recommended total scope: `7 stages`
- Realistic total duration at about `6-8 hours/week`: roughly `6-10 weeks`
- If dish data entry, rule tuning, and UI cleanup expand, `10-12 weeks` is
  normal

### Stage 1: Foundation

Status: `Done`

Scope:

- Qt/QML project setup
- SQLite initialization
- schema and lightweight migrations
- default planning policy and recommendation profiles
- basic app state wiring

### Stage 2: Input Capability

Status: `Substantially done`

Already done:

- schedule page with default import, viewing, and manual add
- schedule edit/delete
- merchant creation, search, edit, and delete
- dish creation, search, edit, archive, and recent-item quick pick
- meal log page with multi-dish meals, weights, recent logs, edit/delete, and
  frequent-dish quick pick

Still missing:

- a final pass on even faster daily-entry UX
- optional extras such as more powerful search, favorites, and bulk actions

Practical target:

- logging one real meal should get close to `30 seconds`

### Stage 3: Recommendation Engine V1

Status: `Near complete`

Already done:

- recommendation now runs inside the existing `RecommendationEngine` V2 core
- current time window is used as the recommendation trigger
- schedule, dish tags, merchant cost/time data, recent meals, and meal-history
  compensation affect scoring
- recommendation output is persisted into `recommendation_records`
- home page can output `3` candidates with reasons, warnings, and score
  breakdown
- supplement parsing now affects temporary weights and post-meal sleep-plan
  modifiers

Still missing:

- final polish on recommendation explainability and confidence signals
- frontend-adjustable weight UI
- more real-world tuning confidence from longer feedback usage

Required rule coverage to treat this stage as complete:

- reduce high-carb/high-burden options when there is class after the meal
- adjust dinner timing/burden around evening classes
- keep snack/drink items low-impact on full-meal recommendation
- compensate when recent meals are repeatedly skewed toward one nutrition side

### Stage 4: Feedback Loop

Status: `Well underway`

Scope:

- post-meal feedback UI or dialog
- store fullness, sleepiness, comfort, focus impact, and repeat willingness
- feed feedback back into local scoring weights
- summarize historical feedback so weight tuning has visible evidence

Completion signal:

- the same dish can score differently over time based on actual outcomes
- recent feedback and recommendation-hit quality are visible without querying
  SQLite manually

### Stage 5: Management Completion

Status: `Partially done`

Scope:

- edit/delete for merchant, dish, schedule, and meal log
- search and filtering
- faster selection for frequent merchants and dishes

Completion signal:

- no database hand edits are needed for normal maintenance

### Stage 6: LLM Enhancement Layer

Status: `Partially started`

Already done:

- there is already an OpenAI-compatible supplement parser for natural-language
  temporary preference weights

Still missing for full completion:

- final provider decision such as Deepseek if desired
- natural-language dish name to structured dish tags
- natural-language feedback to structured weight changes
- optional secondary explanation or reranking support

Important rule:

- LLM stays as an enhancement layer and must not replace the local rule engine

### Stage 7: Android Polish And Release

Status: `Not started`

Scope:

- Chinese text/encoding cleanup
- mobile interaction polish
- empty states and error handling
- Android packaging and on-device validation

Completion signal:

- the app is stable enough for daily phone usage

## Milestones

- `M1`: stable schedule + merchant + dish + meal logging
  - substantially achieved
- `M2`: 3 useful explainable recommendations from real user data
  - substantially achieved; tuning and frontend controls still need work
- `M3`: feedback changes future recommendations
  - well underway
- `M4`: natural-language enhancement for input and feedback
  - partially achieved

## Recommended Delivery Order

1. Deepen feedback analytics with drill-down detail and actionable weight hints
2. Add a frontend-adjustable weight UI and API settings UI
3. Finish management-completion polish and faster daily-entry UX
4. Clean up encoding and UX rough edges
5. Expand LLM enhancement
6. Finish Android packaging and release polish

## Recent Changes

### `2026-04-18`

- Added the first usable rule-based recommendation engine:
  - reads planning policy, recommendation profiles, schedule, dishes,
    merchants, and recent meal logs
  - scores dishes and outputs 3 candidates with reasons
  - avoids immediate repeats using recent `meal_logs` + `meal_log_dishes`
- Added `MealLogRepository::loadRecentDishIds()` so recommendation scoring can
  penalize recent repeats
- Changed recommendation interaction on the home page:
  - added `Judge Recommendation` button
  - removed the old assumption that recommendation should always auto-run
- Changed recommendation timing logic:
  - recommend based on current time window
  - if the current day is outside active planning scope, roll forward to the
    next active planning day
- Added natural-language supplement parsing:
  - supplement text is sent to an OpenAI-compatible chat completions endpoint
  - parsed output becomes temporary weights such as hunger, carb preference,
    drink preference, budget flexibility, and class-skip flags
  - parsed weights influence the next manual recommendation run
- Added API config helpers in `AppConfig`:
  - `MEALADVISOR_LLM_API_KEY`
  - `MEALADVISOR_LLM_API_URL`
  - `MEALADVISOR_LLM_MODEL`
  - fallback to `OPENAI_API_KEY`
- Linked `Qt6::Network` in CMake for supplement parsing requests
- Completed the must-do and quick-win input tasks:
  - `ScheduleManager` now supports update/delete with validation
  - `MealLogManager` now supports dish search, frequent dishes, edit/delete,
    and update-in-place saves
  - `FoodManager` now supports merchant/dish search, frequent items,
    edit/delete, and stronger numeric validation
- Reworked `SchedulePage`, `FoodPage`, and `MealLogPage`:
  - each page now exposes edit/delete actions directly in the UI
  - merchant/dish search is available from the Food page
  - frequent merchants and dishes can be reused as quick picks
  - meal logs can be loaded back into the form and updated
  - meal log form now applies time-based defaults for meal type and weekday
- Cleaned the schedule page text enough to remove the broken garbled UI labels
- Replaced the broken default schedule seed strings with safe placeholder names
  so the project can compile while preserving the imported time structure

### `2026-04-19`

- Added `docs/recommendation-metrics-table.md` as the current source of truth
  for:
  - Chinese metric names
  - scoring structure
  - dynamic weighting
  - post-meal sleep plan rules
  - warning layer rules
  - reserved LLM supplement and feedback interfaces
- Added a standing development requirement:
  recommendation-weight logic should keep a reserved adjustable interface for
  future frontend tuning
- Added a long-task continuation direction:
  the next major implementation target should be `Recommendation Engine V2
  Core`, followed by feedback/persistence work and only then frontend-adjustable
  weight UI
- Completed `Recommendation Engine V2 Core` in the existing
  `src/recommendation/recommendationengine.*`:
  - replaced the old one-pass hard-coded formula with a structured weight/config
    pipeline
  - added dynamic group weights for class lunch / normal daytime / dinner
  - added explicit V2 scoring families:
    scene fit, nutrition fit, preference fit, diversity fit, budget fit, and
    intent fit
  - added a reserved `setWeightOverrides()` / `clearWeightOverrides()` interface
    plus exposed active weight configuration for future frontend tuning
- Upgraded recent-meal compensation:
  - uses a longer non-breakfast recent-meal window by default
  - checks same-dish repeat, same-merchant repeat, and recent nutrition
    compensation
  - reads historical dish structure through `MealLogRepository` +
    `meal_log_dishes` instead of only a short recent dish-id list
- Extended supplement parsing and engine inputs with post-meal sleep planning:
  - `post_meal_sleep_plan`
  - `planned_nap_minutes`
  - `sleep_need_level`
  - `sleep_plan_confidence`
  - these modifiers now affect carb fit, sleepiness risk, and total occupied
    time penalties
- Recommendation output is now Chinese-ready on the backend:
  - Chinese summary text
  - Chinese reasons and warnings
  - per-candidate score breakdown metadata
- Added `DishRepository::loadAllDishes()` so recent meal compensation can still
  read historical dish tags even if a dish was archived later
- Verified desktop compilation again in `build/desktop-debug`
  using Qt's bundled `cmake.exe`; note that MinGW's `bin` path must be present
  in `PATH` for command-line builds because `g++` otherwise cannot invoke the
  assembler stage correctly
- Added feedback and persistence infrastructure:
  - `meal_feedback` now includes `taste_rating` and `repeat_willingness` in
    schema plus migration coverage
  - added `MealFeedbackRepository`
  - added `RecommendationRecordRepository`
- Recommendation runs now persist top-3 outputs into
  `recommendation_records`
- New meal logs now backfill `selected_dish_id` on the latest pending
  recommendation record using the highest-impact selected dish as the
  representative choice
- `RecommendationEngine` now reads aggregated dish-level feedback and uses it
  to:
  - smooth `tasteRatingFit`
  - smooth `repeatWillingnessFit`
  - improve `preferenceScoreFit`
  - adjust digestive/sleepiness risk using historical comfort, sleepiness, and
    focus feedback
  - emit extra positive reasons and warnings from historical feedback when
    enough samples exist
- `MealLogManager` and `MealLogPage` now support a minimal feedback persistence
  loop for recent meals:
  - fullness
  - sleepiness
  - comfort
  - focus impact
  - taste rating
  - repeat willingness
  - would eat again
  - free-text note
- Desktop build compiles successfully after the feedback/persistence changes
- Desktop executable also starts successfully after the latest feedback/linkage
  refactor
- Polished the feedback-linked UI surfaces:
  - `Home` recommendation cards now show `reasons`, `warnings`, and score
    breakdown chips directly
  - `Meals` cards now show recommendation linkage details such as matched
    recommendation id, top-N hit rank, selected dish, and linked feedback
    record id
- Added a repository read path for recommendation linkage by meal log so the UI
  can read back selection matches without guessing
- Added a first feedback analytics surface to `Meals`:
  - feedback coverage across recent meals
  - linked recommendation hit quality (`top-1` / `top-2` / `top-3`)
  - stable favorites from historical taste + repeat signals
  - high-sleepiness dishes and low-repeat dishes that may need stronger
    penalties or lower preference weight
- Extended feedback analytics with trend and context signals:
  - `Context Split` compares class-pressure meals vs relaxed meals
  - `Ranking Momentum` compares recent top-1 hit rate vs older linked meals
  - `Improving Dishes` and `Worsening Dishes` compare recent vs older
    per-dish feedback windows
- Deepened the `Meals` feedback loop without rebuilding the recommendation
  core:
  - `MealLogManager::feedbackInsights` is no longer just summary text; each
    insight now carries structured supporting meals, supporting dishes, and
    weight-hint metadata
  - `MealLogPage` now renders a drill-down detail panel for the selected
    insight
  - drill-down detail can jump into supporting meals via the existing meal
    editor flow and can filter the existing dish picker to the implicated dish
  - added a synthesized `Weight Adjustment Suggestions` insight that turns
    stable patterns into readable manual-tuning suggestions without applying
    automatic overrides
  - kept recent-meal recommendation linkage, top-N hit display, and feedback
    save/edit flow intact
- Re-verified desktop compile in `build/desktop-debug` after the drill-down
  analytics changes
- Re-verified that `MealAdvisor.exe` starts successfully after the new Meals
  page drill-down UI changes
- Polished Meals-page feedback drill-down usability instead of changing the
  recommendation core:
  - supporting meals now prioritize representative evidence first using dish
    overlap, saved feedback presence, and linked recommendation hit context
  - supporting dishes now carry representative-priority metadata and no longer
    rely only on their original signal ordering
  - insight titles, summaries, detail copy, and weight hints are now much more
    Chinese-readable and less engineering-key-driven
- Added lightweight recommendation-history inspect by reusing existing
  recommendation linkage:
  - `RecommendationRecordRepository` can now load the linked record's context
    summary and original top-3 candidates
  - `MealLogManager` attaches that history snapshot onto linked meals
  - `MealLogPage` can now render the historical top-3 recommendation snapshot
    inline on recent meal cards
- Re-verified desktop compile in `build/desktop-debug` after the latest
  Meals-page drill-down and recommendation-history inspect changes
- Re-verified that `MealAdvisor.exe` starts successfully after the latest
  Meals-page drill-down polish
- Continued iterating inside the existing `MealLogManager`,
  `RecommendationRecordRepository`, and `MealLogPage` instead of changing the
  recommendation core or redesigning schemas:
  - added quick-scan fields for each insight:
    `priorityHeadline`, `quickLookText`, and `evidenceSummary`
  - added more stable tie-breaker rules for supporting meals and dishes so
    representative samples do not reshuffle as easily under equal priority
  - added lightweight inspect summary fields for linked recommendation history:
    `contextHeadline`, `selectionSummary`, `comparisonSummary`,
    `selectedReason`, and `topCandidateReason`
  - updated Meals-page copy to make insight cards, weight hints, and
    recommendation-history inspect read more like product prompts and less like
    raw engine/debug fields
- Re-verified desktop compile in `build/desktop-debug` after the latest
  quick-scan insight and inspect-summary polish
- Re-verified that `MealAdvisor.exe` starts successfully after the latest
  Meals-page copy / inspect iteration
- Continued iterating in the existing `MealLogManager`,
  `RecommendationRecordRepository`, and `MealLogPage` without changing the
  recommendation core or redesigning schemas:
  - added ordered `scanSteps` for each feedback insight so detail panels can
    tell the user what to inspect first
  - strengthened representative-sample ordering again:
    supporting meals now favor meals that combine relevant dish overlap,
    richer saved feedback, and recommendation-feedback linkage; supporting
    dishes now also prefer samples backed by better linked meal evidence
  - expanded lightweight recommendation-history inspect with inline compare
    fields:
    `compareGuideText`, `scoreGapSummary`, selected/top candidate names, and
    score snippets for the existing linked recommendation card
  - updated the Meals page to render the new scan-step block and a clearer
    selected-vs-top1 compare block inline on linked meals
- Re-verified desktop compile in `build/desktop-debug` after the latest
  insight-ordering and inline-compare polish
- Re-verified that `MealAdvisor.exe` starts successfully after the latest
  insight-ordering and inline-compare polish
- Continued polishing the same existing Feedback Loop surfaces without
  redesigning the schema or recommendation core:
  - rewired `MealLogManager` insight / weight-hint copy through cleaner helper
    text so `scanSteps`, `priorityHeadline`, `quickLookText`, representative
    support notes, and lightweight inspect summaries no longer read like
    corrupted field dumps
  - kept the current representative ordering logic but stabilized the
    user-facing ordering rationale for supporting meals / dishes, so the top
    samples now explain more clearly why they are worth checking first
  - normalized the main Meals-page Chinese copy for feedback insights,
    drill-down detail, linked recommendation inspect, inline compare, and the
    in-card feedback editor instead of leaving half-polished mojibake strings
- Re-verified desktop compile in `build/desktop-debug` after the latest Meals
  page copy cleanup / helper normalization
- Re-verified that `MealAdvisor.exe` starts successfully after the latest
  Meals page copy cleanup / helper normalization

### `2026-04-20`

- Continued iterating only inside the existing `MealLogManager`,
  `RecommendationRecordRepository`, and `MealLogPage` instead of touching the
  recommendation core, settings, or schema:
  - insight detail now highlights the first scan step explicitly before the
    rest of `scanSteps`, and long detail text is pushed down into a later
    `补充说明` block so the user can decide faster what to inspect first
  - supporting meals now break ties with compare richness and score-gap
    availability, while supporting dishes also look at top supporting-meal
    quality and representative feedback density to keep ordering more stable on
    real data
  - recommendation-history inspect now surfaces a short
    `comparePriorityHeadline`, numeric score-gap value, cleaner summary text,
    and candidate badge / score-gap tags for the inline compare card and the
    historical top-3 list
  - cleaned another small round of Meals-page Chinese product copy in the main
    logging form and detail panel without changing the overall layout
- Re-verified desktop compile in `build/desktop-debug` after the latest
  Meals-page first-look / tie-breaker / inline-compare polish
- Re-verified that `MealAdvisor.exe` starts successfully after the latest
  Meals-page first-look / tie-breaker / inline-compare polish
- Ran a repo-wide encoding scan across `app/qml`, `src`, and `docs` using
  UTF-8 file reads instead of relying on PowerShell console output
- Confirmed that `app/qml/Main.qml`, `FoodPage.qml`, `SchedulePage.qml`,
  `MealLogPage.qml`, `src/recommendation/recommendationengine.cpp`, and
  `src/data/recommendationrecordrepository.cpp` are file-level UTF-8 clean for
  the current user-visible strings inspected in this round
- Fixed the actual damaged Meals-page copy block in
  `src/core/meallogmanager.cpp`:
  `normalizedInsightSummaryV3`, `normalizedInsightDetailV3`,
  `representativeMealSupportPriorityLabelV3`,
  `dishSupportPriorityLabelV3`, and dish signal summary text now output
  readable Chinese again instead of `?` placeholders
- Re-verified desktop compile in `build/desktop-debug` after the Meals-page
  encoding repair
- Re-verified that `MealAdvisor.exe` launches successfully after the Meals-page
  encoding repair
- Reclassified `docs/recommendation-metrics-table.md`:
  its current UTF-8 file contents are readable and structurally useful, so it
  should no longer be treated as broadly encoding-damaged by default

## Next Steps

1. Continue targeted encoding verification on runtime-visible dynamic strings,
   prioritizing any remaining older manager/repository helper copy that still
   reaches `Home`, `Food`, `Schedule`, or `Meals`.
2. Validate the repaired Meals-page insight summary/detail/sample-priority copy
   on real meal data to confirm the wording is now readable enough in-context.
3. Keep tightening the feedback loop from the existing Meals page only where
   real usage still shows ambiguity, especially:
   - whether the first-scan-step highlight plus reordered `补充说明` block
     actually reduce long-text reading
   - whether representative-sample ordering still looks stable on real data
   - whether recommendation-history inspect needs only another small inline
     compare pass or something deeper later
4. Then add a frontend-adjustable weight UI and a settings UI for API
   configuration.
5. Restore the real default schedule seed text after confirming the intended
   course names and locations.

## Long-Task Plan

Use this sequence in the next window if continuing recommendation work:

1. Read `docs/recommendation-metrics-table.md` and `docs/memory.md` first.
2. Treat `Recommendation Engine V2 Core` as already landed in the existing
   engine.
3. The next implementation target should be polishing the feedback-linked UI
   and settings surfaces, not rebuilding the engine core.
4. The immediate next task should be drill-down detail for feedback insights,
   then suggested weight adjustments from stable patterns.
5. Do not start with frontend weight UI unless the backend feedback/persistence
   phase is already stable.
6. When continuing recommendation work, preserve the current weight/config
   structure and extend it instead of re-hardcoding formulas into
   `runDecision()`.
7. Keep breakfast excluded from the recent-meal compensation path unless there
   is an explicit requirement change.

## Open Issues

- `docs/handoff.md` and older planning docs had drifted behind the real code.
  They have been updated again this round, but this still needs discipline on
  future changes.
- PowerShell console output in this environment can misdisplay UTF-8 Chinese as
  mojibake; suspected encoding problems should be confirmed from file contents
  or runtime UI, not terminal echoes alone.
- The largest clearly user-visible damaged block found in this scan was the
  Meals-page insight helper copy in `src/core/meallogmanager.cpp`; that block
  has now been repaired, but older helper branches outside the scanned runtime
  path may still need verification later.
- The default schedule seed still uses temporary safe placeholders instead of
  the original intended Chinese course names.
- `docs/recommendation-metrics-table.md` is currently readable in UTF-8 and can
  be used as a literal reference again; it should not be assumed corrupted
  unless a future scan finds a real damaged segment.
- Supplement parsing currently depends on an external API but there is no
  in-app settings screen for credentials or endpoint selection.
- `recommendation_records` now persists selected meal linkage more explicitly,
  and there is now a lightweight inline inspect path on Meals cards, but there
  is still no dedicated full recommendation-history view or candidate-to-
  candidate diff.
- `meal_feedback` aggregation now includes recency decay and snack/beverage
  down-weighting, but it is still dish-centric and does not yet model stronger
  context slices like class-pressure vs relaxed meals.
- Feedback analytics drill-down and readable weight suggestions are now much
  more usable on the Meals page, but the new quick-scan copy and sample-
  priority heuristics still need validation against more real usage rather than
  one fixed rule set.
- The main Meals-page feedback insight / recommendation inspect / feedback
  editor copy is now readable again on the currently scanned path, but older
  non-Meals helper copy and unscanned runtime branches may still need targeted
  verification before declaring the encoding-cleanup effort done.

## Quick Start For New Window

Read these files in order:

1. `README.md`
2. `docs/project-plan.md`
3. `docs/data-model.md`
4. `docs/product-rules.md`
5. `docs/memory.md`
6. `docs/handoff.md`

Then continue the requested task directly from the current implementation
without re-planning the whole product.
