import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    component AutoHeightRectangle: Rectangle {
        Layout.preferredHeight: implicitHeight
        implicitHeight: childrenRect.height > 0
                        ? childrenRect.y + childrenRect.height + childrenRect.y
                        : 0
    }

    component ReadableButton: Button {
        id: readableButton
        property bool primary: false

        Layout.preferredHeight: 42
        flat: true
        padding: 0

        background: Rectangle {
            radius: 14
            color: !readableButton.enabled
                   ? "#eadfce"
                   : readableButton.primary
                     ? (readableButton.down ? "#8f5926" : (readableButton.hovered ? "#b36f2e" : "#a8662c"))
                     : (readableButton.down ? "#ead6bd" : (readableButton.hovered ? "#f5e7d3" : "#fff9f0"))
            border.color: readableButton.primary ? "#8f5926" : "#dcc3a4"
            border.width: 1
        }

        contentItem: Label {
            text: readableButton.text
            color: !readableButton.enabled ? "#9a8d7d" : (readableButton.primary ? "#fffaf2" : "#473525")
            font.pixelSize: 15
            font.bold: readableButton.primary
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            maximumLineCount: 1
        }
    }

    component ModeButton: Button {
        id: modeButton
        property bool selected: false

        Layout.fillWidth: true
        Layout.preferredHeight: 42
        flat: true

        background: Rectangle {
            radius: 14
            color: modeButton.selected ? "#9b632d" : "#fff9f0"
            border.color: modeButton.selected ? "#9b632d" : "#dcc3a4"
            border.width: 1
        }

        contentItem: Label {
            text: modeButton.text
            color: modeButton.selected ? "#fffaf2" : "#473525"
            font.bold: modeButton.selected
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    component DrawerSectionButton: Button {
        id: drawerButton
        property bool selected: false

        Layout.fillWidth: true
        Layout.preferredHeight: 42
        flat: true

        background: Rectangle {
            radius: 14
            color: drawerButton.selected ? "#9b632d" : "#fff7ec"
            border.color: drawerButton.selected ? "#9b632d" : "#dec4a3"
            border.width: 1
        }

        contentItem: Label {
            text: drawerButton.text
            color: drawerButton.selected ? "#fffaf2" : "#51402f"
            font.pixelSize: 13
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    component FeedbackMetric: RowLayout {
        id: metric
        property string label: ""
        property alias value: box.value

        spacing: 8
        Layout.preferredHeight: 44

        Label {
            Layout.preferredWidth: 52
            Layout.alignment: Qt.AlignVCenter
            text: metric.label
            color: "#6b5846"
        }

        SpinBox {
            id: box
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            from: 1
            to: 5
            value: 3
        }
    }

    component MenuIconButton: ToolButton {
        id: menuButton

        Layout.preferredWidth: 44
        Layout.preferredHeight: 44
        padding: 0

        background: Rectangle {
            radius: 16
            color: menuButton.down ? "#ead6bd" : (menuButton.hovered ? "#f6e8d3" : "#fff9f0")
            border.color: "#dec4a3"
            border.width: 1
        }

        contentItem: Item {
            implicitWidth: 44
            implicitHeight: 44

            Column {
                anchors.centerIn: parent
                spacing: 4

                Repeater {
                    model: 3

                    Rectangle {
                        width: 18
                        height: 2
                        radius: 1
                        color: "#6d4928"
                    }
                }
            }
        }
    }

    component CloseIconButton: ToolButton {
        id: closeButton

        Layout.preferredWidth: 40
        Layout.preferredHeight: 40
        padding: 0

        background: Rectangle {
            radius: 16
            color: closeButton.down ? "#ead6bd" : (closeButton.hovered ? "#f6e8d3" : "#fff9f0")
            border.color: "#dec4a3"
            border.width: 1
        }

        contentItem: Item {
            implicitWidth: 40
            implicitHeight: 40

            Rectangle {
                width: 18
                height: 2
                radius: 1
                color: "#6d4928"
                anchors.centerIn: parent
                rotation: 45
            }

            Rectangle {
                width: 18
                height: 2
                radius: 1
                color: "#6d4928"
                anchors.centerIn: parent
                rotation: -45
            }
        }
    }

    component StyledTextArea: TextArea {
        id: styledTextArea
        property string hintText: ""

        font.pixelSize: 15
        color: "#3f3024"
        placeholderText: activeFocus || text.length > 0 ? "" : hintText
        placeholderTextColor: "#9c8b78"
        wrapMode: TextEdit.Wrap
        clip: true
        leftPadding: 14
        rightPadding: 14
        topPadding: 12
        bottomPadding: 12

        background: Rectangle {
            radius: 14
            color: "#fffaf2"
            border.color: styledTextArea.activeFocus ? "#b97834" : "#dec4a3"
            border.width: styledTextArea.activeFocus ? 2 : 1
        }
    }

    component StyledTextField: TextField {
        id: styledTextField
        property string hintText: ""

        font.pixelSize: 15
        color: "#3f3024"
        placeholderText: activeFocus || text.length > 0 ? "" : hintText
        placeholderTextColor: "#9c8b78"
        leftPadding: 14
        rightPadding: 14

        background: Rectangle {
            radius: 14
            color: "#fffaf2"
            border.color: styledTextField.activeFocus ? "#b97834" : "#dec4a3"
            border.width: styledTextField.activeFocus ? 2 : 1
        }
    }

    id: window

    width: 420
    height: 800
    minimumWidth: 320
    minimumHeight: 560
    visible: true
    title: "MealAdvisor"
    color: "#f4eadc"

    property string drawerSection: "llm"
    property bool pendingRecommendation: false
    property string recommendDraft: ""
    property string feedbackDraft: ""
    property int selectedMealId: 0
    property string feedbackStatus: "选择一条最近餐次后，可以直接补餐后反馈。"
    property bool feedbackManualVisible: false
    property int fullnessValue: 3
    property int sleepinessValue: 3
    property int comfortValue: 3
    property int focusValue: 3
    property int tasteValue: 3
    property int repeatValue: 3
    property bool wouldEatAgain: true
    property string taskMode: "recommend"
    property string dishDraft: ""
    property string routineDraft: ""
    property string taskState: "idle"
    property string taskIntent: ""
    property string taskSummary: ""
    property var taskActions: []
    property var taskMissingFields: []
    property bool taskRequiresConfirmation: false
    property bool pendingFeedbackParsePreview: false
    property bool pendingDishParsePreview: false
    property int dishImportMerchantId: 0

    function clampScore(value, fallbackValue) {
        const parsed = Number(value || 0)
        if (parsed >= 1 && parsed <= 5) {
            return parsed
        }
        return fallbackValue
    }

    function findMeal(mealId) {
        for (let i = 0; i < mealLogManager.recentMeals.length; ++i) {
            if (mealLogManager.recentMeals[i].id === mealId) {
                return mealLogManager.recentMeals[i]
            }
        }
        return null
    }

    function selectedMeal() {
        return findMeal(selectedMealId)
    }

    function setTaskPreview(intent, summary, actions, requiresConfirmation, missingFields, state) {
        taskIntent = intent
        taskSummary = summary
        taskActions = actions ? actions : []
        taskRequiresConfirmation = !!requiresConfirmation
        taskMissingFields = missingFields ? missingFields : []
        taskState = state && state.length > 0 ? state : "preview"
    }

    function clearTaskPreview() {
        setTaskPreview("", "", [], false, [], "idle")
    }

    function taskModeLabel(mode) {
        if (mode === "feedback") {
            return "反馈"
        }
        if (mode === "dish") {
            return "菜品"
        }
        if (mode === "routine") {
            return "日常"
        }
        return "推荐"
    }

    function activeTaskPlaceholder() {
        if (taskMode === "feedback") {
            return "说说上一餐真实感受，比如有点撑、犯困、还想不想复吃..."
        }
        if (taskMode === "dish") {
            return "输入看到的菜品，比如商家、菜名、价格、口味和饭后负担..."
        }
        if (taskMode === "routine") {
            return "输入临时日常，比如今晚考试、运动、通勤或睡眠安排..."
        }
        return "下一餐想吃什么、预算、赶不赶时间..."
    }

    function draftForMode(mode) {
        if (mode === "feedback") {
            return feedbackDraft
        }
        if (mode === "dish") {
            return dishDraft
        }
        if (mode === "routine") {
            return routineDraft
        }
        return recommendDraft
    }

    function updateDraftForActiveMode(text) {
        if (taskMode === "feedback") {
            feedbackDraft = text
        } else if (taskMode === "dish") {
            dishDraft = text
        } else if (taskMode === "routine") {
            routineDraft = text
        } else {
            recommendDraft = text
        }
    }

    function switchTaskMode(mode) {
        updateDraftForActiveMode(composerInput.text)
        taskMode = mode
        composerInput.text = draftForMode(mode)
        clearTaskPreview()
        if (mode === "feedback") {
            ensureSelectedMeal()
        } else if (mode === "dish" && dishImportMerchantId <= 0
                   && foodManager.merchants.length > 0) {
            dishImportMerchantId = foodManager.merchants[0].id
        }
    }

    function selectedDishMerchantName() {
        for (let i = 0; i < foodManager.merchants.length; ++i) {
            if (foodManager.merchants[i].id === dishImportMerchantId) {
                return foodManager.merchants[i].name
            }
        }
        return foodManager.merchants.length > 0 ? foodManager.merchants[0].name : ""
    }

    function selectedDishMerchantId() {
        if (dishImportMerchantId > 0) {
            return dishImportMerchantId
        }
        return foodManager.merchants.length > 0 ? foodManager.merchants[0].id : 0
    }

    function loadFeedbackFromMeal(meal) {
        if (!meal) {
            selectedMealId = 0
            fullnessValue = 3
            sleepinessValue = 3
            comfortValue = 3
            focusValue = 3
            tasteValue = 3
            repeatValue = 3
            wouldEatAgain = true
            feedbackDraft = ""
            feedbackManualVisible = false
            feedbackStatus = "还没有可选择的餐次记录。"
            return
        }

        selectedMealId = meal.id
        fullnessValue = meal.feedbackSaved ? clampScore(meal.feedbackFullnessLevel, 3) : 3
        sleepinessValue = meal.feedbackSaved ? clampScore(meal.feedbackSleepinessLevel, 3) : 3
        comfortValue = meal.feedbackSaved ? clampScore(meal.feedbackComfortLevel, 3) : 3
        focusValue = meal.feedbackSaved ? clampScore(meal.feedbackFocusImpactLevel, 3) : 3
        tasteValue = meal.feedbackSaved ? clampScore(meal.feedbackTasteRating, 3) : 3
        repeatValue = meal.feedbackSaved ? clampScore(meal.feedbackRepeatWillingness, 3) : 3
        wouldEatAgain = meal.feedbackSaved ? meal.feedbackWouldEatAgain : true
        feedbackDraft = meal.feedbackText ? meal.feedbackText : ""
        feedbackManualVisible = meal.feedbackSaved
        feedbackStatus = meal.feedbackSaved ? "已加载这餐已有反馈，可继续更新。" : "这餐还没有反馈，填写后会保存到现有反馈表。"
    }

    function ensureSelectedMeal() {
        if (mealLogManager.recentMeals.length === 0) {
            loadFeedbackFromMeal(null)
            return
        }

        const current = findMeal(selectedMealId)
        loadFeedbackFromMeal(current ? current : mealLogManager.recentMeals[0])
    }

    function maybeRunPendingRecommendation() {
        if (!pendingRecommendation || recommendationEngine.busy
                || recommendationEngine.supplementState === "parsing") {
            return
        }

        pendingRecommendation = false
        recommendationEngine.runDecision()
    }

    function sendComposer() {
        updateDraftForActiveMode(composerInput.text)
        if (taskMode === "feedback") {
            parseFeedbackForPreview()
            return
        }
        if (taskMode === "dish") {
            parseDishForPreview()
            return
        }
        if (taskMode === "routine") {
            previewRoutineScaffold()
            return
        }

        pendingRecommendation = true
        if (recommendDraft.trim().length > 0) {
            setTaskPreview("recommend_next_meal",
                           "正在解析你的补充需求，随后仍由本地推荐引擎给出最终排序。",
                           ["解析自然语言补充", "刷新本地三条推荐", "展示本地理由和风险提醒"],
                           false,
                           [],
                           "parsing")
            recommendationEngine.parseSupplement(recommendDraft)
            maybeRunPendingRecommendation()
        } else {
            pendingRecommendation = false
            setTaskPreview("recommend_next_meal",
                           "正在使用当前时间、课表和本地规则刷新推荐。",
                           ["刷新本地三条推荐", "展示本地理由和风险提醒"],
                           false,
                           [],
                           "parsing")
            recommendationEngine.runDecision()
        }
    }

    function selectedMealSummary() {
        const meal = selectedMeal()
        if (!meal) {
            return ""
        }

        return meal.eatenAt + " | " + meal.mealTypeLabel + " | "
                + (meal.dishSummary.length > 0 ? meal.dishSummary : "未记录菜品")
    }

    function saveFeedback(successText) {
        if (selectedMealId <= 0) {
            feedbackStatus = "请先选择一条最近餐次。"
            return false
        }

        if (mealLogManager.saveMealFeedback(selectedMealId,
                                            fullnessValue,
                                            sleepinessValue,
                                            comfortValue,
                                            focusValue,
                                            wouldEatAgain,
                                            tasteValue,
                                            repeatValue,
                                            feedbackDraft)) {
            feedbackStatus = successText && successText.length > 0 ? successText : "反馈已保存。"
            appState.reload()
            ensureSelectedMeal()
            return true
        } else {
            feedbackStatus = mealLogManager.lastError
            return false
        }
    }

    function applyParsedFeedbackToFields() {
        const parsed = recommendationEngine.parsedFeedback
        fullnessValue = clampScore(parsed.fullnessLevel, fullnessValue)
        sleepinessValue = clampScore(parsed.sleepinessLevel, sleepinessValue)
        comfortValue = clampScore(parsed.comfortLevel, comfortValue)
        focusValue = clampScore(parsed.focusImpactLevel, focusValue)
        tasteValue = clampScore(parsed.tasteRating, tasteValue)
        repeatValue = clampScore(parsed.repeatWillingness, repeatValue)
        wouldEatAgain = parsed.wouldEatAgain
        if (parsed.freeTextFeedback && parsed.freeTextFeedback.length > 0) {
            feedbackDraft = parsed.freeTextFeedback
        }
        composerInput.text = feedbackDraft
    }

    function parseFeedbackForPreview() {
        ensureSelectedMeal()
        if (selectedMealId <= 0) {
            feedbackStatus = "请先保存一条餐次，再补餐后反馈。"
            setTaskPreview("record_previous_meal_feedback",
                           "没有可写入的最近餐次。",
                           [],
                           false,
                           ["最近餐次"],
                           "needs_clarification")
            return
        }
        if (feedbackDraft.trim().length === 0) {
            feedbackManualVisible = true
            feedbackStatus = "先写一句饭后感受；也可以直接使用手动打分。"
            setTaskPreview("record_previous_meal_feedback",
                           "需要先输入反馈文本，或在抽屉里手动打分后保存。",
                           ["目标餐次：" + selectedMealSummary()],
                           true,
                           ["饭后感受"],
                           "needs_clarification")
            return
        }

        pendingFeedbackParsePreview = true
        feedbackStatus = "正在解析反馈，解析后需要确认才会保存。"
        setTaskPreview("record_previous_meal_feedback",
                       "正在把自然语言反馈解析为本地反馈字段。",
                       ["目标餐次：" + selectedMealSummary(), "解析口味、复吃、饱腹、犯困、舒适、专注"],
                       true,
                       [],
                       "parsing")
        recommendationEngine.parseFeedback(feedbackDraft, selectedMealSummary())
    }

    function parseAndSaveFeedback() {
        parseFeedbackForPreview()
    }

    function parseDishForPreview() {
        if (dishDraft.trim().length === 0) {
            setTaskPreview("import_dishes",
                           "需要先输入菜品描述。",
                           [],
                           true,
                           ["菜品描述"],
                           "needs_clarification")
            return
        }
        if (selectedDishMerchantId() <= 0) {
            setTaskPreview("import_dishes",
                           "导入菜品前需要至少有一个商家。可先在管理抽屉的餐食配置里新增商家。",
                           ["不会直接写入数据库"],
                           true,
                           ["商家"],
                           "needs_clarification")
            return
        }

        pendingDishParsePreview = true
        setTaskPreview("import_dishes",
                       "正在解析菜品，解析结果会先预览，确认后才会通过 FoodManager 保存。",
                       ["目标商家：" + selectedDishMerchantName(), "解析菜名、价格、用餐方式和低/中/高标签"],
                       true,
                       [],
                       "parsing")
        recommendationEngine.parseDishInput(dishDraft, selectedDishMerchantName())
    }

    function previewRoutineScaffold() {
        if (routineDraft.trim().length === 0) {
            setTaskPreview("import_temporary_routine",
                           "需要先输入临时日常内容。",
                           [],
                           false,
                           ["临时日常描述"],
                           "needs_clarification")
            return
        }

        setTaskPreview("import_temporary_routine",
                       "已生成临时日常预览。本轮还没有新增临时日常表，所以不会写入数据库或影响推荐。",
                       ["输入内容：" + routineDraft, "后续需要 temporary_events 持久化和推荐上下文合并"],
                       false,
                       [],
                       "preview")
    }

    function confirmTaskPreview() {
        if (taskIntent === "record_previous_meal_feedback") {
            taskState = "applying"
            if (saveFeedback("反馈已保存。")) {
                feedbackManualVisible = false
                setTaskPreview("record_previous_meal_feedback",
                               "反馈已通过 MealLogManager 保存到目标餐次。",
                               ["已更新反馈表", "可在反馈与记录里继续查看或修改"],
                               false,
                               [],
                               "applied")
            } else {
                setTaskPreview("record_previous_meal_feedback",
                               mealLogManager.lastError.length > 0 ? mealLogManager.lastError : "反馈保存失败。",
                               [],
                               true,
                               [],
                               "failed")
            }
            return
        }

        if (taskIntent === "import_dishes") {
            const dish = recommendationEngine.parsedDish
            const merchantId = selectedDishMerchantId()
            if (merchantId <= 0 || !dish || !dish.name || dish.name.length === 0) {
                setTaskPreview("import_dishes",
                               "菜品预览信息不完整，暂不能导入。",
                               [],
                               true,
                               ["商家", "菜品名"],
                               "needs_clarification")
                return
            }

            taskState = "applying"
            const ok = foodManager.addDish(dish.name,
                                           merchantId,
                                           dish.category || "",
                                           Number(dish.price || 0),
                                           dish.defaultDiningMode || "dine_in",
                                           Number(dish.eatTimeMinutes || 15),
                                           Number(dish.acquireEffortScore || 1),
                                           dish.carbLevel || "medium",
                                           dish.fatLevel || "medium",
                                           dish.proteinLevel || "medium",
                                           dish.vitaminLevel || "medium",
                                           dish.fiberLevel || "medium",
                                           dish.satietyLevel || "medium",
                                           dish.digestiveBurdenLevel || "medium",
                                           dish.sleepinessRiskLevel || "medium",
                                           dish.flavorLevel || "medium",
                                           dish.odorLevel || "low",
                                           !!dish.isCombo,
                                           !!dish.isBeverage,
                                           Number(dish.mealImpactWeight || 1.0),
                                           dish.notes || "")
            if (ok) {
                mealLogManager.reload()
                recommendationEngine.runDecision()
                setTaskPreview("import_dishes",
                               "菜品已通过 FoodManager 保存。",
                               [dish.name + " | " + selectedDishMerchantName(), "本地推荐已刷新"],
                               false,
                               [],
                               "applied")
            } else {
                setTaskPreview("import_dishes",
                               foodManager.lastError.length > 0 ? foodManager.lastError : "菜品保存失败。",
                               [],
                               true,
                               [],
                               "failed")
            }
        }
    }

    function syncLlmFields() {
        apiKeyField.text = appConfig.llmApiKey
        apiUrlField.text = appConfig.llmApiUrl
        modelField.text = appConfig.llmModel
    }

    Component.onCompleted: {
        recommendationEngine.refreshSupplementConfigState()
        mealLogManager.reload()
        ensureSelectedMeal()
    }

    Connections {
        target: recommendationEngine

        function onSupplementChanged() {
            maybeRunPendingRecommendation()
        }

        function onBusyChanged() {
            maybeRunPendingRecommendation()
            if (!recommendationEngine.busy && taskIntent === "recommend_next_meal"
                    && taskState === "parsing") {
                setTaskPreview("recommend_next_meal",
                               "本地推荐已刷新。排序仍由本地规则生成，LLM 只负责解析补充需求。",
                               ["查看当前三条推荐", "确认推荐理由和提醒"],
                               false,
                               [],
                               "preview")
            }
        }

        function onFeedbackParseChanged() {
            if (!pendingFeedbackParsePreview) {
                return
            }

            const state = recommendationEngine.feedbackParseState
            if (state === "success") {
                pendingFeedbackParsePreview = false
                applyParsedFeedbackToFields()
                feedbackManualVisible = true
                feedbackStatus = "已解析反馈，请确认分数后保存。"
                setTaskPreview("record_previous_meal_feedback",
                               "反馈已解析，请确认后写入目标餐次。",
                               [
                                   "目标餐次：" + selectedMealSummary(),
                                   "口味 " + tasteValue + " / 复吃 " + repeatValue,
                                   "饱腹 " + fullnessValue + " / 犯困 " + sleepinessValue,
                                   "舒适 " + comfortValue + " / 专注 " + focusValue
                               ],
                               true,
                               [],
                               "preview")
                return
            }

            if (state !== "parsing" && state.length > 0) {
                pendingFeedbackParsePreview = false
                feedbackManualVisible = true
                feedbackStatus = recommendationEngine.feedbackParseStatus
                setTaskPreview("record_previous_meal_feedback",
                               recommendationEngine.feedbackParseStatus,
                               ["可改用手动打分保存", "不会写入无效解析结果"],
                               true,
                               [],
                               "failed")
            }
        }

        function onDishParseChanged() {
            if (!pendingDishParsePreview) {
                return
            }

            const state = recommendationEngine.dishParseState
            if (state === "success") {
                pendingDishParsePreview = false
                const dish = recommendationEngine.parsedDish
                setTaskPreview("import_dishes",
                               "菜品已解析，请确认后导入。",
                               [
                                   "目标商家：" + selectedDishMerchantName(),
                                   "菜品：" + (dish.name || "未命名"),
                                   "价格：" + Number(dish.price || 0).toFixed(0) + " 元",
                                   "标签：碳水 " + (dish.carbLevel || "medium")
                                       + " / 蛋白 " + (dish.proteinLevel || "medium")
                                       + " / 负担 " + (dish.digestiveBurdenLevel || "medium")
                                       + " / 犯困 " + (dish.sleepinessRiskLevel || "medium")
                               ],
                               true,
                               [],
                               "preview")
                return
            }

            if (state !== "parsing" && state.length > 0) {
                pendingDishParsePreview = false
                setTaskPreview("import_dishes",
                               recommendationEngine.dishParseStatus,
                               ["可去餐食配置手动新增", "不会写入无效解析结果"],
                               true,
                               [],
                               "failed")
            }
        }
    }

    Connections {
        target: mealLogManager

        function onStateChanged() {
            if (selectedMealId > 0 || mealLogManager.recentMeals.length > 0) {
                ensureSelectedMeal()
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#fff9f0" }
            GradientStop { position: 0.55; color: "#f4eadc" }
            GradientStop { position: 1.0; color: "#eadcc8" }
        }
    }

    Rectangle {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: 170
        color: "#ead0ad"
        opacity: 0.55
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        AutoHeightRectangle {
            Layout.fillWidth: true
            radius: 22
            color: "#fdf7ed"
            border.color: "#ead8bf"
            border.width: 1

            RowLayout {
                x: 16
                y: 16
                width: parent.width - 32
                height: implicitHeight
                spacing: 14

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    Label {
                        Layout.fillWidth: true
                        text: "MealAdvisor"
                        color: "#3d2d22"
                        font.pixelSize: 28
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: false
                        text: appState.planningSummary + " · " + appState.budgetSummary
                        color: "#52645b"
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                }

                MenuIconButton {
                    ToolTip.visible: hovered
                    ToolTip.text: "打开管理抽屉"
                    onClicked: {
                        syncLlmFields()
                        managementDrawer.open()
                    }
                }
            }
        }

        ScrollView {
            id: resultScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ColumnLayout {
                id: contentColumn

                width: Math.max(resultScroll.availableWidth, 1)
                height: implicitHeight
                spacing: 12

                Loader {
                    Layout.fillWidth: true
                    active: taskState !== "idle"
                    visible: active
                    sourceComponent: taskPreviewComponent
                }

                Loader {
                    Layout.fillWidth: true
                    sourceComponent: recommendComponent
                }
            }
        }

        AutoHeightRectangle {
            Layout.fillWidth: true
            radius: 22
            color: "#fdf7ed"
            border.color: "#ead8bf"
            border.width: 1

            ColumnLayout {
                x: 14
                y: 14
                width: parent.width - 28
                height: implicitHeight
                spacing: 12

                GridLayout {
                    Layout.fillWidth: true
                    columns: width < 360 ? 2 : 4
                    columnSpacing: 8
                    rowSpacing: 8

                    ModeButton {
                        text: "推荐"
                        selected: taskMode === "recommend"
                        onClicked: switchTaskMode("recommend")
                    }

                    ModeButton {
                        text: "反馈"
                        selected: taskMode === "feedback"
                        onClicked: switchTaskMode("feedback")
                    }

                    ModeButton {
                        text: "菜品"
                        selected: taskMode === "dish"
                        onClicked: switchTaskMode("dish")
                    }

                    ModeButton {
                        text: "日常"
                        selected: taskMode === "routine"
                        onClicked: switchTaskMode("routine")
                    }
                }

                ComboBox {
                    id: dishMerchantSelector
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    visible: taskMode === "dish"
                    textRole: "name"
                    valueRole: "id"
                    model: foodManager.merchants
                    onActivated: dishImportMerchantId = currentValue
                    Component.onCompleted: {
                        if (dishImportMerchantId <= 0 && foodManager.merchants.length > 0) {
                            dishImportMerchantId = foodManager.merchants[0].id
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    StyledTextArea {
                        id: composerInput
                        Layout.fillWidth: true
                        Layout.preferredHeight: 92
                        hintText: activeTaskPlaceholder()
                        wrapMode: TextEdit.Wrap
                        text: recommendDraft
                        onTextChanged: updateDraftForActiveMode(text)
                    }

                    ReadableButton {
                        Layout.preferredWidth: 88
                        Layout.preferredHeight: 92
                        primary: true
                        enabled: taskMode === "feedback"
                                 ? !recommendationEngine.feedbackParseBusy
                                 : taskMode === "dish"
                                   ? !recommendationEngine.dishParseBusy
                                   : taskMode === "recommend"
                                     ? (!recommendationEngine.busy
                                        && recommendationEngine.supplementState !== "parsing")
                                     : true
                        text: (recommendationEngine.busy
                               || recommendationEngine.supplementState === "parsing"
                               || recommendationEngine.feedbackParseBusy
                               || recommendationEngine.dishParseBusy)
                              ? "处理中" : "发送"
                        onClicked: sendComposer()
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: false
                    text: mealLogManager.lastError
                    color: "#9a4b2f"
                    wrapMode: Text.Wrap
                }
            }
        }
    }

    Component {
        id: taskPreviewComponent

        AutoHeightRectangle {
            Layout.fillWidth: true
            radius: 18
            color: taskState === "failed" ? "#fff5f1"
                   : taskState === "applied" ? "#f1f7ec"
                   : "#fff5e8"
            border.color: taskState === "failed" ? "#e9b9aa"
                          : taskState === "applied" ? "#b9d0a9"
                          : "#dec4a3"
            border.width: 1

            ColumnLayout {
                x: 16
                y: 16
                width: parent.width - 32
                height: implicitHeight
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        Layout.fillWidth: true
                        text: taskModeLabel(taskMode) + "预览"
                        color: "#3d2d22"
                        font.pixelSize: 20
                        font.bold: true
                        wrapMode: Text.Wrap
                    }

                    Label {
                        text: taskState === "parsing" ? "解析中"
                              : taskState === "needs_clarification" ? "需补充"
                              : taskState === "applied" ? "已应用"
                              : taskState === "failed" ? "未应用"
                              : "待确认"
                        color: taskState === "failed" ? "#8a4635" : "#6b5846"
                        font.bold: true
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: taskSummary
                    color: "#6b5846"
                    wrapMode: Text.Wrap
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: taskActions.length > 0
                    spacing: 4

                    Repeater {
                        model: taskActions

                        Label {
                            Layout.fillWidth: true
                            text: "- " + modelData
                            color: "#705e4d"
                            wrapMode: Text.Wrap
                        }
                    }
                }

                Flow {
                    Layout.fillWidth: true
                    visible: taskMissingFields.length > 0
                    spacing: 8

                    Repeater {
                        model: taskMissingFields

                        AutoHeightRectangle {
                            width: missingLabel.implicitWidth + 18
                            height: missingLabel.implicitHeight + 10
                            radius: 10
                            color: "#fffaf2"
                            border.color: "#d59b7c"
                            border.width: 1

                            Label {
                                id: missingLabel
                                anchors.centerIn: parent
                                text: "缺：" + modelData
                                color: "#8a4635"
                            }
                        }
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    visible: taskRequiresConfirmation
                    columns: width < 360 ? 1 : 2
                    columnSpacing: 8
                    rowSpacing: 8

                    ReadableButton {
                        Layout.fillWidth: true
                        primary: true
                        enabled: taskState === "preview"
                                 || (taskState === "failed"
                                     && taskIntent === "record_previous_meal_feedback")
                        text: taskIntent === "import_dishes" ? "确认导入"
                              : taskIntent === "record_previous_meal_feedback" ? "确认保存"
                              : "确认"
                        onClicked: confirmTaskPreview()
                    }

                    ReadableButton {
                        Layout.fillWidth: true
                        text: taskIntent === "import_dishes" && taskMissingFields.length > 0
                              ? "打开餐食配置"
                              : "清除预览"
                        onClicked: {
                            if (taskIntent === "import_dishes" && taskMissingFields.length > 0) {
                                drawerSection = "food"
                                syncLlmFields()
                                managementDrawer.open()
                            } else {
                                clearTaskPreview()
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: recommendComponent

        ColumnLayout {
            width: contentColumn.width
            spacing: 12

        AutoHeightRectangle {
            Layout.fillWidth: true
            radius: 18
            color: "#fdf7ed"
            border.color: "#ead8bf"
            border.width: 1

            ColumnLayout {
                x: 16
                y: 16
                width: parent.width - 32
                height: implicitHeight
                spacing: 10

                Label {
                    Layout.fillWidth: true
                    text: "当前推荐"
                    color: "#3d2d22"
                    font.pixelSize: 22
                    font.bold: true
                    wrapMode: Text.Wrap
                }

                Label {
                    Layout.fillWidth: true
                    text: recommendationEngine.summary
                    color: "#6b5846"
                    wrapMode: Text.Wrap
                }
            }
        }

        Repeater {
            model: recommendationEngine.candidates

            AutoHeightRectangle {
                Layout.fillWidth: true
                radius: 16
                color: "#fff9f0"
                border.color: "#ead8bf"
                border.width: 1

                ColumnLayout {
                    x: 14
                    y: 14
                    width: parent.width - 28
                    height: implicitHeight
                    spacing: 8

                    Label {
                        Layout.fillWidth: true
                        text: "#" + modelData.rank + " " + modelData.dishName
                              + (modelData.merchantName.length > 0 ? (" | " + modelData.merchantName) : "")
                        color: "#3f3024"
                        font.bold: true
                        wrapMode: Text.Wrap
                    }

                    Label {
                        Layout.fillWidth: true
                        text: "分数 " + modelData.score + " | 价格 "
                              + Number(modelData.price).toFixed(0) + " 元"
                        color: "#6b5846"
                        wrapMode: Text.Wrap
                    }

                    Label {
                        visible: modelData.reason && modelData.reason.length > 0
                        Layout.fillWidth: true
                        text: modelData.reason
                        color: "#705e4d"
                        wrapMode: Text.Wrap
                    }

                    ColumnLayout {
                        visible: modelData.reasons && modelData.reasons.length > 0
                        Layout.fillWidth: true
                        spacing: 4

                        Repeater {
                            model: modelData.reasons

                            Label {
                                Layout.fillWidth: true
                                text: "- " + modelData
                                color: "#705e4d"
                                wrapMode: Text.Wrap
                            }
                        }
                    }

                    AutoHeightRectangle {
                        visible: modelData.warnings && modelData.warnings.length > 0
                        Layout.fillWidth: true
                        radius: 12
                        color: "#fff5f1"
                        border.color: "#e9b9aa"
                        border.width: 1

                        ColumnLayout {
                            x: 10
                            y: 10
                            width: parent.width - 20
                            height: implicitHeight
                            spacing: 4

                            Label {
                                text: "提醒"
                                color: "#8a4635"
                                font.bold: true
                            }

                            Repeater {
                                model: modelData.warnings

                                Label {
                                    Layout.fillWidth: true
                                    text: "- " + modelData
                                    color: "#8a4635"
                                    wrapMode: Text.Wrap
                                }
                            }
                        }
                    }

                    Flow {
                        Layout.fillWidth: true
                        visible: modelData.breakdown && modelData.breakdown.length > 0
                        spacing: 8

                        Repeater {
                            model: modelData.breakdown

                            AutoHeightRectangle {
                                width: breakdownLabel.implicitWidth + 16
                                height: breakdownLabel.implicitHeight + 10
                                radius: 10
                                color: "#f5e7d3"
                                border.color: "#dec4a3"
                                border.width: 1

                                Label {
                                    id: breakdownLabel
                                    anchors.centerIn: parent
                                    text: modelData.label + " " + modelData.score
                                    color: "#6b5846"
                                }
                            }
                        }
                    }
                }
            }
        }

        }
    }

    Component {
        id: feedbackComponent

        ColumnLayout {
            width: Math.max(parent ? parent.width : contentColumn.width, 1)
            spacing: 12

        AutoHeightRectangle {
            Layout.fillWidth: true
            radius: 18
            color: "#fdf7ed"
            border.color: "#ead8bf"
            border.width: 1

            ColumnLayout {
                x: 16
                y: 16
                width: parent.width - 32
                height: implicitHeight
                spacing: 12

                Label {
                    Layout.fillWidth: true
                    text: "饭后反馈"
                    color: "#3d2d22"
                    font.pixelSize: 22
                    font.bold: true
                    wrapMode: Text.Wrap
                }

                ComboBox {
                    id: mealSelector
                    Layout.fillWidth: true
                    Layout.preferredHeight: 46
                    textRole: "label"
                    valueRole: "id"
                    model: mealLogManager.recentMeals.map(function(meal) {
                        return {
                            id: meal.id,
                            label: meal.eatenAt + " | " + meal.mealTypeLabel + " | "
                                   + (meal.dishSummary.length > 0 ? meal.dishSummary : "未记录菜品")
                        }
                    })
                    onActivated: {
                        const meal = findMeal(currentValue)
                        loadFeedbackFromMeal(meal)
                    }
                    Component.onCompleted: {
                        if (selectedMealId > 0) {
                            currentIndex = indexOfValue(selectedMealId)
                        }
                    }
                }

                AutoHeightRectangle {
                    Layout.fillWidth: true
                    radius: 14
                    color: "#fff5e8"
                    border.color: "#dec4a3"
                    border.width: 1

                    ColumnLayout {
                        x: 12
                        y: 12
                        width: parent.width - 24
                        height: implicitHeight
                        spacing: 6

                        Label {
                            Layout.fillWidth: true
                            text: {
                                const meal = selectedMeal()
                                return meal ? (meal.dishSummary.length > 0 ? meal.dishSummary : "这餐没有菜品摘要")
                                            : "暂无最近餐次"
                            }
                            color: "#473525"
                            font.bold: true
                            wrapMode: Text.Wrap
                        }

                        Label {
                            Layout.fillWidth: true
                            text: {
                                const meal = selectedMeal()
                                return meal ? meal.feedbackSummary : "先在餐次记录里保存一餐。"
                            }
                            color: "#786754"
                            wrapMode: Text.Wrap
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: "优先用一句话记录这餐的真实感受；能连上 LLM 时会解析成具体分数，确认后才保存。"
                    color: "#6b5846"
                    wrapMode: Text.Wrap
                }

                StyledTextArea {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 96
                    text: window.feedbackDraft
                    hintText: "例如：味道不错但有点撑，饭后半小时犯困，下次可以少点一点。"
                    wrapMode: TextEdit.Wrap
                    onTextChanged: window.feedbackDraft = text
                    Component.onCompleted: hintText = "例如：味道不错但有点撑，饭后有点犯困。"
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: width < 420 ? 1 : 2
                    columnSpacing: 8
                    rowSpacing: 8

                    ReadableButton {
                        Layout.fillWidth: true
                        enabled: selectedMealId > 0 && !recommendationEngine.feedbackParseBusy
                        text: recommendationEngine.feedbackParseBusy ? "解析中..." : "解析预览反馈"
                        onClicked: parseAndSaveFeedback()
                    }

                    ReadableButton {
                        Layout.fillWidth: true
                        text: feedbackManualVisible ? "收起手动打分" : "手动打分"
                        onClicked: feedbackManualVisible = !feedbackManualVisible
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: feedbackManualVisible
                    text: "手动分数说明：1 表示很弱/很差，5 表示很强/很好；犯困 5 表示很困。"
                    color: "#786754"
                    wrapMode: Text.Wrap
                }

                GridLayout {
                    id: feedbackMetrics
                    Layout.fillWidth: true
                    visible: feedbackManualVisible
                    columns: feedbackMetrics.width < 390 ? 1 : 2
                    columnSpacing: 12
                    rowSpacing: 8

                    FeedbackMetric {
                        Layout.fillWidth: true
                        label: "口味"
                        value: window.tasteValue
                        onValueChanged: window.tasteValue = value
                    }

                    FeedbackMetric {
                        Layout.fillWidth: true
                        label: "复吃"
                        value: window.repeatValue
                        onValueChanged: window.repeatValue = value
                    }

                    FeedbackMetric {
                        Layout.fillWidth: true
                        label: "饱腹"
                        value: window.fullnessValue
                        onValueChanged: window.fullnessValue = value
                    }

                    FeedbackMetric {
                        Layout.fillWidth: true
                        label: "犯困"
                        value: window.sleepinessValue
                        onValueChanged: window.sleepinessValue = value
                    }

                    FeedbackMetric {
                        Layout.fillWidth: true
                        label: "舒适"
                        value: window.comfortValue
                        onValueChanged: window.comfortValue = value
                    }

                    FeedbackMetric {
                        Layout.fillWidth: true
                        label: "专注"
                        value: window.focusValue
                        onValueChanged: window.focusValue = value
                    }
                }

                CheckBox {
                    Layout.fillWidth: true
                    visible: feedbackManualVisible
                    Layout.preferredHeight: 42
                    text: "下次还愿意吃"
                    checked: window.wouldEatAgain
                    onToggled: window.wouldEatAgain = checked
                }

                ReadableButton {
                    Layout.fillWidth: true
                    visible: feedbackManualVisible
                    enabled: selectedMealId > 0
                    text: "确认保存反馈"
                    onClicked: saveFeedback("手动反馈已保存。")
                }

                Label {
                    Layout.fillWidth: true
                    text: feedbackStatus
                    color: "#6b5846"
                    wrapMode: Text.Wrap
                }
            }
        }

        AutoHeightRectangle {
            Layout.fillWidth: true
            radius: 16
            color: "#fff9f0"
            border.color: "#ead8bf"
            border.width: 1

            ColumnLayout {
                x: 16
                y: 16
                width: parent.width - 32
                height: implicitHeight
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: "最近餐次"
                    color: "#3d2d22"
                    font.bold: true
                    wrapMode: Text.Wrap
                }

                Repeater {
                    model: mealLogManager.recentMeals.slice(0, 5)

                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 52
                        flat: true
                        onClicked: loadFeedbackFromMeal(modelData)

                        background: Rectangle {
                            radius: 10
                            color: modelData.id === selectedMealId ? "#f2dcc1" : "#fffaf2"
                            border.color: modelData.id === selectedMealId ? "#c98b3d" : "#dec4a3"
                            border.width: 1
                        }

                        contentItem: Label {
                            text: modelData.eatenAt + " | "
                                  + (modelData.dishSummary.length > 0 ? modelData.dishSummary : modelData.mealTypeLabel)
                            color: "#5c4937"
                            wrapMode: Text.Wrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
        }
    }

    Drawer {
        id: managementDrawer
        edge: Qt.RightEdge
        modal: true
        interactive: false
        width: window.width < 640 || window.height < 520
               ? Math.max(300, window.width - 12)
               : Math.min(window.width - 16, Math.max(420, Math.round(window.width * 0.52)))
        height: window.height
        y: 0
        onOpened: syncLlmFields()

        background: Rectangle {
            color: "#fff9f0"
            border.color: "#dec4a3"
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: "管理"
                    color: "#3d2d22"
                    font.pixelSize: 22
                    font.bold: true
                    elide: Text.ElideRight
                }

                CloseIconButton {
                    ToolTip.visible: hovered
                    ToolTip.text: "关闭"
                    onClicked: managementDrawer.close()
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: managementDrawer.width < 520 ? 2 : 3
                columnSpacing: 8
                rowSpacing: 8

                Repeater {
                    model: [
                        { key: "llm", label: "LLM 调试" },
                        { key: "schedule", label: "课表配置" },
                        { key: "food", label: "餐食配置" },
                        { key: "feedback", label: "饭后反馈" },
                        { key: "meals", label: "反馈与记录" }
                    ]

                    DrawerSectionButton {
                        text: modelData.label
                        selected: drawerSection === modelData.key
                        onClicked: drawerSection = modelData.key
                    }
                }
            }


            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: drawerSection === "llm" ? 0
                              : drawerSection === "schedule" ? 1
                              : drawerSection === "food" ? 2
                              : drawerSection === "feedback" ? 3
                              : 4

                ScrollView {
                    id: llmScroll
                    clip: true
                    contentWidth: availableWidth
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    ColumnLayout {
                        width: Math.max(llmScroll.availableWidth, 1)
                        height: implicitHeight
                        spacing: 12

                        AutoHeightRectangle {
                            Layout.fillWidth: true
                            radius: 16
                            color: "#fff5e8"
                            border.color: "#dec4a3"
                            border.width: 1

                            ColumnLayout {
                                x: 14
                                y: 14
                                width: parent.width - 28
                                height: implicitHeight
                                spacing: 10

                                Label {
                                    Layout.fillWidth: true
                                    text: "LLM 调试"
                                    color: "#3d2d22"
                                    font.pixelSize: 20
                                    font.bold: true
                                    wrapMode: Text.Wrap
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: appConfig.llmConfigSummary
                                    color: "#6b5846"
                                    wrapMode: Text.Wrap
                                }
                            }
                        }

                        AutoHeightRectangle {
                            Layout.fillWidth: true
                            radius: 16
                            color: "#fffaf2"
                            border.color: "#dec4a3"
                            border.width: 1

                            ColumnLayout {
                                x: 14
                                y: 14
                                width: parent.width - 28
                                height: implicitHeight
                                spacing: 10

                                StyledTextField {
                                    id: apiKeyField
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 46
                                    hintText: "API Key"
                                    echoMode: TextInput.Password
                                    verticalAlignment: TextInput.AlignVCenter
                                }

                                StyledTextField {
                                    id: apiUrlField
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 46
                                    hintText: appConfig.effectiveLlmApiUrl.length > 0
                                              ? appConfig.effectiveLlmApiUrl
                                              : "https://api.openai.com/v1/chat/completions"
                                    verticalAlignment: TextInput.AlignVCenter
                                }

                                StyledTextField {
                                    id: modelField
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 46
                                    hintText: appConfig.effectiveLlmModel.length > 0
                                              ? appConfig.effectiveLlmModel
                                              : "gpt-4o-mini"
                                    verticalAlignment: TextInput.AlignVCenter
                                }

                                GridLayout {
                                    id: llmActions
                                    Layout.fillWidth: true
                                    columns: llmActions.width < 440 ? 1 : 3
                                    columnSpacing: 8
                                    rowSpacing: 8

                                    ReadableButton {
                                        Layout.fillWidth: true
                                        text: "保存"
                                        onClicked: {
                                            appConfig.saveLlmSettings(apiKeyField.text,
                                                                      apiUrlField.text,
                                                                      modelField.text)
                                            recommendationEngine.refreshSupplementConfigState()
                                        }
                                    }

                                    ReadableButton {
                                        Layout.fillWidth: true
                                        text: recommendationEngine.llmConnectionTestBusy ? "测试中..." : "测试连接"
                                        enabled: !recommendationEngine.llmConnectionTestBusy
                                        onClicked: recommendationEngine.testLlmConnection(apiKeyField.text,
                                                                                          apiUrlField.text,
                                                                                          modelField.text)
                                    }

                                    ReadableButton {
                                        Layout.fillWidth: true
                                        text: "清空本地配置"
                                        onClicked: {
                                            appConfig.clearLlmSettings()
                                            syncLlmFields()
                                            recommendationEngine.refreshSupplementConfigState()
                                        }
                                    }

                                    ReadableButton {
                                        Layout.fillWidth: true
                                        text: "刷新推荐"
                                        enabled: !recommendationEngine.busy
                                        onClicked: recommendationEngine.runDecision()
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    visible: recommendationEngine.llmConnectionTestState !== "idle"
                                    text: recommendationEngine.llmConnectionTestStatus
                                    color: recommendationEngine.llmConnectionTestState === "success"
                                           ? "#3f6b3b"
                                           : recommendationEngine.llmConnectionTestState === "testing"
                                             ? "#6b5846"
                                             : "#8a4f2b"
                                    wrapMode: Text.Wrap
                                }
                            }
                        }
                    }
                }

                SchedulePage {
                }

                FoodPage {
                }

                ScrollView {
                    id: feedbackScroll
                    clip: true
                    contentWidth: availableWidth
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                    ColumnLayout {
                        id: feedbackDrawerContent
                        width: Math.max(feedbackScroll.availableWidth, 1)
                        height: implicitHeight
                        spacing: 12

                        Loader {
                            Layout.fillWidth: true
                            Layout.preferredHeight: item ? item.implicitHeight : 0
                            sourceComponent: feedbackComponent
                        }
                    }
                }

                MealLogPage {
                }
            }
        }
    }
}
