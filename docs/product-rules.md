# Product Rules

## Core Goal

The app helps decide what to eat next.

Primary decision priorities:

1. On class days, prefer food that is less likely to cause sleepiness
2. On class days, prefer lower burden and shorter meal cost in time/effort
3. On non-class planned days, prefer health first while staying within budget

## Planning Scope

- Active planning days: Tuesday to Friday
- Commute days are included in planning
- Saturday to Monday are not actively planned by default

## Breakfast Rules

- Breakfast is logged but not actively recommended by default
- Typical breakfast may be:
  - banana
  - chocolate biscuits
  - McDonald's breakfast such as McMuffin
- Simple self-prepared breakfast may be recorded as `0 RMB`

## Budget Rules

- Normal daily budget: `80 RMB`
- Flexible upper range: `100+ RMB`
- Budget is not the highest-priority factor, but it should still be tracked

## Personal Preference and Health Constraints

- User is a 20-year-old male
- BMI is around `25.4`
- User is overweight and relatively sensitive to high-carb meals
- Raw family medical history must not be stored in the app
- Health preference should be represented through indirect scoring, not medical history fields

## Schedule Rules

- Class schedule is one of the main recommendation inputs
- Wednesday has late evening classes in periods `11-12`
- Meal timing assumptions:
  - breakfast before morning classes
  - lunch after `12:00` if afternoon classes start early
  - lunch after `13:00` if afternoon classes start later
  - dinner around `18:00` on evening-class days before class
  - dinner after `20:30` on evening-class days after class
  - dinner around `19:00` on days without evening classes

## Merchant Rules

Merchant fields should focus on:

- price level
- dine-in / takeaway / delivery availability
- delivery ETA
- distance / route cost
- queue time
- availability notes

Availability notes should cover things like:

- KFC Thursday promotions
- seasonal dishes
- easy stock-out dishes
- limited-time availability

Business hours are not part of the current scope.

## Dish Rules

Dish modeling rules:

- One dish belongs to one merchant only
- Nutrition indicators use 3 levels only:
  - `low`
  - `medium`
  - `high`
- GI is not stored as a separate field for now; it is folded into carb judgment

Current dish indicators:

- carb level
- fat level
- protein level
- vitamin level
- fiber level
- satiety level
- digestive burden level
- sleepiness risk level
- flavor level
- odor level

Notes may still store free-form details such as:

- strong smell in dorm
- heavy seasoning
- whether the smell affects sleep

## Combo and Add-on Rules

- Combo meals should be split into separate dish records when possible
- One meal can contain multiple dishes
- Drinks and snacks such as milk tea or fruit tea are recorded as separate dish items
- Small add-ons should use a lower `meal_impact_weight`
- Default `meal_impact_weight` is `1.0`

## Recommendation Output Rules

- Output 3 ranked recommendations
- Each recommendation should include a reason
- Ranking should come from weighted multi-factor scoring

## Feedback Rules

Post-meal feedback is supported and should later affect recommendation weights.

Useful feedback signals include:

- fullness
- sleepiness
- comfort
- focus impact
- whether the user would eat it again

## LLM Rules

The app remains local-first and rule-first.

Deepseek or another LLM should be used as an enhancement layer for:

- turning natural-language dish input into structured tags
- turning natural-language feedback into structured adjustments
- optionally helping reorder candidates if rule-only ranking is weak

LLM must not be the only decision source for core recommendation logic.
