// src/qml/views/ThumbnailPreview.qml
// Fase 6B — Visualizador e Seletor de Thumbnails com IA
// Grid de 3 variantes de alta conversão geradas automaticamente a partir do vídeo.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Drift

Item {
    id: root
    implicitWidth: 440
    implicitHeight: 680

    property int selectedVariant: 0
    property bool isPortrait: false
    property var generatedPaths: [
        "image://thumbnail/variant_1",
        "image://thumbnail/variant_2",
        "image://thumbnail/variant_3"
    ]

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
            GradientStop { position: 0.0; color: "#f59e0b" }
            GradientStop { position: 0.5; color: "#ef4444" }
            GradientStop { position: 1.0; color: "#ec4899" }
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
                    color: "#221311"; border.color: "#f59e0b"; border.width: 1
                    Text { anchors.centerIn: parent; text: "🎨"; font.pixelSize: 18 }
                }

                ColumnLayout {
                    spacing: 2
                    Layout.fillWidth: true
                    Text {
                        text: "Thumbnails com IA"
                        font { pixelSize: 17; weight: Font.Bold; family: "Inter" }
                        color: "#ffffff"
                    }
                    Text {
                        text: "3 variantes automáticas com tipografia de alto impacto"
                        font { pixelSize: 11; family: "Inter" }
                        color: "#94a3b8"
                    }
                }
            }

            // ── Format Toggle (16:9 vs 9:16)
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Rectangle {
                    Layout.fillWidth: true; height: 34; radius: 6
                    color: !root.isPortrait ? "#1e293b" : "#0f172a"
                    border.color: !root.isPortrait ? "#f59e0b" : "#1e293b"

                    RowLayout {
                        anchors.centerIn: parent; spacing: 6
                        Text { text: "🖥"; font.pixelSize: 13 }
                        Text {
                            text: "16:9 (YouTube)"
                            font { pixelSize: 11; weight: Font.Bold; family: "Inter" }
                            color: !root.isPortrait ? "#ffffff" : "#94a3b8"
                        }
                    }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: root.isPortrait = false
                    }
                }

                Rectangle {
                    Layout.fillWidth: true; height: 34; radius: 6
                    color: root.isPortrait ? "#1e293b" : "#0f172a"
                    border.color: root.isPortrait ? "#f59e0b" : "#1e293b"

                    RowLayout {
                        anchors.centerIn: parent; spacing: 6
                        Text { text: "📱"; font.pixelSize: 13 }
                        Text {
                            text: "9:16 (Shorts/Reels)"
                            font { pixelSize: 11; weight: Font.Bold; family: "Inter" }
                            color: root.isPortrait ? "#ffffff" : "#94a3b8"
                        }
                    }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: root.isPortrait = true
                    }
                }
            }

            // ── Title Input Field
            ColumnLayout {
                Layout.fillWidth: true; spacing: 4
                Text {
                    text: "Texto da Miniatura (Impacto Máximo)"
                    font { pixelSize: 11; weight: Font.DemiBold; family: "Inter" }
                    color: "#94a3b8"
                }
                TextField {
                    id: thumbTitle
                    Layout.fillWidth: true
                    text: app.lastScriptHook !== "" ? app.lastScriptHook : "O SEGREDO REVELADO"
                    font { pixelSize: 12; family: "Inter"; weight: Font.Bold }
                    color: "#ffffff"
                    background: Rectangle {
                        color: "#0f111a"; radius: 6
                        border.color: thumbTitle.activeFocus ? "#f59e0b" : "#1e293b"
                    }
                }
            }

            // ── 3 Variants Cards
            Text {
                text: "Selecione a Variante para Publicação"
                font { pixelSize: 12; weight: Font.DemiBold; family: "Inter" }
                color: "#cbd5e1"
            }

            Repeater {
                model: [
                    { id: "dark_mystery", name: "Variante 1: Dark Mystery", badge: "CHOCANTE!", color: "#ef4444", desc: "Fundo contrastante, vinheta escura e texto branco com borda vermelha" },
                    { id: "viral_gold", name: "Variante 2: Viral Gold", badge: "REVELADO!", color: "#f59e0b", desc: "Gradiente dourado, alto contraste e tipografia amarela ultra-visível" },
                    { id: "high_impact", name: "Variante 3: Impacto Máximo", badge: "EXCLUSIVO!", color: "#06b6d4", desc: "Ciano vibrante, magenta e contornos de alta definição" }
                ]

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 120
                    radius: 8
                    color: isSelected ? "#1c1424" : (cardMa.containsMouse ? "#141522" : "#0f111a")
                    border.color: isSelected ? modelData.color : (cardMa.containsMouse ? "#334155" : "#1e293b")
                    border.width: isSelected ? 1.5 : 1

                    readonly property bool isSelected: root.selectedVariant === index

                    RowLayout {
                        anchors { fill: parent; margins: 10 }
                        spacing: 12

                        // Preview box (mock or generated)
                        Rectangle {
                            width: root.isPortrait ? 60 : 120
                            height: 100
                            radius: 6
                            color: "#181a26"
                            clip: true

                            // Mock thumbnail styling
                            Rectangle {
                                anchors.fill: parent
                                gradient: Gradient {
                                    orientation: Gradient.Vertical
                                    GradientStop { position: 0.0; color: "#0a0b12" }
                                    GradientStop { position: 1.0; color: modelData.color }
                                }
                                opacity: 0.35
                            }

                            Text {
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.margins: 4
                                text: modelData.badge
                                font { pixelSize: 8; weight: Font.Bold; family: "Inter" }
                                color: "#ffffff"
                            }

                            Text {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.margins: 4
                                text: thumbTitle.text.left(14) + ".."
                                font { pixelSize: 9; weight: Font.Bold; family: "Impact" }
                                color: "#ffffff"
                            }
                        }

                        // Details
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true
                                Text {
                                    text: modelData.name
                                    font { pixelSize: 12; weight: Font.Bold; family: "Inter" }
                                    color: isSelected ? "#ffffff" : "#e2e8f0"
                                }
                                Item { Layout.fillWidth: true }
                                Rectangle {
                                    width: 18; height: 18; radius: 9
                                    color: isSelected ? modelData.color : "transparent"
                                    border.color: isSelected ? modelData.color : "#64748b"
                                    border.width: 1.5
                                    Text {
                                        visible: isSelected
                                        anchors.centerIn: parent
                                        text: "✓"
                                        font { pixelSize: 11; weight: Font.Bold }
                                        color: "#ffffff"
                                    }
                                }
                            }

                            Text {
                                text: modelData.desc
                                font { pixelSize: 10; family: "Inter" }
                                color: "#94a3b8"
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }

                    MouseArea {
                        id: cardMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.selectedVariant = index
                    }
                }
            }

            // ── Action Buttons
            Rectangle {
                Layout.fillWidth: true; height: 46; radius: 8
                color: genMa.containsMouse ? "#d97706" : "#f59e0b"

                RowLayout {
                    anchors.centerIn: parent; spacing: 8
                    Text { text: "⚡"; font.pixelSize: 16 }
                    Text {
                        text: "Usar Esta Thumbnail para o Vídeo"
                        font { pixelSize: 13; weight: Font.Bold; family: "Inter" }
                        color: "#ffffff"
                    }
                }

                MouseArea {
                    id: genMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        // Confirma seleção
                    }
                }
            }

            Item { Layout.fillHeight: true; implicitHeight: 12 }
        }
    }
}
