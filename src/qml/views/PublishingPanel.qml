// src/qml/views/PublishingPanel.qml
// Fase 6A — Painel de Publicação Direta e Agendamento nas Redes Sociais
// Conexão OAuth2 com YouTube, TikTok, Instagram Reels e X.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Drift

Item {
    id: root
    implicitWidth: 440
    implicitHeight: 720

    property var pubManager: app.publishingManager
    property int selectedPlatform: 0
    property string activeJobId: ""
    property double uploadProgress: 0.0
    property string uploadStatus: ""
    property bool isUploading: false
    property string publishedUrl: ""

    // Listen to publishing events
    Connections {
        target: pubManager
        function onPublishProgress(jobId, fraction, status) {
            if (jobId === root.activeJobId) {
                root.uploadProgress = fraction
                root.uploadStatus = status
                root.isUploading = (fraction < 1.0)
            }
        }
        function onPublishFinished(jobId, success, videoUrl, error) {
            if (jobId === root.activeJobId) {
                root.isUploading = false
                if (success) {
                    root.publishedUrl = videoUrl
                    publishSuccessAnim.start()
                } else {
                    root.uploadStatus = "Erro: " + error
                }
            }
        }
    }

    // ── Background
    Rectangle {
        anchors.fill: parent
        color: "#0a0a14"
        radius: 12
    }

    // ── Top Gradient Accent
    Rectangle {
        width: parent.width; height: 3; radius: 1.5
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#ef4444" }
            GradientStop { position: 0.5; color: "#ec4899" }
            GradientStop { position: 1.0; color: "#8b5cf6" }
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.topMargin: 4
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            anchors.margins: 18
            spacing: 16

            // ── Header
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Rectangle {
                    width: 38; height: 38; radius: 10
                    color: "#18121f"; border.color: "#ec4899"; border.width: 1
                    Text { anchors.centerIn: parent; text: "🚀"; font.pixelSize: 18 }
                }

                ColumnLayout {
                    spacing: 2
                    Layout.fillWidth: true
                    Text {
                        text: "Publicação Direta"
                        font { pixelSize: 17; weight: Font.Bold; family: "Inter" }
                        color: "#ffffff"
                    }
                    Text {
                        text: "Poste ou agende no YouTube, TikTok, Instagram e X"
                        font { pixelSize: 11; family: "Inter" }
                        color: "#94a3b8"
                    }
                }
            }

            // ── Platforms Selector Grid
            Text {
                text: "Selecione a Plataforma"
                font { pixelSize: 12; weight: Font.DemiBold; family: "Inter" }
                color: "#cbd5e1"
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                rowSpacing: 8; columnSpacing: 8

                Repeater {
                    model: pubManager ? pubManager.availablePlatforms() : []

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 64
                        radius: 8
                        color: isSelected ? "#1f1224" : (platMa.containsMouse ? "#13141f" : "#0f111a")
                        border.color: isSelected ? modelData.color : (platMa.containsMouse ? "#334155" : "#1e293b")
                        border.width: isSelected ? 1.5 : 1

                        readonly property bool isSelected: root.selectedPlatform === modelData.index

                        ColumnLayout {
                            anchors { fill: parent; margins: 8 }
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                Text {
                                    text: modelData.icon
                                    font.pixelSize: 16
                                }
                                Text {
                                    text: modelData.name
                                    font { pixelSize: 12; weight: Font.Bold; family: "Inter" }
                                    color: "#ffffff"
                                    Layout.fillWidth: true
                                }
                                Rectangle {
                                    width: 8; height: 8; radius: 4
                                    color: modelData.connected ? "#10b981" : "#64748b"
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Text {
                                    text: modelData.connected ? modelData.account : "Desconectado"
                                    font { pixelSize: 10; family: "Inter" }
                                    color: modelData.connected ? "#94a3b8" : "#64748b"
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Text {
                                    text: modelData.connected ? "Desconectar" : "Conectar"
                                    font { pixelSize: 9; weight: Font.DemiBold; family: "Inter" }
                                    color: modelData.connected ? "#f43f5e" : "#38bdf8"

                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            if (modelData.connected) {
                                                pubManager.disconnectPlatform(modelData.index)
                                            } else {
                                                pubManager.startAuth(modelData.index)
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        MouseArea {
                            id: platMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.selectedPlatform = modelData.index
                        }
                    }
                }
            }

            // ── Form: Title & Caption
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                Text {
                    text: "Título da Publicação"
                    font { pixelSize: 11; weight: Font.DemiBold; family: "Inter" }
                    color: "#94a3b8"
                }

                TextField {
                    id: titleInput
                    Layout.fillWidth: true
                    text: app.lastScriptHook !== "" ? app.lastScriptHook : "O Mistério Inexplicável #shorts"
                    font { pixelSize: 12; family: "Inter" }
                    color: "#ffffff"
                    background: Rectangle {
                        color: "#0f111a"; radius: 6
                        border.color: titleInput.activeFocus ? "#ec4899" : "#1e293b"
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                Text {
                    text: "Descrição / Legenda"
                    font { pixelSize: 11; weight: Font.DemiBold; family: "Inter" }
                    color: "#94a3b8"
                }

                TextArea {
                    id: descInput
                    Layout.fillWidth: true
                    implicitHeight: 70
                    wrapMode: Text.Wrap
                    text: (app.lastScriptBody !== "" ? app.lastScriptBody.left(120) + "..." : "Confira os detalhes surpreendentes dessa história!") + "\n\n#viral #curiosidades #historia"
                    font { pixelSize: 11; family: "Inter" }
                    color: "#cbd5e1"
                    background: Rectangle {
                        color: "#0f111a"; radius: 6
                        border.color: descInput.activeFocus ? "#ec4899" : "#1e293b"
                    }
                }
            }

            // ── Hashtags Quick Chips
            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Repeater {
                    model: ["#Shorts", "#Viral", "#Curiosidades", "#DarkStudio", "#Mistério"]

                    Rectangle {
                        height: 24
                        width: tagText.implicitWidth + 14
                        radius: 12
                        color: chipMa.containsMouse ? "#271731" : "#161320"
                        border.color: "#3b2144"

                        Text {
                            id: tagText
                            anchors.centerIn: parent
                            text: modelData
                            font { pixelSize: 10; family: "Inter" }
                            color: "#d8b4fe"
                        }

                        MouseArea {
                            id: chipMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (!descInput.text.includes(modelData)) {
                                    descInput.text = descInput.text + " " + modelData
                                }
                            }
                        }
                    }
                }
            }

            // ── Scheduling Options
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: schedCol.implicitHeight + 16
                radius: 8
                color: "#0f111a"; border.color: "#1e293b"; border.width: 1

                ColumnLayout {
                    id: schedCol
                    anchors { fill: parent; margins: 10 }
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Agendamento"
                            font { pixelSize: 11; weight: Font.Bold; family: "Inter" }
                            color: "#e2e8f0"
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: schedSwitch.checked ? "Agendar" : "Publicar Imediato"
                            font { pixelSize: 10; family: "Inter" }
                            color: schedSwitch.checked ? "#38bdf8" : "#10b981"
                        }
                        Switch {
                            id: schedSwitch
                            checked: false
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: schedSwitch.checked
                        spacing: 8

                        Repeater {
                            model: ["Hoje 18:00", "Hoje 21:00", "Amanhã 12:00", "Amanhã 19:00"]
                            Rectangle {
                                Layout.fillWidth: true
                                height: 28
                                radius: 6
                                color: timeMa.containsMouse ? "#1e293b" : "#131826"
                                border.color: "#334155"

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData
                                    font { pixelSize: 9; weight: Font.DemiBold; family: "Inter" }
                                    color: "#38bdf8"
                                }

                                MouseArea {
                                    id: timeMa
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        // Quick schedule preset
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ── Upload Progress Banner
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 56
                radius: 8
                color: "#131422"
                border.color: "#ec4899"
                border.width: 1
                visible: root.isUploading

                ColumnLayout {
                    anchors { fill: parent; margins: 10 }
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: root.uploadStatus
                            font { pixelSize: 11; family: "Inter" }
                            color: "#f472b6"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                        Text {
                            text: Math.round(root.uploadProgress * 100) + "%"
                            font { pixelSize: 11; weight: Font.Bold; family: "Inter" }
                            color: "#ffffff"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 6; radius: 3; color: "#1e1e2e"
                        Rectangle {
                            width: parent.width * Math.max(0.0, Math.min(1.0, root.uploadProgress))
                            height: parent.height; radius: 3
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "#ec4899" }
                                GradientStop { position: 1.0; color: "#ef4444" }
                            }
                        }
                    }
                }
            }

            // ── Published Result Link
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 46
                radius: 8
                color: "#06281e"
                border.color: "#10b981"
                border.width: 1
                visible: root.publishedUrl !== "" && !root.isUploading

                RowLayout {
                    anchors { fill: parent; margins: 10 }
                    spacing: 8
                    Text { text: "🎉"; font.pixelSize: 16 }
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 1
                        Text {
                            text: "Publicado com sucesso!"
                            font { pixelSize: 11; weight: Font.Bold; family: "Inter" }
                            color: "#10b981"
                        }
                        Text {
                            text: root.publishedUrl
                            font { pixelSize: 10; family: "Inter" }
                            color: "#34d399"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            // ── Main Action Button
            Rectangle {
                Layout.fillWidth: true
                height: 48
                radius: 8
                color: root.isUploading ? "#475569" : (publishMa.containsMouse ? "#db2777" : "#ec4899")

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: root.isUploading ? "⏳" : (schedSwitch.checked ? "📅" : "🚀")
                        font.pixelSize: 16
                    }
                    Text {
                        text: root.isUploading
                              ? "Enviando Vídeo..."
                              : (schedSwitch.checked ? "Agendar Publicação" : "Publicar Agora")
                        font { pixelSize: 13; weight: Font.Bold; family: "Inter" }
                        color: "#ffffff"
                    }
                }

                MouseArea {
                    id: publishMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    enabled: !root.isUploading
                    onClicked: {
                        if (!pubManager) return
                        const videoPath = "C:/Users/Alex/Videos/render_output.mp4"
                        root.activeJobId = pubManager.publishVideo(
                            root.selectedPlatform,
                            videoPath,
                            titleInput.text.trim(),
                            descInput.text.trim(),
                            ["shorts", "viral", "darkstudio"],
                            "",
                            schedSwitch.checked ? QDateTime.currentDateTime().addSecs(3600 * 3) : null,
                            true
                        )
                    }
                }
            }

            Item { Layout.fillHeight: true; implicitHeight: 12 }
        }
    }

    SequentialAnimation {
        id: publishSuccessAnim
        PropertyAnimation { target: root; property: "opacity"; from: 0.6; to: 1.0; duration: 400 }
    }
}
