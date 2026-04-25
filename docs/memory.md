# Project Memory

Last updated: `2026-04-26`

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
- Root path: `D:\Codex\2026-04-18-qt-c-app`
- Stack: `Qt Quick/QML + C++ + SQLite`
- Desktop build: compiling successfully in `build/desktop-debug`
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
- Stage 7 preflight second pass has now repaired higher-risk runtime-visible
  copy:
  Home planning/status summaries, recommendation initial/context text, LLM
  parser validation details, Food / Meals / Schedule manager guardrail errors,
  and the default schedule/planning seed labels are now Chinese-visible; two
  dense Meals quick-action rows now wrap instead of squeezing on narrow screens
- Stage 7 preflight third pass has now fixed a real desktop/mobile-width
  runtime usability issue:
  the main QML cards now compute content height instead of collapsing into
  overlapping text, normal enabled button text is forced readable through a
  local QML button wrapper, and Food / Schedule enum selectors now show Chinese
  labels while still passing the original enum values to C++
- Stage 7 preflight fourth pass has now tightened lower-page narrow-width
  behavior:
  Food / Schedule / Meals lower forms switch to one-column entry where needed,
  dense lower-card title/action rows now stack into readable label-plus-action
  flows, the Meals feedback score editor is no longer a confusing three-column
  grid, and the desktop window now allows a narrower smoke width for mobile
  preflight checks
- Stage 7 final desktop/manual preflight pass has now fixed the last
  must-fix usability gaps found in this pass:
  Food's dish clear action now clears both search text and merchant filter,
  Meals has an explicit clear-search path for empty dish-picker results, and
  Meals save can now be clicked with no selected dishes so the existing C++
  guardrail message is reachable instead of hidden behind a disabled button
- Stage 7 Android packaging preflight has now produced a verified arm64 debug
  APK:
  `build/android-arm64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk`.
  A small Android-only CMake include-order fix was required for NDK 27 libc++
  headers after the Qt Android build added the raw NDK sysroot include path.
  No device or configured emulator was available in this shell, so Android
  install/launch and touch validation remain the next real runtime step.
- A new Stage 7 frontend-direction plan has been captured in
  `docs/chat-style-frontend-refactor-plan.md`. The next implementation prompt
  now prioritizes a chat-style main page with `需求推荐` / `饭后反馈` modes, a
  minimum-width-first half-width right drawer for LLM / schedule / food /
  feedback management, and a light frosted-glass visual style.
- The chat-style frontend shell has now landed in `app/qml/Main.qml`: visible
  top tabs / `SwipeView` were replaced by a frosted-glass main page with
  `需求推荐` and `饭后反馈` modes, a bottom multiline composer, parser-then-
  recommendation sequencing, structured recent-meal feedback saving, and a
  right-side management drawer for LLM debug / Schedule / Food / Meals.
- The follow-up Stage 7 QML layout refinement has now simplified the main page
  back to recommendation-first, moved structured `饭后反馈` into the right Drawer,
  made Drawer section buttons consistently sized/aligned, made the LLM debug
  action row stack/wrap predictably, and rebuilt/copied a refreshed Android
  arm64 debug APK to the desktop.
- The targeted Food / Feedback readability pass has now landed:
  FoodPage's repeated low/medium/high dish-attribute selectors are visible
  labeled fields, Drawer feedback is natural-language-first, and a bounded
  strict-JSON `feedback_parser_v1` path maps successful LLM parses into the
  existing `meal_feedback` fields while preserving manual scoring fallback.
- The latest targeted mobile shell polish in `app/qml/Main.qml` removes the
  header planning/budget line, fixes the bottom composer placeholder padding /
  send-button clipping risk, and moves the main shell / Drawer shared controls
  to a beige plus brown-yellow palette.
- All QML text inputs now hide their placeholder while focused or populated
  instead of letting the Android/Material style float the hint above the field.
  `Main.qml` styled inputs use a separate `hintText` property; Food, Schedule,
  and Meals plain inputs use the same focused-or-nonempty binding directly.
- Drawer `LLM 调试` now has a front-end `测试连接` action. It calls
  `RecommendationEngine::testLlmConnection(...)` with the current input values,
  falls back to the existing AppConfig/env/default values for blank fields, and
  reports testing/success/error state in the Drawer.
- Android HTTPS/TLS support has been fixed for the LLM connection path:
  `CMakeLists.txt` now pulls the pinned KDAB `android_openssl` package during
  Android configure, packages `libssl_3.so` / `libcrypto_3.so` through
  `add_android_openssl_libraries(MealAdvisor)`, and `src/main.cpp` sets
  `ANDROID_OPENSSL_SUFFIX=_3` before Qt starts. The rebuilt APK now contains
  `lib/arm64-v8a/libplugins_tls_qopensslbackend_arm64-v8a.so`,
  `lib/arm64-v8a/libssl_3.so`, and `lib/arm64-v8a/libcrypto_3.so`.
- A generated warm beige / brown-yellow launcher icon has been added to the
  project resource library at `resources/icons/mealadvisor-launcher-source.png`
  and packaged into Android `mipmap-*` launcher resources under
  `app/android/res/`.
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

