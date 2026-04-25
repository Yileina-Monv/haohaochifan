import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    component AutoHeightRectangle: Rectangle {
        Layout.preferredHeight: implicitHeight
        implicitHeight: childrenRect.height > 0
                        ? childrenRect.y + childrenRect.height + childrenRect.y
                        : 0
    }
    component ReadableButton: Button {
        id: readableButton

        contentItem: Label {
            text: readableButton.text
            color: readableButton.enabled ? "#2c241b" : "#8a8176"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
    id: root
    clip: true
    readonly property bool narrowLayout: root.availableWidth < 380
    property int selectedInsightIndex: -1
    property var mealTypeOptions: [
        { label: "早餐", value: "breakfast" },
        { label: "午餐", value: "lunch" },
        { label: "晚餐", value: "dinner" },
        { label: "加餐", value: "snack" }
    ]
    property var locationTypeOptions: [
        { label: "校内", value: "campus" },
        { label: "宿舍", value: "dorm" },
        { label: "通勤路上", value: "commute" },
        { label: "校外", value: "off_campus" }
    ]
    property var diningModeOptions: [
        { label: "堂食", value: "dine_in" },
        { label: "打包", value: "takeaway" },
        { label: "外卖", value: "delivery" }
    ]

    function weekdayValueForDate(date) {
        const day = date.getDay()
        return day === 0 ? 7 : day
    }

    function weekdayIndexForValue(value) {
        for (let i = 0; i < weekdayBox.count; ++i) {
            if (weekdayBox.model[i].value === value) {
                return i
            }
        }
        return 0
    }

    function mealTypeForHour(hour) {
        if (hour < 10) {
            return "breakfast"
        }
        if (hour < 15) {
            return "lunch"
        }
        if (hour < 21) {
            return "dinner"
        }
        return "snack"
    }

    function comboIndexByValue(box, value) {
        for (let i = 0; i < box.count; ++i) {
            const item = box.model[i]
            if (item && item.value !== undefined) {
                if (item.value === value) {
                    return i
                }
                continue
            }

            if (item === value) {
                return i
            }
        }
        return box.count > 0 ? 0 : -1
    }

    function applyDefaults() {
        const now = new Date()
        mealTypeBox.currentIndex = comboIndexByValue(mealTypeBox, mealTypeForHour(now.getHours()))
        weekdayBox.currentIndex = weekdayIndexForValue(weekdayValueForDate(now))
        locationTypeBox.currentIndex = comboIndexByValue(locationTypeBox, "campus")
        diningModeBox.currentIndex = comboIndexByValue(diningModeBox, "dine_in")
        eatenAtField.text = Qt.formatDateTime(now, "yyyy-MM-ddThh:mm:ss")
        totalPriceField.text = "0"
        eatTimeField.text = "15"
        nextClassField.text = "0"
        hungerSlider.value = 3
        energySlider.value = 3
        classAfterMealCheck.checked = false
        moodField.text = ""
        mealNotesField.text = ""
        portionRatioField.text = ""
        selectedDishNoteField.text = ""
        mealLogManager.setDishSearch("")
        dishSearchField.text = ""
    }

    function resetEditor() {
        mealLogManager.cancelEditingMealLog()
        applyDefaults()
    }

    function currentInsight() {
        if (selectedInsightIndex < 0 || selectedInsightIndex >= mealLogManager.feedbackInsights.length) {
            return null
        }
        return mealLogManager.feedbackInsights[selectedInsightIndex]
    }

    function selectInsight(index) {
        if (index >= 0 && index < mealLogManager.feedbackInsights.length) {
            selectedInsightIndex = index
        }
    }

    function selectedInsightHasMeal(mealId) {
        const insight = currentInsight()
        if (!insight || !insight.supportingMeals) {
            return false
        }

        for (let i = 0; i < insight.supportingMeals.length; ++i) {
            if (insight.supportingMeals[i].id === mealId) {
                return true
            }
        }
        return false
    }

    function selectedInsightHasDish(dishId) {
        const insight = currentInsight()
        if (!insight || !insight.supportingDishes) {
            return false
        }

        for (let i = 0; i < insight.supportingDishes.length; ++i) {
            if (insight.supportingDishes[i].id === dishId) {
                return true
            }
        }
        return false
    }

    function focusDishSearch(dishName) {
        dishSearchField.text = dishName
        mealLogManager.setDishSearch(dishName)
        dishPicker.currentIndex = 0
    }

    function optionLabel(options, value) {
        for (let i = 0; i < options.length; ++i) {
            if (options[i].value === value) {
                return options[i].label
            }
        }
        return value
    }

    function copyMealDishesFromTemplate(meal) {
        if (!meal || !meal.dishItems) {
            return
        }

        mealLogManager.clearSelection()
        let carriedDiningMode = ""
        for (let i = 0; i < meal.dishItems.length; ++i) {
            const dishItem = meal.dishItems[i]
            mealLogManager.addSelectedDish(
                dishItem.id,
                Number(dishItem.portionRatio || "0"),
                dishItem.customNotes || ""
            )
            if (!carriedDiningMode && dishItem.defaultDiningMode) {
                carriedDiningMode = dishItem.defaultDiningMode
            }
        }

        if (!carriedDiningMode && meal.diningMode) {
            carriedDiningMode = meal.diningMode
        }

        if (carriedDiningMode) {
            diningModeBox.currentIndex = comboIndexByValue(diningModeBox, carriedDiningMode)
        }
    }

    function applyMealTemplate(meal, dishesOnly) {
        if (!meal) {
            return
        }

        mealLogManager.cancelEditingMealLog()
        copyMealDishesFromTemplate(meal)
        mealLogManager.setDishSearch("")
        dishSearchField.text = ""
        dishPicker.currentIndex = 0
        portionRatioField.text = ""
        selectedDishNoteField.text = ""

        if (dishesOnly) {
            return
        }

        const now = new Date()
        mealTypeBox.currentIndex = comboIndexByValue(mealTypeBox, meal.mealType)
        weekdayBox.currentIndex = weekdayIndexForValue(weekdayValueForDate(now))
        locationTypeBox.currentIndex = comboIndexByValue(locationTypeBox, meal.locationType)
        diningModeBox.currentIndex = comboIndexByValue(diningModeBox, meal.diningMode)
        eatenAtField.text = Qt.formatDateTime(now, "yyyy-MM-ddThh:mm:ss")
        totalPriceField.text = String(meal.totalPrice)
        eatTimeField.text = String(meal.totalEatTimeMinutes)
        nextClassField.text = "0"
        hungerSlider.value = 3
        energySlider.value = 3
        classAfterMealCheck.checked = false
        moodField.text = ""
        mealNotesField.text = ""
    }

    function weightDirectionLabel(direction) {
        if (direction === "increase") {
            return "建议提高"
        }
        if (direction === "decrease") {
            return "建议降低"
        }
        return "建议复核"
    }

    function candidateRankLabel(rank) {
        if (rank > 0) {
            return "候选 " + rank
        }
        return "候选"
    }

    function toneCardColor(tone, selected) {
        if (selected) {
            if (tone === "good") {
                return "#e3f3e6"
            }
            if (tone === "risk") {
                return "#ffe8e1"
            }
            return "#f9f0dd"
        }

        if (tone === "good") {
            return "#eef8ef"
        }
        if (tone === "risk") {
            return "#fff0ec"
        }
        return "#f7f4ea"
    }

    function toneBorderColor(tone, selected) {
        if (selected) {
            if (tone === "good") {
                return "#77aa7a"
            }
            if (tone === "risk") {
                return "#cc816f"
            }
            return "#b79858"
        }

        if (tone === "good") {
            return "#b9d9bb"
        }
        if (tone === "risk") {
            return "#e4bbb0"
        }
        return "#ddd0ad"
    }

    function readableWeightDirectionLabel(direction) {
        if (direction === "increase") {
            return "建议提高"
        }
        if (direction === "decrease") {
            return "建议降低"
        }
        return "建议复核"
    }

    function readableCandidateRankLabel(rank) {
        if (rank > 0) {
            return "第 " + rank + " 候选"
        }
        return "候选"
    }

    function loadMealForEdit(mealId) {
        const meal = mealLogManager.loadMealLogForEdit(mealId)
        if (!meal || meal.id === undefined) {
            return
        }

        mealTypeBox.currentIndex = comboIndexByValue(mealTypeBox, meal.mealType)
        weekdayBox.currentIndex = weekdayIndexForValue(meal.weekday)
        locationTypeBox.currentIndex = comboIndexByValue(locationTypeBox, meal.locationType)
        diningModeBox.currentIndex = comboIndexByValue(diningModeBox, meal.diningMode)
        eatenAtField.text = meal.eatenAt
        totalPriceField.text = String(meal.totalPrice)
        eatTimeField.text = String(meal.totalEatTimeMinutes)
        nextClassField.text = String(meal.minutesUntilNextClass)
        classAfterMealCheck.checked = meal.hasClassAfterMeal
        hungerSlider.value = meal.preMealHungerLevel > 0 ? meal.preMealHungerLevel : 3
        energySlider.value = meal.preMealEnergyLevel > 0 ? meal.preMealEnergyLevel : 3
        moodField.text = meal.moodTag
        mealNotesField.text = meal.notes
    }

    function readableWeightDirectionLabelV2(direction) {
        if (direction === "increase") {
            return "建议提高"
        }
        if (direction === "decrease") {
            return "建议降低"
        }
        return "建议复核"
    }

    function readableCandidateRankLabelV2(rank) {
        if (rank > 0) {
            return "第 " + rank + " 候选"
        }
        return "候选"
    }

    function addPickedDish(dishMap) {
        const defaultWeight = dishMap ? dishMap.mealImpactWeight : 1.0
        const rawWeight = Number(portionRatioField.text || defaultWeight)
        const selectionWasEmpty = mealLogManager.selectedDishes.length === 0
        const ok = mealLogManager.addSelectedDish(
            dishMap.id,
            rawWeight > 0 ? rawWeight : defaultWeight,
            selectedDishNoteField.text
        )
        if (ok) {
            if (selectionWasEmpty && dishMap.defaultDiningMode) {
                diningModeBox.currentIndex = comboIndexByValue(diningModeBox, dishMap.defaultDiningMode)
            }
            portionRatioField.text = ""
            selectedDishNoteField.text = ""
            dishSearchField.text = ""
            mealLogManager.setDishSearch("")
            dishPicker.currentIndex = 0
        }
    }

    Component.onCompleted: applyDefaults()

    Connections {
        target: mealLogManager

        function onStateChanged() {
            if (mealLogManager.feedbackInsights.length === 0) {
                root.selectedInsightIndex = -1
                return
            }

            if (root.selectedInsightIndex < 0 || root.selectedInsightIndex >= mealLogManager.feedbackInsights.length) {
                root.selectedInsightIndex = 0
            }
        }
    }

    ColumnLayout {
        width: root.availableWidth
        height: implicitHeight
        spacing: 16

        AutoHeightRectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 24
            color: "#f7efe8"
            border.color: "#d6c5b4"
            border.width: 1

            ColumnLayout {
                x: 20
                y: 20
                width: parent.width - 20 * 2
                height: implicitHeight
                spacing: 10

                Label {
                    text: mealLogManager.editingMealLogId > 0 ? "编辑餐次" : "餐次记录"
                    font.pixelSize: 28
                    font.bold: true
                    color: "#2d241a"
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: "先从最近吃过的整餐或常点菜开始复用，再补今天这顿的细节，尽量把一次真实录餐压到几十秒。"
                    color: "#5a4a3b"
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    ReadableButton {
                        text: "刷新"
                        onClicked: {
                            mealLogManager.reload()
                            appState.reload()
                        }
                    }

                    ReadableButton {
                        visible: mealLogManager.editingMealLogId > 0
                        text: "取消编辑"
                        onClicked: resetEditor()
                    }
                }

                Label {
                    visible: mealLogManager.lastError.length > 0
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: mealLogManager.lastError
                    color: "#a13f2d"
                }
            }
        }

        AutoHeightRectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#f2eadf"
            visible: mealLogManager.recentMeals.length > 0

            ColumnLayout {
                x: 20
                y: 20
                width: parent.width - 20 * 2
                height: implicitHeight
                spacing: 12

                Label {
                    text: "最近餐次快捷复用"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#3b2b1f"
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: "优先复用最近真的吃过的一整餐；如果这顿只想沿用菜品组合，就点“只加菜品”。"
                    color: "#6b5644"
                }

                Repeater {
                    model: mealLogManager.recentMeals

                    AutoHeightRectangle {
                        Layout.fillWidth: true
                        visible: index < 4
                        radius: 14
                        color: "#fff8ef"
                        border.color: "#dcc9b5"
                        border.width: 1

                        ColumnLayout {
                            x: 12
                            y: 12
                            width: parent.width - 12 * 2
                            height: implicitHeight
                            spacing: 6

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: (modelData.mealTypeLabel ? modelData.mealTypeLabel : modelData.mealType) + " | " + modelData.eatenAt
                                font.bold: true
                                color: "#2e241a"
                            }

                            Flow {
                                Layout.fillWidth: true
                                spacing: 8

                                ReadableButton {
                                    text: "复用这餐"
                                    onClicked: applyMealTemplate(modelData, false)
                                }

                                ReadableButton {
                                    text: "只加菜品"
                                    onClicked: applyMealTemplate(modelData, true)
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: modelData.dishSummary
                                color: "#5f4d3d"
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: "地点 " + root.optionLabel(root.locationTypeOptions, modelData.locationType)
                                      + " | 方式 " + root.optionLabel(root.diningModeOptions, modelData.diningMode)
                                      + " | 价格 " + modelData.totalPrice
                                      + " | 用餐 " + modelData.totalEatTimeMinutes + " 分钟"
                                color: "#7b6756"
                            }
                        }
                    }
                }
            }
        }

        AutoHeightRectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#e6efe4"

            ColumnLayout {
                x: 20
                y: 20
                width: parent.width - 20 * 2
                height: implicitHeight
                spacing: 12

                Label {
                    text: "餐次信息"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#223127"
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: "时间默认填现在；复用最近餐次时，会回填最常重复的信息，避免你每顿都从零开始。"
                    color: "#45614b"
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.narrowLayout ? 1 : 2
                    columnSpacing: 12
                    rowSpacing: 10

                    ComboBox {
                        id: mealTypeBox
                        Layout.fillWidth: true
                        model: root.mealTypeOptions
                        textRole: "label"
                        valueRole: "value"
                    }

                    ComboBox {
                        id: weekdayBox
                        Layout.fillWidth: true
                        model: [
                            { label: "周一", value: 1 },
                            { label: "周二", value: 2 },
                            { label: "周三", value: 3 },
                            { label: "周四", value: 4 },
                            { label: "周五", value: 5 },
                            { label: "周六", value: 6 },
                            { label: "周日", value: 7 }
                        ]
                        textRole: "label"
                        valueRole: "value"
                    }

                    ComboBox {
                        id: locationTypeBox
                        Layout.fillWidth: true
                        model: root.locationTypeOptions
                        textRole: "label"
                        valueRole: "value"
                    }

                    ComboBox {
                        id: diningModeBox
                        Layout.fillWidth: true
                        model: root.diningModeOptions
                        textRole: "label"
                        valueRole: "value"
                    }

                    TextField {
                        id: eatenAtField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "用餐时间（例如 2026-04-22T12:30:00）"
                    }

                    TextField {
                        id: totalPriceField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "总价"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }

                    TextField {
                        id: eatTimeField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "用餐分钟数"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    TextField {
                        id: nextClassField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "距离下节课还有几分钟"
                        enabled: classAfterMealCheck.checked
                        opacity: classAfterMealCheck.checked ? 1.0 : 0.6
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    Slider {
                        id: hungerSlider
                        Layout.fillWidth: true
                        from: 1
                        to: 5
                        value: 3
                        stepSize: 1
                    }

                    Slider {
                        id: energySlider
                        Layout.fillWidth: true
                        from: 1
                        to: 5
                        value: 3
                        stepSize: 1
                    }
                }

                CheckBox {
                    id: classAfterMealCheck
                    text: "餐后有课"
                    onToggled: {
                        if (!checked) {
                            nextClassField.text = "0"
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: classAfterMealCheck.checked
                          ? "勾选后请补上距离下节课的分钟数，保存时会做一致性校验。"
                          : "没课时会自动按 0 分钟保存，避免留下不一致状态。"
                    color: "#5d7359"
                }

                TextField {
                    id: moodField
                    Layout.fillWidth: true
                    placeholderText: activeFocus || text.length > 0 ? "" : "餐前状态标签"
                }

                TextArea {
                    id: mealNotesField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    placeholderText: activeFocus || text.length > 0 ? "" : "这顿饭的备注"
                    wrapMode: TextEdit.Wrap
                }
            }
        }

        AutoHeightRectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#eef0df"

            ColumnLayout {
                x: 20
                y: 20
                width: parent.width - 20 * 2
                height: implicitHeight
                spacing: 12

                Label {
                    text: "给这餐加菜"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#353a20"
                }

                TextField {
                    id: dishSearchField
                    Layout.fillWidth: true
                    placeholderText: activeFocus || text.length > 0 ? "" : "搜索菜品，回车直接加入第一项"
                    onTextChanged: {
                        mealLogManager.setDishSearch(text)
                        dishPicker.currentIndex = 0
                    }
                    onAccepted: {
                        if (mealLogManager.filteredAvailableDishes.length > 0) {
                            addPickedDish(mealLogManager.filteredAvailableDishes[0])
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: mealLogManager.filteredAvailableDishes.length > 0
                          ? (dishSearchField.text.length > 0
                                 ? "当前命中 " + mealLogManager.filteredAvailableDishes.length + " 道菜；结果会优先把更贴近搜索、且最近更常用的菜排在前面。"
                                 : "当前可选 " + mealLogManager.filteredAvailableDishes.length + " 道菜；未搜索时会优先把最近更常用的菜排在前面。")
                          : "没有命中当前搜索，可以先清空搜索或去 Food 页面补菜品。"
                    color: "#617047"
                }

                ReadableButton {
                    text: "清空搜索"
                    visible: dishSearchField.text.length > 0
                    onClicked: {
                        dishSearchField.text = ""
                        mealLogManager.setDishSearch("")
                        dishPicker.currentIndex = 0
                    }
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    visible: mealLogManager.frequentDishes.length > 0

                    Repeater {
                        model: mealLogManager.frequentDishes

                        ReadableButton {
                            text: modelData.name
                            onClicked: addPickedDish(modelData)
                        }
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.narrowLayout ? 1 : 2
                    columnSpacing: 12
                    rowSpacing: 10

                    ComboBox {
                        id: dishPicker
                        Layout.fillWidth: true
                        model: mealLogManager.filteredAvailableDishes
                        textRole: "name"
                        valueRole: "id"
                    }

                    TextField {
                        id: portionRatioField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "影响权重（留空用默认值）"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }
                }

                TextField {
                    id: selectedDishNoteField
                    Layout.fillWidth: true
                    placeholderText: activeFocus || text.length > 0 ? "" : "这道菜的备注（可留空）"
                }

                ReadableButton {
                    text: "加入本餐"
                    enabled: mealLogManager.filteredAvailableDishes.length > 0
                    onClicked: {
                        const dishMap = mealLogManager.filteredAvailableDishes[dishPicker.currentIndex]
                        if (dishMap) {
                            addPickedDish(dishMap)
                        }
                    }
                }

                Label {
                    visible: mealLogManager.availableDishes.length === 0
                    text: "请先在 Food 页面添加菜品，再来记录餐次。"
                    color: "#8a5a34"
                }
            }
        }

        AutoHeightRectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#e4eaf1"

            ColumnLayout {
                x: 20
                y: 20
                width: parent.width - 20 * 2
                height: implicitHeight
                spacing: 10

                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: "已选菜品"
                        font.pixelSize: 20
                        font.bold: true
                        color: "#243041"
                    }

                    ReadableButton {
                        text: "清空"
                        onClicked: mealLogManager.clearSelection()
                    }
                }

                Repeater {
                    model: mealLogManager.selectedDishes

                    AutoHeightRectangle {
                        id: mealCard
                        Layout.fillWidth: true
                        radius: 14
                        color: "#f8fbff"
                        border.color: "#c7d3e2"
                        border.width: 1

                        ColumnLayout {
                            x: 12
                            y: 12
                            width: parent.width - 12 * 2
                            height: implicitHeight
                            spacing: 8

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Label {
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: modelData.name + " | " + modelData.merchantName
                                    font.bold: true
                                    color: "#243041"
                                }

                                Label {
                                    text: "权重 " + modelData.portionRatio + " | 犯困风险 " + modelData.sleepinessRiskLevel
                                    color: "#4f6178"
                                }

                                Label {
                                    visible: modelData.customNotes.length > 0
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: modelData.customNotes
                                    color: "#6a788c"
                                }
                            }

                            Flow {
                                Layout.fillWidth: true
                                spacing: 8

                                ReadableButton {
                                    text: "移除"
                                    onClicked: mealLogManager.removeSelectedDish(index)
                                }
                            }
                        }
                    }
                }

                ReadableButton {
                    text: mealLogManager.editingMealLogId > 0 ? "更新餐次" : "保存餐次"
                    onClicked: {
                        const ok = mealLogManager.saveMealLog(
                            mealTypeBox.currentValue,
                            eatenAtField.text,
                            weekdayBox.currentValue,
                            classAfterMealCheck.checked,
                            Number(nextClassField.text || "0"),
                            locationTypeBox.currentValue,
                            diningModeBox.currentValue,
                            Number(totalPriceField.text || "0"),
                            Number(eatTimeField.text || "0"),
                            Math.round(hungerSlider.value),
                            Math.round(energySlider.value),
                            moodField.text,
                            mealNotesField.text
                        )

                        if (ok) {
                            appState.reload()
                            resetEditor()
                        }
                    }
                }
            }
        }

        AutoHeightRectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#ece4d8"
            visible: mealLogManager.feedbackInsights.length > 0

            ColumnLayout {
                x: 20
                y: 20
                width: parent.width - 20 * 2
                height: implicitHeight
                spacing: 10

                Label {
                    text: "反馈洞察"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#24364d"
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: "先看结论，再看证据。每条洞察都会把最值得先看的建议、菜样本和餐次样本排在前面。"
                    color: "#50657d"
                }

                Repeater {
                    model: mealLogManager.feedbackInsights

                    AutoHeightRectangle {
                        property bool selected: index === root.selectedInsightIndex
                        Layout.fillWidth: true
                        radius: 14
                        color: root.toneCardColor(modelData.tone, selected)
                        border.color: root.toneBorderColor(modelData.tone, selected)
                        border.width: selected ? 2 : 1

                        ColumnLayout {
                            x: 12
                            y: 12
                            width: parent.width - 12 * 2
                            height: implicitHeight
                            spacing: 6

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Label {
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: modelData.title
                                    font.bold: true
                                    color: "#2e241a"
                                }

                                Label {
                                    visible: modelData.weightHintCount > 0
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: modelData.weightHintCount + " 条调权建议"
                                    color: "#6d593b"
                                }
                            }

                            Label {
                                visible: !!modelData.priorityHeadline
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: modelData.priorityHeadline
                                font.bold: true
                                color: "#36536f"
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: modelData.summary
                                color: "#4f6178"
                            }

                            Label {
                                visible: !!modelData.quickLookText
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: modelData.quickLookText
                                color: "#6a788c"
                            }

                            Label {
                                visible: !!modelData.evidenceSummary
                                text: modelData.evidenceSummary
                                color: "#7a6a54"
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.selectInsight(index)
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                }

                AutoHeightRectangle {
                    Layout.fillWidth: true
                    radius: 16
                    visible: root.currentInsight() !== null
                    color: "#fff8ef"
                    border.color: "#d7c3a8"
                    border.width: 1

                    ColumnLayout {
                        x: 16
                        y: 16
                        width: parent.width - 16 * 2
                        height: implicitHeight
                        spacing: 12

                        Label {
                            visible: root.currentInsight() !== null
                            text: root.currentInsight() ? root.currentInsight().title + " | 细看" : ""
                            font.pixelSize: 18
                            font.bold: true
                            color: "#2e241a"
                        }

                        AutoHeightRectangle {
                            Layout.fillWidth: true
                            visible: root.currentInsight() !== null
                            radius: 12
                            color: "#f2ede4"
                            border.color: "#dfd1bc"
                            border.width: 1

                            ColumnLayout {
                                x: 10
                                y: 10
                                width: parent.width - 10 * 2
                                height: implicitHeight
                                spacing: 4

                                Label {
                                    visible: root.currentInsight() && root.currentInsight().priorityHeadline
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: root.currentInsight() ? root.currentInsight().priorityHeadline : ""
                                    font.bold: true
                                    color: "#36536f"
                                }

                                Label {
                                    visible: root.currentInsight() && root.currentInsight().quickLookText
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: root.currentInsight() ? root.currentInsight().quickLookText : ""
                                    color: "#5b4c3d"
                                }

                                Label {
                                    visible: root.currentInsight() && root.currentInsight().evidenceSummary
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: root.currentInsight() ? root.currentInsight().evidenceSummary : ""
                                    color: "#7a6a54"
                                }
                            }
                        }

                        AutoHeightRectangle {
                            Layout.fillWidth: true
                            visible: root.currentInsight()
                                     && root.currentInsight().firstScanStepTitle
                                     && root.currentInsight().firstScanStepTitle.length > 0
                            radius: 12
                            color: "#eef5ff"
                            border.color: "#c1d3ea"
                            border.width: 1

                            ColumnLayout {
                                x: 10
                                y: 10
                                width: parent.width - 10 * 2
                                height: implicitHeight
                                spacing: 5

                                Label {
                                    text: "先看什么"
                                    font.bold: true
                                    color: "#2d4a6a"
                                }

                                Label {
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: root.currentInsight() ? root.currentInsight().firstScanStepTitle : ""
                                    font.bold: true
                                    color: "#36536f"
                                }

                                Label {
                                    visible: root.currentInsight()
                                             && root.currentInsight().firstScanStepBody
                                             && root.currentInsight().firstScanStepBody.length > 0
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: root.currentInsight() ? root.currentInsight().firstScanStepBody : ""
                                    color: "#4f6178"
                                }

                                Label {
                                    visible: root.currentInsight()
                                             && root.currentInsight().remainingScanStepCount > 0
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: "后面还有 " + root.currentInsight().remainingScanStepCount + " 步，按下面顺序再看。"
                                    color: "#6c7e90"
                                }
                            }
                        }

                        ColumnLayout {
                            visible: root.currentInsight() && root.currentInsight().scanSteps && root.currentInsight().scanSteps.length > 0
                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                text: "建议按这个顺序看"
                                font.bold: true
                                color: "#33465c"
                            }

                            Repeater {
                                model: root.currentInsight() ? root.currentInsight().scanSteps : []

                                AutoHeightRectangle {
                                    Layout.fillWidth: true
                                    radius: 12
                                    color: "#f2ede4"
                                    border.color: "#dfd1bc"
                                    border.width: 1

                                    ColumnLayout {
                                        x: 10
                                        y: 10
                                        width: parent.width - 10 * 2
                                        height: implicitHeight
                                        spacing: 4

                                        Label {
                                            visible: index > 0
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: modelData.title
                                            font.bold: true
                                            color: "#36536f"
                                        }

                                        Label {
                                            visible: index > 0
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: modelData.body
                                            color: "#5b4c3d"
                                        }
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            visible: root.currentInsight() && root.currentInsight().weightHints && root.currentInsight().weightHints.length > 0
                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                text: "建议怎么调"
                                font.bold: true
                                color: "#33465c"
                            }

                            Repeater {
                                model: root.currentInsight() ? root.currentInsight().weightHints : []

                                AutoHeightRectangle {
                                    Layout.fillWidth: true
                                    radius: 12
                                    color: "#eef5ff"
                                    border.color: "#c1d3ea"
                                    border.width: 1

                                    ColumnLayout {
                                        x: 10
                                        y: 10
                                        width: parent.width - 10 * 2
                                        height: implicitHeight
                                        spacing: 4

                                        Label {
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: (modelData.actionText ? modelData.actionText : root.readableWeightDirectionLabelV2(modelData.direction) + " " + modelData.label)
                                            font.bold: true
                                            color: "#2d4a6a"
                                        }

                                        Label {
                                            visible: !!modelData.strengthLabel
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: "力度：" + (modelData.strengthLabel ? modelData.strengthLabel : modelData.strength)
                                            color: "#52718f"
                                        }

                                        Label {
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: modelData.reason
                                            color: "#4f6178"
                                        }

                                        Label {
                                            visible: !!modelData.key
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: "对应内部权重键：" + modelData.key
                                            color: "#7e94ad"
                                        }
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            visible: root.currentInsight() !== null
                                     && root.currentInsight().detail
                                     && root.currentInsight().detail.length > 0
                            Layout.fillWidth: true
                            spacing: 6

                            Label {
                                text: "补充说明"
                                font.bold: true
                                color: "#33465c"
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: root.currentInsight() ? root.currentInsight().detail : ""
                                color: "#5b4c3d"
                            }
                        }

                        ColumnLayout {
                            visible: root.currentInsight() && root.currentInsight().supportingDishes && root.currentInsight().supportingDishes.length > 0
                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                text: "先看这些菜"
                                font.bold: true
                                color: "#33465c"
                            }

                            Repeater {
                                model: root.currentInsight() ? root.currentInsight().supportingDishes : []

                                AutoHeightRectangle {
                                    Layout.fillWidth: true
                                    radius: 12
                                    color: root.selectedInsightHasDish(modelData.id) ? "#eef7ec" : "#f7faf4"
                                    border.color: root.selectedInsightHasDish(modelData.id) ? "#9dbc97" : "#ccd9c6"
                                    border.width: 1

                                    ColumnLayout {
                                        x: 10
                                        y: 10
                                        width: parent.width - 10 * 2
                                        height: implicitHeight
                                        spacing: 5

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 6

                                            Label {
                                                Layout.fillWidth: true
                                                wrapMode: Text.Wrap
                                                text: modelData.name + (modelData.merchantName.length > 0 ? " | " + modelData.merchantName : "")
                                                font.bold: true
                                                color: "#2e241a"
                                            }

                                            Flow {
                                                Layout.fillWidth: true
                                                spacing: 8

                                                ReadableButton {
                                                    text: "筛到菜品"
                                                    onClicked: root.focusDishSearch(modelData.name)
                                                }
                                            }
                                        }

                                        Label {
                                            visible: !!modelData.supportQuickNote || !!modelData.supportPriorityLabel
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: modelData.supportQuickNote ? modelData.supportQuickNote : modelData.supportPriorityLabel
                                            color: "#55784f"
                                        }

                                        Label {
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: modelData.signalSummary
                                            color: "#4f6178"
                                        }

                                        Label {
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: "口味 " + modelData.avgTasteRating
                                                  + " | 复吃 " + modelData.avgRepeatWillingness
                                                  + " | 犯困 " + modelData.avgSleepinessLevel
                                                  + " | 舒适 " + modelData.avgComfortLevel
                                            color: "#6a788c"
                                        }
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            visible: root.currentInsight() && root.currentInsight().supportingMeals && root.currentInsight().supportingMeals.length > 0
                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                text: "再看这些餐次"
                                font.bold: true
                                color: "#33465c"
                            }

                            Repeater {
                                model: root.currentInsight() ? root.currentInsight().supportingMeals : []

                                AutoHeightRectangle {
                                    Layout.fillWidth: true
                                    radius: 12
                                    color: "#f9f3e7"
                                    border.color: "#d6c7b2"
                                    border.width: 1

                                    ColumnLayout {
                                        x: 10
                                        y: 10
                                        width: parent.width - 10 * 2
                                        height: implicitHeight
                                        spacing: 5

                                        Label {
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: (modelData.mealTypeLabel ? modelData.mealTypeLabel : modelData.mealType) + " | " + modelData.eatenAt
                                            font.bold: true
                                            color: "#2e241a"
                                        }

                                        Flow {
                                            Layout.fillWidth: true
                                            spacing: 8

                                            ReadableButton {
                                                text: "载入这餐"
                                                onClicked: loadMealForEdit(modelData.id)
                                            }
                                        }

                                        Label {
                                            visible: !!modelData.supportPriorityLabel
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: modelData.supportPriorityLabel
                                            color: "#8b5a1d"
                                        }

                                        Label {
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: modelData.dishSummary
                                            color: "#4f6178"
                                        }

                                        Label {
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: (modelData.feedbackQuickSummary ? modelData.feedbackQuickSummary : modelData.feedbackSummary)
                                                  + (modelData.linkedRecommendationCandidateRank > 0
                                                         ? (" | 命中 top-" + modelData.linkedRecommendationCandidateRank)
                                                         : "")
                                            color: "#6a788c"
                                        }

                                        Label {
                                            visible: !!modelData.recommendationPreviewText
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: modelData.recommendationPreviewText
                                            color: "#52718f"
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        AutoHeightRectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#ece4d8"

            ColumnLayout {
                x: 20
                y: 20
                width: parent.width - 20 * 2
                height: implicitHeight
                spacing: 10

                Label {
                    text: "最近记录"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#31241a"
                }

                Repeater {
                    model: mealLogManager.recentMeals

                    AutoHeightRectangle {
                        id: mealCard
                        Layout.fillWidth: true
                        radius: 14
                        color: mealCard.insightHighlighted ? "#fff4d8" : "#fff8ef"
                        border.color: mealCard.insightHighlighted ? "#c79a48" : "#dcc9b5"
                        border.width: mealCard.insightHighlighted ? 2 : 1

                        property int feedbackFullnessLevel: modelData.feedbackSaved ? Math.max(1, modelData.feedbackFullnessLevel) : 3
                        property int feedbackSleepinessLevel: modelData.feedbackSaved ? Math.max(1, modelData.feedbackSleepinessLevel) : 3
                        property int feedbackComfortLevel: modelData.feedbackSaved ? Math.max(1, modelData.feedbackComfortLevel) : 3
                        property int feedbackFocusImpactLevel: modelData.feedbackSaved ? Math.max(1, modelData.feedbackFocusImpactLevel) : 3
                        property int feedbackTasteRating: modelData.feedbackSaved ? Math.max(1, modelData.feedbackTasteRating) : 3
                        property int feedbackRepeatWillingness: modelData.feedbackSaved ? Math.max(1, modelData.feedbackRepeatWillingness) : 3
                        property bool feedbackWouldEatAgain: modelData.feedbackSaved ? modelData.feedbackWouldEatAgain : true
                        property string feedbackTextValue: modelData.feedbackText ? modelData.feedbackText : ""
                        property bool linkedRecommendationResolved: modelData.linkedRecommendationRecordId > 0
                        property bool feedbackLinkedToRecommendation: modelData.feedbackRecommendationRecordId > 0
                        property bool insightHighlighted: root.selectedInsightHasMeal(modelData.id)

                        ColumnLayout {
                            x: 12
                            y: 12
                            width: parent.width - 12 * 2
                            height: implicitHeight
                            spacing: 6

                            Label {
                                Layout.fillWidth: true
                                text: (modelData.mealTypeLabel ? modelData.mealTypeLabel : modelData.mealType) + " | " + modelData.eatenAt
                                font.bold: true
                                color: "#2e241a"
                            }

                            Flow {
                                Layout.fillWidth: true
                                spacing: 8

                                ReadableButton {
                                    text: "复用这餐"
                                    onClicked: applyMealTemplate(modelData, false)
                                }

                                ReadableButton {
                                    text: "只加菜"
                                    onClicked: applyMealTemplate(modelData, true)
                                }

                                ReadableButton {
                                    text: "编辑"
                                    onClicked: loadMealForEdit(modelData.id)
                                }

                                ReadableButton {
                                    text: "删除"
                                    onClicked: {
                                        if (mealLogManager.deleteMealLog(modelData.id)) {
                                            appState.reload()
                                            if (mealLogManager.editingMealLogId === 0) {
                                                applyDefaults()
                                            }
                                        }
                                    }
                                }
                            }

                            Label {
                                visible: mealCard.insightHighlighted
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: "这餐属于当前洞察的代表性样本。"
                                color: "#8b5a1d"
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: modelData.dishSummary
                                color: "#5f4d3d"
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: "价格 " + modelData.totalPrice + "，用餐 " + modelData.totalEatTimeMinutes + " 分钟，下节课前还有 " + modelData.minutesUntilNextClass + " 分钟"
                                color: "#5f4d3d"
                            }

                            Label {
                                visible: modelData.notes.length > 0
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: modelData.notes
                                color: "#7b6756"
                            }

                            AutoHeightRectangle {
                                visible: mealCard.linkedRecommendationResolved || mealCard.feedbackLinkedToRecommendation
                                Layout.fillWidth: true
                                radius: 12
                                color: "#eef5ff"
                                border.color: "#c2d3ea"
                                border.width: 1

                                ColumnLayout {
                                    x: 10
                                    y: 10
                                    width: parent.width - 10 * 2
                                    height: implicitHeight
                                    spacing: 4

                                    Label {
                                        visible: mealCard.linkedRecommendationResolved
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: "匹配到推荐记录 #" + modelData.linkedRecommendationRecordId
                                              + (modelData.linkedRecommendationCandidateRank > 0
                                                     ? (" | 命中 top-" + modelData.linkedRecommendationCandidateRank)
                                                     : "")
                                        font.bold: true
                                        color: "#30486c"
                                    }

                                    Label {
                                        visible: mealCard.linkedRecommendationResolved && modelData.linkedRecommendationDishName.length > 0
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: "命中的菜：" + modelData.linkedRecommendationDishName
                                              + (modelData.linkedRecommendationGeneratedAt.length > 0
                                                     ? (" | 推荐生成于 " + modelData.linkedRecommendationGeneratedAt)
                                                     : "")
                                        color: "#4f6178"
                                    }

                                    Label {
                                        visible: mealCard.feedbackLinkedToRecommendation
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: "这条反馈已回链到推荐记录 #" + modelData.feedbackRecommendationRecordId
                                        color: "#4f6178"
                                    }

                                    Label {
                                        visible: mealCard.linkedRecommendationResolved
                                                 && modelData.recommendationSelectionSummary
                                                 && modelData.recommendationSelectionSummary.length > 0
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: modelData.recommendationSelectionSummary
                                        font.bold: true
                                        color: "#36536f"
                                    }

                                    Label {
                                        visible: mealCard.linkedRecommendationResolved
                                                 && modelData.recommendationComparisonSummary
                                                 && modelData.recommendationComparisonSummary.length > 0
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: modelData.recommendationComparisonSummary
                                        color: "#4f6178"
                                    }

                                    Label {
                                        visible: mealCard.linkedRecommendationResolved
                                                 && modelData.recommendationContextHeadline
                                                 && modelData.recommendationContextHeadline.length > 0
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: "当时场景：" + modelData.recommendationContextHeadline
                                        color: "#52718f"
                                    }

                                    AutoHeightRectangle {
                                        visible: mealCard.linkedRecommendationResolved
                                                 && modelData.recommendationTopCandidateDishName
                                                 && modelData.recommendationTopCandidateDishName.length > 0
                                                 && modelData.recommendationSelectedCandidateDishName
                                                 && modelData.recommendationSelectedCandidateDishName.length > 0
                                        Layout.fillWidth: true
                                        radius: 10
                                        color: "#f7fbff"
                                        border.color: "#d7e2ef"
                                        border.width: 1

                                        ColumnLayout {
                                            x: 10
                                            y: 10
                                            width: parent.width - 10 * 2
                                            height: implicitHeight
                                            spacing: 6

                                            Label {
                                                text: "快速对比"
                                                font.bold: true
                                                color: "#30486c"
                                            }

                                            Label {
                                                visible: modelData.recommendationComparePriorityHeadline
                                                         && modelData.recommendationComparePriorityHeadline.length > 0
                                                Layout.fillWidth: true
                                                wrapMode: Text.Wrap
                                                text: modelData.recommendationComparePriorityHeadline
                                                font.bold: true
                                                color: "#36536f"
                                            }

                                            Label {
                                                visible: modelData.recommendationScoreGapSummary
                                                         && modelData.recommendationScoreGapSummary.length > 0
                                                Layout.fillWidth: true
                                                wrapMode: Text.Wrap
                                                text: modelData.recommendationScoreGapSummary
                                                color: "#4f6178"
                                            }

                                            Label {
                                                visible: modelData.recommendationCompareGuideText
                                                         && modelData.recommendationCompareGuideText.length > 0
                                                Layout.fillWidth: true
                                                wrapMode: Text.Wrap
                                                text: modelData.recommendationCompareGuideText
                                                color: "#6c7e90"
                                            }

                                            AutoHeightRectangle {
                                                Layout.fillWidth: true
                                                radius: 8
                                                color: "#edf4fc"
                                                border.color: "#d7e2ef"
                                                border.width: 1

                                                ColumnLayout {
                                                    x: 8
                                                    y: 8
                                                    width: parent.width - 8 * 2
                                                    height: implicitHeight
                                                    spacing: 3

                                                    Label {
                                                        Layout.fillWidth: true
                                                        wrapMode: Text.Wrap
                                                        text: "当时首推 | " + modelData.recommendationTopCandidateDishName
                                                              + (modelData.recommendationTopCandidateScoreText
                                                                     ? (" | 分数 " + modelData.recommendationTopCandidateScoreText)
                                                                     : "")
                                                        font.bold: true
                                                        color: "#2f4867"
                                                    }

                                                    Label {
                                                        visible: modelData.recommendationTopCandidateReason
                                                                 && modelData.recommendationTopCandidateReason.length > 0
                                                        Layout.fillWidth: true
                                                        wrapMode: Text.Wrap
                                                        text: modelData.recommendationTopCandidateReason
                                                        color: "#4f6178"
                                                    }
                                                }
                                            }

                                            AutoHeightRectangle {
                                                Layout.fillWidth: true
                                                radius: 8
                                                color: "#eef8ef"
                                                border.color: "#d1e2d2"
                                                border.width: 1

                                                ColumnLayout {
                                                    x: 8
                                                    y: 8
                                                    width: parent.width - 8 * 2
                                                    height: implicitHeight
                                                    spacing: 3

                                                    Label {
                                                        Layout.fillWidth: true
                                                        wrapMode: Text.Wrap
                                                        text: "这次实际选择 | " + modelData.recommendationSelectedCandidateDishName
                                                              + (modelData.recommendationSelectedCandidateScoreText
                                                                     ? (" | 分数 " + modelData.recommendationSelectedCandidateScoreText)
                                                                     : "")
                                                        font.bold: true
                                                        color: "#335f46"
                                                    }

                                                    Label {
                                                        visible: modelData.recommendationSelectedReason
                                                                 && modelData.recommendationSelectedReason.length > 0
                                                        Layout.fillWidth: true
                                                        wrapMode: Text.Wrap
                                                        text: modelData.recommendationSelectedReason
                                                        color: "#4f6178"
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    Label {
                                        visible: mealCard.linkedRecommendationResolved
                                                 && modelData.recommendationSelectedReason
                                                 && modelData.recommendationSelectedReason.length > 0
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: "这次命中候选的理由：" + modelData.recommendationSelectedReason
                                        color: "#4f6178"
                                    }

                                    Label {
                                        visible: mealCard.linkedRecommendationResolved
                                                 && modelData.recommendationTopCandidateReason
                                                 && modelData.recommendationTopCandidateReason.length > 0
                                                 && modelData.linkedRecommendationCandidateRank > 1
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: "当时首推的理由：" + modelData.recommendationTopCandidateReason
                                        color: "#6c7e90"
                                    }

                                    Label {
                                        visible: mealCard.linkedRecommendationResolved
                                                 && modelData.recommendationPreviewText
                                                 && modelData.recommendationPreviewText.length > 0
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: modelData.recommendationPreviewText
                                        color: "#4f6178"
                                    }

                                    ColumnLayout {
                                        visible: mealCard.linkedRecommendationResolved
                                                 && modelData.recommendationCandidates
                                                 && modelData.recommendationCandidates.length > 0
                                        Layout.fillWidth: true
                                        spacing: 6

                                        Label {
                                            text: "推荐回看"
                                            font.bold: true
                                            color: "#30486c"
                                        }

                                        Repeater {
                                            model: modelData.recommendationCandidates

                                            AutoHeightRectangle {
                                                Layout.fillWidth: true
                                                radius: 10
                                                color: modelData.selected ? "#dfeeff" : "#f8fbff"
                                                border.color: modelData.selected ? "#7ea4d0" : "#d7e2ef"
                                                border.width: 1

                                                ColumnLayout {
                                                    x: 8
                                                    y: 8
                                                    width: parent.width - 8 * 2
                                                    height: implicitHeight
                                                    spacing: 3

                                                    Label {
                                                        Layout.fillWidth: true
                                                        wrapMode: Text.Wrap
                                                        text: root.readableCandidateRankLabelV2(modelData.rank)
                                                              + (modelData.badgeText ? " | " + modelData.badgeText : "")
                                                              + (modelData.scoreGapTag ? " | " + modelData.scoreGapTag : "")
                                                              + " | " + modelData.dishName
                                                              + " | 分数 " + modelData.scoreText
                                                        font.bold: true
                                                        color: "#2f4867"
                                                    }

                                                    Label {
                                                        visible: modelData.reason.length > 0
                                                        Layout.fillWidth: true
                                                        wrapMode: Text.Wrap
                                                        text: modelData.reason
                                                        color: "#4f6178"
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            AutoHeightRectangle {
                                Layout.fillWidth: true
                                radius: 12
                                color: "#f5eee3"
                                border.color: "#dbcab6"
                                border.width: 1

                                ColumnLayout {
                                    x: 10
                                    y: 10
                                    width: parent.width - 10 * 2
                                    height: implicitHeight
                                    spacing: 8

                                    Label {
                                        text: modelData.feedbackSaved ? "餐后反馈（已保存）" : "餐后反馈"
                                        font.bold: true
                                        color: "#5b4333"
                                    }

                                    GridLayout {
                                        Layout.fillWidth: true
                                        columns: 2
                                        columnSpacing: 10
                                        rowSpacing: 8

                                        Label { text: "口味"; color: "#6a5442" }
                                        SpinBox {
                                            Layout.fillWidth: true
                                            from: 1
                                            to: 5
                                            value: mealCard.feedbackTasteRating
                                            onValueModified: mealCard.feedbackTasteRating = value
                                        }
                                        Label { text: "复吃"; color: "#6a5442" }

                                        SpinBox {
                                            Layout.fillWidth: true
                                            from: 1
                                            to: 5
                                            value: mealCard.feedbackRepeatWillingness
                                            onValueModified: mealCard.feedbackRepeatWillingness = value
                                        }
                                        Label { text: "饱腹"; color: "#6a5442" }
                                        SpinBox {
                                            Layout.fillWidth: true
                                            from: 1
                                            to: 5
                                            value: mealCard.feedbackFullnessLevel
                                            onValueModified: mealCard.feedbackFullnessLevel = value
                                        }

                                        Label { text: "犯困"; color: "#6a5442" }
                                        SpinBox {
                                            Layout.fillWidth: true
                                            from: 1
                                            to: 5
                                            value: mealCard.feedbackSleepinessLevel
                                            onValueModified: mealCard.feedbackSleepinessLevel = value
                                        }
                                        Label { text: "舒适"; color: "#6a5442" }
                                        SpinBox {
                                            Layout.fillWidth: true
                                            from: 1
                                            to: 5
                                            value: mealCard.feedbackComfortLevel
                                            onValueModified: mealCard.feedbackComfortLevel = value
                                        }

                                        Label { text: "专注"; color: "#6a5442" }
                                        SpinBox {
                                            Layout.fillWidth: true
                                            from: 1
                                            to: 5
                                            value: mealCard.feedbackFocusImpactLevel
                                            onValueModified: mealCard.feedbackFocusImpactLevel = value
                                        }
                                    }

                                    CheckBox {
                                        text: "下次还愿意吃"
                                        checked: mealCard.feedbackWouldEatAgain
                                        onToggled: mealCard.feedbackWouldEatAgain = checked
                                    }

                                    TextArea {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 70
                                        text: mealCard.feedbackTextValue
                                        placeholderText: activeFocus || text.length > 0 ? "" : "补一句这餐的主观感受"
                                        wrapMode: TextEdit.Wrap
                                        onTextChanged: mealCard.feedbackTextValue = text
                                    }

                                    ReadableButton {
                                        text: modelData.feedbackSaved ? "更新反馈" : "保存反馈"
                                        onClicked: {
                                            if (mealLogManager.saveMealFeedback(
                                                    modelData.id,
                                                    mealCard.feedbackFullnessLevel,
                                                    mealCard.feedbackSleepinessLevel,
                                                    mealCard.feedbackComfortLevel,
                                                    mealCard.feedbackFocusImpactLevel,
                                                    mealCard.feedbackWouldEatAgain,
                                                    mealCard.feedbackTasteRating,
                                                    mealCard.feedbackRepeatWillingness,
                                                    mealCard.feedbackTextValue)) {
                                                appState.reload()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
