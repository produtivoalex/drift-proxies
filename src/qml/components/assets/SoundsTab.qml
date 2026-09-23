import QtQuick
import QtQuick.Controls.Basic
import QtMultimedia
import Drift
import ".."
import "."

// Sounds Tab: Built-in Viral SFX Library (Whoosh, Pop, Boom, Clicks) + Audio FX Preset Browser
Item {
    id: root

    property string currentTab: "sfx" // "sfx" | "fx"
    property string activeCategory: "all"
    property string searchFilter: ""
    property string previewingId: ""

    readonly property var allCategories: [
        { id: "all", label: qsTr("Todos os SFX") },
        { id: "transicoes", label: qsTr("Transições & Dinâmica") },
        { id: "impacto", label: qsTr("Impactos & Ênfase") },
        { id: "interface", label: qsTr("Pop, UI & Redes") }
    ]

    readonly property var allSfx: EditorState.builtinSfxList()

    readonly property var visibleSfx: {
        const q = searchFilter.trim().toLowerCase()
        const cat = activeCategory
        return allSfx.filter(function(item) {
            const matchesCat = (cat === "all" || item.category === cat)
            if (!matchesCat) return false
            if (q.length === 0) return true
            return item.label.toLowerCase().indexOf(q) >= 0 || item.id.toLowerCase().indexOf(q) >= 0
        })
    }

    MediaPlayer {
        id: sfxPlayer
        audioOutput: AudioOutput { id: sfxOutput }
        onPlaybackStateChanged: {
            if (playbackState === MediaPlayer.StoppedState) {
                root.previewingId = ""
            }
        }
    }

    function togglePreview(id) {
        if (root.previewingId === id && sfxPlayer.playbackState === MediaPlayer.PlayingState) {
            sfxPlayer.stop()
            root.previewingId = ""
            return
        }
        const path = EditorState.builtinSfxPath(id)
        if (path.length > 0) {
            root.previewingId = id
            sfxPlayer.stop()
            sfxPlayer.source = path
            sfxPlayer.play()
        }
    }

    Column {
        anchors.fill: parent
        spacing: 0

        // Mode Navigation Bar: SFX vs Filtros FX
        Rectangle {
            width: parent.width
            height: 44
            color: Theme.panelBackground
            border.width: 0

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: Theme.panelBorder
            }

            Row {
                anchors.centerIn: parent
                spacing: 8

                ThemedButton {
                    text: qsTr("Efeitos Sonoros (SFX)")
                    variant: root.currentTab === "sfx" ? "primary" : "ghost"
                    glyph: Theme.icons.audioLines
                    onClicked: {
                        if (sfxPlayer.playbackState === MediaPlayer.PlayingState)
                            sfxPlayer.stop()
                        root.currentTab = "sfx"
                    }
                }

                ThemedButton {
                    text: qsTr("Filtros de Áudio (FX)")
                    variant: root.currentTab === "fx" ? "primary" : "ghost"
                    glyph: Theme.icons.sliders
                    onClicked: {
                        if (sfxPlayer.playbackState === MediaPlayer.PlayingState)
                            sfxPlayer.stop()
                        root.currentTab = "fx"
                    }
                }
            }
        }

        // SFX View
        Item {
            width: parent.width
            height: parent.height - 44
            visible: root.currentTab === "sfx"

            Flickable {
                anchors.fill: parent
                contentWidth: width
                contentHeight: sfxContentCol.height + Theme.spacing2xl
                clip: true
                ScrollBar.vertical: AppScrollBar { }

                Column {
                    id: sfxContentCol
                    x: Theme.pagePadding
                    width: parent.width - Theme.pagePadding * 2
                    spacing: Theme.spacingMd
                    topPadding: Theme.spacingMd

                    // Subtitle & Explanation
                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: qsTr("Biblioteca embutida de sons virais (Whooshes, Pops, Booms, Cliques) para dar ritmo a Reels, Shorts e TikTok. Ouça o preview e adicione na agulha com 1 clique.")
                        color: Theme.mutedForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeXs
                    }

                    // Search box
                    ThemedTextField {
                        id: searchBox
                        width: parent.width
                        placeholderText: qsTr("Buscar efeito sonoro (ex: whoosh, pop, boom)...")
                        font.family: Theme.fontFamily
                        onTextChanged: root.searchFilter = text
                    }

                    // Category Filter Chips
                    Flickable {
                        width: parent.width
                        height: 32
                        contentWidth: catRow.width
                        flickableDirection: Flickable.HorizontalFlick
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds

                        Row {
                            id: catRow
                            spacing: Theme.spacingSm

                            Repeater {
                                model: root.allCategories
                                delegate: ThemedButton {
                                    required property var modelData
                                    text: modelData.label
                                    variant: root.activeCategory === modelData.id ? "secondary" : "ghost"
                                    onClicked: root.activeCategory = modelData.id
                                }
                            }
                        }
                    }

                    // List of SFX Cards
                    Column {
                        width: parent.width
                        spacing: Theme.spacingSm

                        Repeater {
                            model: root.visibleSfx
                            delegate: Rectangle {
                                id: sfxCard
                                required property var modelData
                                width: parent.width
                                height: 56
                                radius: Theme.radiusMd
                                color: cardHover.hovered ? Theme.panelAccent : Theme.panelBackground
                                border.width: root.previewingId === modelData.id ? 2 : 1
                                border.color: root.previewingId === modelData.id ? Theme.primary : Theme.panelBorder

                                Behavior on color {
                                    ColorAnimation { duration: Theme.durationFast; easing.type: Theme.easing }
                                }
                                Behavior on border.color {
                                    ColorAnimation { duration: Theme.durationFast; easing.type: Theme.easing }
                                }

                                HoverHandler { id: cardHover }

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 12
                                    spacing: 12

                                    // Play / Stop button
                                    IconButton {
                                        anchors.verticalCenter: parent.verticalCenter
                                        glyph: root.previewingId === modelData.id
                                               && sfxPlayer.playbackState === MediaPlayer.PlayingState
                                               ? Theme.icons.pause : Theme.icons.play
                                        variant: root.previewingId === modelData.id ? "primary" : "secondary"
                                        buttonSize: 36
                                        iconSize: 16
                                        tooltip: qsTr("Ouvir prévia")
                                        onClicked: root.togglePreview(modelData.id)
                                    }

                                    // Labels
                                    Column {
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: parent.width - 36 - 100 - 36
                                        spacing: 2

                                        Text {
                                            width: parent.width
                                            text: modelData.label
                                            color: Theme.foreground
                                            font.family: Theme.fontFamily
                                            font.pixelSize: Theme.fontSizeSm
                                            font.bold: true
                                            elide: Text.ElideRight
                                        }

                                        Row {
                                            spacing: 8
                                            Rectangle {
                                                width: durText.width + 8
                                                height: 16
                                                radius: 4
                                                color: Theme.panelAccent
                                                Text {
                                                    id: durText
                                                    anchors.centerIn: parent
                                                    text: modelData.durationFormatted
                                                    color: Theme.mutedForeground
                                                    font.family: Theme.fontFamily
                                                    font.pixelSize: 10
                                                }
                                            }

                                            Text {
                                                text: modelData.category === "transicoes" ? qsTr("Transição")
                                                    : (modelData.category === "impacto" ? qsTr("Impacto") : qsTr("Interface"))
                                                color: Theme.mutedForeground
                                                font.family: Theme.fontFamily
                                                font.pixelSize: 11
                                                anchors.verticalCenter: parent.verticalCenter
                                            }
                                        }
                                    }

                                    // Quick Add to Timeline Button
                                    ThemedButton {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: qsTr("+ Adicionar")
                                        variant: "ghost"
                                        glyph: Theme.icons.plus
                                        tooltip: qsTr("Adicionar efeito na agulha da timeline")
                                        onClicked: {
                                            EditorState.addSfxClip(modelData.id)
                                            Toasts.success(qsTr("SFX “%1” adicionado na timeline!").arg(modelData.label))
                                        }
                                    }
                                }
                            }
                        }

                        // Empty State if no matches
                        Item {
                            width: parent.width
                            height: 100
                            visible: root.visibleSfx.length === 0

                            Column {
                                anchors.centerIn: parent
                                spacing: 8

                                IconGlyph {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    glyph: Theme.icons.audioLines
                                    iconSize: 32
                                    iconColor: Theme.mutedForeground
                                }

                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: qsTr("Nenhum efeito sonoro encontrado.")
                                    color: Theme.mutedForeground
                                    font.family: Theme.fontFamily
                                    font.pixelSize: Theme.fontSizeSm
                                }
                            }
                        }
                    }
                }
            }
        }

        // FX View
        Item {
            width: parent.width
            height: parent.height - 44
            visible: root.currentTab === "fx"

            AudioEffectBrowser {
                anchors.fill: parent
            }
        }
    }
}