### `2026-04-24`

- Continued Stage 7 preflight with a second narrow runtime-copy and density
  pass:
  - inspected Home dynamic recommendation/config/status paths, Meals long-card
    quick reuse and insight sample cards, Food form/list copy, Schedule cards
    and dynamic C++ manager text
  - localized Home `AppState` summaries so planning days, budget, and default
    recommendation profile no longer appear as English phrases
  - localized `ScheduleManager` runtime strings, reset/imported schedule seed
    labels, class-period session labels, weekday labels, period ranges, and
    guardrail errors
  - localized Food and Meals manager validation errors so form failure states
    are visible in Chinese
  - localized recommendation initial/context/fallback copy and supplement
    parser validation-error details while preserving the strict Stage 6 parser
    contract and request shape
  - changed the Meals recent-template card and insight supporting-meal sample
    card header/action rows from fixed `RowLayout` pressure to a wrapping
    label-plus-`Flow` layout
  - updated planning/profile seed labels in `DatabaseManager` for fresh DBs;
    existing DBs keep their data, while Home maps old `Class Day` /
    `Commute Day` names to Chinese for display
  - updated `MealAdvisorValidation` text assertions to match the Chinese
    guardrail messages
  - no schema, recommendation scoring, Stage 6 parser capability, LLM rerank,
    OCR, dish enrichment, or feedback parser work was added
- Verification for this pass:
  - `cmake --build build\desktop-debug --target MealAdvisor` passed
  - `cmake --build build\desktop-debug --target MealAdvisorValidation` passed
  - short offscreen desktop smoke launch passed; `MealAdvisor.exe` stayed
    alive for `5` seconds
  - `MealAdvisorValidation.exe` is back at `36/38`; the two remaining failures
    are still the known non-blocking non-LLM hotpot case and sparse feedback
    insight case
  - Android APK packaging was not run because this pass did not touch Android,
    shared build configuration, packaging, or Android-specific code
- Continued Stage 7 preflight with a third small runtime visual pass:
  - ran the desktop app and inspected Home / Schedule / Food / Meals at the
    default `420x800` size and a narrower `340x800` phone-like width
  - fixed the blocking QML layout problem where cards in Home / Schedule /
    Food / Meals could collapse to near-zero height and overlap controls
  - added a local `ReadableButton` wrapper for normal QML buttons so enabled
    action labels such as LLM config, save, generate, edit, delete, and quick
    reuse stay readable under the desktop style
  - repaired the remaining visibly damaged Home static strings and Home
    action labels
  - changed the Home LLM status/action row to wrap instead of pushing the
    config button off the card
  - changed Food merchant price, dish dining mode, and dish level selectors to
    display Chinese labels while still submitting `budget/mid/high`,
    `dine_in/takeaway/delivery`, and `low/medium/high` values to C++
  - changed Schedule intensity from free-text `medium` entry to a Chinese
    low/medium/high ComboBox that still submits the original enum value
  - did not change schema, recommendation scoring, Stage 6 parser behavior,
    LLM capability scope, Android, or packaging
- Verification for the third pass:
  - `cmake --build build\desktop-debug --target MealAdvisor` passed
  - short desktop smoke launch passed; `MealAdvisor.exe` stayed alive for
    `5` seconds
  - `MealAdvisorValidation.exe` remained `36/38`; the same two known
    non-blocking validation failures remain
  - Android APK packaging was not run because this pass only touched QML and
    docs, with no Android-specific, packaging, build-system, or shared C++
    logic changes

### `2026-04-25`

- Continued Stage 7 preflight with a fourth small frontend-only pass:
  - inspected the existing QML lower-page structures for Home / Schedule /
    Food / Meals after the card-height and readable-button fix
  - fixed remaining must-fix narrow-width risks in the lower Food page:
    merchant and dish forms now collapse from two columns to one on narrower
    widths, nutrition level selectors reduce from three columns to two, lower
    merchant/dish list cards stack long names above edit/delete actions, and
    dense metadata labels wrap instead of clipping
  - fixed remaining must-fix narrow-width risks in the lower Schedule page:
    weekday cards and schedule-entry cards no longer keep long headings and
    edit/delete actions in a single squeezed row, the add/edit form collapses
    to one column on narrower widths, and schedule detail labels wrap
  - fixed remaining must-fix narrow-width risks in Meals:
    the meal info and dish-picker grids collapse to one column, selected-dish
    rows stack the remove action below the details, insight cards and
    supporting-dish cards stack long text above actions, and the feedback score
    editor now uses clear label/value pairs instead of a cramped three-column
    sequence
  - fixed a navigation-level narrow desktop issue by giving the app a smaller
    explicit minimum window size and equal-width top tabs so the four primary
    page entries remain intended to fit at mobile-like widths
  - no schema, recommendation scoring, C++ behavior, Stage 6 parser behavior,
    LLM capability scope, Android, or packaging changes were made
- Verification for the fourth pass:
  - `cmake --build build\desktop-debug --target MealAdvisor` passed
  - a real desktop smoke/screenshot launch passed; `MealAdvisor.exe` stayed
    alive for at least `5` seconds while screenshots were captured
  - `MealAdvisorValidation.exe` remained `36/38`; the same two known
    non-blocking validation failures remain
  - Android APK packaging was not run because this pass only touched QML and
    docs, with no Android-specific, packaging, build-system, or shared C++
    logic changes
