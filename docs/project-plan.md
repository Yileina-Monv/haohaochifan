# Project Plan

## Product Direction

The app solves one concrete problem: deciding what to eat next with enough
context to reduce decision fatigue.

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
- Add LLM support only as a secondary analysis layer
