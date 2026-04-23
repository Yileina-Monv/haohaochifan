# Project Memory

Last updated: `2026-04-23`

## Update Rule

This file is the persistent working memory for the project.

Every time code is modified, update this file before ending the task.

Every time a task changes code or project documentation, also leave the next
concrete new-window handoff prompt in `docs/next-task-prompt.md`.

At minimum, keep these sections current:

1. `Current Snapshot`
2. `Recent Changes`
3. `Next Steps`
4. `Open Issues`

If the current task changes project direction or major behavior, also update
`docs/handoff.md`.

## Current Snapshot

- Project: `MealAdvisor`
- Root path: `D:\Codex\haohaochifan`
- Stack: `Qt Quick/QML + C++ + SQLite`
- Desktop build: compiling successfully in `build/desktop-debug-6103`
- Qt and Android environment are now aligned on this machine:
  Qt `6.10.3` desktop + Android kits, JDK `21`, Android SDK command-line
  tools, build-tools `36.0.0`, platforms `android-35` / `android-36`,
  and NDK `27.2.12479018`
- Estimated overall progress: about `87%` to `90%`
- Main pages currently present: `Home`, `Schedule`, `Food`, `Meals`
- Data layer already includes repositories for planning, schedule, merchants,
  dishes, and meal logs
- Recommendation flow is now manual-triggered from the home page instead of
  auto-refreshing on startup
- Recommendation context is based on the current time window and schedule, not
  on the previous meal
- Home page now includes a natural-language supplement input box that can call
  an OpenAI-compatible API and convert text into temporary scoring weights
- Home page supplement parsing now has a minimum in-app LLM config path for
  API key / URL / model, while still falling back to
  `MEALADVISOR_LLM_*` and `OPENAI_API_KEY`
- Stage 6 supplement parsing now follows the docs-backed
  `supplement_parser_v1` contract:
  strict top-level JSON, fixed 13-field `result`, strict local validation, and
  neutral fallback on invalid output
- Stage 6 supplement parsing has now also been live-verified against the
  DeepSeek chat completions endpoint with real Chinese supplement samples, and
  the request path now explicitly sends
  `response_format = { "type": "json_object" }`
- Stage 6 final local mapping pass is now complete: `budgetFlexIntent = 1.1`
  has a visible downstream effect for relaxed high-budget dinner, while
  `colaIntent` can act as a weak drink-intent proxy but still depends on actual
  beverage / cola candidates existing in local dish data
- Stage 1-6 closeout has now been rechecked on `2026-04-23`; no code blocker
  was found for entering Stage 7 preflight
- Stage 7 preflight has started with a first minimal frontend pass:
  Home / Food visible mojibake was repaired, Schedule's main English copy was
  localized, and a few dense action rows now wrap instead of squeezing on
  phone-width screens
- Supplement parser UI state is now explicit enough to distinguish:
  unconfigured, parsing, success, invalid response, network failure, and
  fallback-to-default
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
- Meals-page daily logging is now noticeably faster without changing schema:
  recent real meals can be reused as quick templates, the main meal form uses
  more Chinese-visible labels, dish search can add the first match on Enter,
  and the first added dish can automatically carry its default dining mode
- The copy-dishes-only quick-reuse path now also keeps dining-mode carry-over
  aligned with the faster manual-add path:
  template dish items expose `defaultDiningMode`, and dish-only reuse falls
  back to the source meal's dining mode when needed
- Food-page dish maintenance is now easier in-place:
  dish search can now be filtered by merchant and also match more operational
  fields such as burden, sleepiness, flavor, odor, and dining mode
- Meals / Food guardrails are now tighter without touching schema:
  meal save no longer silently rewrites invalid timestamps to `now`,
  `classAfterMeal` and `minutesUntilNextClass` are validated together,
  merchant/dish enum fields are normalized and checked, and obviously invalid
  numeric ranges are rejected earlier in the manager layer