- Continued Stage 7 preflight with the final desktop/manual frontend pass
  before Android packaging:
  - reviewed the runtime paths requested for Home, Schedule, Food, and Meals
    against the real QML/C++ bindings, with a real desktop process smoke check
    for app startup stability
  - fixed Food dish-search usability so `清空筛选` now clears both the text
    search and the merchant filter; previously it only cleared the merchant
    filter, leaving a hidden active text filter in place
  - fixed Meals dish-picker empty/search usability by adding an explicit
    `清空搜索` action when search text is active
  - fixed Meals save guardrail reachability by allowing the save button to be
    clicked with no selected dishes, so the existing manager error
    `请至少给这餐添加一道菜。` can actually appear in the page error area
  - no schema, recommendation scoring, C++ behavior, Stage 6 parser behavior,
    LLM capability scope, Android, or packaging changes were made
- Verification for the final desktop/manual preflight pass:
  - `cmake --build build\desktop-debug --target MealAdvisor` passed
  - real desktop smoke launch passed; `MealAdvisor.exe` stayed alive for
    `5` seconds
  - `MealAdvisorValidation.exe` remained `36/38`; the same two known
    non-blocking validation failures remain
  - validation coverage in this pass was mixed: app launch was a real runtime
    desktop process, the click-path findings were static/runtime-binding review
    due the lack of direct GUI click automation in this shell, and
    `MealAdvisorValidation` was the existing local mock/offline regression
    target
  - Android APK packaging was not run because this pass only touched QML/docs,
    with no Android-specific, packaging, build-system, or shared C++ logic
    changes
- Continued Stage 7 with the first Android packaging pass:
  - inspected the existing `build\android-arm64-debug` cache before changing
    build files; it was configured for Qt `6.10.3`, `arm64-v8a`,
    `android-35`, Ninja, JDK `21`, SDK build-tools `36.0.0`, and NDK
    `27.2.12479018`
  - reproduced an Android-only compile blocker where NDK 27 libc++ wrappers
    failed after raw `sysroot/usr/include` appeared before
    `sysroot/usr/include/c++/v1`
  - fixed the blocker in `CMakeLists.txt` by adding an Android-only
    `SYSTEM BEFORE` include for the active NDK libc++ header directory on the
    `MealAdvisor` target
  - built the arm64 debug APK successfully:
    `build/android-arm64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk`
  - verified APK metadata with `aapt`: package
    `org.qtproject.example.MealAdvisor`, `minSdkVersion 28`,
    `targetSdkVersion 36`, native-code `arm64-v8a`
  - checked `adb devices -l`; no real device or emulator was attached
  - checked local AVD availability; no configured AVD was listed, so
    install/launch and touch-path validation were skipped
- Verification after the Android packaging fix:
  - `cmake --build build\desktop-debug --target MealAdvisor` passed after the
    CMake change
  - `MealAdvisorValidation.exe` remained `36/38`; the same two known
    non-blocking validation failures remain
  - validation coverage in this pass was real Android APK packaging plus
    static APK metadata inspection, desktop build regression, and local
    mock/offline validation; Android runtime interaction was skipped because no
    runnable device/emulator was available
- Continued Stage 7 with the chat-style frontend refactor:
  - rewrote `app/qml/Main.qml` from visible top `TabBar` + `SwipeView` into a
    chat-style daily-use shell with `需求推荐` and `饭后反馈` mode buttons, a
    chat/result area, and a bottom multiline composer
  - recommendation mode now sends non-empty composer text through the existing
    LLM supplement parser first, then automatically runs the existing local
    recommendation engine when parsing finishes or falls back
  - feedback mode selects from existing `mealLogManager.recentMeals`, exposes
    existing structured 1-5 feedback fields, and saves through
    `mealLogManager.saveMealFeedback(...)`; no LLM feedback parser or schema
    change was added
  - moved LLM config/debug, `SchedulePage`, `FoodPage`, and `MealLogPage` into
    a right-side Drawer with half-width-preferred / 360px-minimum behavior and
    full-width fallback on narrow windows
  - applied a light frosted-glass visual direction with translucent panels,
    readable contrast, and restrained green/neutral accents
- Verification after the chat-style frontend refactor:
  - desktop build passed:
    `cmake --build build\desktop-debug --target MealAdvisor`
  - real desktop smoke launch passed; `MealAdvisor.exe` stayed alive for
    `5` seconds and emitted no captured QML stderr
  - `MealAdvisorValidation.exe` remained `36/38`; the same two known
    non-blocking validation failures remain
  - Android APK packaging and runtime validation were not rerun in this pass
    because the change was QML-only and desktop/runtime smoke was the targeted
    regression path
