import QtQuick
import QtQuick.Controls.Basic
import Drift
import ".."
import "."

Item {
    id: root

    signal added()

    readonly property var allPresets: EditorState.virtualBackgroundPresets()
    property string activeCategory: "all"
    readonly property string query: search.text.trim().toLowerCase()
    property real blurSliderValue: 0.15

    readonly property var categories: [
        { id: "all", label: qsTr("Todos") },
        { id: "podcast", label: qsTr("🎙️ Podcast") },
        { id: "studio", label: qsTr("🏢 Estúdio & Loft") },
        { id: "gradients", label: qsTr("📷 Cinematográficos") },
        { id: "tech", label: qsTr("⚡ Tech & Neon") },
        { id: "dynamic", label: qsTr("✨ Dinâmicos") }
    ]

    readonly property var currentPresets: {
        const q = root.query
        return root.allPresets.filter(function(p) {
            const matchesCat = root.activeCategory === "all" || p.category === root.activeCategory
            if (!matchesCat)
                return false
            if (q.length === 0)
                return true
            const name = (p.name || "").toLowerCase()
            const desc = (p.description || "").toLowerCase()
            return name.indexOf(q) >= 0 || desc.indexOf(q) >= 0
        })
    }

    Column {
        anchors.fill: parent
        spacing: 8
        padding: Theme.pagePadding

        // Search Bar
        ThemedTextField {
            id: search
            width: parent.width
            placeholderText: qsTr("Buscar cenários de estúdio, podcast, neon…")
            glyph: Theme.icons.zoomIn
        }

        // Category Pills
        Flickable {
            width: parent.width
            height: 28
            contentWidth: categoryRow.width
            flickableDirection: Flickable.HorizontalFlick
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            Row {
                id: categoryRow
                spacing: 6

                Repeater {
                    model: root.categories

                    ThemedButton {
                        text: modelData.label
                        variant: root.activeCategory === modelData.id ? "primary" : "ghost"
                        onClicked: root.activeCategory = modelData.id
                    }
                }
            }
        }

        // Depth of Field (Virtual Bokeh Blur) Slider
        Rectangle {
            width: parent.width
            height: 48
            color: Theme.cardBackground
            radius: Theme.radiusSm
            border.width: 1
            border.color: Theme.cardBorder

            Row {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 12

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Desfoque de Fundo (DoF):")
                    color: Theme.panelForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeSm
                    font.weight: Font.Medium
                }

                Slider {
                    id: blurSlider
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 240
                    from: 0.0
                    to: 0.5
                    value: root.blurSliderValue
                    onMoved: root.blurSliderValue = value
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: Math.round(blurSlider.value * 200) + "%"
                    color: Theme.mutedForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeXs
                }
            }
        }

        // Presets Grid View
        GridView {
            id: grid
            width: parent.width
            height: parent.height - y - 10
            cellWidth: Math.floor(width / (width > 600 ? 3 : 2))
            cellHeight: 140
            clip: true
            model: root.currentPresets

            delegate: Item {
                width: grid.cellWidth
                height: grid.cellHeight

                Rectangle {
                    id: card
                    anchors.fill: parent
                    anchors.margins: 4
                    radius: Theme.radiusMd
                    color: Theme.cardBackground
                    border.width: hoverHandler.hovered ? 2 : 1
                    border.color: hoverHandler.hovered ? Theme.accent : Theme.cardBorder
                    clip: true

                    HoverHandler {
                        id: hoverHandler
                    }

                    // Background Visual Preview (Simulating the virtual set)
                    Rectangle {
                        anchors.fill: parent
                        radius: Theme.radiusMd - 1
                        gradient: Gradient {
                            orientation: modelData.gradientType === 1 ? Gradient.Radial : Gradient.Linear
                            GradientStop { position: 0.0; color: modelData.accentColor ? modelData.accentColor : modelData.primaryColor }
                            GradientStop { position: 0.6; color: modelData.primaryColor }
                            GradientStop { position: 1.0; color: modelData.secondaryColor }
                        }
                        opacity: 0.85
                    }

                    // Bottom info overlay
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 52
                        color: Qt.rgba(0, 0, 0, 0.75)

                        Column {
                            anchors.fill: parent
                            anchors.margins: 6
                            spacing: 2

                            Row {
                                width: parent.width
                                spacing: 4

                                Text {
                                    width: parent.width - badgeRect.width - 4
                                    text: modelData.name
                                    color: "#FFFFFF"
                                    font.family: Theme.fontFamily
                                    font.pixelSize: Theme.fontSizeSm
                                    font.weight: Font.SemiBold
                                    elide: Text.ElideRight
                                }

                                Rectangle {
                                    id: badgeRect
                                    height: 16
                                    width: badgeText.width + 8
                                    radius: 3
                                    color: Theme.accent

                                    Text {
                                        id: badgeText
                                        anchors.centerIn: parent
                                        text: modelData.badge
                                        color: "#FFFFFF"
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 9
                                        font.weight: Font.Bold
                                    }
                                }
                            }

                            Text {
                                width: parent.width
                                text: modelData.description
                                color: "#B0B0B0"
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }
                    }

                    // 1-Click Apply Button Overlay on Hover
                    Rectangle {
                        anchors.fill: parent
                        color: Qt.rgba(0, 0, 0, 0.6)
                        visible: hoverHandler.hovered
                        radius: Theme.radiusMd

                        ThemedButton {
                            anchors.centerIn: parent
                            text: qsTr("⚡ Aplicar Cenário")
                            variant: "primary"
                            onClicked: {
                                EditorState.applyVirtualBackground(
                                    EditorState.selectedTrack,
                                    EditorState.selectedClip,
                                    modelData.id,
                                    root.blurSliderValue
                                )
                                root.added()
                            }
                        }
                    }
                }
            }
        }
    }
}
