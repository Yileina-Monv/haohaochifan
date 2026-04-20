# MealAdvisor

MealAdvisor is a small Qt + C++ Android app for deciding what to eat based on
meal history, schedule pressure, dish preferences, and lightweight nutrition
tags.

## Current Scope

- Android-first app built with Qt Quick/QML and C++
- Local-first data model for dishes, meal logs, and schedule slots
- Rule-based recommendation engine as the default decision path
- Optional LLM enhancement layer for explanation and ranking later

## Project Structure

- `app/qml`: QML views and reusable UI components
- `src/core`: shared application state and domain objects
- `src/data`: persistence, repositories, and storage models
- `src/recommendation`: recommendation rules and scoring logic
- `src/services`: network and API integration layers
- `docs`: product and technical planning notes
- `data`: seed data and local development fixtures
- `resources`: icons and static assets
- `tests`: unit and integration tests

## First Milestone

Build an MVP that can:

1. Save dish metadata and nutrition tags
2. Record breakfast, lunch, and dinner
3. Store class schedule constraints
4. Recommend the next meal with clear reasons

## Next Step

Implement the local data model and database schema before connecting any API.
