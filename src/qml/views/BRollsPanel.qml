// src/qml/views/BRollsPanel.qml
// Fase 5B — Painel de B-Rolls Automáticos
// Grid visual de thumbnails com busca manual ou automática via keywords do roteirista.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Drift

Item {
    id: root
    implicitWidth: 380
    implicitHeight: 640

    // ── Listen for new B-Rolls arriving
    Connections {
        target: app
        function onBrollItemReady(query, localPath, previewUrl, durationSec, source) {
            // ListView model is bound to app.fetchedBRolls(), which auto-updates via
            // brollReadyCountChanged(). Nothing manual needed here.
        }
        function onBrollFetchingChanged() {
            if (!app.brollFetching && app.brollReadyCount > 0)
                successAnim.start()
        }
    }

    // ── Background
    Rectangle {
        anchors.fill: parent
        color: "#0a0a10"
        radius: 12
    }

    // Gradient top accent
    Rectangle {
        width: parent.width; height: 3; radius: 1.5
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#0ea5e9" }
            GradientStop { position: 0.5; color: "#6366f1" }
            GradientStop { position: 1.0; color: "#c026d3" }
        }
    }

    ColumnLayout {
        anchors { fill: parent; margins: 16; topMargin: 20 }
        spacing: 14

        // ── Header
        RowLayout {
            Layout.fillWidth: true; spacing: 10
            Rectangle {
                width: 34; height: 34; radius: 8
                color: "#0f0f20"; border.color: "#0ea5e9"; border.width: 1
                Text { anchors.centerIn: parent; text: "🎬"; font.pixelSize: 16 }
            }
            ColumnLayout {
                spacing: 2
                Text {
                    text: "B-Rolls Auto"
                    font { pixelSize: 16; weight: Font.Bold; family: "Inter" }
                    color: "#f1f5f9"
                }
                Text {
                    text: app.hasPexelsKey || app.hasPixabayKey
                          ? "Pexels · Pixabay · Cache local"
                          : "Configure a API key abaixo"
                    font { pixelSize: 11; family: "Inter" }
                    color: app.hasPexelsKey || app.hasPixabayKey ? "#38bdf8" : "#f59e0b"
                }
            }
            Item { Layout.fillWidth: true }
            // Badge count
            Rectangle {
                visible: app.brollReadyCount > 0
                height: 24; radius: 12
                width: badgeLbl.implicitWidth + 16
                color: "#0c4a6e"
                border.color: "#0ea5e9"; border.width: 1
                SequentialAnimation {
                    id: successAnim
                    NumberAnimation { target: badgeRect; property: "scale"; to: 1.3; duration: 120 }
                    NumberAnimation { target: badgeRect; property: "scale"; to: 1.0; duration: 120 }
                }
                id: badgeRect
                Text {
                    id: badgeLbl; anchors.centerIn: parent
                    text: app.brollReadyCount + " prontos"
                    font { pixelSize: 11; weight: Font.Bold; family: "Inter" }
                    color: "#7dd3fc"
                }
            }
        }

        // ── API key setup strip (shown only when no key is configured)
        Rectangle {
            Layout.fillWidth: true
            height: !app.hasPexelsKey && !app.hasPixabayKey ? 68 : 0
            clip: true; color: "#16100a"; radius: 8
            border.color: "#d97706"; border.width: 1
            Behavior on height { NumberAnimation { duration: 200 } }
            ColumnLayout {
                anchors { fill: parent; margins: 10 }; spacing: 4
                Text {
                    text: "🔑 Cole sua API Key do Pexels (gratuita)"
                    font { pixelSize: 12; weight: Font.Medium; family: "Inter" }
                    color: "#fbbf24"
                }
                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    Rectangle {
                        Layout.fillWidth: true; height: 28; radius: 6
                        color: "#0a0a12"; border.color: "#374151"; border.width: 1
                        TextInput {
                            id: keyField
                            anchors { fill: parent; leftMargin: 8; rightMargin: 8; topMargin: 4 }
                            font { pixelSize: 12; family: "Inter" }
                            color: "#e5e7eb"
                            echoMode: TextInput.Password
                            placeholderText: "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                        }
                    }
                    Rectangle {
                        height: 28; radius: 6; width: 60
                        color: "#0369a1"
                        Text { anchors.centerIn: parent; text: "Salvar"; font { pixelSize: 12; family: "Inter" }; color: "#fff" }
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: { app.configurePexelsApiKey(keyField.text.trim()); keyField.text = "" }
                        }
                    }
                }
            }
        }

        // ── Manual keyword search
        RowLayout {
            Layout.fillWidth: true; spacing: 8
            Rectangle {
                Layout.fillWidth: true; height: 36; radius: 8
                color: "#0d0d1a"; border.color: searchField.activeFocus ? "#6366f1" : "#1f1f2e"; border.width: 1
                Behavior on border.color { ColorAnimation { duration: 150 } }
                TextInput {
                    id: searchField
                    anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                    font { pixelSize: 13; family: "Inter" }
                    color: "#f1f5f9"; verticalAlignment: TextInput.AlignVCenter
                    Keys.onReturnPressed: doSearch()
                }
                Text {
                    visible: searchField.text.length === 0
                    anchors { fill: parent; leftMargin: 10 }
                    verticalAlignment: Text.AlignVCenter
                    text: "Buscar B-Rolls manualmente..."
                    font { pixelSize: 13; family: "Inter" }
                    color: "#374151"
                }
            }
            Rectangle {
                height: 36; width: 36; radius: 8
                color: "#1e1b4b"; border.color: "#6366f1"; border.width: 1
                Text { anchors.centerIn: parent; text: "⌕"; font.pixelSize: 16; color: "#818cf8" }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: doSearch()
                }
            }
        }

        // ── Quick-fetch from last script button
        Rectangle {
            Layout.fillWidth: true; height: 40; radius: 8
            visible: app.lastScriptBrollHints.length > 0
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#0c2340" }
                GradientStop { position: 1.0; color: "#130c2e" }
            }
            border.color: "#0ea5e9"; border.width: 1
            RowLayout {
                anchors { fill: parent; leftMargin: 12; rightMargin: 12 }
                Text {
                    text: "⚡ " + app.lastScriptBrollHints.length + " cenas do roteiro"
                    font { pixelSize: 13; weight: Font.Medium; family: "Inter" }
                    color: "#7dd3fc"
                }
                Item { Layout.fillWidth: true }
                Rectangle {
                    height: 26; radius: 13; width: fetchLbl.implicitWidth + 20
                    color: "#0ea5e9"
                    Text {
                        id: fetchLbl; anchors.centerIn: parent
                        text: "Buscar Todos"
                        font { pixelSize: 11; weight: Font.Bold; family: "Inter" }
                        color: "#fff"
                    }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: app.fetchBRollsFromLastScript(portraitToggle.checked)
                    }
                }
            }
        }

        // ── Progress bar
        ColumnLayout {
            visible: app.brollFetching
            Layout.fillWidth: true; spacing: 6
            ProgressBar {
                Layout.fillWidth: true
                value: app.brollFetchProgress
                background: Rectangle { color: "#111827"; radius: 4; implicitHeight: 6 }
                contentItem: Rectangle {
                    width: parent.visualPosition * parent.width
                    height: parent.height; radius: 4
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#0ea5e9" }
                        GradientStop { position: 1.0; color: "#6366f1" }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: app.brollFetchStatus
                    font { pixelSize: 11; family: "Inter" }; color: "#64748b"
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: "Cancelar"; font { pixelSize: 11; family: "Inter" }; color: "#f87171"
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: app.cancelBRollFetch() }
                }
            }
        }

        // ── Options row
        RowLayout {
            Layout.fillWidth: true; spacing: 12
            // Portrait toggle (Shorts / Reels)
            RowLayout {
                spacing: 6
                Rectangle {
                    width: 34; height: 20; radius: 10
                    color: portraitToggle.checked ? "#0369a1" : "#1f2937"
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Rectangle {
                        width: 16; height: 16; radius: 8
                        anchors.verticalCenter: parent.verticalCenter
                        x: portraitToggle.checked ? 16 : 2
                        Behavior on x { NumberAnimation { duration: 150 } }
                        color: "#ffffff"
                    }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: portraitToggle.checked = !portraitToggle.checked
                    }
                }
                CheckBox { id: portraitToggle; visible: false }
                Text { text: "Modo 9:16 (Shorts)"; font { pixelSize: 11; family: "Inter" }; color: "#6b7280" }
            }
            Item { Layout.fillWidth: true }
            Text {
                text: "🗑 Limpar cache"
                font { pixelSize: 11; family: "Inter" }; color: "#374151"
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: app.pruneBRollCache() }
            }
        }

        // ── B-Roll thumbnail grid
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: "#070710"; radius: 8; border.color: "#111827"; border.width: 1
            clip: true

            // Empty state
            Column {
                visible: app.brollReadyCount === 0 && !app.brollFetching
                anchors.centerIn: parent; spacing: 12
                Text { anchors.horizontalCenter: parent.horizontalCenter; text: "🎬"; font.pixelSize: 36 }
                Text {
                    text: "Seus B-Rolls aparecerão aqui"
                    font { pixelSize: 13; family: "Inter" }; color: "#374151"
                }
                Text {
                    text: "Use o roteiro ou busque acima"
                    font { pixelSize: 11; family: "Inter" }; color: "#1f2937"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }

            GridView {
                id: grid
                anchors { fill: parent; margins: 8 }
                cellWidth: (width - 4) / 2
                cellHeight: cellWidth * 9 / 16 + 36
                clip: true

                model: app.fetchedBRolls()

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                    contentItem: Rectangle { color: "#374151"; radius: 2 }
                }

                delegate: Item {
                    width: grid.cellWidth - 4
                    height: grid.cellHeight - 4

                    Rectangle {
                        anchors.fill: parent; anchors.margins: 2
                        color: "#0d0d1a"; radius: 8
                        border.color: delArea.containsMouse ? "#6366f1" : "#1f2937"
                        border.width: 1
                        clip: true

                        Behavior on border.color { ColorAnimation { duration: 150 } }

                        // Thumbnail
                        Image {
                            id: thumb
                            width: parent.width
                            height: parent.height - 36
                            source: modelData.previewUrl ?? ""
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                            // Fallback
                            Rectangle {
                                visible: thumb.status !== Image.Ready
                                anchors.fill: parent; color: "#0a0a18"
                                Text { anchors.centerIn: parent; text: "🎬"; font.pixelSize: 24 }
                            }
                            // Source badge
                            Rectangle {
                                anchors { top: parent.top; right: parent.right; margins: 4 }
                                height: 16; radius: 8; width: srcBadge.implicitWidth + 10
                                color: modelData.source === "pexels" ? "#0c2a1a" : "#1a1030"
                                border.color: modelData.source === "pexels" ? "#05b454" : "#7c3aed"; border.width: 1
                                Text {
                                    id: srcBadge; anchors.centerIn: parent
                                    text: modelData.source === "pexels" ? "Pexels" : "Pixabay"
                                    font { pixelSize: 9; weight: Font.Bold; family: "Inter" }
                                    color: modelData.source === "pexels" ? "#4ade80" : "#a78bfa"
                                }
                            }
                            // Duration badge
                            Rectangle {
                                anchors { bottom: parent.bottom; left: parent.left; margins: 4 }
                                height: 16; radius: 8; width: durBadge.implicitWidth + 10
                                color: "#000000aa"
                                Text {
                                    id: durBadge; anchors.centerIn: parent
                                    text: modelData.durationSec + "s"
                                    font { pixelSize: 9; family: "Inter" }; color: "#f1f5f9"
                                }
                            }
                        }

                        // Info row
                        RowLayout {
                            anchors { bottom: parent.bottom; left: parent.left; right: parent.right; margins: 6 }
                            height: 28; spacing: 4
                            Text {
                                Layout.fillWidth: true
                                text: modelData.query
                                font { pixelSize: 10; family: "Inter" }; color: "#9ca3af"
                                elide: Text.ElideRight
                            }
                            // Insert to timeline button
                            Rectangle {
                                height: 22; radius: 11; width: 22
                                color: insertArea.containsMouse ? "#0ea5e9" : "#0c1a2e"
                                Behavior on color { ColorAnimation { duration: 100 } }
                                Text { anchors.centerIn: parent; text: "+"; font { pixelSize: 14; weight: Font.Bold }; color: "#fff" }
                                MouseArea {
                                    id: insertArea; anchors.fill: parent; hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: app.insertBRollAtPlayhead(modelData.localPath)
                                }
                                ToolTip.visible: insertArea.containsMouse
                                ToolTip.delay: 600
                                ToolTip.text: "Inserir na timeline no playhead"
                            }
                        }

                        MouseArea {
                            id: delArea; anchors.fill: parent; hoverEnabled: true
                            // Propagate clicks to children
                            onClicked: {}
                        }
                    }
                }
            }
        }
    }

    function doSearch() {
        const q = searchField.text.trim()
        if (q.length < 2) return
        app.fetchBRolls([q], 2, portraitToggle.checked)
        searchField.text = ""
    }
}