- Search and picker ordering are now more intentional on the existing local
  data:
  the Meals dish picker and Food dish search now sort by keyword closeness
  first and recent-use frequency second, while frequent-item tie cases now use
  deterministic tie-breakers instead of depending on repository order
- Home / Food visible copy is now more consistent with the rest of the app:
  the main tab labels and Home action labels are Chinese-visible, and Food
  maintenance copy now better explains search/filter behavior instead of
  looking half-admin / half-debug
- Recommendation candidate ordering is now more stable under equal-score ties:
  `RecommendationEngine` keeps the existing score pipeline but now breaks ties
  by cheaper dish first, then deterministic dish / merchant naming instead of
  leaving equal-score ordering to container iteration
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
  - Stage 5 `Management Completion`: well underway
  - Stage 6 `LLM Enhancement`: minimum viable supplement parser is closed
    enough to hand to Stage 7 preflight
  - Stage 7 `Android Polish/Release`: preflight started

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

Status: `Near complete`

Already done:

- schedule page with default import, viewing, and manual add
- schedule edit/delete
- merchant creation, search, edit, and delete
- dish creation, search, edit, archive, and recent-item quick pick
- meal log page with multi-dish meals, weights, recent logs, edit/delete, and
  frequent-dish quick pick

Still missing:

- longer-session validation that the latest quick-entry defaults and picker
  ordering really hold up in repeated use
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

Status: `Substantially done`

Scope:

- edit/delete for merchant, dish, schedule, and meal log
- search and filtering
- faster selection for frequent merchants and dishes

Completion signal:

- no database hand edits are needed for normal maintenance

### Stage 6: LLM Enhancement Layer

Status: `Minimum viable supplement parser usable`

Already done:

- supplement parsing now uses the docs-backed `supplement_parser_v1` contract
- parser requests now use OpenAI Chat Completions style `system` + `user`
  messages with `temperature = 0`
- parser requests now also send
  `response_format = { "type": "json_object" }` for providers that support
  OpenAI-compatible JSON mode
- parser responses are locally validated for:
  - strict JSON
  - `version = supplement_parser_v1`
  - exact 13-field `result`
  - allowed value sets / enums / nap-minute set
- invalid responses now fall back to a neutral default object instead of being
  applied loosely
- Home now includes a minimum in-app config path for API key / URL / model
  while preserving env var fallback
- validation now covers:
  unconfigured, malformed response, invalid structure, network failure,
  timeout, and valid structured result
- one live DeepSeek verification pass has now been completed for:
  - strong class constraint
  - weak preference
  - nap before class
  - stay awake
  - budget relaxed
  - cola preference

Still missing for full completion:

- a final decision on whether the current DeepSeek-compatible path is the
  long-term default provider
- one fully manual click-driven desktop UI live session if later needed for
  extra confidence beyond the current smoke + mock-server + real-provider pass
- natural-language dish name to structured dish tags
- natural-language feedback to structured weight changes
- optional secondary explanation or reranking support

Important rule:

- LLM stays as an enhancement layer and must not replace the local rule engine

### Stage 7: Android Polish And Release

Status: `Preflight started`

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

1. Finish management-completion polish and validate the faster daily-entry UX
2. Re-run the Android build verification pass after the latest portability fix
3. Add a frontend-adjustable weight UI and API settings UI
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

### `2026-04-21`

- Aligned the local development environment to the documented Qt / Android
  stack:
  - installed Qt `6.10.3` desktop and Android kits alongside the existing
    tools
  - installed Git `2.54.0.windows.1`
  - installed Eclipse Temurin JDK `21.0.10.7-hotspot`
  - installed Android SDK command-line tools, `platform-tools`,
    `build-tools;36.0.0`, `platforms;android-35`, `platforms;android-36`,
    and `ndk;27.2.12479018`
  - updated persistent user env vars:
    `Qt6_DIR`, `CMAKE_PREFIX_PATH`, `QTDIR`, `JAVA_HOME`,
    `ANDROID_HOME`, `ANDROID_SDK_ROOT`, `ANDROID_NDK_ROOT`,
    plus PATH entries for Qt / Java / Android CLI tools
