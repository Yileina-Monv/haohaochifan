# Recommendation Metrics Table

Last updated: `2026-04-19`

## Scope

This document defines the current Chinese-facing recommendation metrics,
calculation roles, dynamic weighting rules, warning output rules, and the
reserved LLM supplement interface.

Internal field names may stay in English in code and database tables.
Frontend labels, user-facing reasons, and warnings should be Chinese unless the
term is naturally abbreviated, such as `GI`.

## 1. Recommendation Structure

The recommendation flow is split into three layers:

1. `过滤层`
   Remove inactive, unavailable, or obviously impossible options.
2. `评分层`
   Score remaining dishes using weighted metrics.
3. `警告层`
   Keep warnings separate from ranking when the risk should be visible but not
   always dominate the sort order.

## 2. 一级指标总表

| 一级指标 | 中文展示名 | 作用 | 是否直接参与排序 | 备注 |
| --- | --- | --- | --- | --- |
| Scene Fit | 场景适配分 | 判断这道菜是否适合当前时间、课表和就餐方式 | 是 | 核心场景分 |
| Nutrition Fit | 营养平衡分 | 判断营养结构、负担和碳水适配 | 是 | 午餐有课时更重要 |
| Preference Fit | 个人偏好分 | 反映个人口味评分和复吃意愿 | 是 | 来自吃后反馈 |
| Diversity Fit | 多餐补偿分 | 防止重复推荐并补偿最近饮食结构偏差 | 是 | 不能天天同一种 |
| Budget Fit | 预算适配分 | 判断价格是否适合本餐预算 | 是 | 软约束 |
| Intent Fit | 当前需求匹配分 | 响应自然语言补充输入 | 是 | 临时偏好 |
| Warning Layer | 风险提醒 | 输出犯困、高负担、气味等提醒 | 否，默认不直接决定排序 | 特别适用于晚餐 |

## 3. 二级指标明细

### 3.1 场景适配

| 内部字段建议 | 中文名 | 含义 | 典型来源 | 建议作用 |
| --- | --- | --- | --- | --- |
| time_fit | 时间可行性 | 是否来得及获取并吃完 | 距离、排队、配送 ETA、吃饭用时 | 直接加减分 |
| class_fit | 课前适配度 | 饭后有课时是否稳妥 | 课表、距离下节课分钟数 | 直接加减分 |
| dining_mode_fit | 用餐方式匹配 | 堂食/打包/配送是否适合当前场景 | merchant + dish 标签 | 直接加减分 |
| meal_type_fit | 餐次匹配 | 是否适合早餐/午餐/晚餐/加餐 | dish 类型、meal type | 直接加减分 |
| post_meal_sleep_modifier | 饭后睡眠修正 | 根据饭后是否睡觉调整惩罚系数 | LLM 补充输入 | 场景放大器 |

### 3.2 营养平衡

| 内部字段建议 | 中文名 | 含义 | 典型来源 | 建议作用 |
| --- | --- | --- | --- | --- |
| carb_fit | 碳水适配 | 当前场景下碳水是否合适 | dish tag + GI + intent | 直接加减分 |
| fat_fit | 脂肪适配 | 当前场景下脂肪是否合适 | dish tag | 直接加减分 |
| protein_support | 蛋白质支持 | 是否有足够蛋白质 | dish tag | 直接加分 |
| fiber_support | 膳食纤维支持 | 是否有足够纤维 | dish tag | 直接加分 |
| vitamin_support | 维生素支持 | 是否有足够维生素 | dish tag | 直接加分 |
| satiety_support | 饱腹感 | 是否能支撑本餐 | dish tag + intent | 直接加分 |
| digestive_burden_fit | 消化负担 | 饭后是否容易沉重 | dish tag | 直接加减分 |
| sleepiness_risk_fit | 晕碳/犯困风险 | 吃后是否容易困 | dish tag + feedback | 有课时强扣，晚餐偏 warning |
| gi_fit | GI 适配 | 升糖速度是否合适 | 后续预留字段 | 与碳水适配联动 |

