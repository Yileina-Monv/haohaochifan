CREATE TABLE IF NOT EXISTS user_profiles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    display_name TEXT,
    weight_status TEXT,
    visceral_fat_risk TEXT,
    carb_sensitivity TEXT,
    default_budget_min REAL,
    default_budget_max REAL,
    bmi_value REAL,
    age_years INTEGER,
    biological_sex TEXT,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS recommendation_profiles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    has_class_priority INTEGER NOT NULL DEFAULT 0,
    health_priority INTEGER NOT NULL DEFAULT 0,
    budget_priority INTEGER NOT NULL DEFAULT 0,
    time_effort_priority INTEGER NOT NULL DEFAULT 0,
    satiety_priority INTEGER NOT NULL DEFAULT 0,
    carb_control_priority INTEGER NOT NULL DEFAULT 0,
    fat_control_priority INTEGER NOT NULL DEFAULT 0,
    protein_priority INTEGER NOT NULL DEFAULT 0,
    sleepiness_avoid_priority INTEGER NOT NULL DEFAULT 0,
    breakfast_recommendation_enabled INTEGER NOT NULL DEFAULT 0,
    enabled_weekdays TEXT,
    planning_scope TEXT NOT NULL DEFAULT 'all_enabled_days',
    is_default INTEGER NOT NULL DEFAULT 0,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS planning_policies (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    enabled_weekdays TEXT NOT NULL,
    include_commute_days INTEGER NOT NULL DEFAULT 1,
    skip_non_enabled_days INTEGER NOT NULL DEFAULT 1,
    default_daily_budget REAL,
    flexible_budget_cap REAL,
    breakfast_recommendation_enabled INTEGER NOT NULL DEFAULT 0,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS class_periods (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    period_index INTEGER NOT NULL UNIQUE,
    start_time TEXT NOT NULL,
    end_time TEXT NOT NULL,
    session_group TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS schedule_entries (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    weekday INTEGER NOT NULL,
    period_start INTEGER NOT NULL,
    period_end INTEGER NOT NULL,
    course_name TEXT NOT NULL,
    location TEXT,
    campus_zone TEXT,
    intensity_level TEXT,
    notes TEXT,
    FOREIGN KEY (period_start) REFERENCES class_periods (period_index),
    FOREIGN KEY (period_end) REFERENCES class_periods (period_index)
);

CREATE TABLE IF NOT EXISTS merchants (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    campus_area TEXT,
    distance_minutes INTEGER,
    queue_time_minutes INTEGER,
    supports_dine_in INTEGER NOT NULL DEFAULT 1,
    supports_takeaway INTEGER NOT NULL DEFAULT 0,
    supports_delivery INTEGER NOT NULL DEFAULT 0,
    delivery_eta_minutes INTEGER,
    price_level TEXT,
    notes TEXT
);

CREATE TABLE IF NOT EXISTS dishes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    merchant_id INTEGER,
    category TEXT,
    is_combo INTEGER NOT NULL DEFAULT 0,
    is_beverage INTEGER NOT NULL DEFAULT 0,
    price REAL,
    default_dining_mode TEXT,
    eat_time_minutes INTEGER,
    acquire_effort_score INTEGER,
    carb_level TEXT,
    fat_level TEXT,
    protein_level TEXT,
    vitamin_level TEXT,
    fiber_level TEXT,
    vegetable_level TEXT,
    satiety_level TEXT,
    digestive_burden_level TEXT,
    sleepiness_risk_level TEXT,
    flavor_level TEXT,
    odor_level TEXT,
    meal_impact_weight REAL NOT NULL DEFAULT 1.0,
    notes TEXT,
    health_score INTEGER,
    favorite_score INTEGER,
    is_active INTEGER NOT NULL DEFAULT 1,
    source_type TEXT NOT NULL DEFAULT 'manual',
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL,
    FOREIGN KEY (merchant_id) REFERENCES merchants (id)
);

CREATE TABLE IF NOT EXISTS meal_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    meal_type TEXT NOT NULL,
    eaten_at TEXT NOT NULL,
    weekday INTEGER NOT NULL,
    has_class_after_meal INTEGER NOT NULL DEFAULT 0,
    minutes_until_next_class INTEGER,
    location_type TEXT,
    dining_mode TEXT,
    total_price REAL,
    total_eat_time_minutes INTEGER,
    pre_meal_hunger_level INTEGER,
    pre_meal_energy_level INTEGER,
    mood_tag TEXT,
    notes TEXT,
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS meal_log_dishes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    meal_log_id INTEGER NOT NULL,
    dish_id INTEGER NOT NULL,
    portion_ratio REAL NOT NULL DEFAULT 1.0,
    custom_notes TEXT,
    FOREIGN KEY (meal_log_id) REFERENCES meal_logs (id) ON DELETE CASCADE,
    FOREIGN KEY (dish_id) REFERENCES dishes (id)
);

CREATE TABLE IF NOT EXISTS meal_feedback (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    meal_log_id INTEGER NOT NULL UNIQUE,
    recommendation_record_id INTEGER,
    fullness_level INTEGER,
    sleepiness_level INTEGER,
    comfort_level INTEGER,
    focus_impact_level INTEGER,
    taste_rating INTEGER,
    repeat_willingness INTEGER,
    would_eat_again INTEGER,
    free_text_feedback TEXT,
    created_at TEXT NOT NULL,
    FOREIGN KEY (meal_log_id) REFERENCES meal_logs (id) ON DELETE CASCADE,
    FOREIGN KEY (recommendation_record_id) REFERENCES recommendation_records (id)
);

CREATE TABLE IF NOT EXISTS recommendation_records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    recommended_for_meal_type TEXT NOT NULL,
    generated_at TEXT NOT NULL,
    context_summary TEXT,
    strategy_profile_id INTEGER,
    candidate_1_dish_id INTEGER,
    candidate_1_score REAL,
    candidate_1_reason TEXT,
    candidate_2_dish_id INTEGER,
    candidate_2_score REAL,
    candidate_2_reason TEXT,
    candidate_3_dish_id INTEGER,
    candidate_3_score REAL,
    candidate_3_reason TEXT,
    selected_dish_id INTEGER,
    selected_meal_log_id INTEGER,
    selected_candidate_rank INTEGER,
    FOREIGN KEY (strategy_profile_id) REFERENCES recommendation_profiles (id),
    FOREIGN KEY (candidate_1_dish_id) REFERENCES dishes (id),
    FOREIGN KEY (candidate_2_dish_id) REFERENCES dishes (id),
    FOREIGN KEY (candidate_3_dish_id) REFERENCES dishes (id),
    FOREIGN KEY (selected_dish_id) REFERENCES dishes (id),
    FOREIGN KEY (selected_meal_log_id) REFERENCES meal_logs (id)
);

CREATE TABLE IF NOT EXISTS llm_enrichment_cache (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    raw_input TEXT NOT NULL,
    normalized_name TEXT,
    carb_level TEXT,
    fat_level TEXT,
    protein_level TEXT,
    fiber_level TEXT,
    price_estimate REAL,
    digestive_burden_level TEXT,
    sleepiness_risk_level TEXT,
    confidence_score REAL,
    model_name TEXT,
    prompt_version TEXT,
    raw_response_json TEXT,
    created_at TEXT NOT NULL
);