- Re-verified desktop compilation with Qt `6.10.3` in
  `build/desktop-debug-6103`
- Re-verified Android CMake configure for `arm64-v8a` in
  `build/android-arm64-debug-6103`
- Found a cross-platform compile issue during Android Clang build in
  `src/recommendation/recommendationengine.cpp`:
  `QNetworkRequest request(QUrl(...))` was parsed as a function declaration on
  Android/Clang
- Fixed the supplement-parser request construction by switching to brace
  initialization so the same code now compiles consistently across desktop and
  Android toolchains
- Standardized the new-window continuation workflow:
  - `docs/resume-prompt.md` is now the stable generic template
  - `docs/next-task-prompt.md` is now the concrete next-window handoff prompt
    that should be refreshed after each completed task
  - the prompt workflow now explicitly requires updating `docs/memory.md`
    before finishing and writing the next concrete prompt for the following
    window
- Switched this window back to the Stage 5 priority explicitly requested by
  the user instead of following the older Android-first handoff order from the
  stale prompt docs.
- Completed a focused high-yield Meals quick-entry pass inside the existing
  `MealLogPage` without changing schema, persistence, or feedback behavior:
  - added a top-of-page `鏈€杩戦娆″揩鎹峰鐢╜ surface that can reuse a recent real meal
    as a template or copy only its dish set
  - added the same `澶嶇敤杩欓` / `鍙姞鑿渀 actions onto recent meal cards for shorter
    round-trips while keeping edit/delete intact
  - changed the meal form to use more Chinese-visible labels for meal type,
    weekday, location, and dining mode while still persisting the same
    internal enum strings
  - dish search can now add the first filtered match directly on Enter, clears
    back to the full list after a successful add, and leaves the optional
    portion-weight field blank so the default dish weight can be reused
    without repetitive manual edits
  - when the first dish is added, its default dining mode now carries into the
    meal form automatically to remove a common repeated click
- Added a small but valuable Food maintenance improvement inside the existing
  `FoodManager` and `FoodPage`:
  - dishes can now be filtered by merchant in addition to free-text search
  - dish search now also matches burden, sleepiness, flavor, odor, and dining
    mode fields so everyday maintenance queries need less scrolling
- Re-verified desktop compile in `build/desktop-debug-6103` after the Stage 5
  quick-entry / filtering pass
- Re-verified that `MealAdvisor.exe` starts successfully after the latest
  Meals / Food UX changes by launching it locally and stopping it after a
  short smoke-check window

### `2026-04-22`

- Validated the current Stage 5 quick-entry work against the actual code path
  instead of only the latest handoff notes:
  - recent-meal template reuse is present in `MealLogPage`
  - copy-dishes-only is present in both the top quick-reuse surface and the
    recent-meals list
  - dish search can add the first filtered result on Enter
  - first-dish dining-mode carry-over existed for manual add, but not for the
    dish-only template-reuse path
- Fixed that remaining Meals friction with a minimal in-place change:
  - `MealLogManager` now exposes each template meal dish item's
    `defaultDiningMode` in the existing UI map
  - `MealLogPage::copyMealDishesFromTemplate()` now carries dining mode from
    the first template dish when available and otherwise falls back to the
    source meal's persisted `diningMode`
  - recommendation linkage, feedback persistence, and SQLite schema stay
    unchanged
- Re-verified desktop compile in `build/desktop-debug-6103` after the
  dish-only dining-mode carry fix
- Re-verified that `MealAdvisor.exe` still starts successfully after the fix
  by launching it locally and keeping it alive for `5` seconds before closing
