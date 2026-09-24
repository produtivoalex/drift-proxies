import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import Drift
import ".."
import "."

Item {
    id: root

    signal added()

    readonly property var videoTemplates: [
        {
            id: "shorts-viral",
            title: qsTr("📱 Shorts Viral (Alex Hormozi)"),
            category: "shorts",
            badge: qsTr("Alta Retenção"),
            aspect: "9:16",
            desc: qsTr("Cortes rápidos, legendas com karaokê amarelo/verde, zooms automáticos e ritmo dinâmico para TikTok/Reels."),
            gradient1: "#FF416C",
            gradient2: "#8A2387"
        },
        {
            id: "dark-webdoc",
            title: qsTr("🎬 Dark WebDoc Cinema"),
            category: "webdoc",
            badge: qsTr("Cinematográfico"),
            aspect: "16:9",
            desc: qsTr("Estilo documentário sombrio, vinhetas profundas, movimento Ken Burns lento em imagens e trilha de suspense."),
            gradient1: "#0F2027",
            gradient2: "#203A43"
        },
        {
            id: "podcast-pro",
            title: qsTr("🎙️ Podcast Studio Pro"),
            category: "podcast",
            badge: qsTr("Estúdio"),
            aspect: "16:9 / 9:16",
            desc: qsTr("Fundo de estúdio desfocado (DoF Bokeh), cancelamento de ruído neural, ducking de trilha e legendas limpas."),
            gradient1: "#2C3E50",
            gradient2: "#4CA1AF"
        },
        {
            id: "curiosities-top5",
            title: qsTr("💡 Curiosidades & Top 5"),
            category: "list",
            badge: qsTr("Educativo"),
            aspect: "9:16",
            desc: qsTr("Ritmo ágil com contagem regressiva, efeitos sonoros nas viradas de cena e blocos temáticos."),
            gradient1: "#F37335",
            gradient2: "#FDC830"
        }
    ]

    Column {
        anchors.fill: parent
        spacing: 12
        padding: Theme.pagePadding

        // Header / Description
        Row {
            width: parent.width
            spacing: 8

            Text {
                text: qsTr("📦 Templates Prontos de 1-Clique")
                color: Theme.foreground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeMd
                font.weight: Font.SemiBold
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Global Sync Toggle Card
        Rectangle {
            width: parent.width
            height: 48
            color: Theme.cardBackground
            radius: Theme.radiusMd
            border.width: 1
            border.color: Theme.cardBorder

            Row {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("⚡ Sincronizar estilo/posição em todas as legendas:")
                    color: Theme.panelForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSm
                    font.weight: Font.Medium
                }

                Item {
                    Layout.fillWidth: true
                }

                ThemedSwitch {
                    anchors.verticalCenter: parent.verticalCenter
                    checked: EditorState.syncAllCaptions
                    onToggled: EditorState.setSyncAllCaptions(checked)
                }
            }
        }

        // Templates Cards View
        ListView {
            id: templateList
            width: parent.width
            height: parent.height - y - 10
            spacing: 10
            clip: true
            model: root.videoTemplates

            delegate: Rectangle {
                id: card
                width: templateList.width
                height: 110
                radius: Theme.radiusMd
                color: Theme.cardBackground
                border.width: hoverHandler.hovered ? 2 : 1
                border.color: hoverHandler.hovered ? Theme.accent : Theme.cardBorder
                clip: true

                HoverHandler {
                    id: hoverHandler
                }

                // Background gradient accent on left edge
                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 6
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: modelData.gradient1 }
                        GradientStop { position: 1.0; color: modelData.gradient2 }
                    }
                }

                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.right: actionBtn.left
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    Row {
                        spacing: 8
                        Text {
                            text: modelData.title
                            color: Theme.foreground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSm
                            font.weight: Font.Bold
                        }

                        Rectangle {
                            height: 18
                            width: badgeTxt.width + 10
                            radius: 4
                            color: Theme.accent

                            Text {
                                id: badgeTxt
                                anchors.centerIn: parent
                                text: modelData.badge
                                color: "#FFFFFF"
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                font.weight: Font.Bold
                            }
                        }

                        Rectangle {
                            height: 18
                            width: aspectTxt.width + 8
                            radius: 4
                            color: Theme.surfaceBackground
                            border.width: 1
                            border.color: Theme.cardBorder

                            Text {
                                id: aspectTxt
                                anchors.centerIn: parent
                                text: modelData.aspect
                                color: Theme.mutedForeground
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        text: modelData.desc
                        color: Theme.mutedForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                }

                ThemedButton {
                    id: actionBtn
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("⚡ Aplicar")
                    variant: "primary"
                    onClicked: {
                        EditorState.applyVideoTemplate(modelData.id)
                        root.added()
                    }
                }
            }
        }
    }
}
