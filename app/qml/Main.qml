import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window

    width: 420
    height: 800
    visible: true
    title: "MealAdvisor"

    color: "#f3efe6"

    header: TabBar {
        id: tabBar

        currentIndex: swipeView.currentIndex

        TabButton {
            text: "Home"
        }

        TabButton {
            text: "Schedule"
        }

        TabButton {
            text: "Food"
        }

        TabButton {
            text: "Meals"
        }
    }

    SwipeView {
        id: swipeView

        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        ScrollView {
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: 16

                Rectangle {
                    Layout.fillWidth: true
                    Layout.margins: 16
                    radius: 24
                    color: "#fffaf1"
                    border.color: "#d8ccb8"
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 12

                        Label {
                            text: "Today's Recommendation"
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

                        Rectangle {
                            Layout.fillWidth: true
                            radius: 18
                            color: "#f7f1e7"
                            border.color: "#dccfbe"
                            border.width: 1

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 14
                                spacing: 10

                                Label {
                                    text: "Supplement Input"
                                    font.pixelSize: 18
                                    font.bold: true
                                    color: "#2c241b"
                                }

                                Label {
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: recommendationEngine.apiConfigured
                                          ? recommendationEngine.supplementStatus
                                          : "Set MEALADVISOR_LLM_API_KEY and optionally MEALADVISOR_LLM_API_URL / MEALADVISOR_LLM_MODEL before calling the supplement parser."
                                    color: recommendationEngine.apiConfigured ? "#5d4c3e" : "#8f4b34"
                                }

                                TextArea {
                                    id: supplementInput
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 110
                                    placeholderText: "Example: 我有点饿，我想吃点碳水，下午的课我不上了，我想喝可乐"
                                    wrapMode: TextEdit.Wrap
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Button {
                                        text: recommendationEngine.busy ? "Parsing..." : "Parse Supplement"
                                        enabled: recommendationEngine.apiConfigured && !recommendationEngine.busy
                                        onClicked: recommendationEngine.parseSupplement(supplementInput.text)
                                    }

                                    Button {
                                        text: "Clear Supplement"
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

                                        Rectangle {
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

                        Button {
                            text: "Judge Recommendation"
                            enabled: !recommendationEngine.busy
                            onClicked: recommendationEngine.runDecision()
                        }

                        Repeater {
                            model: recommendationEngine.candidates

                            Rectangle {
                                Layout.fillWidth: true
                                radius: 16
                                color: "#f6efe2"
                                border.color: "#d8ccb8"
                                border.width: 1

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 6

                                    Label {
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: "#" + modelData.rank + " " + modelData.dishName + (modelData.merchantName.length > 0 ? (" | " + modelData.merchantName) : "")
                                        font.bold: true
                                        color: "#2c241b"
                                    }

                                    Label {
                                        text: "Score " + modelData.score + " | RMB " + Number(modelData.price).toFixed(0)
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
                                            text: "Why it fits"
                                            font.bold: true
                                            color: "#3f3124"
                                        }

                                        Repeater {
                                            model: modelData.reasons

                                            Label {
                                                Layout.fillWidth: true
                                                wrapMode: Text.Wrap
                                                text: "• " + modelData
                                                color: "#6a5b4d"
                                            }
                                        }
                                    }

                                    Rectangle {
                                        visible: modelData.warnings && modelData.warnings.length > 0
                                        Layout.fillWidth: true
                                        radius: 12
                                        color: "#fff1eb"
                                        border.color: "#e6b9a9"
                                        border.width: 1

                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.margins: 10
                                            spacing: 4

                                            Label {
                                                text: "Warnings"
                                                font.bold: true
                                                color: "#8a4635"
                                            }

                                            Repeater {
                                                model: modelData.warnings

                                                Label {
                                                    Layout.fillWidth: true
                                                    wrapMode: Text.Wrap
                                                    text: "• " + modelData
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
                                            text: "Score breakdown"
                                            font.bold: true
                                            color: "#3f3124"
                                        }

                                        Flow {
                                            Layout.fillWidth: true
                                            spacing: 8

                                            Repeater {
                                                model: modelData.breakdown

                                                Rectangle {
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

                Rectangle {
                    Layout.fillWidth: true
                    Layout.margins: 16
                    radius: 20
                    color: "#e6efe2"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 8

                        Label {
                            text: "Planned MVP Modules"
                            font.pixelSize: 20
                            font.bold: true
                            color: "#213025"
                        }

                        Label {
                            text: "1. Dish library"
                            color: "#314238"
                        }

                        Label {
                            text: "2. Meal records"
                            color: "#314238"
                        }

                        Label {
                            text: "3. Schedule constraints"
                            color: "#314238"
                        }

                        Label {
                            text: "4. Recommendation engine"
                            color: "#314238"
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.margins: 16
                    radius: 20
                    color: "#efe4d2"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 8

                        Label {
                            text: "Live App State"
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
                            text: "Active dishes: " + appState.activeDishCount
                            color: "#5b4938"
                        }

                        Label {
                            text: "Meal logs: " + appState.mealLogCount
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