- Ran one more high-yield pre-real-data polish pass across the existing
  Meals / Food / Recommendation surfaces instead of expanding scope:
  - `MealLogManager` now rejects invalid meal timestamps instead of silently
    falling back to the current time, validates `classAfterMeal` against
    `minutesUntilNextClass`, and normalizes no-class saves back to `0`
    minutes so meal state cannot drift into contradictory combinations
  - `MealLogPage` now makes that rule visible in the form:
    the next-class-minutes field is disabled when there is no class after the
    meal, numeric input hints are clearer, and the dish-search helper text now
    explains that search results are ranked instead of just filtered
  - `MealLogManager` dish search now ranks by keyword closeness plus
    recent-use count, so the existing Enter-to-add-first-result path is less
    likely to add the wrong first row when multiple dishes loosely match
  - `FoodManager` now validates merchant price level / supported dining modes,
    validates dish enum fields and numeric ranges, stores normalized enum
    values more consistently, and sorts merchant/dish results more
    deterministically under ties
  - `FoodPage` and `Main.qml` got a final visible-copy cleanup pass so the
    most-used daily pages are more Chinese-consistent and explain search /
    filter behavior more clearly without changing page structure
  - `RecommendationEngine` keeps the existing V2 scoring and persistence path,
    but now breaks equal-score candidate ties deterministically instead of
    relying on container order
- Re-verified desktop compile in `build/desktop-debug-6103` after the final
  Meals / Food / Recommendation polish pass
- Re-verified that `MealAdvisor.exe` still starts successfully after the final
  polish pass by launching it locally and closing it after a short smoke check
- Added a desktop-only `MealAdvisorValidation` target plus
  `tools/validation/main.cpp` so the current local code path can be seeded and
  checked without changing schema or clearing the whole SQLite DB
- Added `MEALADVISOR_FIXED_NOW` support in `RecommendationEngine` so local
  recommendation validation can run deterministically against specific class /
  meal timestamps
- Fixed a real runtime bug in `DatabaseManager`:
  the default connection-name helper conflicted with the member function and
  could crash the standalone validation binary during construction
- Fixed a real meal-save guardrail bug in `MealLogManager`:
  invalid non-ISO timestamps such as `2026/04/22 12:30` no longer parse
  permissively and fall through as valid meals
- Ran a validation-first local pass with the user-provided fake data:
  - seeded `8` schedule entries, `6` merchants, `10` dishes, `10` meal logs,
    and `10` feedback records into the existing local DB
  - confirmed desktop build plus a short offscreen desktop smoke run
  - confirmed Android arm64 packaging again via
    `cmake --build build/android-arm64-debug-6103 --target MealAdvisor_make_apk`
  - current validation baseline is `17/19` passed
- Fixed an Android build regression introduced by the validation tooling:
  `MealAdvisorValidation` is now created only on non-Android builds, so the
  arm64 APK flow no longer tries to package the local validation executable
- Finalized the Stage 6 supplement-parser contract in docs:
  - added `docs/llm-supplement-rules.md`
  - added `docs/llm-supplement-prompt.md`
  - fixed the parser scope as strict JSON-only temporary adjustment parsing
  - fixed the output contract to 13 fields under
    `version = supplement_parser_v1`
  - agreed strong fields use fixed-step values in `0.4-2.5`
  - agreed weak fields use fixed-step values in `0.75-1.35`
  - agreed `postMealSleepPlan` stays an enum and `plannedNapMinutes` stays a
    fixed integer field
  - agreed `sleepPlanConfidence` is a governance/gating field rather than a
    direct business-weight bonus

### `2026-04-23`

- Implemented the minimum viable Stage 6 supplement-parser integration against
  the real code path instead of leaving the new contract only in docs:
  - `RecommendationEngine` now sends OpenAI Chat Completions style requests
    using docs-backed `system` + `user` prompts and `temperature = 0`
  - supplement parsing now only accepts the fixed
    `version = supplement_parser_v1` object with the exact 13-field `result`
  - local validation now rejects malformed JSON, wrong version, missing/extra
    keys, invalid enums, invalid fixed-step values, and invalid nap-minute
    values
  - invalid output, network failure, timeout, and unconfigured API now all
    fall back to the neutral default supplement object with explicit state /
    status text
