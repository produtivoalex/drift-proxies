import QtQuick
import QtQuick.Controls.Basic
import QtMultimedia
import Drift
import ".."

// Text tab: click a style pack to drop a styled text clip on the timeline
// (placeholder copy + inline edit). Timed captions live under Subtitles.
// Styles the user saved from the properties Text tab head the list under "My styles".
Item {
    id: root

    // A clip landed on the timeline. The phone shell closes the sheet on this —
    // the thing you came for is behind it.
    signal added()

    readonly property var presets: EditorState.textPresets()

    // A QVariantList from an invokable is not reactive, so the section is refreshed by poking
    // this counter from the controller's signal.
    property int userPresetsTick: 0
    readonly property var userPresets: {
        void root.userPresetsTick
        return EditorState.userTextPresets()
    }

    readonly property var ttsVoices: EditorState.ttsAvailableVoices()
    property bool isTtsAudioPlaying: false

    MediaPlayer {
        id: ttsPreviewPlayer
        audioOutput: AudioOutput { id: ttsAudioOut }
        onPlaybackStateChanged: {
            if (playbackState === MediaPlayer.StoppedState)
                root.isTtsAudioPlaying = false
        }
    }

    function toggleTtsPreview(text, voiceId, rate, pitch) {
        if (root.isTtsAudioPlaying) {
            ttsPreviewPlayer.stop()
            root.isTtsAudioPlaying = false
            return
        }
        if (text.trim().length === 0) return
        const p = pitch !== undefined ? pitch : 1.0
        const path = EditorState.ttsPreviewAudio(text.trim(), voiceId, rate, p)
        if (path.length > 0) {
            root.isTtsAudioPlaying = true
            ttsPreviewPlayer.stop()
            ttsPreviewPlayer.source = path
            ttsPreviewPlayer.play()
        }
    }

    Connections {
        target: EditorState
        function onUserTextPresetsChanged() { root.userPresetsTick++ }
    }

    readonly property string styleFileFilter: qsTr("Drift text style (*.drifttextstyle)")

    function importStyle() {
        const url = FileDialogs.openFile(qsTr("Import text style"), [root.styleFileFilter])
        if (url.toString().length > 0)
            EditorState.importUserTextPreset(url)
    }

    function exportStyle(preset) {
        const url = FileDialogs.saveFile(qsTr("Export text style"), [root.styleFileFilter],
                                         preset.label, "drifttextstyle")
        if (url.toString().length > 0)
            EditorState.exportUserTextPreset(preset.id, url)
    }

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: textColumn.height + Theme.spacing3xl
        clip: true
        ScrollBar.vertical: AppScrollBar { }

        Column {
            id: textColumn
            x: Theme.pagePadding
            width: parent.width - Theme.pagePadding * 2
            spacing: Theme.spacingMd
            topPadding: Theme.pagePadding

            // --- CapCut-Style Text-to-Speech (TTS) Card with Top 5 Brazilian Voices ---
            Rectangle {
                id: ttsCard
                width: parent.width
                implicitHeight: ttsCardCol.implicitHeight + Theme.spacingLg * 2
                radius: Theme.radiusMd
                color: Theme.darkMode ? "#101726" : "#eff6ff"
                border.width: Theme.borderWidth
                border.color: Theme.darkMode ? "#1d4ed8" : "#93c5fd"

                property int selectedVoiceIdx: 0
                readonly property var currentVoiceObj: (root.ttsVoices && root.ttsVoices.length > selectedVoiceIdx) ? root.ttsVoices[selectedVoiceIdx] : ({})

                Column {
                    id: ttsCardCol
                    x: Theme.spacingLg
                    y: Theme.spacingLg
                    width: parent.width - Theme.spacingLg * 2
                    spacing: Theme.spacingSm

                    Row {
                        width: parent.width
                        spacing: Theme.spacingSm

                        IconGlyph {
                            glyph: Theme.icons.audioLines
                            iconSize: 16
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: qsTr("Narração de Texto em Voz (Top 5 Vozes do Brasil)")
                            color: Theme.panelForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSm
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Item {
                            width: 1
                            height: 1
                        }

                        Rectangle {
                            height: 20
                            width: ttsBadgeText.implicitWidth + 12
                            radius: 10
                            color: Theme.darkMode ? "#1e293b" : "#dbeafe"
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                id: ttsBadgeText
                                text: qsTr("🇧🇷 100% Dinâmicas & Rebeldes")
                                color: Theme.darkMode ? "#60a5fa" : "#1d4ed8"
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                font.weight: Font.Medium
                                anchors.centerIn: parent
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: qsTr("Vozes ultrarrealistas e humanizadas para Reels, TikTok, Shorts e canais Dark. Zero robóticas, com respiração natural e sincronização automática de legendas animadas.")
                        color: Theme.mutedForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeXs
                    }

                    ThemedTextArea {
                        id: ttsInputArea
                        width: parent.width
                        implicitHeight: 74
                        placeholderText: qsTr("Digite ou cole aqui o roteiro para ser narrado pelo Drift...")
                    }

                    // --- Grid Seletor das 5 Melhores Vozes do Brasil ---
                    Text {
                        text: qsTr("Escolha a Voz Ideal para seu Vídeo:")
                        color: Theme.panelForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                        topPadding: 2
                    }

                    Flow {
                        width: parent.width
                        spacing: 6

                        Repeater {
                            model: Math.min(5, (root.ttsVoices ? root.ttsVoices.length : 0))

                            delegate: Rectangle {
                                id: voiceChip
                                readonly property var vObj: root.ttsVoices[index] || ({})
                                readonly property bool isSelected: ttsCard.selectedVoiceIdx === index

                                width: (parent.width - 6) / 2 - 1
                                height: 42
                                radius: Theme.radiusSm
                                color: isSelected 
                                    ? (Theme.darkMode ? "#1e3a8a" : "#bfdbfe")
                                    : (Theme.darkMode ? "#1e293b" : "#f1f5f9")
                                border.width: isSelected ? 2 : 1
                                border.color: isSelected ? "#3b82f6" : (Theme.darkMode ? "#334155" : "#cbd5e1")

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    spacing: 6

                                    Column {
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: parent.width - 24
                                        spacing: 1

                                        Text {
                                            text: vObj.vibeTag || vObj.name || ""
                                            color: isSelected ? (Theme.darkMode ? "#ffffff" : "#1e3a8a") : Theme.panelForeground
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 11
                                            font.weight: isSelected ? Font.Bold : Font.Medium
                                            elide: Text.ElideRight
                                            width: parent.width
                                        }

                                        Text {
                                            text: (vObj.gender || "") + " • " + (vObj.lang || "pt-BR")
                                            color: Theme.mutedForeground
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 9
                                            elide: Text.ElideRight
                                            width: parent.width
                                        }
                                    }

                                    Rectangle {
                                        width: 14
                                        height: 14
                                        radius: 7
                                        anchors.verticalCenter: parent.verticalCenter
                                        color: isSelected ? "#3b82f6" : "transparent"
                                        border.width: 1
                                        border.color: isSelected ? "#60a5fa" : Theme.mutedForeground

                                        Rectangle {
                                            width: 6
                                            height: 6
                                            radius: 3
                                            color: "#ffffff"
                                            anchors.centerIn: parent
                                            visible: isSelected
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        ttsCard.selectedVoiceIdx = index
                                        if (vObj.defaultRate) {
                                            ttsRateSlider.value = vObj.defaultRate
                                        }
                                        if (vObj.defaultPitch) {
                                            ttsPitchSlider.value = vObj.defaultPitch
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Card de Descrição da Voz Selecionada
                    Rectangle {
                        width: parent.width
                        implicitHeight: voiceDescText.implicitHeight + 12
                        radius: Theme.radiusSm
                        color: Theme.darkMode ? "#0f172a" : "#f8fafc"
                        border.width: 1
                        border.color: Theme.darkMode ? "#334155" : "#e2e8f0"

                        Row {
                            anchors.fill: parent
                            anchors.margins: 6
                            spacing: 6

                            Text {
                                id: voiceDescText
                                width: parent.width
                                wrapMode: Text.WordWrap
                                text: {
                                    const v = ttsCard.currentVoiceObj
                                    if (v && v.description) {
                                        return "💡 " + v.description
                                    }
                                    return qsTr("💡 Voz neural em português brasileiro otimizada para narrações dinâmicas.")
                                }
                                color: Theme.mutedForeground
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                            }
                        }
                    }

                    // Se houver mais vozes (vozes locais do sistema/piper)
                    Row {
                        width: parent.width
                        spacing: Theme.spacingSm
                        visible: root.ttsVoices && root.ttsVoices.length > 5

                        Text {
                            text: qsTr("Outras vozes:")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ThemedComboBox {
                            id: ttsExtraVoiceCombo
                            width: parent.width - 80
                            model: root.ttsVoices ? root.ttsVoices.slice(5) : []
                            textRole: "name"
                            onActivated: {
                                if (currentIndex >= 0) {
                                    ttsCard.selectedVoiceIdx = currentIndex + 5
                                }
                            }
                        }
                    }

                    // --- Presets Rápidos de Velocidade ---
                    Row {
                        width: parent.width
                        spacing: Theme.spacingSm
                        topPadding: 2

                        Text {
                            text: qsTr("Ritmo Rápido:")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ThemedButton {
                            text: qsTr("1.0x Normal")
                            variant: "secondary"
                            onClicked: ttsRateSlider.value = 1.0
                        }

                        ThemedButton {
                            text: qsTr("🔥 1.1x Viral (Reels/TikTok)")
                            variant: "secondary"
                            onClicked: ttsRateSlider.value = 1.1
                        }

                        ThemedButton {
                            text: qsTr("⚡ 1.25x Ágil")
                            variant: "secondary"
                            onClicked: ttsRateSlider.value = 1.25
                        }
                    }

                    // --- Sliders de Velocidade e Tom ---
                    Row {
                        width: parent.width
                        spacing: Theme.spacingMd

                        Column {
                            width: (parent.width - Theme.spacingMd) * 0.5
                            spacing: 2

                            Text {
                                text: qsTr("Velocidade: %1x").arg(ttsRateSlider.value.toFixed(2))
                                color: Theme.mutedForeground
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                            }

                            ThemedSlider {
                                id: ttsRateSlider
                                width: parent.width
                                from: 0.5
                                to: 2.0
                                stepSize: 0.05
                                value: 1.1
                            }
                        }

                        Column {
                            width: (parent.width - Theme.spacingMd) * 0.5
                            spacing: 2

                            Text {
                                text: {
                                    const diff = Math.round((ttsPitchSlider.value - 1.0) * 100)
                                    return qsTr("Tom (Pitch): %1%").arg(diff >= 0 ? "+" + diff : diff)
                                }
                                color: Theme.mutedForeground
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                            }

                            ThemedSlider {
                                id: ttsPitchSlider
                                width: parent.width
                                from: 0.8
                                to: 1.2
                                stepSize: 0.02
                                value: 1.0
                            }
                        }
                    }

                    ThemedCheckBox {
                        id: ttsSyncSubtitles
                        text: qsTr("Sincronizar e gerar legendas animadas na timeline")
                        checked: true
                    }

                    Row {
                        spacing: Theme.spacingSm
                        topPadding: 4

                        ThemedButton {
                            text: root.isTtsAudioPlaying ? qsTr("Parar Prévia") : qsTr("Ouvir Prévia")
                            variant: "secondary"
                            glyph: root.isTtsAudioPlaying ? Theme.icons.pause : Theme.icons.play
                            enabled: ttsInputArea.text.trim().length > 0
                            onClicked: {
                                const voiceObj = ttsCard.currentVoiceObj || {}
                                const voiceId = voiceObj.id || ""
                                root.toggleTtsPreview(ttsInputArea.text, voiceId, ttsRateSlider.value, ttsPitchSlider.value)
                            }
                        }

                        ThemedButton {
                            text: qsTr("Inserir Narração na Timeline")
                            variant: "primary"
                            glyph: Theme.icons.plus
                            enabled: ttsInputArea.text.trim().length > 0
                            onClicked: {
                                const voiceObj = ttsCard.currentVoiceObj || {}
                                const voiceId = voiceObj.id || ""
                                const ok = EditorState.ttsCreateClip(
                                    ttsInputArea.text.trim(),
                                    voiceId,
                                    ttsRateSlider.value,
                                    ttsPitchSlider.value,
                                    ttsSyncSubtitles.checked
                                )
                                if (ok) {
                                    root.added()
                                    Toasts.success(qsTr("Narração em áudio e legendas sincronizadas inseridas na timeline!"))
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: Theme.borderWidth
                color: Theme.panelBorder
            }

            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                text: qsTr("Click a style to add text at the playhead. Double-click it on the preview to edit.")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            Item {
                width: parent.width
                height: Math.max(myStylesLabel.implicitHeight, importButton.height)

                Text {
                    id: myStylesLabel
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("My styles")
                    color: Theme.panelForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeXs
                    font.weight: Font.Medium
                }

                IconButton {
                    id: importButton
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    glyph: Theme.icons.folderInput
                    variant: "ghost"
                    tooltip: qsTr("Import a text style…")
                    onClicked: root.importStyle()
                }
            }

            Text {
                width: parent.width
                visible: root.userPresets.length === 0
                wrapMode: Text.WordWrap
                text: qsTr("Style some text, then use “Save style…” in the properties Text tab to keep it here.")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            Grid {
                id: userGrid
                width: parent.width
                visible: root.userPresets.length > 0
                columns: Math.max(1, Math.floor((width + Theme.assetCardGap)
                                                / (Theme.assetCardWidth + Theme.assetCardGap)))
                columnSpacing: Theme.assetCardGap
                rowSpacing: Theme.assetCardGap

                Repeater {
                    model: root.userPresets
                    delegate: Column {
                        id: userCard
                        required property var modelData
                        width: Theme.assetCardWidth
                        spacing: Theme.spacingSm

                        scale: userPress.pressed ? 0.97 : (userHover.hovered ? 1.02 : 1.0)
                        Behavior on scale {
                            NumberAnimation { duration: Theme.durationFast; easing.type: Theme.easing }
                        }

                        TextStylePackThumb {
                            width: parent.width
                            height: Math.round(width * 0.55)
                            presetId: userCard.modelData.id
                            hovered: userHover.hovered

                            HoverHandler {
                                id: userHover
                            }

                            TapHandler {
                                id: userPress
                                gesturePolicy: TapHandler.ReleaseWithinBounds
                                onTapped: {
                                    EditorState.addTextClip("", -1, userCard.modelData.id)
                                    root.added()
                                }
                            }

                            TapHandler {
                                acceptedButtons: Qt.RightButton
                                onTapped: cardMenu.popup()
                            }

                            IconButton {
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.margins: 2
                                visible: userHover.hovered || cardMenu.visible
                                glyph: Theme.icons.ellipsis
                                variant: "ghost"
                                buttonSize: 20
                                iconSize: 12
                                tooltip: qsTr("Style options")
                                onClicked: cardMenu.popup()
                            }

                            ThemedContextMenu {
                                id: cardMenu

                                ThemedMenuItem {
                                    text: qsTr("Rename…")
                                    icon.name: Theme.icons.pencil
                                    onTriggered: renameDialog.openFor(userCard.modelData)
                                }
                                ThemedMenuItem {
                                    text: qsTr("Export…")
                                    icon.name: Theme.icons.folderOutput
                                    onTriggered: root.exportStyle(userCard.modelData)
                                }
                                ThemedMenuSeparator { }
                                ThemedMenuItem {
                                    text: qsTr("Delete")
                                    icon.name: Theme.icons.trash
                                    onTriggered: deleteDialog.openFor(userCard.modelData)
                                }
                            }
                        }

                        Text {
                            width: parent.width
                            text: userCard.modelData.label
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignHCenter
                            color: userHover.hovered ? Theme.panelForeground : Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs

                            Behavior on color {
                                ColorAnimation { duration: Theme.durationFast; easing.type: Theme.easing }
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: Theme.borderWidth
                color: Theme.panelBorder
            }

            Text {
                width: parent.width
                text: qsTr("Built-in")
                color: Theme.panelForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
                font.weight: Font.Medium
            }

            Grid {
                id: packGrid
                width: parent.width
                columns: Math.max(1, Math.floor((width + Theme.assetCardGap)
                                                / (Theme.assetCardWidth + Theme.assetCardGap)))
                columnSpacing: Theme.assetCardGap
                rowSpacing: Theme.assetCardGap

                Repeater {
                    model: root.presets
                    delegate: Column {
                        id: packCard
                        required property var modelData
                        width: Theme.assetCardWidth
                        spacing: Theme.spacingSm

                        scale: packPress.pressed ? 0.97 : (packHover.hovered ? 1.02 : 1.0)
                        Behavior on scale {
                            NumberAnimation { duration: Theme.durationFast; easing.type: Theme.easing }
                        }

                        TextStylePackThumb {
                            width: parent.width
                            height: Math.round(width * 0.55)
                            presetId: packCard.modelData.id
                            hovered: packHover.hovered

                            HoverHandler {
                                id: packHover
                            }

                            TapHandler {
                                id: packPress
                                gesturePolicy: TapHandler.ReleaseWithinBounds
                                onTapped: {
                                    EditorState.addTextClip("", -1, packCard.modelData.id)
                                    root.added()
                                }
                            }
                        }

                        Text {
                            width: parent.width
                            text: packCard.modelData.label
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignHCenter
                            color: packHover.hovered ? Theme.panelForeground : Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs

                            Behavior on color {
                                ColorAnimation { duration: Theme.durationFast; easing.type: Theme.easing }
                            }
                        }
                    }
                }
            }
        }
    }

    NameDialog {
        id: renameDialog

        property string presetId: ""

        function openFor(preset) {
            presetId = preset.id
            openWith(qsTr("Rename text style"), preset.label)
        }

        onSubmitted: name => EditorState.renameUserTextPreset(renameDialog.presetId, name)
    }

    ThemedDialog {
        id: deleteDialog

        property string presetId: ""
        property string presetLabel: ""

        title: qsTr("Delete text style")
        acceptText: qsTr("Delete")
        acceptVariant: "destructive"
        preferredWidth: Theme.dialogWidthSm
        acceptOnReturn: false

        function openFor(preset) {
            presetId = preset.id
            presetLabel = preset.label
            open()
        }

        onAccepted: EditorState.deleteUserTextPreset(deleteDialog.presetId)

        contentItem: Text {
            width: parent ? parent.width : 320
            wrapMode: Text.WordWrap
            text: qsTr("Remove “%1” from your saved styles? Clips already using it keep their look.")
                      .arg(deleteDialog.presetLabel)
            color: Theme.panelForeground
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeSm
        }
    }
}
