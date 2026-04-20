import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    clip: true

    property int editingEntryId: 0

    function weekdayIndexForValue(value) {
        for (let i = 0; i < weekdayBox.count; ++i) {
            if (weekdayBox.model[i].value === value) {
                return i
            }
        }
        return 0
    }

    function periodIndexForValue(periodValue) {
        for (let i = 0; i < startPeriodBox.count; ++i) {
            if (scheduleManager.classPeriods[i].periodIndex === periodValue) {
                return i
            }
        }
        return 0
    }

    function resetScheduleForm() {
        editingEntryId = 0
        const today = new Date()
        const weekday = today.getDay() === 0 ? 7 : today.getDay()
        weekdayBox.currentIndex = weekdayIndexForValue(weekday >= 2 && weekday <= 5 ? weekday : 2)
        courseNameField.text = ""
        startPeriodBox.currentIndex = 0
        endPeriodBox.currentIndex = 0
        locationField.text = ""
        campusZoneField.text = ""
        intensityField.text = "medium"
        notesField.text = ""
    }

    function loadEntry(entry) {
        editingEntryId = entry.id
        weekdayBox.currentIndex = weekdayIndexForValue(entry.weekday)
        courseNameField.text = entry.courseName
        startPeriodBox.currentIndex = periodIndexForValue(entry.periodStart)
        endPeriodBox.currentIndex = periodIndexForValue(entry.periodEnd)
        locationField.text = entry.location
        campusZoneField.text = entry.campusZone
        intensityField.text = entry.intensityLevel
        notesField.text = entry.notes
    }

    function saveEntry() {
        let ok = false
        if (editingEntryId > 0) {
            ok = scheduleManager.updateEntry(
                editingEntryId,
                weekdayBox.currentValue,
                startPeriodBox.currentValue,
                endPeriodBox.currentValue,
                courseNameField.text,
                locationField.text,
                campusZoneField.text,
                intensityField.text,
                notesField.text
            )
        } else {
            ok = scheduleManager.addCustomEntry(
                weekdayBox.currentValue,
                startPeriodBox.currentValue,
                endPeriodBox.currentValue,
                courseNameField.text,
                locationField.text,
                campusZoneField.text,
                intensityField.text,
                notesField.text
            )
        }

        if (ok) {
            resetScheduleForm()
        }
    }

    Component.onCompleted: resetScheduleForm()

    ColumnLayout {
        width: root.availableWidth
        spacing: 16

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 24
            color: "#f8f3ea"
            border.color: "#d6c6b2"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                Label {
                    text: "Schedule Management"
                    font.pixelSize: 28
                    font.bold: true
                    color: "#2d2419"
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: "The managed planning window is Tuesday to Friday. You can reset the provided schedule, then edit or delete entries when real class changes happen."
                    color: "#5d4b3a"
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Button {
                        text: "Reload"
                        onClicked: scheduleManager.reload()
                    }

                    Button {
                        text: "Restore Provided Schedule"
                        onClicked: {
                            if (scheduleManager.resetToProvidedSchedule()) {
                                resetScheduleForm()
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        text: "Entries: " + scheduleManager.totalEntryCount
                        color: "#6c5847"
                    }
                }

                Label {
                    visible: scheduleManager.lastError.length > 0
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: scheduleManager.lastError
                    color: "#a13f2d"
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#e7efe1"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                Label {
                    text: "Class Periods"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#243223"
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        model: scheduleManager.classPeriods

                        Rectangle {
                            width: 150
                            height: 72
                            radius: 14
                            color: "#f6faf3"
                            border.color: "#cad9c3"

                            Column {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 4

                                Label {
                                    text: "Period " + modelData.periodIndex
                                    font.bold: true
                                    color: "#314530"
                                }

                                Label {
                                    text: modelData.timeRange
                                    color: "#415a40"
                                }

                                Label {
                                    text: modelData.sessionLabel
                                    color: "#6a7d69"
                                }
                            }
                        }
                    }
                }
            }
        }

        Repeater {
            model: scheduleManager.weekSchedule

            Rectangle {
                Layout.fillWidth: true
                Layout.margins: 16
                radius: 20
                color: modelData.entryCount > 0 ? "#f0e5d5" : "#efece7"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: modelData.label
                            font.pixelSize: 22
                            font.bold: true
                            color: "#322519"
                        }

                        Label {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                            text: modelData.entryCount > 0 ? (modelData.entryCount + " entries") : "No scheduled classes"
                            color: "#6b5643"
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Repeater {
                            model: modelData.entries

                            Rectangle {
                                Layout.fillWidth: true
                                radius: 16
                                color: "#fffaf3"
                                border.color: "#dbc9b3"
                                border.width: 1

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 6

                                    RowLayout {
                                        Layout.fillWidth: true

                                        Label {
                                            Layout.fillWidth: true
                                            wrapMode: Text.Wrap
                                            text: modelData.courseName
                                            font.bold: true
                                            color: "#2d2419"
                                        }

                                        Button {
                                            text: "Edit"
                                            onClicked: loadEntry(modelData)
                                        }

                                        Button {
                                            text: "Delete"
                                            onClicked: {
                                                if (scheduleManager.deleteEntry(modelData.id) && editingEntryId === modelData.id) {
                                                    resetScheduleForm()
                                                }
                                            }
                                        }
                                    }

                                    Label {
                                        text: modelData.periodRange + " | " + modelData.timeRange
                                        color: "#5a4a3c"
                                    }

                                    Label {
                                        visible: modelData.location.length > 0
                                        text: modelData.location
                                        color: "#5a4a3c"
                                    }

                                    Label {
                                        visible: modelData.campusZone.length > 0 || modelData.intensityLevel.length > 0
                                        text: "Zone " + modelData.campusZone + " | intensity " + modelData.intensityLevel
                                        color: "#5a4a3c"
                                    }

                                    Label {
                                        visible: modelData.notes.length > 0
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: modelData.notes
                                        color: "#816a55"
                                    }
                                }
                            }
                        }

                        Label {
                            visible: modelData.entryCount === 0
                            text: "Leave it empty, or add ad-hoc self-study, labs, or temporary plans."
                            color: "#7d756b"
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 16
            radius: 20
            color: "#dde8ef"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: editingEntryId > 0 ? "Edit Schedule Entry" : "Add Schedule Entry"
                        font.pixelSize: 20
                        font.bold: true
                        color: "#223747"
                    }

                    Button {
                        visible: editingEntryId > 0
                        text: "Cancel"
                        onClicked: resetScheduleForm()
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 10

                    ComboBox {
                        id: weekdayBox
                        Layout.fillWidth: true
                        model: [
                            { label: "Tuesday", value: 2 },
                            { label: "Wednesday", value: 3 },
                            { label: "Thursday", value: 4 },
                            { label: "Friday", value: 5 }
                        ]
                        textRole: "label"
                        valueRole: "value"
                    }

                    TextField {
                        id: courseNameField
                        Layout.fillWidth: true
                        placeholderText: "Course name"
                    }

                    ComboBox {
                        id: startPeriodBox
                        Layout.fillWidth: true
                        model: scheduleManager.classPeriods
                        textRole: "timeRange"
                        valueRole: "periodIndex"
                    }

                    ComboBox {
                        id: endPeriodBox
                        Layout.fillWidth: true
                        model: scheduleManager.classPeriods
                        textRole: "timeRange"
                        valueRole: "periodIndex"
                    }

                    TextField {
                        id: locationField
                        Layout.fillWidth: true
                        placeholderText: "Location"
                    }

                    TextField {
                        id: campusZoneField
                        Layout.fillWidth: true
                        placeholderText: "Campus zone"
                    }

                    TextField {
                        id: intensityField
                        Layout.fillWidth: true
                        placeholderText: "Intensity level"
                    }

                    TextField {
                        id: notesField
                        Layout.fillWidth: true
                        placeholderText: "Notes"
                    }
                }

                Button {
                    text: editingEntryId > 0 ? "Update Entry" : "Add Entry"
                    onClicked: saveEntry()
                }
            }
        }
    }
}