- Added the minimum in-app LLM config path without removing env var support:
  - `AppSettings` now stores API key / URL / model locally
  - local saved values take priority
  - blank local values still fall back to
    `MEALADVISOR_LLM_API_KEY / API_URL / MODEL`
  - `OPENAI_API_KEY` fallback is preserved
- Updated the Home supplement area with the smallest practical UI expansion:
  - added an in-app `LLM 配置` dialog
  - surfaced parser state / fallback badges
  - kept the existing supplement input entry point intact
- Extended `MealAdvisorValidation` for Stage 6 parser scenarios:
  - unconfigured -> fallback
  - valid structured result -> accepted
  - non-JSON output -> rejected + fallback
  - invalid structure -> rejected + fallback
  - invalid fixed-set value -> rejected + fallback
  - network failure -> fallback
  - timeout -> fallback
- Re-verified desktop build in `build/desktop-debug-6103`
- Re-ran `MealAdvisorValidation.exe`; current baseline is now `24/26` passed
  and the remaining two failures are the pre-existing recommendation /
  insight-heuristic gaps, not Stage 6 parser regressions
- Re-ran a short desktop app smoke launch after the Stage 6 UI/config changes
- Tightened Stage 6 supplement request stability and validation coverage:
  - `RecommendationEngine` now sends
    `response_format = { "type": "json_object" }` alongside the existing
    OpenAI Chat Completions style payload
  - `MealAdvisorValidation` now also exercises the real `parseSupplement()`
    network path against a local mock server, covering:
    request-shape capture, env/app-config priority, non-JSON output,
    missing fields, extra fields, invalid fixed-set values, `401`, and timeout
  - `MealAdvisorValidation` now has a small
    `--evaluate-response-file` CLI path so live provider responses can be
    checked by the same C++ local validator instead of hand-reimplementing the
    contract
- Completed one real DeepSeek provider pass on `2026-04-23` using six Chinese
  supplement samples and the current Stage 6 payload shape:
  - before the prompt cue tweak, strong-constraint / stay-awake wording tended
    to collapse into `postMealSleepPlan = stay_awake` while leaving
    `classConstraintWeight` and `sleepNeedLevel` neutral
  - after the minimal prompt clarification, the same live-provider pass now
    raises `classConstraintWeight = 1.25`, `sleepNeedLevel = 1.25`, and
    `sleepPlanConfidence = 0.75` for strong-constraint / stay-awake samples
  - `nap_before_class` now stably returns `plannedNapMinutes = 20` with
    `sleepPlanConfidence = 1.0`
  - `weak_preference` currently maps to `flavorIntent = 1.2`
  - `budget_relaxed` currently maps to `budgetFlexIntent = 1.1`
  - `cola_preference` currently maps to `colaIntent = 1.2`
- Re-verified desktop build in `build/desktop-debug-6103` after the prompt /
  validation follow-up changes
- Re-ran the short desktop smoke launch after the final Stage 6 follow-up
- Re-ran `MealAdvisorValidation.exe`; the baseline is now `35/37` passed, and
  the remaining two failures are still the pre-existing recommendation /
  feedback-heuristic gaps outside the supplement parser scope
- Completed the final narrow Stage 6 local mapping pass:
  - `budgetFlexIntent` now expands relaxed no-class dinner budget tolerance
    enough for the current `budgetFlexIntent = 1.1` DeepSeek-style output to
    affect real recommendation ranking
  - a mock-server replay through the real parser path now confirms the parsed
    budget-flex result can raise `牛肉火锅单人套餐` into top-3 for relaxed
    high-budget dinner
  - `colaIntent` now contributes as a weak drink-intent proxy and exact cola
    dishes receive a slightly clearer bonus, but current seeded validation data
    still has no cola / beverage candidate to lift
  - the strict parser contract, app-config priority, env fallback, timeout,
    401, non-JSON, invalid value, missing-field, and extra-field fallback paths
    all remain covered by `MealAdvisorValidation`