### 3.3 个人偏好

| 内部字段建议 | 中文名 | 含义 | 典型来源 | 建议作用 |
| --- | --- | --- | --- | --- |
| taste_rating | 口味评分 | 这道菜个人喜好程度，1-5 | 吃后反馈 | 核心偏好分 |
| repeat_willingness | 还想再吃 | 愿不愿意近期再吃 | 吃后反馈 | 次级偏好分 |
| preference_score | 个人偏好分 | 历史反馈聚合后的稳定偏好分 | 聚合计算 | 直接加减分 |

### 3.4 多餐补偿

| 内部字段建议 | 中文名 | 含义 | 典型来源 | 建议作用 |
| --- | --- | --- | --- | --- |
| same_dish_penalty | 同菜重复惩罚 | 最近几餐是否反复吃同一道菜 | meal logs | 直接扣分 |
| same_merchant_penalty | 同商家重复惩罚 | 最近几餐是否反复吃同一商家 | meal logs | 直接扣分 |
| nutrition_compensation | 营养补偿 | 最近几餐是否高碳、高油、低纤维，需要反向补偿 | meal logs + dish tags | 直接加减分 |

### 3.5 预算适配

| 内部字段建议 | 中文名 | 含义 | 典型来源 | 建议作用 |
| --- | --- | --- | --- | --- |
| price_fit | 价格适配 | 是否落在本餐预算内 | dish price + planning policy | 直接加减分 |
| budget_pressure | 预算压力 | 是否逼近或超过弹性预算 | planning policy | 直接扣分 |
| acquire_cost_fit | 获取成本 | 距离、排队、配送和获取难度的组合 | merchant + dish | 直接加减分 |

### 3.6 当前需求匹配

| 内部字段建议 | 中文名 | 含义 | 典型来源 | 建议作用 |
| --- | --- | --- | --- | --- |
| hunger_intent | 当前饥饿度 | 是否更需要高饱腹 | LLM 补充输入 | 调高饱腹权重 |
| carb_intent | 当前碳水偏好 | 是否想吃更偏碳水 | LLM 补充输入 | 调整碳水适配 |
| drink_intent | 饮料偏好 | 是否明确想喝饮料 | LLM 补充输入 | 放宽饮料候选限制 |
| budget_flex_intent | 预算放宽 | 是否愿意为这餐多花一点 | LLM 补充输入 | 放宽预算约束 |
| skip_class_constraint | 跳过上课约束 | 例如下午课不上了 | LLM 补充输入 | 降低课前惩罚 |

## 4. 饭后睡眠计划指标

This is a new dedicated metric family and should be treated as a scene
modifier, not a normal static dish tag.

| 内部字段建议 | 中文名 | 类型 | 取值建议 | 作用 |
| --- | --- | --- | --- | --- |
| post_meal_sleep_plan | 饭后安排 | enum | `stay_awake` / `nap_before_class` / `no_class` / `unknown` | 决定放大哪类惩罚 |
| planned_nap_minutes | 计划睡多久 | int | `0-180` | 用于总占用时间计算 |
| sleep_need_level | 当前困意 | float | `0.0-1.0` | 表示用户主观睡意 |
| sleep_plan_confidence | 识别置信度 | float | `0.0-1.0` | 表示 LLM 解析可信度 |

### 4.1 饭后睡眠计划规则

| 场景 | 中文说明 | 主要放大项 | 建议处理 |
| --- | --- | --- | --- |
| `stay_awake` | 饭后不睡直接上课 | 碳水适配、晕碳/犯困风险 | 提高碳水和犯困风险惩罚 |
| `nap_before_class` | 饭后先睡再上课 | 总耗时、时间可行性 | 降低单纯碳水惩罚，但明显提高总耗时惩罚 |
| `no_class` | 饭后无课 | warning layer | 减少硬性惩罚 |
| `unknown` | 暂不确定 | 保守中间值 | 保持默认权重 |

### 4.2 总占用时间公式