- Completed the pre-APK-build gate after the chat-style frontend refactor:
  - confirmed the desktop target is up to date:
    `cmake --build build\desktop-debug --target MealAdvisor`
    returned `ninja: no work to do`
  - real desktop smoke launch passed again; `MealAdvisor.exe` stayed alive for
    `5` seconds and emitted no captured QML stderr
  - `MealAdvisorValidation.exe` remained `36/38`; the same two known
    non-blocking validation failures remain
  - inspected the existing Android arm64 build cache: `arm64-v8a`,
    `android-35`, Ninja, Qt host path `C:/Qt/6.10.3/mingw_64`
  - confirmed the previously built APK is still present at
    `build/android-arm64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk`
    with timestamp `2026-04-25 13:17:16`
  - no APK rebuild was run in this gate; the next step can start the Android
    arm64 APK build/rebuild directly
- Built and refreshed the Android arm64 debug APK after the chat-style
  frontend refactor:
  - ran `cmake --build build\android-arm64-debug --target MealAdvisor_make_apk`
    successfully
  - generated APK:
    `build/android-arm64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk`
  - APK size is `66326811` bytes and timestamp is `2026-04-25 18:28:58`
  - `aapt dump badging` confirmed package
    `org.qtproject.example.MealAdvisor`, `minSdkVersion 28`,
    `targetSdkVersion 36`, and native-code `arm64-v8a`
  - copied the generated APK over the desktop copy:
    `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`
  - source and desktop APK SHA256 both match:
    `F6B3FFBAC16B21B3BC4A3F27128E60E1BCE4FBE12A2B3B05DE11CAF7644B8223`
- New Android screenshot/user feedback after the APK refresh:
  - current QML layout is too crowded on mobile/landscape
  - right Drawer content overlaps in the LLM debug section, especially summary
    text and action buttons
  - Drawer section buttons are not visually aligned or consistently sized
  - the main page should be simplified and no longer expose 饭后反馈 as a
    primary mode beside 需求推荐
  - next frontend pass should move structured 饭后反馈 into the side Drawer,
    keep the main page recommendation-first, and fix Drawer scrolling/wrapping
    before the next APK build
- Completed the follow-up Stage 7 QML layout refinement requested by the
  Android screenshot feedback:
  - kept the main page recommendation-first by hiding the visible two-mode
    main-page switch and always loading the recommendation component in the
    main result area
  - moved the structured recent-meal `饭后反馈` editor into the right Drawer as
    its own section, while keeping `MealLogPage` available separately as
    `反馈与记录`
  - changed Drawer section navigation to fixed-height grid buttons and changed
    the LLM debug action row to a responsive grid so summary text and actions
    no longer rely on a cramped free-flow row
  - adjusted Drawer width rules so compact or short mobile/landscape surfaces
    use an almost-full-width Drawer, while wider desktop surfaces keep a
    half-width-style Drawer
  - no schema, C++ recommendation logic, Stage 6 parser behavior, or feedback
    parser capability was changed
- Verification after the QML layout refinement:
  - desktop build passed:
    `cmake --build build\desktop-debug --target MealAdvisor`
  - short desktop smoke launch passed; `MealAdvisor.exe` stayed alive for
    `5` seconds and emitted no captured QML stderr
  - an initial parallel validation/smoke attempt hit a transient SQLite
    `database is locked`; rerunning validation alone restored the existing
    `36/38` baseline with only the two known non-blocking failures
  - rebuilt Android arm64 debug APK successfully with
    `cmake --build build\android-arm64-debug --target MealAdvisor_make_apk`
  - copied the generated APK to
    `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`
  - `aapt dump badging` confirmed package
    `org.qtproject.example.MealAdvisor`, `minSdkVersion 28`,
    `targetSdkVersion 36`, and native-code `arm64-v8a`
  - source and desktop APK SHA256 both match:
    `248246B69DA58594E1732A04B171E798847B06E04B2A0F8719295B747B2F4958`
  - `adb devices -l` still showed no attached device, so Android install,
    launch, touch, keyboard, and scrolling validation remain pending
- New Android screenshot feedback after the recommendation-first Drawer APK:
  - the top-right Drawer button does not render correctly on Android; the
    current QML uses a damaged text glyph (`鈽?`) as the ToolButton label, so
    Android shows an unsupported / broken symbol instead of a clear menu icon
  - the main page still exposes too much supplement-parser UI: the header
    status, the `未配置` chip, `清空补充`, and `当前没有补充说明` make the page feel
    like a debug/parser surface instead of a simple meal recommendation entry
  - the empty recommendation preview is duplicated visually: the same
    “点击生成推荐...” guidance appears in the main recommendation card and again
    in a separate empty-state card
  - the LLM debug Drawer still has overlapping text and controls; likely causes
    are the long `appConfig.llmConfigSummary` / `recommendationEngine`
    supplement labels staying in the same auto-height card as TextFields and
    action buttons, plus Android text wrapping / implicit-height calculation
    not expanding the card enough at the current Drawer width
  - the Drawer feedback section also shows stacked text/control overlap near
    the meal selector and checkbox area, suggesting the moved
    `feedbackComponent` should not be embedded through a bare Loader inside a
    ScrollView without a stronger content-width/implicit-height wrapper
  - the next pass should be QML-only and should remove visible “补充说明”
    surfaces from the main page, replace the broken top-right text glyph with a
    robust menu affordance, and harden Drawer card/layout heights against
    Android wrapping
