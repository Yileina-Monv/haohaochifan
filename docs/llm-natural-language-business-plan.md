# LLM Natural-Language Business Plan

Last updated: `2026-04-26`

## Goal

Add a small natural-language layer on top of the existing deterministic
MealAdvisor core. The main page should stay recommendation-first and visually
simple, but it should expose a mode switch that lets the user talk to the app
for four concrete workflows:

1. `下一餐吃什么` and why.
2. Feedback on the previous meal.
3. Natural-language dish import from dishes the user sees.
4. Temporary daily routine import.

The LLM must not become the source of truth for scoring, persistence, or data
integrity. It should parse, summarize, and prepare actions. C++ managers and
repositories should validate, preview, and execute those actions.

## Current LLM Surface

The current LLM layer uses an OpenAI Chat Completions-compatible endpoint:

- Config resolution:
  - in-app API key / URL / model first
  - `MEALADVISOR_LLM_API_KEY`, `MEALADVISOR_LLM_API_URL`,
    `MEALADVISOR_LLM_MODEL`
  - `OPENAI_API_KEY` for key fallback
  - default URL `https://api.openai.com/v1/chat/completions`
  - default model `gpt-4o-mini`
- Existing calls:
  - `testLlmConnection(...)`
  - `parseSupplement(text)`
  - `parseFeedback(text, mealSummary)`
  - `parseDishInput(text, merchantName)`
- Existing contract style:
  - strict JSON object
  - local schema validation
  - neutral/manual fallback on invalid response, network failure, timeout, or
    missing config

## Non-LLM Changes Required

### 1. Unified Natural-Language Task Contract

Introduce a local contract that all natural-language workflows share. The LLM
should return an intent plus validated payloads, not arbitrary UI text.

Recommended concepts:

- `intent`
  - `recommend_next_meal`
  - `record_previous_meal_feedback`
  - `import_dishes`
  - `import_temporary_routine`
- `summary`
  - short human-readable Chinese result for the preview panel
- `actions`
  - structured list of proposed changes
- `requiresConfirmation`
  - true for any write action
- `missingFields`
  - fields the UI should ask the user to clarify
- `confidence`
  - local display/gating only; not a scoring shortcut

The app should never persist LLM output directly. All write actions must go
through existing or new manager methods.

### 2. Main-Page Action Panel State Machine

Keep the main page clean, but add a compact mode switch and one shared input
surface:

- Mode switch:
  - `推荐`
  - `反馈`
  - `菜品`
  - `日常`
- Shared input box.
- Parse/send button.
- Result preview area.
- Confirm/apply button when the result writes data.
- Failure/manual fallback path.

Suggested UI states:

- `idle`
- `parsing`
- `preview`
- `needs_clarification`
- `applying`
- `applied`
- `failed`

### 3. Recommendation Context Export

For `下一餐吃什么&推荐理由`, the LLM should not replace local scoring. Add a
non-LLM method that exposes a bounded recommendation snapshot:

- next meal type and target time
- class/routine pressure
- current supplement summary
- top candidates from local `runDecision()`
- local scores, warnings, and reasons
- recent meal/dish context
- known feedback signals where available

LLM output should explain and ask follow-up questions if useful. The local
engine remains the final scorer.

### 4. Previous-Meal Feedback Targeting

For `对上一餐有什么反馈`, add or expose manager methods that identify the
feedback target before calling the LLM:

- latest meal
- latest meal without feedback
- meal summary string
- existing feedback state

The preview must clearly show which meal will receive the feedback. The actual
save should continue through `mealLogManager.saveMealFeedback(...)`.

### 5. Dish Import Preview Transaction

The current single-dish parse fills the form. For natural-language import, add
a batch/import transaction layer:

- parse one or more dishes
- optionally parse/create a merchant candidate
- detect duplicate merchants and dishes
- normalize categories, dining modes, numeric ranges, and levels
- show a preview list
- allow the user to accept/reject individual items
- apply accepted items through `FoodManager` methods

This should be separate from directly saving LLM output.

### 6. Temporary Routine Data Model

`导入临时日常` should not be forced into the permanent class schedule. Add a
temporary routine/event layer that can affect recommendation context:

- table/model: `temporary_events` or equivalent
- fields:
  - title
  - event type: class, exam, meeting, commute, sport, sleep, social, other
  - date
  - start time
  - end time
  - location / zone
  - intensity
  - notes
  - expires/active flag if needed
- manager:
  - list active temporary events
  - create/update/delete event
  - expire old events
- recommendation integration:
  - merge formal schedule entries and active temporary events into meal
    context
  - let temporary events affect time pressure, location, and after-meal
    sleepiness constraints

### 7. Validation and Safety

Every LLM action must have a deterministic validation path:

- strict schema validation
- range validation
- enum validation
- duplicate detection for import workflows
- confirmation before writes
- clear manual fallback
- no API key logging
- no silent neutral saves for failed parser output

## Suggested Implementation Phases

### Phase A: Contract and Shell

- Add `NaturalLanguageTask` or equivalent C++ service/manager.
- Add main-page mode switch and shared text input.
- Route mode-specific calls through the new service.
- Implement only preview/no-op wiring first if necessary.

### Phase B: Next-Meal Conversation

- Export local recommendation snapshot.
- Let LLM produce a conversational explanation based on local top candidates.
- Do not allow LLM rerank to override the local top choice in this phase.

### Phase C: Previous-Meal Feedback

- Add latest/unfeedbacked meal target selection.
- Reuse the existing `feedback_parser_v1` parser.
- Add a preview before saving parsed feedback from the main page.

### Phase D: Dish Import

- Extend single-dish parsing into batch `dish_import_v1`.
- Add import preview and duplicate handling.
- Confirm then write via `FoodManager`.

### Phase E: Temporary Routine

- Add schema and repository/manager for temporary events.
- Add `temporary_routine_v1` parser.
- Merge active temporary events into recommendation context.
- Add confirm/apply UI from the main page.

## Boundaries

Do not do these as part of the first pass:

- Replace local recommendation scoring with LLM ranking.
- Let LLM directly write to SQLite.
- Add OCR/image parsing unless explicitly requested later.
- Add cloud sync or multi-user account behavior.
- Expand into calorie counting or full nutrition lookup.