```text
总占用时间 = 获取时间 + 吃饭时间 + 计划睡眠时间 + 起床缓冲时间
```

建议：

- `获取时间` = 距离 / 排队 / 配送 ETA / 获取费力度估算后的组合值
- `吃饭时间` = dish `eat_time_minutes`
- `计划睡眠时间` = `planned_nap_minutes`
- `起床缓冲时间` = 默认 `10-20` 分钟

### 4.3 黑名单或极重扣分规则

| 规则 | 建议处理 |
| --- | --- |
| 高碳水 + 高吃饭用时 + `stay_awake` + 饭后有课 | 强黑名单或极重扣分 |
| 高碳水 + 高总占用时间 + `nap_before_class` + 距离下节课时间不足 | 强黑名单或极重扣分 |
| 高碳水 + 晕碳风险高 + 高消化负担 | 叠加重扣 |

## 5. 推荐分项计算建议

### 5.1 总分结构

```text
综合推荐分 =
    场景适配分
  + 营养平衡分
  + 个人偏好分
  + 多餐补偿分
  + 预算适配分
  + 当前需求匹配分
  + 细微修正
```

### 5.2 默认分项组成

| 分项 | 中文名 | 建议内部组成 |
| --- | --- | --- |
| Scene Fit | 场景适配分 | 时间可行性 + 课前适配度 + 用餐方式匹配 + 餐次匹配 |
| Nutrition Fit | 营养平衡分 | 碳水适配 + 脂肪适配 + 蛋白质支持 + 膳食纤维支持 + 维生素支持 + 饱腹感 + 消化负担 + GI |
| Preference Fit | 个人偏好分 | 口味评分 + 还想再吃 |
| Diversity Fit | 多餐补偿分 | 同菜重复惩罚 + 同商家重复惩罚 + 营养补偿 |
| Budget Fit | 预算适配分 | 价格适配 + 预算压力 + 获取成本 |
| Intent Fit | 当前需求匹配分 | 饥饿度 + 碳水偏好 + 饮料偏好 + 预算放宽 + 跳过上课约束 |

### 5.3 个人偏好聚合

建议不要直接裸平均，使用平滑平均：

```text
平滑口味分 = (全局均值 * 3 + 用户均值 * 样本数) / (3 + 样本数)
```

建议：

- `全局均值` 初始可设为 `3.2`
- `口味评分` 取值 `1-5`
- 最终可映射到 `0-100`

## 6. 多餐补偿规则

| 规则 | 建议值 | 说明 |
| --- | --- | --- |
| 最近 3 餐同 dish 出现 1 次 | `-8` | 轻微扣分 |
| 最近 3 餐同 dish 出现 2 次 | `-18` | 明显扣分 |
| 最近 3 餐同 dish 出现 3 次 | `-35` | 强扣分 |
| 最近 3 餐同 merchant 连续出现 | `-10 ~ -20` | 防止商家单一 |
| 最近两餐都高碳 | 当前高碳 `-10`，高蛋白/高纤维 `+8` | 营养补偿 |
| 最近两餐都高负担高油 | 当前高负担 `-12` | 负担补偿 |
| 最近两餐都偏轻且不够饱 | 当前高饱腹主餐 `+8` | 饱腹补偿 |

## 7. 动态权重建议

### 7.1 午餐且 120 分钟内有课

| 分项 | 权重 |
| --- | --- |
| 场景适配分 | 30 |
| 营养平衡分 | 25 |
| 个人偏好分 | 15 |
| 多餐补偿分 | 15 |
| 预算适配分 | 10 |
| 当前需求匹配分 | 5 |

### 7.2 普通白天

| 分项 | 权重 |
| --- | --- |
| 场景适配分 | 25 |
| 营养平衡分 | 25 |
| 个人偏好分 | 20 |
| 多餐补偿分 | 15 |
| 预算适配分 | 10 |
| 当前需求匹配分 | 5 |

### 7.3 晚餐