- Completed the targeted QML-only Android layout fix from that screenshot
  feedback:
  - replaced the top-right Drawer text glyph with a QML-drawn three-line menu
    button, and replaced the Drawer close text glyph with a QML-drawn close
    icon so Android rendering no longer depends on font glyph support
  - removed visible supplement-parser debugging UI from the main page: the
    header now uses app planning/budget status, the main recommendation card no
    longer shows parser state chips, `清空补充`, supplement summary text, or
    supplement weight chips, and the duplicate empty-state card was removed
  - kept the bottom composer and existing parser-then-local-recommendation path
    wired internally, but changed the busy button label to a neutral `处理中`
  - split Drawer LLM debug into separate status/config and form cards, removed
    nonessential supplement status text from that Drawer panel, and gave
    TextFields/actions explicit stable heights
  - hardened the Drawer feedback layout with explicit ComboBox/CheckBox/button
    heights, one-column fallback for narrow score controls, and a real
    ScrollView content wrapper instead of a bare Loader
  - no C++, schema, recommendation scoring, Stage 6 parser contract, feedback
    parser, OCR, rerank, or dish enrichment work was changed
- Verification after the targeted QML-only fix:
  - `git diff --check -- app/qml/Main.qml` passed
  - desktop build passed:
    `cmake --build build\desktop-debug --target MealAdvisor`
  - `MealAdvisorValidation.exe` remains `36/38`; the only failures are still
    the two known non-blocking baseline cases
  - 5-second desktop smoke launch passed; the process stayed alive and captured
    stderr was empty
  - rebuilt Android arm64 debug APK successfully with
    `cmake --build build\android-arm64-debug --target MealAdvisor_make_apk`
  - copied the generated APK to
    `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`
  - source and desktop APK SHA256 both match:
    `B66CD47CCEC29ED62FFCF973A586AF46F934B0AC99BC5CBFFD432B392D5B7F6F`
  - APK size is `66347675` bytes
  - `aapt dump badging` confirmed package
    `org.qtproject.example.MealAdvisor`, `minSdkVersion 28`,
    `targetSdkVersion 36`, and native-code `arm64-v8a`
  - the actual SDK path used for this check is
    `C:\Users\Administrator\AppData\Local\Android\Sdk`
  - `adb devices -l` still showed no attached devices, so Android install,
    launch, touch, keyboard, and scrolling validation remain pending
