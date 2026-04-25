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

    component LevelField: ColumnLayout {
        id: levelField
        property string label: ""
        property alias currentIndex: box.currentIndex
        property alias currentValue: box.currentValue
        property alias count: box.count
        property alias model: box.model

        Layout.fillWidth: true
        spacing: 5

        Label {
            Layout.fillWidth: true
            text: levelField.label
            color: "#4d5333"
            font.pixelSize: 13
            font.bold: true
            elide: Text.ElideRight
        }

        ComboBox {
            id: box
            Layout.fillWidth: true
            Layout.preferredHeight: 42
            model: root.levelOptions
            textRole: "label"
            valueRole: "value"
        }
    }

    id: root
    clip: true

    readonly property bool narrowLayout: root.availableWidth < 380
    property int editingMerchantId: 0
    property int editingDishId: 0
    property var priceLevelOptions: [
        { label: "实惠", value: "budget" },
        { label: "中等", value: "mid" },
        { label: "偏高", value: "high" }
    ]
    property var diningModeOptions: [
        { label: "堂食", value: "dine_in" },
        { label: "打包", value: "takeaway" },
        { label: "外卖", value: "delivery" }
    ]
    property var levelOptions: [
        { label: "低", value: "low" },
        { label: "中", value: "medium" },
        { label: "高", value: "high" }
    ]

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

    function optionLabel(options, value) {
        for (let i = 0; i < options.length; ++i) {
            if (options[i].value === value) {
                return options[i].label
            }
        }
        return value
    }

    function merchantIndexForId(merchantId) {
        for (let i = 0; i < dishMerchantBox.count; ++i) {
            if (dishMerchantBox.valueAt(i) === merchantId) {
                return i
            }
        }
        return dishMerchantBox.count > 0 ? 0 : -1
    }

    function dishMerchantFilterOptions() {
        return [{ id: 0, name: "全部商家" }].concat(foodManager.merchants)
    }

    function dishMerchantFilterIndexForId(merchantId) {
        const options = dishMerchantFilterOptions()
        for (let i = 0; i < options.length; ++i) {
            if (options[i].id === merchantId) {
                return i
            }
        }
        return 0
    }

    function resetMerchantForm() {
        editingMerchantId = 0
        merchantNameField.text = ""
        merchantAreaField.text = ""
        merchantPriceLevelBox.currentIndex = 0
        merchantDistanceField.text = "0"
        merchantQueueField.text = "0"
        merchantDeliveryField.text = "0"
        merchantDineInCheck.checked = true
        merchantTakeawayCheck.checked = false
        merchantDeliveryCheck.checked = false
        merchantNotesField.text = ""
    }

    function loadMerchantForEdit(merchant) {
        editingMerchantId = merchant.id
        merchantNameField.text = merchant.name
        merchantAreaField.text = merchant.campusArea
        merchantPriceLevelBox.currentIndex = comboIndexByValue(merchantPriceLevelBox, merchant.priceLevel)
        merchantDistanceField.text = String(merchant.distanceMinutes)
        merchantQueueField.text = String(merchant.queueTimeMinutes)
        merchantDeliveryField.text = String(merchant.deliveryEtaMinutes)
        merchantDineInCheck.checked = merchant.supportsDineIn
        merchantTakeawayCheck.checked = merchant.supportsTakeaway
        merchantDeliveryCheck.checked = merchant.supportsDelivery
        merchantNotesField.text = merchant.notes
    }

    function resetDishForm() {
        editingDishId = 0
        dishNameField.text = ""
        dishCategoryField.text = ""
        dishPriceField.text = "0"
        dishDiningModeBox.currentIndex = 0
        dishEatTimeField.text = "15"
        dishEffortField.text = "1"
        dishImpactField.text = "1.0"
        carbBox.currentIndex = 0
        fatBox.currentIndex = 0
        proteinBox.currentIndex = 0
        vitaminBox.currentIndex = 0
        fiberBox.currentIndex = 0
        satietyBox.currentIndex = 0
        burdenBox.currentIndex = 0
        sleepinessBox.currentIndex = 0
        flavorBox.currentIndex = 0
        odorBox.currentIndex = 0
        comboCheck.checked = false
        beverageCheck.checked = false
        dishNotesField.text = ""
        if (dishMerchantBox.count > 0) {
            dishMerchantBox.currentIndex = 0
        }
    }

    function loadDishForEdit(dish) {
        editingDishId = dish.id
        dishNameField.text = dish.name
        dishCategoryField.text = dish.category
        dishPriceField.text = String(dish.price)
        dishDiningModeBox.currentIndex = comboIndexByValue(dishDiningModeBox, dish.defaultDiningMode)
        dishEatTimeField.text = String(dish.eatTimeMinutes)
        dishEffortField.text = String(dish.acquireEffortScore)
        dishImpactField.text = String(dish.mealImpactWeight)
        carbBox.currentIndex = comboIndexByValue(carbBox, dish.carbLevel)
        fatBox.currentIndex = comboIndexByValue(fatBox, dish.fatLevel)
        proteinBox.currentIndex = comboIndexByValue(proteinBox, dish.proteinLevel)
        vitaminBox.currentIndex = comboIndexByValue(vitaminBox, dish.vitaminLevel)
        fiberBox.currentIndex = comboIndexByValue(fiberBox, dish.fiberLevel)
        satietyBox.currentIndex = comboIndexByValue(satietyBox, dish.satietyLevel)
        burdenBox.currentIndex = comboIndexByValue(burdenBox, dish.digestiveBurdenLevel)
        sleepinessBox.currentIndex = comboIndexByValue(sleepinessBox, dish.sleepinessRiskLevel)
        flavorBox.currentIndex = comboIndexByValue(flavorBox, dish.flavorLevel)
        odorBox.currentIndex = comboIndexByValue(odorBox, dish.odorLevel)
        comboCheck.checked = dish.isCombo
        beverageCheck.checked = dish.isBeverage
        dishNotesField.text = dish.notes
        dishMerchantBox.currentIndex = merchantIndexForId(dish.merchantId)
    }

    function saveMerchant() {
        let ok = false
        if (editingMerchantId > 0) {
            ok = foodManager.updateMerchant(
                editingMerchantId,
                merchantNameField.text,
                merchantAreaField.text,
                merchantPriceLevelBox.currentValue,
                merchantDineInCheck.checked,
                merchantTakeawayCheck.checked,
                merchantDeliveryCheck.checked,
                Number(merchantDeliveryField.text || "0"),
                Number(merchantDistanceField.text || "0"),
                Number(merchantQueueField.text || "0"),
                merchantNotesField.text
            )
        } else {
            ok = foodManager.addMerchant(
                merchantNameField.text,
                merchantAreaField.text,
                merchantPriceLevelBox.currentValue,
                merchantDineInCheck.checked,
                merchantTakeawayCheck.checked,
                merchantDeliveryCheck.checked,
                Number(merchantDeliveryField.text || "0"),
                Number(merchantDistanceField.text || "0"),
                Number(merchantQueueField.text || "0"),
                merchantNotesField.text
            )
        }
        if (ok) {
            resetMerchantForm()
        }
    }

    function saveDish() {
        let ok = false
        if (editingDishId > 0) {
            ok = foodManager.updateDish(
                editingDishId,
                dishNameField.text,
                dishMerchantBox.currentValue,
                dishCategoryField.text,
                Number(dishPriceField.text || "0"),
                dishDiningModeBox.currentValue,
                Number(dishEatTimeField.text || "0"),
                Number(dishEffortField.text || "0"),
                carbBox.currentValue,
                fatBox.currentValue,
                proteinBox.currentValue,
                vitaminBox.currentValue,
                fiberBox.currentValue,
                satietyBox.currentValue,
                burdenBox.currentValue,
                sleepinessBox.currentValue,
                flavorBox.currentValue,
                odorBox.currentValue,
                comboCheck.checked,
                beverageCheck.checked,
                Number(dishImpactField.text || "1"),
                dishNotesField.text
            )
        } else {
            ok = foodManager.addDish(
                dishNameField.text,
                dishMerchantBox.currentValue,
                dishCategoryField.text,
                Number(dishPriceField.text || "0"),
                dishDiningModeBox.currentValue,
                Number(dishEatTimeField.text || "0"),
                Number(dishEffortField.text || "0"),
                carbBox.currentValue,
                fatBox.currentValue,
                proteinBox.currentValue,
                vitaminBox.currentValue,
                fiberBox.currentValue,
                satietyBox.currentValue,
                burdenBox.currentValue,
                sleepinessBox.currentValue,
                flavorBox.currentValue,
                odorBox.currentValue,
                comboCheck.checked,
                beverageCheck.checked,
                Number(dishImpactField.text || "1"),
                dishNotesField.text
            )
        }
        if (ok) {
            resetDishForm()
        }
    }

    Component.onCompleted: {
        resetMerchantForm()
        resetDishForm()
    }

    ColumnLayout {
        width: root.availableWidth
        height: implicitHeight
        spacing: 16

        AutoHeightRectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 24
            color: "#f7f1e8"
            border.color: "#d7c8b9"
            border.width: 1

            ColumnLayout {
                x: 20
                y: 20
                width: parent.width - 20 * 2
                height: implicitHeight
                spacing: 10

                Label {
                    text: "商家与菜品管理"
                    font.pixelSize: 28
                    font.bold: true
                    color: "#2c241b"
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: "集中维护商家和菜品。优先保证搜索、编辑、归档和复用顺手，不需要再手改数据库。"
                    color: "#5d4c3e"
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 12

                    ReadableButton {
                        text: "刷新"
                        onClicked: {
                            foodManager.reload()
                            if (editingMerchantId > 0) {
                                resetMerchantForm()
                            }
                            if (editingDishId > 0) {
                                resetDishForm()
                            }
                        }
                    }

                    Label {
                        text: "商家数：" + foodManager.merchantCount
                        color: "#6d5846"
                    }

                    Label {
                        text: "菜品数：" + foodManager.dishCount
                        color: "#6d5846"
                    }
                }

                Label {
                    visible: foodManager.lastError.length > 0
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: foodManager.lastError
                    color: "#a13f2d"
                }
            }
        }

        AutoHeightRectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#e5efe6"

            ColumnLayout {
                x: 20
                y: 20
                width: parent.width - 20 * 2
                height: implicitHeight
                spacing: 12

                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: editingMerchantId > 0 ? "编辑商家" : "新增商家"
                        font.pixelSize: 20
                        font.bold: true
                        color: "#223126"
                    }

                    ReadableButton {
                        visible: editingMerchantId > 0
                        text: "取消"
                        onClicked: resetMerchantForm()
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.narrowLayout ? 1 : 2
                    columnSpacing: 12
                    rowSpacing: 10

                    TextField {
                        id: merchantNameField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "商家名"
                    }

                    TextField {
                        id: merchantAreaField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "校区区域 / 楼栋"
                    }

                    ComboBox {
                        id: merchantPriceLevelBox
                        Layout.fillWidth: true
                        model: root.priceLevelOptions
                        textRole: "label"
                        valueRole: "value"
                    }

                    TextField {
                        id: merchantDistanceField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "步行分钟数"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    TextField {
                        id: merchantQueueField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "排队分钟数"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    TextField {
                        id: merchantDeliveryField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "外卖预计分钟数"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 12

                    CheckBox {
                        id: merchantDineInCheck
                        text: "堂食"
                    }

                    CheckBox {
                        id: merchantTakeawayCheck
                        text: "打包"
                    }

                    CheckBox {
                        id: merchantDeliveryCheck
                        text: "外卖"
                    }
                }

                TextArea {
                    id: merchantNotesField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    placeholderText: activeFocus || text.length > 0 ? "" : "可用性备注，例如档口活动、容易售罄、时段限制"
                    wrapMode: TextEdit.Wrap
                }

                ReadableButton {
                    text: editingMerchantId > 0 ? "更新商家" : "保存商家"
                    onClicked: saveMerchant()
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

                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: editingDishId > 0 ? "编辑菜品" : "新增菜品"
                        font.pixelSize: 20
                        font.bold: true
                        color: "#36391f"
                    }

                    ReadableButton {
                        visible: editingDishId > 0
                        text: "取消"
                        onClicked: resetDishForm()
                    }
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    visible: foodManager.frequentMerchants.length > 0

                    Repeater {
                        model: foodManager.frequentMerchants

                        ReadableButton {
                            text: "常用商家：" + modelData.name
                            onClicked: dishMerchantBox.currentIndex = merchantIndexForId(modelData.id)
                        }
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.narrowLayout ? 1 : 2
                    columnSpacing: 12
                    rowSpacing: 10

                    TextField {
                        id: dishNameField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "菜品名"
                    }

                    ComboBox {
                        id: dishMerchantBox
                        Layout.fillWidth: true
                        model: foodManager.merchants
                        textRole: "name"
                        valueRole: "id"
                    }

                    TextField {
                        id: dishCategoryField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "分类"
                    }

                    TextField {
                        id: dishPriceField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "价格"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }

                    ComboBox {
                        id: dishDiningModeBox
                        Layout.fillWidth: true
                        model: root.diningModeOptions
                        textRole: "label"
                        valueRole: "value"
                    }

                    TextField {
                        id: dishEatTimeField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "用餐分钟数"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    TextField {
                        id: dishEffortField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "获取成本（1 容易 - 3 费事）"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    TextField {
                        id: dishImpactField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "餐次影响权重（普通餐 1.0，饮料/加餐可更低）"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }
                }

                AutoHeightRectangle {
                    Layout.fillWidth: true
                    radius: 14
                    color: "#f8f6e7"
                    border.color: "#d8d3af"
                    border.width: 1

                    ColumnLayout {
                        x: 12
                        y: 12
                        width: parent.width - 24
                        height: implicitHeight
                        spacing: 4

                        Label {
                            Layout.fillWidth: true
                            text: "菜品标签（低 / 中 / 高）"
                            color: "#36391f"
                            font.bold: true
                            wrapMode: Text.Wrap
                        }

                        Label {
                            Layout.fillWidth: true
                            text: "这些是菜品特征，不是总评分；高蛋白/高纤维通常有利，高负担/高犯困风险通常不利。"
                            color: "#6a6b45"
                            wrapMode: Text.Wrap
                        }
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.narrowLayout ? 1 : 2
                    columnSpacing: 12
                    rowSpacing: 10

                    LevelField { id: carbBox; label: "碳水" }
                    LevelField { id: fatBox; label: "脂肪" }
                    LevelField { id: proteinBox; label: "蛋白" }
                    LevelField { id: vitaminBox; label: "维生素" }
                    LevelField { id: fiberBox; label: "纤维" }
                    LevelField { id: satietyBox; label: "饱腹" }
                    LevelField { id: burdenBox; label: "消化负担" }
                    LevelField { id: sleepinessBox; label: "犯困风险" }
                    LevelField { id: flavorBox; label: "口味" }
                    LevelField { id: odorBox; label: "气味" }
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 12

                    CheckBox {
                        id: comboCheck
                        text: "套餐 / 组合餐"
                    }

                    CheckBox {
                        id: beverageCheck
                        text: "饮料 / 零食"
                    }
                }

                TextArea {
                    id: dishNotesField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 90
                    placeholderText: activeFocus || text.length > 0 ? "" : "备注，例如口味、气味、适合场景"
                    wrapMode: TextEdit.Wrap
                }

                ReadableButton {
                    text: editingDishId > 0 ? "更新菜品" : "保存菜品"
                    enabled: foodManager.merchantCount > 0
                    onClicked: saveDish()
                }

                Label {
                    visible: foodManager.merchantCount === 0
                    text: "请先至少添加一个商家，再创建菜品。"
                    color: "#8a5a34"
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
                    text: "商家列表"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#31241a"
                }

                TextField {
                    Layout.fillWidth: true
                    placeholderText: activeFocus || text.length > 0 ? "" : "搜索商家、区域或备注"
                    text: foodManager.merchantSearch
                    onTextChanged: foodManager.setMerchantSearch(text)
                }

                Repeater {
                    model: foodManager.filteredMerchants

                    AutoHeightRectangle {
                        Layout.fillWidth: true
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

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                Label {
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: modelData.name + " | " + root.optionLabel(root.priceLevelOptions, modelData.priceLevel)
                                    font.bold: true
                                    color: "#2e241a"
                                }

                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    ReadableButton {
                                        text: "编辑"
                                        onClicked: loadMerchantForEdit(modelData)
                                    }

                                    ReadableButton {
                                        text: "删除"
                                        onClicked: {
                                            if (foodManager.deleteMerchant(modelData.id) && editingMerchantId === modelData.id) {
                                                resetMerchantForm()
                                            }
                                        }
                                    }
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: "步行 " + modelData.distanceMinutes + " 分钟，排队 " + modelData.queueTimeMinutes + " 分钟，外卖约 " + modelData.deliveryEtaMinutes + " 分钟"
                                color: "#5f4d3d"
                            }

                            Label {
                                visible: modelData.campusArea.length > 0
                                text: modelData.campusArea
                                color: "#5f4d3d"
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: modelData.notes
                                color: "#7b6756"
                                visible: modelData.notes.length > 0
                            }
                        }
                    }
                }

                Label {
                    visible: foodManager.filteredMerchants.length === 0
                    text: "当前搜索下没有命中的商家。"
                    color: "#7b6756"
                }
            }
        }

        AutoHeightRectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#e3e9f1"

            ColumnLayout {
                x: 20
                y: 20
                width: parent.width - 20 * 2
                height: implicitHeight
                spacing: 10

                Label {
                    text: "菜品列表"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#243041"
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: root.narrowLayout ? 1 : 2
                    columnSpacing: 12
                    rowSpacing: 10

                    TextField {
                        id: dishSearchField
                        Layout.fillWidth: true
                        placeholderText: activeFocus || text.length > 0 ? "" : "搜索菜品、商家、风险标签或用餐方式"
                        text: foodManager.dishSearch
                        onTextChanged: foodManager.setDishSearch(text)
                    }

                    ComboBox {
                        id: dishMerchantFilterBox
                        Layout.fillWidth: true
                        model: root.dishMerchantFilterOptions()
                        textRole: "name"
                        valueRole: "id"
                        currentIndex: root.dishMerchantFilterIndexForId(foodManager.dishMerchantFilterId)
                        onActivated: foodManager.setDishMerchantFilterId(currentValue)
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        text: "当前命中 " + foodManager.filteredDishes.length + " / " + foodManager.dishCount
                              + "。搜索会优先把更贴近关键词、且最近更常用的菜排在前面。"
                        color: "#5b6d84"
                    }

                    ReadableButton {
                        text: "清空筛选"
                        visible: foodManager.dishSearch.length > 0 || foodManager.dishMerchantFilterId > 0
                        onClicked: {
                            dishSearchField.text = ""
                            foodManager.setDishSearch("")
                            foodManager.setDishMerchantFilterId(0)
                        }
                    }
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    visible: foodManager.frequentDishes.length > 0

                    Repeater {
                        model: foodManager.frequentDishes

                        ReadableButton {
                            text: modelData.name
                            onClicked: loadDishForEdit(modelData)
                        }
                    }
                }

                Repeater {
                    model: foodManager.filteredDishes

                    AutoHeightRectangle {
                        Layout.fillWidth: true
                        radius: 14
                        color: "#f8fbff"
                        border.color: "#c6d3e3"
                        border.width: 1

                        ColumnLayout {
                            x: 12
                            y: 12
                            width: parent.width - 12 * 2
                            height: implicitHeight
                            spacing: 6

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                Label {
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: modelData.name + " | " + modelData.merchantName
                                    font.bold: true
                                    color: "#243041"
                                }

                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    ReadableButton {
                                        text: "编辑"
                                        onClicked: loadDishForEdit(modelData)
                                    }

                                    ReadableButton {
                                        text: "归档"
                                        onClicked: {
                                            if (foodManager.deleteDish(modelData.id) && editingDishId === modelData.id) {
                                                resetDishForm()
                                            }
                                        }
                                    }
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: "分类 " + modelData.category + " | 价格 " + modelData.price + " | 方式 " + root.optionLabel(root.diningModeOptions, modelData.defaultDiningMode)
                                color: "#465a74"
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: "碳/脂/蛋白/维生素/纤维："
                                      + root.optionLabel(root.levelOptions, modelData.carbLevel) + " / "
                                      + root.optionLabel(root.levelOptions, modelData.fatLevel) + " / "
                                      + root.optionLabel(root.levelOptions, modelData.proteinLevel) + " / "
                                      + root.optionLabel(root.levelOptions, modelData.vitaminLevel) + " / "
                                      + root.optionLabel(root.levelOptions, modelData.fiberLevel)
                                color: "#465a74"
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: "饱腹 " + root.optionLabel(root.levelOptions, modelData.satietyLevel)
                                      + "，负担 " + root.optionLabel(root.levelOptions, modelData.digestiveBurdenLevel)
                                      + "，犯困 " + root.optionLabel(root.levelOptions, modelData.sleepinessRiskLevel)
                                      + "，气味 " + root.optionLabel(root.levelOptions, modelData.odorLevel)
                                      + "，权重 " + modelData.mealImpactWeight
                                color: "#465a74"
                            }

                            Label {
                                Layout.fillWidth: true
                                wrapMode: Text.Wrap
                                text: modelData.notes
                                color: "#6b7a8f"
                                visible: modelData.notes.length > 0
                            }
                        }
                    }
                }

                Label {
                    visible: foodManager.filteredDishes.length === 0
                    text: "当前搜索下没有命中的菜品。"
                    color: "#6b7a8f"
                }
            }
        }
    }
}