| 分项 | 权重 |
| --- | --- |
| 场景适配分 | 20 |
| 营养平衡分 | 20 |
| 个人偏好分 | 25 |
| 多餐补偿分 | 20 |
| 预算适配分 | 10 |
| 当前需求匹配分 | 5 |

## 8. 晕碳、碳水、GI 的关系

The key conclusion is:

- `碳水` 不是唯一惩罚项
- `晕碳/犯困风险` 才是课程密集场景下的主扣分项
- `GI`、脂肪、消化负担、份量和个人历史反馈都会共同影响犯困风险

### 8.1 指标角色区分

| 指标 | 中文名 | 角色 |
| --- | --- | --- |
| carb_level / carb_fit | 碳水 / 碳水适配 | 上游营养因子 |
| gi_fit | GI 适配 | 上游营养因子 |
| digestive_burden_fit | 消化负担 | 上游风险因子 |
| sleepiness_risk_fit | 晕碳/犯困风险 | 场景主惩罚项 |

### 8.2 课程密集场景建议

| 指标 | 处理建议 |
| --- | --- |
| 碳水适配 | 轻到中度扣分 |
| GI 适配 | 轻到中度扣分 |
| 消化负担 | 中度扣分 |
| 晕碳/犯困风险 | 中到重度扣分，优先级最高 |

## 9. 晚餐规则与警告层

### 9.1 晚餐排序原则

- 晚餐不把 `晕碳/犯困风险` 当成最强排序惩罚
- 晚餐更看重口味、多样性和整体负担
- 但必须输出晚间风险提醒

### 9.2 晚餐警告文案建议

| 中文警告标签 | 文案建议 |
| --- | --- |
| 晚间犯困风险 | 这道菜晚餐后可能更犯困 |
| 晕碳风险 | 高碳水组合，晚间可能更想躺着休息 |
| 高负担提醒 | 这顿饭整体偏重，可能影响晚间清醒状态 |
| 气味影响提醒 | 如果饭后在宿舍或室内活动，气味可能更明显 |

## 10. 前端汉化映射建议

| 内部字段 | 前端中文 |
| --- | --- |
| sleepinessRiskLevel | 犯困风险 |
| digestiveBurdenLevel | 消化负担 |
| mealImpactWeight | 整餐影响权重 |
| defaultDiningMode | 默认就餐方式 |
| acquireEffortScore | 获取费力度 |
| satietyLevel | 饱腹感 |
| odorLevel | 气味强度 |
| carbLevel | 碳水水平 |
| fatLevel | 脂肪水平 |
| proteinLevel | 蛋白质水平 |
| vitaminLevel | 维生素水平 |
| fiberLevel | 膳食纤维水平 |
| priceLevel | 价格档位 |
| deliveryEtaMinutes | 配送预计时间 |
| queueTimeMinutes | 排队时间 |
| distanceMinutes | 距离时间 |

## 11. LLM 补充输入接口预留

### 11.1 推荐补充输入

```json
{
  "hunger_intent": 0.0,
  "carb_intent": 0.0,
  "drink_intent": 0.0,
  "budget_flex_intent": 0.0,
  "skip_class_constraint": false,
  "post_meal_sleep_plan": "unknown",
  "planned_nap_minutes": 0,
  "sleep_need_level": 0.0,
  "sleep_plan_confidence": 0.0
}
```

### 11.2 吃后反馈输入

```json
{
  "taste_rating": 3,
  "repeat_willingness": 3,
  "post_meal_sleepiness": 0.0,
  "post_meal_comfort": 0.0,
  "post_meal_focus_impact": 0.0,
  "notes": ""
}
```

## 12. 当前建议的实现顺序

1. Add post-meal feedback fields:
   - `taste_rating`
   - `repeat_willingness`
2. Feed the aggregated preference score into recommendation scoring.
3. Add multi-meal compensation:
   - same dish
   - same merchant
   - recent nutrition compensation
4. Add the post-meal sleep plan interface and scene modifier.
5. Convert recommendation reasons and warnings into Chinese.
6. Gradually convert the rest of the frontend to Chinese labels.