- New user screenshot feedback and product-direction request:
  - In the Drawer `餐食配置` / FoodPage dish form, the visible repeated `低`
    values are unlabeled level selectors. The current code order is:
    `carbBox`, `fatBox`, `proteinBox`, `vitaminBox`, `fiberBox`,
    `satietyBox`, `burdenBox`, `sleepinessBox`, `flavorBox`, and `odorBox`.
    They mean 碳水、脂肪、蛋白、维生素、纤维、饱腹、消化负担、犯困风险、口味、
    气味, each using the existing `low / medium / high` enum shown as
    `低 / 中 / 高`.
  - The screenshot makes clear that showing only `低` is not readable enough:
    the user cannot tell which food attribute each selector controls, and the
    semantics differ by field (`蛋白高` can be good, while `犯困风险高` is bad).
  - The next UI pass should wrap each level selector in a labeled field, add a
    short section title such as `菜品标签（低/中/高）`, keep the stored enum values
    unchanged, and use Android-safe one-column / two-column responsive layout
    so labels and controls never overlap.
  - Numeric fields around the same form should also become clearer where
    possible: for example `获取成本（1 容易 - 3 费事）` and
    `餐次影响权重（普通餐 1.0，饮料/加餐可更低）`.
  - The user also requested a behavior change for `饭后反馈`: feedback should
    prefer natural-language LLM input first, and if the LLM cannot connect or
    cannot return a valid parse, the app should fall back to concrete manual
    score controls.
  - This is a new planned capability beyond the previously avoided
    feedback-parser scope. It should still preserve the local-first boundary:
    no recommendation-core rewrite, no schema change unless a blocker is found,
    no LLM rerank, and existing `meal_feedback` fields remain the persistence
    target.
  - Planned feedback flow:
    1. User selects a recent meal.
    2. Primary input is a free-text feedback TextArea.
    3. Primary action tries an OpenAI-compatible strict-JSON feedback parser
       using existing `AppConfig` LLM settings.
    4. On parser success, map the structured result into existing feedback
       fields: fullness, sleepiness, comfort, focus, would-eat-again, taste,
       repeat willingness, and free text; then save through the existing
       feedback save path.
    5. On unconfigured API, network failure, timeout, auth failure, or invalid
       JSON/contract, do not silently save neutral defaults; reveal the manual
       score controls and keep the user's text so they can save concrete
       scores.
    6. Keep an explicit `手动打分` path available even when LLM is configured.
  - Suggested feedback parser contract for the implementation prompt:
    `feedback_parser_v1`, strict JSON only, fixed score values `1..5`,
    `wouldEatAgain` boolean, optional concise normalized feedback text, and
    neutral/manual fallback on any contract violation.
  - This planned pass has now been implemented without changing schema,
    recommendation-core scoring, LLM rerank behavior, OCR, or dish enrichment:
    FoodPage wraps every dish level selector in a labeled field, adds
    `菜品标签（低 / 中 / 高）` helper copy, and clarifies nearby numeric placeholders.
  - Drawer `饭后反馈` now uses free-text feedback as the primary path. The primary
    action calls a strict OpenAI-compatible `feedback_parser_v1` parser through
    existing `AppConfig` settings. Successful parses save through the existing
    `mealLogManager.saveMealFeedback(...)` path; unconfigured API, network
    failure, timeout, non-JSON, missing fields, invalid scores, or invalid
    booleans reveal the manual 1-5 scoring controls instead of saving neutral
    defaults.
  - A small follow-up screenshot pass fixed the main-page bottom composer
    contrast by giving the TextArea a clear white field/background and using
    the local readable button wrapper for the send button.
  - Desktop visual checks were captured under `build/screenshots/` for the
    main page, LLM Drawer, Food Drawer, Food tag section, and Feedback Drawer.
    The inspected states did not show obvious app-side overlap or bare
    unlabeled Food level selectors; one Feedback screenshot contained an
    unrelated external GamePP overlay.
  - Verification after this pass:
    - `git diff --check` passed for the touched code/docs files
    - desktop build target `MealAdvisor` and validation target
      `MealAdvisorValidation` are up to date
    - `MealAdvisorValidation.exe` reports `46/48`; all new feedback-parser
      cases pass, and the only failures are still the two known non-blocking
      baseline cases
    - 5-second desktop smoke launch passed with empty captured stderr
    - Android arm64 debug APK rebuilt and copied to
      `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`
    - source and desktop APK SHA256 both match:
      `F12ED5FA86AF39AE2D3D8C7218FE0EF4CCB2EB6F36144B6CF515D5A966F785B4`
    - APK size is `66399035` bytes, timestamp `2026-04-25 23:51:55`
    - `aapt dump badging` confirmed package
      `org.qtproject.example.MealAdvisor`, `minSdkVersion 28`,
      `targetSdkVersion 36`, and native-code `arm64-v8a`
    - `adb devices -l` showed no attached device, and
      `emulator -list-avds` returned no configured AVD, so Android install,
      launch, touch, keyboard, and screenshot validation remain blocked.
  - A follow-up Test Android Apps pass using the `android-emulator-qa` flow was
    run after the plugin was available. The environment state was unchanged:
    `adb devices -l` returned only the device-list header, and
    `emulator -list-avds` returned no AVD names. The APK was not installed or
    launched. Source and desktop APK SHA256 were rechecked and still match
    `F12ED5FA86AF39AE2D3D8C7218FE0EF4CCB2EB6F36144B6CF515D5A966F785B4`;
    `aapt dump badging` still reports package
    `org.qtproject.example.MealAdvisor`,
    `minSdkVersion 28`, `targetSdkVersion 36`, launchable activity
    `org.qtproject.qt.android.bindings.QtActivity`, and native-code
    `arm64-v8a`.
  - A targeted `Main.qml` visual polish pass then addressed the supplied mobile
    screenshot:
    - hid the header planning/budget summary line under `MealAdvisor`
    - replaced the bottom composer with a padded `StyledTextArea` so the hint
      stays inside the input instead of touching the border or overlapping the
      cursor
    - widened and restyled the send button as a rectangular primary action so
      the label is not ellipsized
    - moved the main shell, Drawer buttons, cards, text fields, and text areas
      from the green palette to a Claude-like beige / brown-yellow palette with
      more consistent radii and borders
    - Desktop build passed for `MealAdvisor` and `MealAdvisorValidation`
    - A first validation attempt was blocked by a concurrent smoke process
      locking SQLite; the validation was rerun serially and returned `46/48`
      with only the two known non-blocking baseline failures
    - 5-second desktop smoke launch passed with empty stderr
    - Android arm64 debug APK rebuilt and copied to
      `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`
    - source and desktop APK SHA256 both match
      `F68DC057BFE10E5F736AF2A69E58E4A5C5FDD1F48833A460FBCABF877936F618`
    - APK size is `66402219` bytes, timestamp `2026-04-26 00:31`
    - `aapt dump badging` still reports package
      `org.qtproject.example.MealAdvisor`, `minSdkVersion 28`,
      `targetSdkVersion 36`, launchable activity
      `org.qtproject.qt.android.bindings.QtActivity`, and native-code
      `arm64-v8a`
    - `adb devices -l` still returned no attached devices and
      `emulator -list-avds` still returned no configured AVD names, so Android
      install/launch/touch/keyboard/screenshot validation remains blocked
    - A desktop window screenshot attempt captured the desktop background
      instead of the Qt window, so it was deleted and not treated as visual
      evidence.
  - Added the generated launcher icon to the resource library and Android
    package:
    - source icon saved at `resources/icons/mealadvisor-launcher-source.png`
    - Android mipmap launcher PNGs generated at
      `app/android/res/mipmap-mdpi/ic_launcher.png`,
      `mipmap-hdpi`, `mipmap-xhdpi`, `mipmap-xxhdpi`, and `mipmap-xxxhdpi`
    - added `app/android/AndroidManifest.xml` with
      `android:icon="@mipmap/ic_launcher"` on the application and activity
    - set `QT_ANDROID_PACKAGE_SOURCE_DIR` and `QT_ANDROID_APP_ICON` on the
      Android `MealAdvisor` target in `CMakeLists.txt`
    - set Qt CMake policy `QTP0002` to `NEW` to avoid the Android deployment
      path dev warning
    - Android arm64 debug APK rebuilt and copied to
      `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`
    - source and desktop APK SHA256 both match
      `900BAE252B06BA94DC141D6514A043122E2270FB9E0BCBB645EABF08FBA6A321`
    - APK size is `66402625` bytes, timestamp `2026-04-26 01:16`
    - `aapt dump badging` now reports `application-icon-160`,
      `application-icon-240`, `application-icon-320`, `application-icon-480`,
      and `application-icon-640`, with application and launchable activity
      icon paths resolving to `res/mipmap-*-v4/ic_launcher.png`
    - Desktop build target still configures/builds, and
      `MealAdvisorValidation.exe` remains `46/48` with only the same two known
      non-blocking baseline failures.
  - A targeted input / LLM debug pass then addressed the latest Android
    screenshot feedback:
    - `Main.qml` styled text fields/areas now use `hintText`, while every
      direct `placeholderText` binding in `FoodPage.qml`, `SchedulePage.qml`,
      and `MealLogPage.qml` now returns an empty string whenever the control is
      focused or already has text. This prevents the Android/Material floating
      hint behavior across the app.
    - Drawer `LLM 调试` now includes `测试连接`; it tests the current API
      Key / URL / Model fields without forcing a save first, while blank fields
      still fall back through the existing `AppConfig` / environment-variable
      chain.
    - `RecommendationEngine` now exposes `llmConnectionTestState`,
      `llmConnectionTestStatus`, and `llmConnectionTestBusy`, plus an
      OpenAI-compatible lightweight Chat Completions test request with a
      10-second timeout.
    - `MealAdvisorValidation.exe` now includes two mock-server tests for the
      connection test request and response path. The latest run reports
      `48/50`; the only failures remain the two known non-blocking baseline
      cases: non-LLM relaxed high-budget dinner does not lift hotpot into
      top-3, and sparse seeded feedback does not emit
      `sleepiness_watch` / `stable_favorites` / `low_repeat`.
    - Desktop build target `MealAdvisor` and validation target
      `MealAdvisorValidation` both build successfully.
    - 5-second desktop smoke launch passed with empty stderr.
    - Android arm64 debug APK rebuilt and copied to
      `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`.
    - Source and desktop APK SHA256 both match:
      `99C45E2FE1AC1B4E894CBB43711AB2BD6B470A20C641DA5DDD12E3AE655BF19B`.
    - APK size is `66436201` bytes, timestamp `2026-04-26 01:30:04`.
    - `aapt dump badging` still reports package
      `org.qtproject.example.MealAdvisor`, `minSdkVersion 28`,
      `targetSdkVersion 36`, launcher icons under `res/mipmap-*-v4`,
      launchable activity `org.qtproject.qt.android.bindings.QtActivity`, and
      native-code `arm64-v8a`.
    - `adb devices -l` still returned no attached device, and
      `emulator -list-avds` still returned no configured AVD names, so Android
      install/launch/touch/keyboard/screenshot validation remains blocked.
  - A targeted Android TLS fix then addressed the real-device screenshot where
    Drawer `LLM 调试` failed with `TLS initialization failed`:
    - `CMakeLists.txt` now uses `FetchContent` to pull pinned
      `KDAB/android_openssl` commit
      `b71f1470962019bd89534a2919f5925f93bc5779` for Android builds and calls
      `add_android_openssl_libraries(MealAdvisor)`.
    - `src/main.cpp` sets `ANDROID_OPENSSL_SUFFIX=_3` before constructing
      `QGuiApplication` and logs `QSslSocket::supportsSsl()` / SSL library
      versions on startup.
    - Desktop build target `MealAdvisor` and validation target
      `MealAdvisorValidation` both build successfully.
    - `MealAdvisorValidation.exe` still reports `48/50` with only the same two
      known non-blocking baseline failures.
    - 5-second desktop smoke launch passed with empty stderr.
    - Android arm64 debug APK rebuilt and copied to
      `C:\Users\Administrator\Desktop\MealAdvisor-arm64-debug.apk`.
    - Source and desktop APK SHA256 both match:
      `6157107CD27478ECC716CC630378465B12E7B885D30A96906EF0768F701500F2`.
    - APK size is `68925434` bytes, timestamp `2026-04-26 01:53:20`.
    - `aapt list` confirms the APK now includes
      `lib/arm64-v8a/libcrypto_3.so`,
      `lib/arm64-v8a/libssl_3.so`, and
      `lib/arm64-v8a/libplugins_tls_qopensslbackend_arm64-v8a.so`.
    - `aapt dump badging` still reports package
      `org.qtproject.example.MealAdvisor`, `minSdkVersion 28`,
      `targetSdkVersion 36`, launcher icons under `res/mipmap-*-v4`,
      launchable activity `org.qtproject.qt.android.bindings.QtActivity`, and
      native-code `arm64-v8a`.
    - `adb devices -l` and `emulator -list-avds` are still empty in this shell,
      so the fix has packaging evidence but still needs a real-device retest of
      Drawer `LLM 调试` -> `测试连接`.

