# Project Plan

## Product Direction

The app solves one concrete problem: deciding what to eat next with enough
context to reduce decision fatigue.

The next product direction is a natural-language command layer on the main
page. It should keep the current simple recommendation-first shell, but add a
small mode switch for:

- next-meal recommendation explanation
- previous-meal feedback
- dish import
- temporary routine import

The long-term plan for this layer lives in
`docs/llm-natural-language-business-plan.md`.

## MVP Boundary

- Manual dish entry with lightweight nutrition tags
- Meal log for breakfast, lunch, and dinner
- Class schedule input
- Rule-based next-meal recommendation
- Clear explanation for each recommendation

## Deferred Until After MVP

- Automatic nutrition lookup
- Full calorie counting
- Advanced statistics dashboards
- Cloud sync
- Multi-user support

## Immediate Technical Decisions

- Use Qt Quick/QML for the Android UI
- Use C++ for domain logic and persistence
- Keep local recommendation logic deterministic
- Keep LLM support as a secondary parsing, summarization, and action-preview
  layer; local managers validate and execute writes
- Do not let LLM directly write SQLite or replace local recommendation scoring
