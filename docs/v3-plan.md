# V3 Recommendation Model Plan

Last updated: 2026-04-27

Implementation status: initial V3 budget-gate logic is implemented in
`RecommendationEngine` as of 2026-04-27. Budget is no longer part of the
default weighted score, the parser accepts `budgetMode` / `budgetLimitYuan`,
and strict / relaxed budget modes apply the fixed over-budget gate penalty.

## 1. Goal

V3 should make recommendation ranking better match the user's current natural-language demand.
The main change is to move budget from a default scoring factor into a triggered budget gate.

The current V2 behavior can feel too rigid because cheap, fast, neutral dishes can win even when
the user explicitly says they want a high-budget, high-satiety, low-carb beef meal. V3 should make
explicit user intent stronger while keeping the local rule-based engine as the final authority.

## 2. V2 Problem Summary

Current budget behavior appears in two places:

- `Budget Fit` is a default group score and contributes to every recommendation run.
- `budgetFlexIntent` also contributes through `Intent Fit`.

This means budget affects ranking even when the user did not ask to control spending. It also means
"high budget" can still behave like a softened penalty instead of letting more relevant high-price
dishes compete on nutrition, satiety, and scenario fit.

Example problem:

- User says: "need to be full, very high budget, eat beef, lower carb to avoid afternoon sleepiness,
  cannot nap."
- A beef hotpot can match beef, low carb, high protein, and satiety, but V2 may still push it down
  because price and time are always part of the weighted score.

## 3. Budget Gate Rule

V3 budget should not be a default positive or negative scoring factor.

Default behavior:

- If the user does not mention budget, budget does not affect ranking.
- A dish is not rewarded for being cheap.
- A dish is not punished for being expensive.

Triggered behavior:

- If the user mentions a budget limit or budget-control intent, set a budget line.
- If a dish price is above the budget line, apply a heavy penalty.
- Do not hard-filter the dish out. Keep it as fallback if too few candidates remain.
- Any over-budget candidate must show a warning.

Chosen penalty:

```text
if budgetMode != "none" and dish.price > budgetLimitYuan:
    totalScore -= 40
```

## 4. Budget Language Mapping

V3 should distinguish budget modes explicitly.

```text
budgetMode = none | strict | relaxed
budgetLimitYuan = number
```

Mapping rules:

| User expression | budgetMode | budgetLimitYuan |
| --- | --- | ---: |
| no budget expression | none | 0 |
| explicit amount, e.g. "no more than 30" | strict | user amount |
| "budget is very low", "as cheap as possible" | strict | 15 |
| "control budget", "cheap", "not too expensive" | strict | 40 |
| "very high budget", "want something better", "budget is high" | relaxed | 100 |

Notes:

- "High budget" does not reward expensive dishes. It only raises the penalty line to 100 RMB.
- Dishes above 100 RMB still receive the over-budget penalty.
- "Budget does not matter" is still open for discussion. It can either map to `none` or
  `relaxed + 100`; the current draft prefers `relaxed + 100` for consistency with high-budget wording.

## 5. Scoring Structure Change

Remove default budget scoring from the main weighted score.

Current V2 top-level groups:

```text
Scene Fit
Nutrition Fit
Preference Fit
Diversity Fit
Budget Fit
Intent Fit
```

V3 top-level groups:

```text
Scene Fit
Nutrition Fit
Preference Fit
Diversity Fit
Intent Fit
Budget Gate
Warning Layer
```

Weight decision:

- Remove `group.budget_fit` from normal scoring.
- Move the old 10% budget weight into `group.intent_fit`.
- Keep acquisition/time cost, but treat it as scene cost rather than budget cost.

Budget Gate is applied after the normal score is calculated.

## 6. Parser Contract Draft

The supplement parser should add budget-specific fields instead of overloading
`budgetFlexIntent`.

Required V3 result shape should include:

```json
{
  "budgetMode": "none",
  "budgetLimitYuan": 0
}
```

Validation:

- `budgetMode` must be one of `none`, `strict`, `relaxed`.
- `budgetLimitYuan` must be a number from `0` to `999`.
- If `budgetMode == "none"`, `budgetLimitYuan` should be `0`.
- If `budgetMode != "none"`, `budgetLimitYuan` should be greater than `0`.

Compatibility:

- Existing `budgetFlexIntent` can remain temporarily for old parser responses.
- V3 ranking should not use `budgetFlexIntent` as a direct score component.

## 7. Output and Explanation

Recommendation cards should explain budget behavior only when budget was triggered.

Examples:

- Within budget line: "价格在本次预算线内。"
- Over budget line: "已超过本次预算线，排序中已大幅降权。"
- No budget mentioned: do not show budget reason by default.

The breakdown can show `Budget Gate` separately from the main score so the user understands that it
is a constraint layer, not a normal preference score.

## 8. Example Scenarios

### High-budget beef lunch

Input:

```text
需要吃饱，预算超高，吃点牛肉，少碳水避免下午困，没法睡觉
```

Expected behavior:

- `budgetMode = relaxed`
- `budgetLimitYuan = 100`
- Beef, high satiety, low carb, high protein should be strongly favored.
- A 100 RMB beef hotpot should not be punished by default budget scoring.
- A dish over 100 RMB should receive the fixed over-budget penalty.

### Budget-control lunch

Input:

```text
今天控制预算，吃便宜点
```

Expected behavior:

- `budgetMode = strict`
- `budgetLimitYuan = 40`
- Dishes above 40 RMB receive `-40`.
- They can remain as fallback but should normally not beat line-in candidates.

### Very low budget

Input:

```text
预算很少，随便吃点
```

Expected behavior:

- `budgetMode = strict`
- `budgetLimitYuan = 15`
- Dishes above 15 RMB receive `-40`.

### Explicit amount

Input:

```text
不超过 30 元
```

Expected behavior:

- `budgetMode = strict`
- `budgetLimitYuan = 30`
- Dishes above 30 RMB receive `-40`.

## 9. Test Ideas

- No-budget recommendation should not change solely because a dish is cheap or expensive.
- High-budget beef and low-carb scenario should lift beef hotpot relative to cheap neutral items.
- `control budget` should push over-40 RMB dishes down.
- `budget very low` should push over-15 RMB dishes down.
- Explicit amount parsing should override fuzzy mappings.
- Over-budget candidates should still appear if there are not enough viable in-budget candidates.
- Existing parser fallback should still produce usable recommendations.

## 10. Open Discussion Items

- Should "budget does not matter" mean `none` or `relaxed + 100`?
- Should the fixed over-budget penalty stay at `40`, or should later versions scale by overage?
- Should budget warning be shown in the top card summary, candidate warning list, or both?
- Should `budgetFlexIntent` be removed after V3 stabilizes, or kept as a compatibility alias?
