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

        contentItem: Label {
            text: readableButton.text
            color: readableButton.enabled ? "#2c241b" : "#8a8176"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
    id: window

    width: 420
    height: 800
    minimumWidth: 320
    minimumHeight: 560
    visible: true
    title: "MealAdvisor"

    color: "#f3efe6"

    Dialog {
        id: llmConfigDialog

        modal: true
        width: Math.min(window.width - 32, 360)
        title: "LLM 配置"
        standardButtons: Dialog.NoButton

        onOpened: {
            apiKeyField.text = appConfig.llmApiKey
            apiUrlField.text = appConfig.llmApiUrl
            modelField.text = appConfig.llmModel
        }

        ColumnLayout {
            width: parent.width
            spacing: 10

            Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: appConfig.llmConfigSummary
                color: "#5d4c3e"
            }

            TextField {
                id: apiKeyField
                Layout.fillWidth: true
                placeholderText: "API Key"
                echoMode: TextInput.Password
            }

            TextField {
                id: apiUrlField
                Layout.fillWidth: true
                placeholderText: appConfig.effectiveLlmApiUrl.length > 0
                                 ? appConfig.effectiveLlmApiUrl
                                 : "https://api.openai.com/v1/chat/completions"
            }

            TextField {
                id: modelField
                Layout.fillWidth: true
                placeholderText: appConfig.effectiveLlmModel.length > 0
                                 ? appConfig.effectiveLlmModel
                                 : "gpt-4o-mini"
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: "留空会继续回退到环境变量；API URL 可指向 DeepSeek 兼容端点。"
                color: "#7b6653"
            }

            Flow {
                Layout.fillWidth: true
                spacing: 8

                ReadableButton {
                    text: "保存"
                    onClicked: {
                        appConfig.saveLlmSettings(apiKeyField.text,
                                                  apiUrlField.text,
                                                  modelField.text)
                        llmConfigDialog.close()
                    }
                }

                ReadableButton {
                    text: "清空本地配置"
                    onClicked: {
                        appConfig.clearLlmSettings()
                        llmConfigDialog.close()
                    }
                }

                ReadableButton {
                    text: "关闭"
                    onClicked: llmConfigDialog.close()
                }
            }
        }
    }

    header: TabBar {
        id: tabBar

        currentIndex: swipeView.currentIndex

        TabButton {
            width: tabBar.width / 4
            text: "首页"
        }

        TabButton {
            width: tabBar.width / 4
            text: "课表"
        }

        TabButton {
            width: tabBar.width / 4
            text: "食物"
        }

        TabButton {
            width: tabBar.width / 4
            text: "餐次"
        }
    }

    SwipeView {
        id: swipeView

        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        ScrollView {
            clip: true

            ColumnLayout {
                width: swipeView.width
                height: implicitHeight
                spacing: 16

                AutoHeightRectangle {
                    Layout.fillWidth: true
                    Layout.margins: 16
                    radius: 24
                    color: "#fffaf1"
                    border.color: "#d8ccb8"
                    border.width: 1

                    ColumnLayout {
                        x: 20
                        y: 20
                        width: parent.width - 20 * 2
                        height: implicitHeight
                        spacing: 12

                        Label {
                            text: "当前推荐"
                            font.pixelSize: 28
                            font.bold: true
                            color: "#2c241b"
                        }

                        Label {
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
                            text: recommendationEngine.summary
                            color: "#4f4336"
                        }

                        AutoHeightRectangle {
                            Layout.fillWidth: true
                            radius: 18
                            color: "#f7f1e7"
                            border.color: "#dccfbe"
                            border.width: 1

                            ColumnLayout {
                                x: 14
                                y: 14
                                width: parent.width - 14 * 2
                                height: implicitHeight
                                spacing: 10

                                Label {
                                    text: "补充说明"
                                    font.pixelSize: 18
                                    font.bold: true
                                    color: "#2c241b"
                                }

                                Label {
                                    Layout.fillWidth: true
                                    visible: false
                                    wrapMode: Text.Wrap
                                    text: recommendationEngine.apiConfigured
                                          ? recommendationEngine.supplementStatus
                                          : "要使用补充说明解析，请先配置 API Key，并按需补充 API URL / Model。"
                                    color: recommendationEngine.apiConfigured ? "#5d4c3e" : "#8f4b34"
                                }

                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    AutoHeightRectangle {
                                        radius: 10
                                        color: recommendationEngine.supplementState === "success"
                                               ? "#d8ead2"
                                               : recommendationEngine.supplementState === "parsing"
                                                 ? "#f1e0b5"
                                                 : recommendationEngine.supplementState === "invalid_response"
                                                   ? "#f5d5cd"
                                                   : recommendationEngine.supplementState === "network_error"
                                                     ? "#f5d5cd"
                                                     : recommendationEngine.supplementState === "unconfigured"
                                                       ? "#eadfd3"
                                                       : "#e6ddd2"
                                        border.color: "#ccbca8"
                                        border.width: 1
                                        height: stateLabel.implicitHeight + 8
                                        width: stateLabel.implicitWidth + 14

                                        Label {
                                            id: stateLabel
                                            anchors.centerIn: parent
                                            text: recommendationEngine.supplementState === "success"
                                                  ? "解析成功"
                                                  : recommendationEngine.supplementState === "parsing"
                                                    ? "正在解析"
                                                    : recommendationEngine.supplementState === "invalid_response"
                                                      ? "格式非法"
                                                      : recommendationEngine.supplementState === "network_error"
                                                        ? "接口失败"
                                                        : recommendationEngine.supplementState === "unconfigured"
                                                          ? "未配置"
                                                          : "待命"
                                            color: "#4f4336"
                                        }
                                    }

                                    AutoHeightRectangle {
                                        visible: recommendationEngine.supplementFallbackActive
                                        radius: 10
                                        color: "#f5d5cd"
                                        border.color: "#d4a493"
                                        border.width: 1
                                        height: fallbackLabel.implicitHeight + 8
                                        width: fallbackLabel.implicitWidth + 14

                                        Label {
                                            id: fallbackLabel
                                            anchors.centerIn: parent
                                            text: "已回退默认参数"
                                            color: "#8f4b34"
                                        }
                                    }

                                    ReadableButton {
                                        text: "LLM 配置"
                                        enabled: !recommendationEngine.busy
                                        onClicked: llmConfigDialog.open()
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: recommendationEngine.supplementStatus
                                    color: recommendationEngine.supplementState === "success"
                                           ? "#4e6a45"
                                           : recommendationEngine.supplementState === "parsing"
                                             ? "#7a5d1e"
                                             : recommendationEngine.supplementState === "unconfigured"
                                               ? "#8f4b34"
                                               : recommendationEngine.supplementState === "invalid_response"
                                                 ? "#8f4b34"
                                                 : recommendationEngine.supplementState === "network_error"
                                                   ? "#8f4b34"
                                                   : "#5d4c3e"
                                }

                                TextArea {
                                    id: supplementInput
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 110
                                    placeholderText: "例如：我有点饿，想吃点碳水，下午的课不上了，想喝可乐"
                                    wrapMode: TextEdit.Wrap
                                }

                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    ReadableButton {
                                        text: recommendationEngine.busy ? "解析中..." : "解析补充说明"
                                        enabled: appConfig.llmApiConfigured && !recommendationEngine.busy
                                        onClicked: recommendationEngine.parseSupplement(supplementInput.text)
                                    }

                                    ReadableButton {
                                        text: "清空补充说明"
                                        enabled: !recommendationEngine.busy
                                        onClicked: recommendationEngine.clearSupplement()
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: recommendationEngine.supplementSummary
                                    color: "#6a5b4d"
                                }

                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Repeater {
                                        model: recommendationEngine.supplementWeights

                                        AutoHeightRectangle {
                                            radius: 12
                                            color: "#efe5d7"
                                            border.color: "#d6c5b4"
                                            border.width: 1
                                            height: weightLabel.implicitHeight + 12
                                            width: weightLabel.implicitWidth + 18

                                            Label {
                                                id: weightLabel
                                                anchors.centerIn: parent
                                                text: modelData.label + ": " + modelData.value
                                                color: "#5d4c3e"
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        ReadableButton {
                            text: "生成推荐"
                            enabled: !recommendationEngine.busy
                            onClicked: recommendationEngine.runDecision()
                        }

                        Repeater {
                            model: recommendationEngine.candidates

                            AutoHeightRectangle {
                                Layout.fillWidth: true
                                radius: 16
                                color: "#f6efe2"
                                border.color: "#d8ccb8"
                                border.width: 1

                                ColumnLayout {
                                    x: 14
                                    y: 14
                                    width: parent.width - 14 * 2
                                    height: implicitHeight
                                    spacing: 6

                                    Label {
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: "#" + modelData.rank + " " + modelData.dishName + (modelData.merchantName.length > 0 ? (" | " + modelData.merchantName) : "")
                                        font.bold: true
                                        color: "#2c241b"
                                    }

                                    Label {
                                        text: "分数 " + modelData.score + " | 价格 " + Number(modelData.price).toFixed(0) + " 元"
                                        color: "#5d4c3e"
                                    }

                                    Label {
                                        visible: modelData.reason && modelData.reason.length > 0
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: modelData.reason
                                        color: "#6a5b4d"
                                    }

                                    ColumnLayout {
                                        visible: modelData.reasons && modelData.reasons.length > 0
                                        Layout.fillWidth: true
                                        spacing: 4

                                        Label {
                                            text: "推荐理由"
                                            font.bold: true
                                            color: "#3f3124"
                                        }

                                        Repeater {
                                            model: modelData.reasons

                                            Label {
                                                Layout.fillWidth: true
                                                wrapMode: Text.Wrap
                                                text: "- " + modelData
                                                color: "#6a5b4d"
                                            }
                                        }
                                    }

                                    AutoHeightRectangle {
                                        visible: modelData.warnings && modelData.warnings.length > 0
                                        Layout.fillWidth: true
                                        radius: 12
                                        color: "#fff1eb"
                                        border.color: "#e6b9a9"
                                        border.width: 1

                                        ColumnLayout {
                                            x: 10
                                            y: 10
                                            width: parent.width - 10 * 2
                                            height: implicitHeight
                                            spacing: 4

                                            Label {
                                                text: "提醒"
                                                font.bold: true
                                                color: "#8a4635"
                                            }

                                            Repeater {
                                                model: modelData.warnings

                                                Label {
                                                    Layout.fillWidth: true
                                                    wrapMode: Text.Wrap
                                                    text: "- " + modelData
                                                    color: "#8a4635"
                                                }
                                            }
                                        }
                                    }

                                    ColumnLayout {
                                        visible: modelData.breakdown && modelData.breakdown.length > 0
                                        Layout.fillWidth: true
                                        spacing: 6

                                        Label {
                                            text: "分项得分"
                                            font.bold: true
                                            color: "#3f3124"
                                        }

                                        Flow {
                                            Layout.fillWidth: true
                                            spacing: 8

                                            Repeater {
                                                model: modelData.breakdown

                                                AutoHeightRectangle {
                                                    radius: 12
                                                    color: "#eee4d6"
                                                    border.color: "#d3c3ad"
                                                    border.width: 1
                                                    height: breakdownLabel.implicitHeight + 12
                                                    width: breakdownLabel.implicitWidth + 18

                                                    Label {
                                                        id: breakdownLabel
                                                        anchors.centerIn: parent
                                                        text: modelData.label + " " + modelData.score
                                                        color: "#5d4c3e"
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Label {
                            visible: recommendationEngine.candidates.length === 0
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
                            text: recommendationEngine.previewRecommendation()
                            color: "#6a5b4d"
                        }
                    }
                }

                AutoHeightRectangle {
                    Layout.fillWidth: true
                    Layout.margins: 16
                    radius: 20
                    color: "#e6efe2"

                    ColumnLayout {
                        x: 20
                        y: 20
                        width: parent.width - 20 * 2
                        height: implicitHeight
                        spacing: 8

                        Label {
                            text: "当前 MVP 模块"
                            font.pixelSize: 20
                            font.bold: true
                            color: "#213025"
                        }

                        Label {
                            text: "1. 菜品库"
                            color: "#314238"
                        }

                        Label {
                            text: "2. 餐次记录"
                            color: "#314238"
                        }

                        Label {
                            text: "3. 课表约束"
                            color: "#314238"
                        }

                        Label {
                            text: "4. 推荐引擎"
                            color: "#314238"
                        }
                    }
                }

                AutoHeightRectangle {
                    Layout.fillWidth: true
                    Layout.margins: 16
                    radius: 20
                    color: "#efe4d2"

                    ColumnLayout {
                        x: 20
                        y: 20
                        width: parent.width - 20 * 2
                        height: implicitHeight
                        spacing: 8

                        Label {
                            text: "当前应用状态"
                            font.pixelSize: 20
                            font.bold: true
                            color: "#3b2c1e"
                        }

                        Label {
                            text: appState.planningSummary
                            color: "#5b4938"
                        }

                        Label {
                            text: appState.defaultProfileName
                            color: "#5b4938"
                        }

                        Label {
                            text: appState.budgetSummary
                            color: "#5b4938"
                        }

                        Label {
                            text: "启用菜品：" + appState.activeDishCount
                            color: "#5b4938"
                        }

                        Label {
                            text: "餐次记录：" + appState.mealLogCount
                            color: "#5b4938"
                        }

                        Label {
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
                            text: appState.databasePath
                            color: "#7b6653"
                        }
                    }
                }
            }
        }

        SchedulePage {
        }

        FoodPage {
        }

        MealLogPage {
        }
    }
}
