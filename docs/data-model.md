# Data Model Draft

## Known Product Assumptions

Based on the current discussion, the first version of the app should optimize
for:

- If there are classes, prefer food that is unlikely to cause sleepiness
- If there are classes, prefer shorter meal time and lower burden
- If there are no classes, prefer health first while staying within budget
- The user is overweight, has relatively high visceral fat, and is sensitive to
  high-carb meals
- The app should return 3 ranked recommendations with weighted reasons
- Post-meal feedback can be collected and used to adjust later suggestions
- LLM support should translate natural language dish input into structured tags,
  and may later help with ranking if rule-based sorting is not good enough
- Breakfast usually happens before morning classes
- Breakfast usually does not need active recommendation because time is tight
- Typical breakfast can be banana, chocolate biscuits, or a McDonald's breakfast
  set; simple self-prepared breakfast may be recorded as `0 RMB`
- Lunch is usually after 12:00 if afternoon classes start early, and after 13:00
  if afternoon classes start later
- Dinner is usually around 18:00 or after 20:30 on evening-class days, and
  around 19:00 on non-evening-class days
- Daily budget is usually around `80 RMB`, with occasional flexibility above
  `100 RMB`
- Planning should cover Tuesday to Friday only
- Commute days should still be planned
- Saturday to Monday should not be included in active planning
- Family history should not be stored as raw medical history in the app

## Domain Objects

### UserProfile

Stores user-level stable attributes.

- `id`
- `display_name`
- `weight_status`
- `visceral_fat_risk`
- `carb_sensitivity`
- `default_budget_min`
- `default_budget_max`
- `bmi_value`
- `age_years`
- `biological_sex`
- `created_at`
- `updated_at`

### RecommendationProfile

Represents adjustable decision goals. This is important because your goal is
dynamic: class days and free days should use different priorities.

- `id`
- `name`
- `has_class_priority`
- `health_priority`
- `budget_priority`
- `time_effort_priority`
- `satiety_priority`
- `carb_control_priority`
- `fat_control_priority`
- `protein_priority`
- `sleepiness_avoid_priority`
- `breakfast_recommendation_enabled`
- `enabled_weekdays`
- `planning_scope` (`class_day`, `commute_day`, `all_enabled_days`)
- `is_default`
- `created_at`
- `updated_at`

### PlanningPolicy

Stores global planning boundaries that are not tied to one specific dish.

- `id`
- `name`
- `enabled_weekdays`
- `include_commute_days`
- `skip_non_enabled_days`
- `default_daily_budget`
- `flexible_budget_cap`
- `breakfast_recommendation_enabled`
- `created_at`
- `updated_at`

### ClassPeriod

Stores the time definition for each period. The current screenshots suggest a
layout like:

- 1: `08:30-09:10`
- 2: `09:15-09:55`
- 3: `10:15-10:55`
- 4: `11:00-11:40`
- 5: `11:45-12:25`
- 6: `13:30-14:10`
- 7: `14:15-14:55`
- 8: `15:00-15:40`
- 9: `16:00-16:40`
- 10: `16:45-17:25`
- 11: `19:00-19:40`
- 12: `19:45-20:20`

Fields:

- `id`
- `period_index`
- `start_time`
- `end_time`
- `session_group` (`morning`, `afternoon`, `evening`)

### ScheduleEntry

Stores weekly class constraints.

- `id`
- `weekday`
- `period_start`
- `period_end`
- `course_name`
- `location`
- `campus_zone`
- `intensity_level`
- `notes`

### Merchant

Represents where the meal comes from.

- `id`
- `name`
- `campus_area`
- `distance_minutes`
- `queue_time_minutes`
- `supports_dine_in`
- `supports_takeaway`
- `supports_delivery`
- `price_level`
- `notes`

### Dish

Represents a selectable food item or meal set.

- `id`
- `name`
- `merchant_id`
- `category`
- `is_combo`
- `is_beverage`
- `price`
- `default_dining_mode`
- `eat_time_minutes`
- `acquire_effort_score`
- `carb_level`
- `fat_level`
- `protein_level`
- `fiber_level`
- `vegetable_level`
- `satiety_level`
- `digestive_burden_level`
- `sleepiness_risk_level`
- `health_score`
- `favorite_score`
- `is_active`
- `source_type` (`manual`, `llm_enriched`)
- `created_at`
- `updated_at`

### MealLog

Stores one actual meal event.

- `id`
- `meal_type` (`breakfast`, `lunch`, `dinner`, `snack`)
- `eaten_at`
- `weekday`
- `has_class_after_meal`
- `minutes_until_next_class`
- `location_type`
- `dining_mode`
- `total_price`
- `total_eat_time_minutes`
- `pre_meal_hunger_level`
- `pre_meal_energy_level`
- `mood_tag`
- `notes`
- `created_at`

### MealLogDish

A meal may include more than one dish, so this join table is safer than storing
only one dish per meal.

This also covers:

- set meals such as `main + rice + drink`
- dinner add-ons such as milk tea or fruit tea

- `id`
- `meal_log_id`
- `dish_id`
- `portion_ratio`
- `custom_notes`

### MealFeedback

Stores post-meal feedback and powers personalization.

- `id`
- `meal_log_id`
- `fullness_level`
- `sleepiness_level`
- `comfort_level`
- `focus_impact_level`
- `would_eat_again`
- `free_text_feedback`
- `created_at`

### RecommendationRecord

Stores what the app recommended at a given moment. This will help later when
you want to compare recommendation quality against actual outcomes.

- `id`
- `recommended_for_meal_type`
- `generated_at`
- `context_summary`
- `strategy_profile_id`
- `candidate_1_dish_id`
- `candidate_1_score`
- `candidate_1_reason`
- `candidate_2_dish_id`
- `candidate_2_score`
- `candidate_2_reason`
- `candidate_3_dish_id`
- `candidate_3_score`
- `candidate_3_reason`
- `selected_dish_id`

### LlmEnrichmentCache

Stores structured outputs produced from natural language dish input, so the app
does not need to call the API every time.

- `id`
- `raw_input`
- `normalized_name`
- `carb_level`
- `fat_level`
- `protein_level`
- `fiber_level`
- `price_estimate`
- `digestive_burden_level`
- `sleepiness_risk_level`
- `confidence_score`
- `model_name`
- `prompt_version`
- `raw_response_json`
- `created_at`

## Key Modeling Decisions

1. Use categorical nutrition tags first, not exact calories
2. Separate `Dish` from `MealLog`, because one dish can be eaten many times
3. Support multiple dishes per meal from the start
4. Keep class periods and weekly schedule separate
5. Keep LLM outputs cached and auditable
6. Store recommendation records for later tuning
7. Do not store sensitive raw family-history medical data
8. Represent cardiovascular preference indirectly through scoring priorities
9. Treat beverages and add-ons as normal dish records linked through
   `MealLogDish`
10. Keep breakfast in the data model but disable recommendation by default
11. Limit active planning to Tuesday through Friday unless policy changes later

## What Is Still Missing

Only a few things are still unresolved for schema freeze:

1. Whether the app should still log Saturday to Monday meals even when not
   recommending them
2. Whether commute day logic should add a specific "carryable food" priority
3. Whether breakfast logs should default to hidden from the main recommendation
   timeline

These are not blockers for coding the first schema.
