import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    clip: true

    property int editingMerchantId: 0
    property int editingDishId: 0

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
        merchantPriceLevelBox.currentIndex = Math.max(0, merchantPriceLevelBox.find(merchant.priceLevel))
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
        dishDiningModeBox.currentIndex = Math.max(0, dishDiningModeBox.find(dish.defaultDiningMode))
        dishEatTimeField.text = String(dish.eatTimeMinutes)
        dishEffortField.text = String(dish.acquireEffortScore)
        dishImpactField.text = String(dish.mealImpactWeight)
        carbBox.currentIndex = Math.max(0, carbBox.find(dish.carbLevel))
        fatBox.currentIndex = Math.max(0, fatBox.find(dish.fatLevel))
        proteinBox.currentIndex = Math.max(0, proteinBox.find(dish.proteinLevel))
        vitaminBox.currentIndex = Math.max(0, vitaminBox.find(dish.vitaminLevel))
        fiberBox.currentIndex = Math.max(0, fiberBox.find(dish.fiberLevel))
        satietyBox.currentIndex = Math.max(0, satietyBox.find(dish.satietyLevel))
        burdenBox.currentIndex = Math.max(0, burdenBox.find(dish.digestiveBurdenLevel))
        sleepinessBox.currentIndex = Math.max(0, sleepinessBox.find(dish.sleepinessRiskLevel))
        flavorBox.currentIndex = Math.max(0, flavorBox.find(dish.flavorLevel))
        odorBox.currentIndex = Math.max(0, odorBox.find(dish.odorLevel))
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
                merchantPriceLevelBox.currentText,
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
                merchantPriceLevelBox.currentText,
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
                dishDiningModeBox.currentText,
                Number(dishEatTimeField.text || "0"),
                Number(dishEffortField.text || "0"),
                carbBox.currentText,
                fatBox.currentText,
                proteinBox.currentText,
                vitaminBox.currentText,
                fiberBox.currentText,
                satietyBox.currentText,
                burdenBox.currentText,
                sleepinessBox.currentText,
                flavorBox.currentText,
                odorBox.currentText,
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
                dishDiningModeBox.currentText,
                Number(dishEatTimeField.text || "0"),
                Number(dishEffortField.text || "0"),
                carbBox.currentText,
                fatBox.currentText,
                proteinBox.currentText,
                vitaminBox.currentText,
                fiberBox.currentText,
                satietyBox.currentText,
                burdenBox.currentText,
                sleepinessBox.currentText,
                flavorBox.currentText,
                odorBox.currentText,
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
        spacing: 16

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 24
            color: "#f7f1e8"
            border.color: "#d7c8b9"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
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

                    Button {
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

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#e5efe6"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: editingMerchantId > 0 ? "编辑商家" : "新增商家"
                        font.pixelSize: 20
                        font.bold: true
                        color: "#223126"
                    }

                    Button {
                        visible: editingMerchantId > 0
                        text: "取消"
                        onClicked: resetMerchantForm()
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 10

                    TextField {
                        id: merchantNameField
                        Layout.fillWidth: true
                        placeholderText: "商家名"
                    }

                    TextField {
                        id: merchantAreaField
                        Layout.fillWidth: true
                        placeholderText: "校区区域 / 楼栋"
                    }

                    ComboBox {
                        id: merchantPriceLevelBox
                        Layout.fillWidth: true
                        model: ["budget", "mid", "high"]
                    }

                    TextField {
                        id: merchantDistanceField
                        Layout.fillWidth: true
                        placeholderText: "步行分钟数"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    TextField {
                        id: merchantQueueField
                        Layout.fillWidth: true
                        placeholderText: "排队分钟数"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    TextField {
                        id: merchantDeliveryField
                        Layout.fillWidth: true
                        placeholderText: "外卖预计分钟数"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }
                }

                RowLayout {
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
                    placeholderText: "可用性备注，例如档口活动、容易售罄、时段限制"
                    wrapMode: TextEdit.Wrap
                }

                Button {
                    text: editingMerchantId > 0 ? "更新商家" : "保存商家"
                    onClicked: saveMerchant()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#eef0df"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: editingDishId > 0 ? "编辑菜品" : "新增菜品"
                        font.pixelSize: 20
                        font.bold: true
                        color: "#36391f"
                    }

                    Button {
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

                        Button {
                            text: "常用商家：" + modelData.name
                            onClicked: dishMerchantBox.currentIndex = merchantIndexForId(modelData.id)
                        }
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 10

                    TextField {
                        id: dishNameField
                        Layout.fillWidth: true
                        placeholderText: "菜品名"
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
                        placeholderText: "分类"
                    }

                    TextField {
                        id: dishPriceField
                        Layout.fillWidth: true
                        placeholderText: "价格"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }

                    ComboBox {
                        id: dishDiningModeBox
                        Layout.fillWidth: true
                        model: ["dine_in", "takeaway", "delivery"]
                    }

                    TextField {
                        id: dishEatTimeField
                        Layout.fillWidth: true
                        placeholderText: "用餐分钟数"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    TextField {
                        id: dishEffortField
                        Layout.fillWidth: true
                        placeholderText: "获取成本 1-3"
                        inputMethodHints: Qt.ImhDigitsOnly
                    }

                    TextField {
                        id: dishImpactField
                        Layout.fillWidth: true
                        placeholderText: "餐次影响权重"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 3
                    columnSpacing: 12
                    rowSpacing: 10

                    ComboBox { id: carbBox; Layout.fillWidth: true; model: ["low", "medium", "high"] }
                    ComboBox { id: fatBox; Layout.fillWidth: true; model: ["low", "medium", "high"] }
                    ComboBox { id: proteinBox; Layout.fillWidth: true; model: ["low", "medium", "high"] }
                    ComboBox { id: vitaminBox; Layout.fillWidth: true; model: ["low", "medium", "high"] }
                    ComboBox { id: fiberBox; Layout.fillWidth: true; model: ["low", "medium", "high"] }
                    ComboBox { id: satietyBox; Layout.fillWidth: true; model: ["low", "medium", "high"] }
                    ComboBox { id: burdenBox; Layout.fillWidth: true; model: ["low", "medium", "high"] }
                    ComboBox { id: sleepinessBox; Layout.fillWidth: true; model: ["low", "medium", "high"] }
                    ComboBox { id: flavorBox; Layout.fillWidth: true; model: ["low", "medium", "high"] }
                }

                ComboBox {
                    id: odorBox
                    Layout.fillWidth: true
                    model: ["low", "medium", "high"]
                }

                RowLayout {
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
                    placeholderText: "备注，例如口味、气味、适合场景"
                    wrapMode: TextEdit.Wrap
                }

                Button {
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

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#ece4d8"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                Label {
                    text: "商家列表"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#31241a"
                }

                TextField {
                    Layout.fillWidth: true
                    placeholderText: "搜索商家、区域或备注"
                    text: foodManager.merchantSearch
                    onTextChanged: foodManager.setMerchantSearch(text)
                }

                Repeater {
                    model: foodManager.filteredMerchants

                    Rectangle {
                        Layout.fillWidth: true
                        radius: 14
                        color: "#fff8ef"
                        border.color: "#dcc9b5"
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 6

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    Layout.fillWidth: true
                                    text: modelData.name + " | " + modelData.priceLevel
                                    font.bold: true
                                    color: "#2e241a"
                                }

                                Button {
                                    text: "编辑"
                                    onClicked: loadMerchantForEdit(modelData)
                                }

                                Button {
                                    text: "删除"
                                    onClicked: {
                                        if (foodManager.deleteMerchant(modelData.id) && editingMerchantId === modelData.id) {
                                            resetMerchantForm()
                                        }
                                    }
                                }
                            }

                            Label {
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

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#e3e9f1"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                Label {
                    text: "菜品列表"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#243041"
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 10

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "搜索菜品、商家、风险标签或用餐方式"
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

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        text: "当前命中 " + foodManager.filteredDishes.length + " / " + foodManager.dishCount
                              + "。搜索会优先把更贴近关键词、且最近更常用的菜排在前面。"
                        color: "#5b6d84"
                    }

                    Button {
                        text: "清空筛选"
                        visible: foodManager.dishMerchantFilterId > 0
                        onClicked: foodManager.setDishMerchantFilterId(0)
                    }
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    visible: foodManager.frequentDishes.length > 0

                    Repeater {
                        model: foodManager.frequentDishes

                        Button {
                            text: modelData.name
                            onClicked: loadDishForEdit(modelData)
                        }
                    }
                }

                Repeater {
                    model: foodManager.filteredDishes

                    Rectangle {
                        Layout.fillWidth: true
                        radius: 14
                        color: "#f8fbff"
                        border.color: "#c6d3e3"
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 6

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: modelData.name + " | " + modelData.merchantName
                                    font.bold: true
                                    color: "#243041"
                                }

                                Button {
                                    text: "编辑"
                                    onClicked: loadDishForEdit(modelData)
                                }

                                Button {
                                    text: "归档"
                                    onClicked: {
                                        if (foodManager.deleteDish(modelData.id) && editingDishId === modelData.id) {
                                            resetDishForm()
                                        }
                                    }
                                }
                            }

                            Label {
                                text: "分类 " + modelData.category + " | 价格 " + modelData.price + " | 方式 " + modelData.defaultDiningMode
                                color: "#465a74"
                            }

                            Label {
                                text: "碳/脂/蛋白/维生素/纤维：" + modelData.carbLevel + " / " + modelData.fatLevel + " / " + modelData.proteinLevel + " / " + modelData.vitaminLevel + " / " + modelData.fiberLevel
                                color: "#465a74"
                            }

                            Label {
                                text: "饱腹 " + modelData.satietyLevel + "，负担 " + modelData.digestiveBurdenLevel + "，犯困 " + modelData.sleepinessRiskLevel + "，气味 " + modelData.odorLevel + "，权重 " + modelData.mealImpactWeight
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
