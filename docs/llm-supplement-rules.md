# LLM Supplement Rules

Last updated: `2026-04-23`

## Scope

This document defines the agreed Stage 6 rule boundary for natural-language
supplement parsing in MealAdvisor.

The LLM acts only as a parser:

- input: one user's natural-language supplement text plus current meal context
- output: one strict JSON object
- responsibility: convert temporary user intent into structured adjustment
  parameters for the current recommendation run

The LLM is not allowed to:

- recommend specific dishes directly
- replace the local recommendation engine
- rewrite base group weights
- modify database facts such as merchants, dishes, prices, schedules, or
  feedback history

## Core Principles

1. The local recommendation engine remains the primary decision source.
2. The LLM only supplies temporary per-run adjustment signals.
3. The output must be strict JSON with fixed keys.
4. The local app must validate every returned field before applying it.
5. Invalid or partial responses must fall back to a neutral default object.

## Output Contract

The parser should return exactly one JSON object:

```json
{
  "version": "supplement_parser_v1",
  "result": {
    "hungerIntent": 1.0,
    "carbIntent": 1.0,
    "drinkIntent": 1.0,
    "budgetFlexIntent": 1.0,
    "classConstraintWeight": 1.0,
    "postMealSleepPlan": "unknown",
    "plannedNapMinutes": 0,
    "sleepNeedLevel": 1.0,
    "sleepPlanConfidence": 0.0,
    "proteinIntent": 1.0,
    "colaIntent": 1.0,
    "flavorIntent": 1.0,
    "relaxedTimePreference": 1.0
  }
}
```

Rules:

- `version` is required and must stay `supplement_parser_v1`
- `result` is required
- all 13 fields are required
- no extra keys are allowed
- no markdown, prose, or chain-of-thought is allowed

## Field Classes

### Strong-judgment fields

These are allowed to use strong multipliers because they represent temporary
scene pressure or after-meal state changes.

Allowed values:

```text
0.4, 0.5, 0.65, 0.8, 1.0, 1.25, 1.6, 2.0, 2.5
```

Fields:

- `classConstraintWeight`
- `sleepNeedLevel`
- `relaxedTimePreference`

Related non-multiplier strong fields:

- `postMealSleepPlan`
- `plannedNapMinutes`

Interpretation:

- values above `1.0` strengthen the corresponding scene signal
- values below `1.0` weaken the corresponding scene signal
- extreme values should only be used under explicit, high-confidence language

### Weak-judgment fields

These are ordinary preference signals and must stay in a limited range.

Allowed values:

```text
0.75, 0.85, 0.95, 1.0, 1.1, 1.2, 1.35
```

Fields:

- `hungerIntent`
- `carbIntent`
- `drinkIntent`
- `budgetFlexIntent`
- `proteinIntent`
- `colaIntent`
- `flavorIntent`

Interpretation:

- values above `1.0` mean stronger preference
- values below `1.0` mean weaker preference or mild avoidance
- weak signals must not override strong scene constraints on their own

### Governance field

`sleepPlanConfidence`

Allowed values:

```text
0.0, 0.25, 0.5, 0.75, 1.0
```

Interpretation:

- this field is used as a gating / confidence input
- it should not behave like a direct business-weight bonus

## Enum And Integer Fields

### `postMealSleepPlan`

Allowed values:

- `stay_awake`
- `nap_before_class`
- `no_class`
- `unknown`

### `plannedNapMinutes`

Allowed values:

- `0, 10, 15, 20, 30, 40, 45, 60, 90`

## Per-Field Rule Table

| Field | Chinese meaning | Class | Allowed strength | Main effect area |
| --- | --- | --- | --- | --- |
| `hungerIntent` | 饥饿程度 | weak | `0.75-1.35` | satiety-related weighting |
| `carbIntent` | 碳水偏好 | weak | `0.75-1.35` | carb target / carb fit |
| `drinkIntent` | 饮品偏好 | weak | `0.75-1.35` | drink candidate lift |
| `budgetFlexIntent` | 预算放宽 | weak | `0.75-1.35` | budget tolerance / pressure |
| `classConstraintWeight` | 上课或赶场约束强度 | strong | `0.4-2.5` | class fit / time fit / alertness pressure |
| `postMealSleepPlan` | 饭后安排 | enum | fixed enum | carb / sleepiness / time interpretation |
| `plannedNapMinutes` | 午睡时间 | integer | fixed set | total occupied time |
| `sleepNeedLevel` | 困意强度 | strong | `0.4-2.5` | sleep-related amplification |
| `sleepPlanConfidence` | 饭后安排识别置信度 | governance | `0.0-1.0` fixed set | gating only |
| `proteinIntent` | 蛋白质偏好 | weak | `0.75-1.35` | protein support lift |
| `colaIntent` | 可乐偏好 | weak | `0.75-1.35` | cola / beverage bonus |
| `flavorIntent` | 重口/满足感偏好 | weak | `0.75-1.35` | flavor bonus |
| `relaxedTimePreference` | 接受更宽松耗时 | strong | `0.4-2.5` | time pressure relaxation / tightening |

## Priority Rules

1. Explicit reality change beats ordinary preference.
2. Explicit stay-awake requirement beats ordinary carb preference.
3. Explicit nap/rest plan beats default class-pressure assumptions.
4. If the text is vague, stay conservative and return neutral values.

## Local Validation Rules

The app must validate all LLM output locally:

1. parse must succeed as JSON
2. top-level object must contain `version` and `result`
3. `version` must equal `supplement_parser_v1`
4. `result` must contain exactly the 13 expected keys
5. all values must match their allowed enums / fixed sets
6. any violation triggers fallback to the neutral default object

## Neutral Default Object

```json
{
  "version": "supplement_parser_v1",
  "result": {
    "hungerIntent": 1.0,
    "carbIntent": 1.0,
    "drinkIntent": 1.0,
    "budgetFlexIntent": 1.0,
    "classConstraintWeight": 1.0,
    "postMealSleepPlan": "unknown",
    "plannedNapMinutes": 0,
    "sleepNeedLevel": 1.0,
    "sleepPlanConfidence": 0.0,
    "proteinIntent": 1.0,
    "colaIntent": 1.0,
    "flavorIntent": 1.0,
    "relaxedTimePreference": 1.0
  }
}
```

## Implementation Notes

- The LLM should not be asked to produce final total scores.
- The LLM should not be asked to output free-form reasoning.
- The app should use `temperature = 0` where possible.
- If the API supports `json_schema` / structured outputs, prefer that over
  prompt-only JSON instructions.