- Re-verified desktop app build in `build/desktop-debug-6103`
- Re-ran short desktop smoke launch after the mapping pass
- Re-ran `MealAdvisorValidation.exe`; the baseline is now `36/38` passed after
  adding the new Stage 6 budget-flex replay case. The remaining two failures
  are still the pre-existing non-LLM high-budget recommendation case and the
  sparse feedback-insight heuristic case.
- A new live external provider replay was not run in this shell because no
  `MEALADVISOR_LLM_*`, `OPENAI_API_KEY`, or stored app LLM settings were
  available; confidence for this pass comes from the already completed
  DeepSeek pass plus the real local network parser replay.
- Completed a Stage 1-6 closeout confirmation without changing application
  code:
  - reviewed the current product docs, Stage 6 parser docs, Home / Food /
    Meals / Schedule QML, `RecommendationEngine`, and `MealAdvisorValidation`
  - confirmed desktop build with
    `cmake --build build/desktop-debug-6103 --target MealAdvisor`
  - confirmed a short offscreen desktop smoke launch: `MealAdvisor.exe`
    started and stayed alive for `5` seconds before the test stopped it
  - re-ran `MealAdvisorValidation.exe`; baseline remains `36/38` passed
  - the remaining failures are still the known non-LLM high-budget dinner
    limitation and sparse feedback-insight heuristic limitation
  - no Android APK build was run because this closeout changed no shared
    build, packaging, Android, or Stage 6 application code
  - no real provider replay was run because this closeout changed no Stage 6
    code and the current shell has no `MEALADVISOR_LLM_*` or `OPENAI_API_KEY`
    values
- Started Stage 7 preflight with a deliberately small frontend repair pass:
  - checked the runtime-loaded Home / Meals / Food / Schedule QML surfaces
    through desktop build and offscreen launch
  - fixed real user-visible mojibake in `Main.qml` and `FoodPage.qml`
  - localized the most visible `SchedulePage.qml` English copy while leaving
    the existing schedule data model and manager behavior unchanged
  - changed the densest Home / Food / Schedule action rows from fixed
    `RowLayout` to wrapping `Flow` controls so narrow mobile widths are less
    likely to clip buttons
  - did not change schema, recommendation scoring, Stage 6 parser behavior, or
    LLM capability scope
  - re-verified desktop build with
    `cmake --build build/desktop-debug-6103 --target MealAdvisor`
  - re-ran a short offscreen desktop app smoke launch; `MealAdvisor.exe`
    started and stayed alive for `5` seconds before being stopped
  - re-ran `MealAdvisorValidation.exe`; baseline remains `36/38`, with only
    the two known non-blocking failures
  - Android APK packaging was not run because only QML/docs changed, with no
    shared build, packaging, Android, or C++ code change

## Next Steps

Priority note:

- Stage 1-6 can be treated as sealed for Stage 7 preflight. Do not keep
  expanding Stage 6 or the recommendation core unless a new real regression is
  found.

1. Continue Stage 7 preflight with a second small pass rather than a broad
   redesign: inspect Meals long-card density, Home dynamic recommendation text,
   Food / Schedule form wrapping, empty states, and real phone-width touch
   ergonomics.
2. Keep the two known validation failures classified correctly:
   the non-LLM high-budget dinner case is a local recommendation/context
   limitation, and the missing `sleepiness_watch` / `stable_favorites` /
   `low_repeat` insight case is sparse-data heuristic behavior.
3. If a real provider key is available in a later window, optionally replay
   the same six DeepSeek samples once more, but do not block Stage 7 on that
   unless the provider output regresses.
