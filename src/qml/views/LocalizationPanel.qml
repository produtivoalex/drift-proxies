// src/qml/views/LocalizationPanel.qml
// Fase 5C — Dublagem Multiidioma & Localização Automática
// Interface Dark Studio para traduzir, sintetizar voz neural e sincronizar legendas em 8 idiomas.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Drift

Item {
    id: root
    implicitWidth: 420
    implicitHeight: 680

    property string selectedLangCode: "en-US"
    property double speechRate: 1.0
    property bool showSettings: false

    // ── Listen for completion events
    Connections {
        target: app
        function onDubbingFinished(success, audioPath, error) {
            if (success) {
                successAnim.start()
            }
        }
    }

    // ── Background
    Rectangle {
        anchors.fill: parent
        color: "#0a0a12"
        radius: 12
    }

    // ── Top Gradient Accent
    Rectangle {
        width: parent.width
        height: 3
        radius: 1.5
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#10b981" }
            GradientStop { position: 0.5; color: "#06b6d4" }
            GradientStop { position: 1.0; color: "#6366f1" }
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
                    width: 38
                    height: 38
                    radius: 10
                    color: "#0f172a"
                    border.color: "#06b6d4"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "🌐"
                        font.pixelSize: 18
                    }
                }

                ColumnLayout {
                    spacing: 2
                    Layout.fillWidth: true

                    Text {
                        text: "Dublagem & Localização"
                        font { pixelSize: 17; weight: Font.Bold; family: "Inter" }
                        color: "#ffffff"
                    }

                    Text {
                        text: "Expanda seu vídeo para o mundo em 1 clique"
                        font { pixelSize: 11; family: "Inter" }
                        color: "#94a3b8"
                    }
                }

                // Settings toggle button
                Rectangle {
                    width: 32
                    height: 32
                    radius: 8
                    color: settingsBtn.containsMouse ? "#1e293b" : "#0f172a"
                    border.color: root.showSettings ? "#06b6d4" : "#334155"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "⚙"
                        font.pixelSize: 14
                        color: root.showSettings ? "#06b6d4" : "#cbd5e1"
                    }

                    MouseArea {
                        id: settingsBtn
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.showSettings = !root.showSettings
                    }
                }
            }

            // ── Settings Box (DeepL & LibreTranslate configuration)
            Rectangle {
                Layout.fillWidth: true
                visible: root.showSettings
                implicitHeight: settingsCol.implicitHeight + 24
                color: "#0f172a"
                radius: 10
                border.color: "#334155"
                border.width: 1

                ColumnLayout {
                    id: settingsCol
                    anchors { fill: parent; margins: 12 }
                    spacing: 10

                    Text {
                        text: "Configuração de Tradução"
                        font { pixelSize: 12; weight: Font.DemiBold; family: "Inter" }
                        color: "#38bdf8"
                    }

                    // DeepL Key
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: "DeepL API Key (Alta Qualidade — Grátis 500k chars)"
                            font { pixelSize: 10; family: "Inter" }
                            color: "#94a3b8"
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            TextField {
                                id: deeplInput
                                Layout.fillWidth: true
                                placeholderText: "ex: 12345678-abcd-...:fx"
                                echoMode: TextInput.Password
                                font { pixelSize: 11; family: "Inter" }
                                color: "#ffffff"
                                background: Rectangle {
                                    color: "#090d16"
                                    radius: 6
                                    border.color: deeplInput.activeFocus ? "#06b6d4" : "#1e293b"
                                }
                            }

                            Button {
                                text: "Salvar"
                                onClicked: {
                                    if (deeplInput.text.trim().length > 0) {
                                        app.configureDeepLApiKey(deeplInput.text.trim())
                                        deeplInput.text = ""
                                    }
                                }
                            }
                        }
                    }

                    // LibreTranslate URL
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: "LibreTranslate URL (Fallback Gratuito)"
                            font { pixelSize: 10; family: "Inter" }
                            color: "#94a3b8"
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            TextField {
                                id: libreInput
                                Layout.fillWidth: true
                                text: "https://libretranslate.com"
                                font { pixelSize: 11; family: "Inter" }
                                color: "#ffffff"
                                background: Rectangle {
                                    color: "#090d16"
                                    radius: 6
                                    border.color: libreInput.activeFocus ? "#06b6d4" : "#1e293b"
                                }
                            }

                            Button {
                                text: "Salvar"
                                onClicked: {
                                    if (libreInput.text.trim().length > 0) {
                                        app.configureLibreTranslateUrl(libreInput.text.trim())
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ── Target Language Selection
            Text {
                text: "Selecione o Idioma Alvo"
                font { pixelSize: 12; weight: Font.DemiBold; family: "Inter" }
                color: "#cbd5e1"
            }

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                rowSpacing: 8
                columnSpacing: 8

                Repeater {
                    model: app.supportedDubbingLanguages

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 52
                        radius: 8
                        color: isSelected ? "#0c2838" : (cardMa.containsMouse ? "#111827" : "#0f172a")
                        border.color: isSelected ? "#06b6d4" : (cardMa.containsMouse ? "#334155" : "#1e293b")
                        border.width: isSelected ? 1.5 : 1

                        readonly property bool isSelected: root.selectedLangCode === modelData.code

                        RowLayout {
                            anchors { fill: parent; margins: 10 }
                            spacing: 10

                            Text {
                                text: modelData.flag
                                font.pixelSize: 22
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: modelData.label
                                    font { pixelSize: 12; weight: Font.DemiBold; family: "Inter" }
                                    color: isSelected ? "#ffffff" : "#e2e8f0"
                                }

                                Text {
                                    text: modelData.code + " • " + (modelData.defaultVoiceId.split("-")[2] || "Neural")
                                    font { pixelSize: 9; family: "Inter" }
                                    color: isSelected ? "#38bdf8" : "#64748b"
                                }
                            }

                            Text {
                                visible: isSelected
                                text: "✓"
                                font { pixelSize: 14; weight: Font.Bold }
                                color: "#06b6d4"
                            }
                        }

                        MouseArea {
                            id: cardMa
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.selectedLangCode = modelData.code
                        }
                    }
                }
            }

            // ── Speech Rate Slider
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "Velocidade da Voz"
                        font { pixelSize: 11; weight: Font.DemiBold; family: "Inter" }
                        color: "#94a3b8"
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: root.speechRate.toFixed(2) + "x"
                        font { pixelSize: 11; weight: Font.Bold; family: "Inter" }
                        color: "#06b6d4"
                    }
                }

                Slider {
                    id: speedSlider
                    Layout.fillWidth: true
                    from: 0.8
                    to: 1.4
                    stepSize: 0.05
                    value: 1.0
                    onValueChanged: root.speechRate = value
                }
            }

            // ── Progress Bar & Status (Visible during dubbing)
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: progressCol.implicitHeight + 16
                radius: 8
                color: "#0f172a"
                border.color: "#1e293b"
                border.width: 1
                visible: app.dubbingActive || (app.lastDubbedAudioPath !== "")

                ColumnLayout {
                    id: progressCol
                    anchors { fill: parent; margins: 10 }
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: app.dubbingActive ? app.dubbingStatus : "Dublagem Concluída!"
                            font { pixelSize: 11; family: "Inter" }
                            color: app.dubbingActive ? "#38bdf8" : "#10b981"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                        Text {
                            text: Math.round(app.dubbingProgress * 100) + "%"
                            font { pixelSize: 11; weight: Font.Bold; family: "Inter" }
                            color: "#ffffff"
                        }
                    }

                    // Bar
                    Rectangle {
                        Layout.fillWidth: true
                        height: 6
                        radius: 3
                        color: "#1e293b"

                        Rectangle {
                            width: parent.width * Math.max(0.0, Math.min(1.0, app.dubbingProgress))
                            height: parent.height
                            radius: 3
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "#06b6d4" }
                                GradientStop { position: 1.0; color: "#10b981" }
                            }
                        }
                    }
                }
            }

            // ── Insert Dubbed Audio Button (after success)
            Rectangle {
                Layout.fillWidth: true
                height: 44
                radius: 8
                visible: !app.dubbingActive && (app.lastDubbedAudioPath !== "")
                color: insertBtn.containsMouse ? "#059669" : "#10b981"

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 8
                    Text { text: "📥"; font.pixelSize: 16 }
                    Text {
                        text: "Inserir Áudio Dublado na Timeline"
                        font { pixelSize: 13; weight: Font.Bold; family: "Inter" }
                        color: "#ffffff"
                    }
                }

                MouseArea {
                    id: insertBtn
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: app.insertDubbedAudioAtPlayhead()
                }
            }

            // ── Main Action Button
            Rectangle {
                Layout.fillWidth: true
                height: 48
                radius: 8
                color: app.dubbingActive
                       ? "#ef4444"
                       : (mainActionMa.containsMouse ? "#0284c7" : "#0ea5e9")

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: app.dubbingActive ? "⏹" : "🌐"
                        font.pixelSize: 16
                    }

                    Text {
                        text: app.dubbingActive
                              ? "Cancelar Dublagem"
                              : "Dublar Projeto para " + root.selectedLangCode
                        font { pixelSize: 13; weight: Font.Bold; family: "Inter" }
                        color: "#ffffff"
                    }
                }

                MouseArea {
                    id: mainActionMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (app.dubbingActive) {
                            app.cancelDubbing()
                        } else {
                            app.dubProject(root.selectedLangCode, "", root.speechRate)
                        }
                    }
                }
            }

            // Bottom space
            Item { Layout.fillHeight: true; implicitHeight: 12 }
        }
    }

    // Success glow animation
    SequentialAnimation {
        id: successAnim
        PropertyAnimation { target: root; property: "opacity"; from: 0.7; to: 1.0; duration: 400 }
    }
}