## Next Steps

Priority note:

- Stage 1-6 can be treated as sealed for Stage 7 preflight. Do not keep
  expanding Stage 6 or the recommendation core unless a new real regression is
  found.

Immediate next work:

- Attach a real Android device or create an AVD, install the latest APK, and
  first re-test Drawer `LLM 调试` -> `测试连接` against DeepSeek. The previous
  real-device failure was `TLS initialization failed`; the APK now packages the
  Android OpenSSL backend and should be rechecked before doing broader UI QA.
  Then validate launcher icon, updated beige main shell, placeholder hiding
  while typing, Food labels, and Feedback LLM/manual fallback with screenshots,
  UI-tree inspection, keyboard entry, scrolling, and portrait/landscape checks.

1. Run real Android runtime validation on an attached device or newly
   configured AVD. Use adb/UI-tree-derived coordinates, screenshots, and
   logcat if anything crashes.
2. Focus the Android pass on the newly changed surfaces: Drawer `LLM 调试`
   should no longer fail with `TLS initialization failed`; if it still fails,
   capture logcat lines around `Device supports SSL:` / `qt.network.ssl`.
   The main header should
   no longer show the planning/budget line; placeholder hints should disappear
   while any input is focused or populated instead of floating above the field;
   Drawer `LLM 调试` should show a usable `测试连接` status; Food tags must show
   labels rather than bare `低 / 中 / 高`; and Drawer `饭后反馈` must support both
   LLM parse/save and manual fallback/save. Also confirm Android launcher and
   recents icons show the new meal-bowl icon instead of a blank/default icon.