4. Continue avoiding dish enrichment, OCR, schema redesign, feedback parser,
   rerank, or recommendation-core rewrite until Stage 7 basics are stable.

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
- Older handoff prompts had temporarily drifted to an Android-first follow-up,
  but the current user explicitly prioritized Stage 5 management-completion
  UX; the working memory now reflects that priority shift.
- PowerShell console output in this environment can misdisplay UTF-8 Chinese as
  mojibake; suspected encoding problems should be confirmed from file contents
  or runtime UI, not terminal echoes alone.
- Home / Food / Schedule now have a first Stage 7 text cleanup, but dynamic
  strings sourced from C++ managers and seeded data should still be checked in
  runtime UI before declaring all user-visible mojibake closed.
- Android SDK / NDK / Qt environment is aligned locally and the arm64 APK build
  has now been re-verified after the portability and packaging fixes; rerun it
  again only if Android-specific code or the Qt/Gradle environment changes.
- The largest clearly user-visible damaged block found in this scan was the
  Meals-page insight helper copy in `src/core/meallogmanager.cpp`; that block
  has now been repaired, but older helper branches outside the scanned runtime
  path may still need verification later.
- The default schedule seed still uses temporary safe placeholders instead of
  the original intended Chinese course names.
- `docs/recommendation-metrics-table.md` is currently readable in UTF-8 and can
  be used as a literal reference again; it should not be assumed corrupted
  unless a future scan finds a real damaged segment.
- Supplement parsing now has:
  - in-app config priority over env vars
  - explicit `response_format = json_object` on the request path
  - mock-server coverage for request shape plus failure states
  - one real DeepSeek provider pass with six Chinese samples
  but there has still not been a fully manual click-driven desktop UI live
  session in this workspace; current confidence comes from code-path capture,
  smoke launch, and real provider responses validated through the C++ parser.
- The Stage 6 parser contract and final local mapping pass are now implemented.
  Budget-relax intent has visible downstream effect through the real parser
  path; cola/drink intent is mapped locally, but current seed data still lacks
  actual cola / beverage candidates, so richer drink behavior is a data or
  future add-on-candidate issue rather than a parser blocker.
- The new Meals quick-template path plus the latest search-order / guardrail
  pass still need a longer real-data usage session and Android touch
  interaction to confirm they feel better in practice rather than only
  compiling cleanly.
- The current recommendation baseline still does not raise
  `牛肉火锅单人套餐` into top-3 for the relaxed no-class high-budget dinner
  validation scenario, because that intent is not explicitly represented in the
  current non-LLM local context. This is no longer a Stage 6 blocker because
  the parsed `budgetFlexIntent` path now does raise it.
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
- The current `feedbackInsights()` output for the seeded 10-log validation data
  reaches `feedback_coverage`, `recommendation_hits`,
  `weight_adjustment_suggestions`, and `context_split`, but still does not emit
  `sleepiness_watch`, `stable_favorites`, or `low_repeat`; current heuristics
  still favor repeated exact-dish evidence over sparse pattern-level signals.
- The main Meals-page feedback insight / recommendation inspect / feedback
  editor copy is now readable again on the currently scanned path, but older
  non-Meals helper copy and unscanned runtime branches may still need targeted
  verification before declaring the encoding-cleanup effort done.
- Stage 5 polish is now close to the point of diminishing returns before real
  usage:
  the highest-value remaining work is mostly validation and Android closure,
  not another broad UI feature pass.

## Quick Start For New Window

Prefer using `docs/next-task-prompt.md` for the immediate next task.
Use `docs/resume-prompt.md` only as the generic fallback template.

Read these files in order:

1. `docs/next-task-prompt.md`
2. `README.md`
3. `docs/project-plan.md`
4. `docs/data-model.md`
5. `docs/product-rules.md`
6. `docs/memory.md`
7. `docs/handoff.md`

Then continue the requested task directly from the current implementation
without re-planning the whole product.