3. Keep any follow-up fix QML-only and tightly scoped unless Android evidence
   reveals a real C++ or persistence bug.
4. Keep the feedback parser local-first and bounded: strict JSON parser only,
   existing `AppConfig` settings, existing `meal_feedback` persistence, no
   recommendation-core rewrite, no LLM rerank, and no schema change unless a
   concrete blocker is found.
5. Keep the two known validation failures classified correctly:
   the non-LLM high-budget dinner case is a local recommendation/context
   limitation, and the missing `sleepiness_watch` / `stable_favorites` /
   `low_repeat` insight case is sparse-data heuristic behavior.
6. If a real provider key is available in a later window, optionally run one
   live Drawer feedback parse sample, but do not block Stage 7 on provider
   replay unless the strict parser contract regresses.
7. Continue avoiding dish enrichment, OCR, schema redesign, LLM rerank, or
   recommendation-core rewrite until Stage 7 basics are stable.

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
- Home / Food / Schedule / Meals now have multiple Stage 7 frontend cleanup
  passes covering high-risk copy, enum display, card height, button readability,
  lower-page narrow-width wrapping, Food/Menus search clearing, and Meals save
  guardrail reachability. Remaining text/layout work should be targeted
  Android/runtime confirmation, not another broad sweep.
- Android SDK / NDK / Qt environment is aligned locally and the arm64 APK build
  has now been re-verified after the portability and packaging fixes. The
  current APK path is
  `build/android-arm64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk`.
  The build currently depends on an Android-only CMake workaround that puts
  NDK libc++ headers before the raw NDK sysroot include path.
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
  but the latest desktop preflight still did not have direct GUI click
  automation available in this shell; current confidence comes from QML/C++
  path review, real desktop process smoke launch, local mock/offline
  validation, and prior real provider responses validated through the C++
  parser.
- The Stage 6 parser contract and final local mapping pass are now implemented.
  Budget-relax intent has visible downstream effect through the real parser
  path; cola/drink intent is mapped locally, but current seed data still lacks
  actual cola / beverage candidates, so richer drink behavior is a data or
  future add-on-candidate issue rather than a parser blocker.
- The new Meals quick-template path plus the latest search-order / guardrail
  pass still need a longer real-data usage session and Android touch
  interaction to confirm they feel better in practice rather than only
  compiling cleanly.
- The fourth and final desktop Stage 7 passes reduced obvious desktop
  narrow-width and empty/search/save-path risks, but they are still not a
  substitute for a real Android touch pass with native screen sizing,
  keyboard entry, and scrolling.
- Android install/launch validation is still blocked because `adb devices -l`
  shows no attached device and `emulator -list-avds` returns no configured AVD
  in this shell. The latest APK has been rebuilt, inspected with `aapt`, and
  copied to the desktop, so the next confidence step is a real device or newly
  configured AVD, not another packaging-only pass.
- The simplified recommendation-first shell and Drawer feedback move have
  build-level, validation-target, screenshot, and process-start smoke coverage.
  The beige main shell polish, bottom composer overlap fix, new Food labels,
  and LLM-first feedback flow still need real Android click/touch validation
  for Drawer open/close, section switching, recommendation send, feedback
  parse/manual fallback save, keyboard behavior, and narrow-screen scrolling.
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
