import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window
import Drift
import ".."

Item {
    id: root

    property int clipDataRevision: 0
    readonly property var clipData: {
        void clipDataRevision
        return EditorState.selectedClipData
    }
    readonly property bool hasSelection: !!clipData && Object.keys(clipData).length > 0
    readonly property string clipKind: hasSelection ? (clipData.kind || "") : ""
    readonly property var propVolume: { "key": "volume", "label": qsTr("Volume"), "def": 1.0, "decimals": 2 }

    // "Recommended" packs by display width like openai-whisper does; the numbered entries cap
    // words per caption on top of that.
    readonly property var captionLengthOptions: {
        const options = [{ label: qsTr("Recommended caption length"), words: 0 }]
        options.push({ label: qsTr("1 word per caption"), words: 1 })
        for (let n = 2; n <= 8; ++n)
            options.push({ label: qsTr("%1 words per caption").arg(n), words: n })
        return options
    }

    readonly property var captionStyleOptions: [
        { label: qsTr("TikTok Amarelo Viral (Recomendado)"), id: "tiktok-viral-yellow" },
        { label: qsTr("TikTok Verde Neon"), id: "tiktok-neon-green" },
        { label: qsTr("TikTok Ciano Glow"), id: "tiktok-cyan-glow" },
        { label: qsTr("Hormozi / Beast Style"), id: "hormozi-beast" },
        { label: qsTr("Reels Pill Highlight"), id: "reels-pill-box" },
        { label: qsTr("Shorts 1 Palavra"), id: "shorts-single-word" },
        { label: qsTr("Padrão Clássico"), id: "subtitle" }
    ]

    height: audioTabColumn.height
    implicitHeight: audioTabColumn.height

    function refreshFields() {}

    Connections {
        target: EditorState
        function onSelectionChanged() { root.clipDataRevision++ }
        function onSelectedClipDataChanged() { root.clipDataRevision++ }
        function onTracksChanged() { root.clipDataRevision++ }
    }

    Column {
        id: audioTabColumn
        width: root.width
        spacing: Theme.spacingXl

        EmptyState {
            visible: root.clipKind !== "audio" && root.clipKind !== "video"
            width: parent.width
            compact: true
            glyph: Theme.icons.volumeOff
            title: qsTr("No audio")
            hint: qsTr("This clip has no audio track.")
        }

        PropertyKeyframeRow {
            width: root.width
            visible: root.clipKind === "audio" || root.clipKind === "video"
            propDef: root.propVolume
            keyframeList: (root.clipData.keyframes && root.clipData.keyframes.volume && root.clipData.keyframes.volume.points) || []
            useSlider: true
            sliderFrom: 0
            sliderTo: 2
            percent: true
            decibels: true
        }

        // ----- Pan (stereo balance) ----------------------------------------
        Column {
            id: panSection
            width: parent.width
            spacing: Theme.spacingSm
            visible: root.clipKind === "audio" || root.clipKind === "video"

            readonly property real panValue: {
                void root.clipDataRevision
                return (root.clipData && root.clipData.pan !== undefined)
                       ? root.clipData.pan : 0.0
            }

            Row {
                width: parent.width

                Text {
                    width: parent.width / 2
                    text: qsTr("Pan")
                    color: Theme.mutedForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeXs
                }

                Text {
                    width: parent.width / 2
                    horizontalAlignment: Text.AlignRight
                    // "L 50" / "C" / "R 50" reads faster than a signed fraction, and the
                    // sign convention for pan is not something a user should have to recall.
                    text: {
                        const v = panSection.panValue
                        if (Math.abs(v) < 0.005)
                            return qsTr("C")
                        return (v < 0 ? qsTr("L %1") : qsTr("R %1"))
                                   .arg(Math.round(Math.abs(v) * 100))
                    }
                    color: Theme.panelForeground
                    font.family: Theme.monoFontFamily
                    font.pixelSize: Theme.fontSizeSm
                }
            }

            ThemedSlider {
                id: panSlider
                label: qsTr("Pan")
                width: parent.width
                from: -1
                to: 1
                Binding on value {
                    when: !panSlider.pressed
                    value: panSection.panValue
                }
                onMoved: EditorState.previewSetClipPan(
                             EditorState.selectedTrack, EditorState.selectedClip, value)
                onPressedChanged: {
                    if (pressed)
                        EditorState.beginPreviewDrag(qsTr("Pan changed"))
                    else
                        EditorState.commitPreviewDrag()
                }
            }

            ThemedButton {
                text: qsTr("Centre")
                enabled: Math.abs(panSection.panValue) >= 0.005
                onClicked: EditorState.setClipPan(
                               EditorState.selectedTrack, EditorState.selectedClip, 0)
            }
        }

        // ----- Audio Track Selection (Multi-Track) -------------------------
        Column {
            id: audioTrackSection
            width: parent.width
            spacing: Theme.spacingSm
            visible: (root.clipKind === "audio" || root.clipKind === "video") && audioTrackModel.length > 1

            property var audioTrackModel: {
                void root.clipDataRevision
                if (EditorState.selectedTrack < 0 || EditorState.selectedClip < 0)
                    return []
                return EditorState.clipAudioStreams(EditorState.selectedTrack, EditorState.selectedClip)
            }

            Text {
                width: parent.width
                text: qsTr("Audio track")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            ThemedComboBox {
                id: audioStreamSelector
                width: parent.width
                textRole: "label"
                valueRole: "index"
                model: audioTrackSection.audioTrackModel
                currentIndex: {
                    const currentIdx = (root.clipData && root.clipData.audioStreamIndex !== undefined)
                                       ? root.clipData.audioStreamIndex : 0
                    for (let i = 0; i < audioTrackSection.audioTrackModel.length; ++i) {
                        if (audioTrackSection.audioTrackModel[i].index === currentIdx)
                            return i
                    }
                    return 0
                }
                onActivated: {
                    EditorState.setClipAudioStreamIndex(
                        EditorState.selectedTrack, EditorState.selectedClip, currentValue)
                }
            }

            ThemedButton {
                visible: root.clipKind === "video" && EditorState.separateAudioAvailable
                width: parent.width
                text: qsTr("Extract all audio tracks")
                onClicked: {
                    EditorState.separateAllAudioTracks(EditorState.selectedTrack, EditorState.selectedClip)
                }
            }
        }

        Rectangle {
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            height: 1
            color: Theme.panelBorder
            opacity: 0.5
        }

        // ----- Voz de Estúdio / Enhance Voice (1-Click Pro Audio) ------------
        Rectangle {
            id: studioVoiceCard
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            implicitHeight: studioVoiceCol.implicitHeight + 20
            radius: Theme.radiusMd
            color: studioVoiceSwitch.checked
                   ? (Theme.darkMode ? "#141e2e" : "#eff6ff")
                   : Theme.panelAccent
            border.width: 1
            border.color: studioVoiceSwitch.checked
                          ? (Theme.darkMode ? "#2563eb" : "#3b82f6")
                          : Theme.panelBorder

            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
            Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }

            Column {
                id: studioVoiceCol
                x: 10
                y: 10
                width: parent.width - 20
                spacing: Theme.spacingSm

                Row {
                    width: parent.width
                    spacing: 8

                    Text {
                        text: "🎙️"
                        font.pixelSize: Theme.fontSizeBase
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        width: parent.width - 32 - studioVoiceSwitch.width
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: qsTr("Voz de Estúdio (Enhance Voice)")
                            color: Theme.panelForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSm
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: qsTr("Clareza, calor de microfone e nivelamento de broadcast")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    ThemedSwitch {
                        id: studioVoiceSwitch
                        anchors.verticalCenter: parent.verticalCenter
                        checked: {
                            void root.clipDataRevision
                            return (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0)
                                ? EditorState.isStudioVoiceEnabled(EditorState.selectedTrack, EditorState.selectedClip)
                                : false
                        }
                        onToggled: {
                            if (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0) {
                                EditorState.setStudioVoiceEnabled(
                                    EditorState.selectedTrack, EditorState.selectedClip, checked)
                                root.clipDataRevision++
                            }
                        }
                    }
                }

                // Presets row when active
                Column {
                    width: parent.width
                    visible: studioVoiceSwitch.checked
                    spacing: 6

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: Theme.panelBorder
                        opacity: 0.5
                    }

                    Row {
                        width: parent.width
                        spacing: 4

                        Text {
                            text: qsTr("Perfis:")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ThemedButton {
                            text: qsTr("Podcast Quente")
                            variant: "ghost"
                            tooltip: qsTr("Graves aveludados e calor estilo Shure SM7B")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.applyStudioVoicePreset(t, c, 0.8, 0.6, 0.7)
                                Toasts.info(qsTr("Perfil Podcast Quente aplicado"))
                            }
                        }

                        ThemedButton {
                            text: qsTr("Cristalina")
                            variant: "ghost"
                            tooltip: qsTr("Máxima presença e ar para fones de ouvido")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.applyStudioVoicePreset(t, c, 0.4, 0.9, 0.6)
                                Toasts.info(qsTr("Perfil Cristalino aplicado"))
                            }
                        }

                        ThemedButton {
                            text: qsTr("Rádio FM")
                            variant: "ghost"
                            tooltip: qsTr("Compressão e firmeza de locutor de rádio")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.applyStudioVoicePreset(t, c, 0.7, 0.8, 0.9)
                                Toasts.info(qsTr("Perfil Rádio FM aplicado"))
                            }
                        }
                    }
                }
            }
        }

        // ----- Auto-Ducking Inteligente (Abaixar Música na Fala) ------------
        Rectangle {
            id: autoDuckingCard
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            implicitHeight: duckingCol.implicitHeight + 20
            radius: Theme.radiusMd
            color: hasDucking
                   ? (Theme.darkMode ? "#141e2e" : "#eff6ff")
                   : Theme.panelAccent
            border.width: 1
            border.color: hasDucking
                          ? (Theme.darkMode ? "#2563eb" : "#3b82f6")
                          : Theme.panelBorder

            readonly property bool hasDucking: {
                void root.clipDataRevision
                return (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0)
                    ? EditorState.hasAutoDucking(EditorState.selectedTrack, EditorState.selectedClip)
                    : false
            }

            property real duckingDb: -14.0
            property real fadeSpeedSec: 0.4
            property real holdTimeSec: 1.0

            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
            Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }

            Column {
                id: duckingCol
                x: 10
                y: 10
                width: parent.width - 20
                spacing: Theme.spacingSm

                Row {
                    width: parent.width
                    spacing: 8

                    Text {
                        text: "📉"
                        font.pixelSize: Theme.fontSizeBase
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        width: parent.width - 32 - (statusBadge.visible ? statusBadge.width : 0)
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: qsTr("Auto-Ducking Inteligente")
                            color: Theme.panelForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSm
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: qsTr("Abaixa a música automaticamente ao detectar fala")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    Rectangle {
                        id: statusBadge
                        anchors.verticalCenter: parent.verticalCenter
                        visible: autoDuckingCard.hasDucking
                        width: badgeText.implicitWidth + 12
                        height: 22
                        radius: 11
                        color: Theme.darkMode ? "#1e3a8a" : "#dbeafe"

                        Text {
                            id: badgeText
                            anchors.centerIn: parent
                            text: qsTr("Ativo")
                            color: Theme.darkMode ? "#93c5fd" : "#1d4ed8"
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            font.weight: Font.Bold
                        }
                    }
                }

                // Divider
                Rectangle {
                    width: parent.width
                    height: 1
                    color: Theme.panelBorder
                    opacity: 0.5
                }

                // Presets de atenuação
                Column {
                    width: parent.width
                    spacing: 6

                    Row {
                        width: parent.width
                        spacing: 4

                        Text {
                            text: qsTr("Redução de Volume:")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: qsTr("%1 dB (%2%)").arg(Math.round(autoDuckingCard.duckingDb))
                                                     .arg(Math.round(Math.pow(10, autoDuckingCard.duckingDb / 20) * 100))
                            color: Theme.panelForeground
                            font.family: Theme.monoFontFamily
                            font.pixelSize: Theme.fontSizeXs
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: 6

                        ThemedButton {
                            text: qsTr("Suave (-9 dB)")
                            variant: Math.abs(autoDuckingCard.duckingDb - (-9)) < 0.5 ? "solid" : "ghost"
                            tooltip: qsTr("Música ainda bem audível ao fundo")
                            onClicked: autoDuckingCard.duckingDb = -9.0
                        }

                        ThemedButton {
                            text: qsTr("Padrão (-14 dB)")
                            variant: Math.abs(autoDuckingCard.duckingDb - (-14)) < 0.5 ? "solid" : "ghost"
                            tooltip: qsTr("Equilíbrio perfeito de clareza vocal e ambiência")
                            onClicked: autoDuckingCard.duckingDb = -14.0
                        }

                        ThemedButton {
                            text: qsTr("Forte (-20 dB)")
                            variant: Math.abs(autoDuckingCard.duckingDb - (-20)) < 0.5 ? "solid" : "ghost"
                            tooltip: qsTr("Música bem baixa para destaque total da voz")
                            onClicked: autoDuckingCard.duckingDb = -20.0
                        }
                    }

                    ThemedSlider {
                        width: parent.width
                        from: -30
                        to: -3
                        stepSize: 1
                        value: autoDuckingCard.duckingDb
                        onMoved: autoDuckingCard.duckingDb = value
                    }
                }

                // Velocidade de fade
                Row {
                    width: parent.width
                    spacing: 6

                    Text {
                        text: qsTr("Fade:")
                        color: Theme.mutedForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeXs
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    ThemedButton {
                        text: qsTr("Rápido (0.2s)")
                        variant: Math.abs(autoDuckingCard.fadeSpeedSec - 0.2) < 0.05 ? "solid" : "ghost"
                        onClicked: autoDuckingCard.fadeSpeedSec = 0.2
                    }

                    ThemedButton {
                        text: qsTr("Suave (0.4s)")
                        variant: Math.abs(autoDuckingCard.fadeSpeedSec - 0.4) < 0.05 ? "solid" : "ghost"
                        onClicked: autoDuckingCard.fadeSpeedSec = 0.4
                    }

                    ThemedButton {
                        text: qsTr("Longo (0.8s)")
                        variant: Math.abs(autoDuckingCard.fadeSpeedSec - 0.8) < 0.05 ? "solid" : "ghost"
                        onClicked: autoDuckingCard.fadeSpeedSec = 0.8
                    }
                }

                // Botões de ação
                Row {
                    width: parent.width
                    spacing: 8

                    ThemedButton {
                        text: qsTr("⚡ Aplicar Auto-Ducking")
                        variant: "solid"
                        tooltip: qsTr("Gera curvas suaves na música sincronizadas com as falas e legendas")
                        onClicked: {
                            const t = EditorState.selectedTrack
                            const c = EditorState.selectedClip
                            if (t >= 0 && c >= 0) {
                                EditorState.applyAutoDucking(t, c, autoDuckingCard.duckingDb,
                                                             autoDuckingCard.fadeSpeedSec,
                                                             autoDuckingCard.holdTimeSec)
                                root.clipDataRevision++
                            }
                        }
                    }

                    ThemedButton {
                        text: qsTr("↺ Redefinir Volume")
                        variant: "ghost"
                        enabled: autoDuckingCard.hasDucking
                        tooltip: qsTr("Remove as curvas de ducking e restaura o volume contínuo de 100%")
                        onClicked: {
                            const t = EditorState.selectedTrack
                            const c = EditorState.selectedClip
                            if (t >= 0 && c >= 0) {
                                EditorState.clearAutoDucking(t, c)
                                root.clipDataRevision++
                            }
                        }
                    }
                }
            }
        }

        // ----- Áudio 8D / Binaural 360° ------------------------------------
        Rectangle {
            id: eightDCard
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            implicitHeight: eightDCol.implicitHeight + 20
            radius: Theme.radiusMd
            color: eightDSwitch.checked
                   ? (Theme.darkMode ? "#141e2e" : "#eff6ff")
                   : Theme.panelAccent
            border.width: 1
            border.color: eightDSwitch.checked
                          ? (Theme.darkMode ? "#2563eb" : "#3b82f6")
                          : Theme.panelBorder

            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
            Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }

            Column {
                id: eightDCol
                x: 10
                y: 10
                width: parent.width - 20
                spacing: Theme.spacingSm

                Row {
                    width: parent.width
                    spacing: 8

                    Text {
                        text: "🎧"
                        font.pixelSize: Theme.fontSizeBase
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        width: parent.width - 32 - eightDSwitch.width
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: qsTr("Áudio 8D (Binaural 360°)")
                            color: Theme.panelForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSm
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: qsTr("Som girando ao redor da cabeça (ouvir de fones)")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    ThemedSwitch {
                        id: eightDSwitch
                        anchors.verticalCenter: parent.verticalCenter
                        checked: {
                            void root.clipDataRevision
                            return (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0)
                                ? EditorState.isEightDEnabled(EditorState.selectedTrack, EditorState.selectedClip)
                                : false
                        }
                        onToggled: {
                            if (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0) {
                                EditorState.setEightDEnabled(
                                    EditorState.selectedTrack, EditorState.selectedClip, checked)
                                root.clipDataRevision++
                            }
                        }
                    }
                }

                // Controls when active
                Column {
                    width: parent.width
                    visible: eightDSwitch.checked
                    spacing: 6

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: Theme.panelBorder
                        opacity: 0.5
                    }

                    Row {
                        width: parent.width
                        spacing: 6

                        Text {
                            text: qsTr("Velocidade:")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ThemedButton {
                            text: qsTr("Lenta (8s)")
                            variant: "ghost"
                            tooltip: qsTr("Giro suave e imersivo")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.setEightDSpeed(t, c, 0.125)
                                Toasts.info(qsTr("Velocidade 8D ajustada para 8s"))
                            }
                        }

                        ThemedButton {
                            text: qsTr("Média (5s)")
                            variant: "ghost"
                            tooltip: qsTr("Giro clássico de música 8D")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.setEightDSpeed(t, c, 0.20)
                                Toasts.info(qsTr("Velocidade 8D ajustada para 5s"))
                            }
                        }

                        ThemedButton {
                            text: qsTr("Rápida (3s)")
                            variant: "ghost"
                            tooltip: qsTr("Giro dinâmico e acelerado")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.setEightDSpeed(t, c, 0.33)
                                Toasts.info(qsTr("Velocidade 8D ajustada para 3s"))
                            }
                        }
                    }
                }
            }
        }

        // ----- Festa ao Lado (Vizinho / Parede) -----------------------------
        Rectangle {
            id: partyCard
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            implicitHeight: partyCol.implicitHeight + 20
            radius: Theme.radiusMd
            color: partySwitch.checked
                   ? (Theme.darkMode ? "#141e2e" : "#eff6ff")
                   : Theme.panelAccent
            border.width: 1
            border.color: partySwitch.checked
                          ? (Theme.darkMode ? "#2563eb" : "#3b82f6")
                          : Theme.panelBorder

            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
            Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }

            Column {
                id: partyCol
                x: 10
                y: 10
                width: parent.width - 20
                spacing: Theme.spacingSm

                Row {
                    width: parent.width
                    spacing: 8

                    Text {
                        text: "🏠"
                        font.pixelSize: Theme.fontSizeBase
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        width: parent.width - 32 - partySwitch.width
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: qsTr("Festa ao Lado (Vizinho / Parede)")
                            color: Theme.panelForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSm
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: qsTr("Música abafada através da parede com eco de cômodo")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    ThemedSwitch {
                        id: partySwitch
                        anchors.verticalCenter: parent.verticalCenter
                        checked: {
                            void root.clipDataRevision
                            return (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0)
                                ? EditorState.isPartyNextDoorEnabled(EditorState.selectedTrack, EditorState.selectedClip)
                                : false
                        }
                        onToggled: {
                            if (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0) {
                                EditorState.setPartyNextDoorEnabled(
                                    EditorState.selectedTrack, EditorState.selectedClip, checked)
                                root.clipDataRevision++
                            }
                        }
                    }
                }

                // Presets when active
                Column {
                    width: parent.width
                    visible: partySwitch.checked
                    spacing: 6

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: Theme.panelBorder
                        opacity: 0.5
                    }

                    Row {
                        width: parent.width
                        spacing: 6

                        Text {
                            text: qsTr("Cenários:")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ThemedButton {
                            text: qsTr("Quarto ao Lado")
                            variant: "ghost"
                            tooltip: qsTr("Abafado através da parede com grave encorpado")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.applyPartyNextDoorPreset(t, c, 0)
                            }
                        }

                        ThemedButton {
                            text: qsTr("No Banheiro")
                            variant: "ghost"
                            tooltip: qsTr("Mais abafado com eco de azulejo estilo festa")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.applyPartyNextDoorPreset(t, c, 1)
                            }
                        }

                        ThemedButton {
                            text: qsTr("Vizinho de Cima")
                            variant: "ghost"
                            tooltip: qsTr("Apenas o grave do subwoofer vibrando o teto")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.applyPartyNextDoorPreset(t, c, 2)
                            }
                        }
                    }
                }
            }
        }

        // ----- Isolador Vocal & Separador de Música -------------------------
        Rectangle {
            id: vocalIsolationCard
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            implicitHeight: vocalIsolationCol.implicitHeight + 20
            radius: Theme.radiusMd
            color: isVocalActive
                   ? (Theme.darkMode ? "#141e2e" : "#eff6ff")
                   : Theme.panelAccent
            border.width: 1
            border.color: isVocalActive
                          ? (Theme.darkMode ? "#2563eb" : "#3b82f6")
                          : Theme.panelBorder

            readonly property bool isVocalActive: {
                void root.clipDataRevision
                return (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0)
                    ? EditorState.isVocalIsolationEnabled(EditorState.selectedTrack, EditorState.selectedClip)
                    : false
            }
            readonly property int currentMode: {
                void root.clipDataRevision
                return (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0)
                    ? EditorState.vocalIsolationMode(EditorState.selectedTrack, EditorState.selectedClip)
                    : 0
            }

            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
            Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }

            Column {
                id: vocalIsolationCol
                x: 10
                y: 10
                width: parent.width - 20
                spacing: Theme.spacingSm

                Row {
                    width: parent.width
                    spacing: 8

                    Text {
                        text: "🎙️"
                        font.pixelSize: Theme.fontSizeBase
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        width: parent.width - 32 - vocalSwitch.width
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: qsTr("Isolador Vocal & Karaokê")
                            color: Theme.panelForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSm
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: qsTr("Separação de fala e trilha instrumental em tempo real")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    ThemedSwitch {
                        id: vocalSwitch
                        anchors.verticalCenter: parent.verticalCenter
                        checked: vocalIsolationCard.isVocalActive
                        onToggled: {
                            if (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0) {
                                EditorState.setVocalIsolation(
                                    EditorState.selectedTrack, EditorState.selectedClip, checked, vocalIsolationCard.currentMode)
                                root.clipDataRevision++
                            }
                        }
                    }
                }

                // Controls when active
                Column {
                    width: parent.width
                    visible: vocalSwitch.checked
                    spacing: 6

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: Theme.panelBorder
                        opacity: 0.5
                    }

                    Row {
                        width: parent.width
                        spacing: 6

                        ThemedButton {
                            text: qsTr("🗣️ Isolar Voz (Apenas Fala)")
                            variant: vocalIsolationCard.currentMode === 0 ? "solid" : "ghost"
                            tooltip: qsTr("Muda o foco para a voz e remove o fundo musical")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.setVocalIsolation(t, c, true, 0)
                                root.clipDataRevision++
                            }
                        }

                        ThemedButton {
                            text: qsTr("🎵 Remover Voz (Karaokê)")
                            variant: vocalIsolationCard.currentMode === 1 ? "solid" : "ghost"
                            tooltip: qsTr("Cancela a voz e mantém o instrumental com graves")
                            onClicked: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                EditorState.setVocalIsolation(t, c, true, 1)
                                root.clipDataRevision++
                            }
                        }
                    }
                }

                // Music Splitter action button
                ThemedButton {
                    width: parent.width
                    text: qsTr("✂️ Separar em 2 Faixas (Voz + Instrumental)")
                    variant: "ghost"
                    tooltip: qsTr("Duplica o clipe na timeline dividindo em faixa de voz e faixa de música")
                    onClicked: {
                        const t = EditorState.selectedTrack
                        const c = EditorState.selectedClip
                        if (t >= 0 && c >= 0) {
                            EditorState.splitVocalAndMusicTracks(t, c)
                            root.clipDataRevision++
                        }
                    }
                }
            }
        }

        // ----- Telefone Vintage & Rádio Lo-Fi ------------------------------
        Rectangle {
            id: retroAudioCard
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            implicitHeight: retroCol.implicitHeight + 20
            radius: Theme.radiusMd
            color: isRetroActive
                   ? (Theme.darkMode ? "#141e2e" : "#eff6ff")
                   : Theme.panelAccent
            border.width: 1
            border.color: isRetroActive
                          ? (Theme.darkMode ? "#2563eb" : "#3b82f6")
                          : Theme.panelBorder

            readonly property bool isPhoneOn: {
                void root.clipDataRevision
                return (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0)
                    ? EditorState.isTelephoneEnabled(EditorState.selectedTrack, EditorState.selectedClip)
                    : false
            }
            readonly property bool isRadioOn: {
                void root.clipDataRevision
                return (EditorState.selectedTrack >= 0 && EditorState.selectedClip >= 0)
                    ? EditorState.isLofiRadioEnabled(EditorState.selectedTrack, EditorState.selectedClip)
                    : false
            }
            readonly property bool isRetroActive: isPhoneOn || isRadioOn

            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
            Behavior on border.color { ColorAnimation { duration: Theme.durationFast } }

            Column {
                id: retroCol
                x: 10
                y: 10
                width: parent.width - 20
                spacing: Theme.spacingSm

                Row {
                    width: parent.width
                    spacing: 8

                    Text {
                        text: "📻"
                        font.pixelSize: Theme.fontSizeBase
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        width: parent.width - 32
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: qsTr("Efeitos Retrô (Telefone & Rádio Lo-Fi)")
                            color: Theme.panelForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSm
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: qsTr("Texturas vintage, som telefônico e calor analógico")
                            color: Theme.mutedForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    color: Theme.panelBorder
                    opacity: 0.5
                }

                // Row Telefone
                Column {
                    width: parent.width
                    spacing: 4

                    Row {
                        width: parent.width
                        spacing: 8

                        Text {
                            text: qsTr("📞 Voz de Telefone:")
                            color: Theme.panelForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ThemedSwitch {
                            checked: retroAudioCard.isPhoneOn
                            anchors.verticalCenter: parent.verticalCenter
                            onToggled: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                if (t >= 0 && c >= 0) {
                                    EditorState.setTelephoneEnabled(t, c, checked)
                                    root.clipDataRevision++
                                }
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        visible: retroAudioCard.isPhoneOn
                        spacing: 4

                        ThemedButton {
                            text: qsTr("Ligação")
                            variant: "ghost"
                            tooltip: qsTr("Voz típica de ligação de celular")
                            onClicked: {
                                EditorState.applyTelephonePreset(EditorState.selectedTrack, EditorState.selectedClip, 0)
                                root.clipDataRevision++
                            }
                        }

                        ThemedButton {
                            text: qsTr("Telefone Antigo")
                            variant: "ghost"
                            tooltip: qsTr("Corneta de telefone antigo de carbono")
                            onClicked: {
                                EditorState.applyTelephonePreset(EditorState.selectedTrack, EditorState.selectedClip, 1)
                                root.clipDataRevision++
                            }
                        }

                        ThemedButton {
                            text: qsTr("Interfone / Walkie")
                            variant: "ghost"
                            tooltip: qsTr("Estilo rádio comunicador militar")
                            onClicked: {
                                EditorState.applyTelephonePreset(EditorState.selectedTrack, EditorState.selectedClip, 2)
                                root.clipDataRevision++
                            }
                        }
                    }
                }

                // Row Rádio Lo-Fi
                Column {
                    width: parent.width
                    spacing: 4

                    Row {
                        width: parent.width
                        spacing: 8

                        Text {
                            text: qsTr("📻 Rádio Lo-Fi & Vinil:")
                            color: Theme.panelForeground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeXs
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        ThemedSwitch {
                            checked: retroAudioCard.isRadioOn
                            anchors.verticalCenter: parent.verticalCenter
                            onToggled: {
                                const t = EditorState.selectedTrack
                                const c = EditorState.selectedClip
                                if (t >= 0 && c >= 0) {
                                    EditorState.setLofiRadioEnabled(t, c, checked)
                                    root.clipDataRevision++
                                }
                            }
                        }
                    }

                    Row {
                        width: parent.width
                        visible: retroAudioCard.isRadioOn
                        spacing: 4

                        ThemedButton {
                            text: qsTr("Lo-Fi Beats")
                            variant: "ghost"
                            tooltip: qsTr("Corte suave de agudos e calor retrô")
                            onClicked: {
                                EditorState.applyLofiRadioPreset(EditorState.selectedTrack, EditorState.selectedClip, 0)
                                root.clipDataRevision++
                            }
                        }

                        ThemedButton {
                            text: qsTr("Vinil Retrô")
                            variant: "ghost"
                            tooltip: qsTr("Flutter e textura de disco de vinil")
                            onClicked: {
                                EditorState.applyLofiRadioPreset(EditorState.selectedTrack, EditorState.selectedClip, 1)
                                root.clipDataRevision++
                            }
                        }

                        ThemedButton {
                            text: qsTr("Fita Cassete")
                            variant: "ghost"
                            tooltip: qsTr("Saturação suave de fita magnética")
                            onClicked: {
                                EditorState.applyLofiRadioPreset(EditorState.selectedTrack, EditorState.selectedClip, 2)
                                root.clipDataRevision++
                            }
                        }
                    }
                }
            }
        }

        // ----- Noise removal ---------------------------------------------
        Column {
            id: denoiseSection
            width: parent.width
            spacing: Theme.spacingSm
            visible: root.clipKind === "audio" || root.clipKind === "video"

            // Whether the model is on disk is a one-shot filesystem answer, not a
            // binding, hence the reset below when an addon of this kind appears.
            // The runtime that runs it is a second, separate addon.
            property bool denoiseReady: EditorState.denoiseAvailable()
            property bool runtimeReady: Addons.runtimeAvailable()

            Connections {
                target: Addons
                function onKindChanged(kind) {
                    if (kind === "denoise-model")
                        denoiseSection.denoiseReady = EditorState.denoiseAvailable()
                    else if (kind === "onnxruntime")
                        denoiseSection.runtimeReady = Addons.runtimeAvailable()
                }
            }

            Text {
                width: parent.width
                text: qsTr("Noise")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            ThemedButton {
                visible: denoiseSection.denoiseReady && denoiseSection.runtimeReady
                width: parent.width
                text: qsTr("Remove noise…")
                enabled: !EditorState.denoising
                onClicked: {
                    const data = EditorState.selectedClipData
                    root.Window.window.openDenoise(
                        EditorState.selectedTrack, EditorState.selectedClip,
                        data.duration !== undefined ? data.duration : 0)
                }
            }

            ThemedButton {
                visible: !denoiseSection.denoiseReady || !denoiseSection.runtimeReady
                width: parent.width
                text: denoiseSection.runtimeReady
                      ? qsTr("Download noise removal (about 9 MB)")
                      : qsTr("Install AI engine first")
                variant: "primary"
                onClicked: root.Window.window.openAddonManager(
                    denoiseSection.runtimeReady ? "denoise-model" : "onnxruntime")
            }
        }

        Rectangle {
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            height: 1
            color: Theme.panelBorder
            opacity: 0.5
        }

        Column {
            id: smartCutSection
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            spacing: Theme.spacingSm

            property bool settingsOpen: false
            property real silenceThreshold: 0.02
            property real minSilenceDuration: 0.35
            property real speechPadding: 0.08

            Text {
                width: parent.width
                text: qsTr("Corte de Silêncio (Smart Cut)")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            ThemedButton {
                width: parent.width
                variant: "primary"
                glyph: Theme.icons.scissors
                text: qsTr("Remover Silêncios Automaticamente")
                tooltip: qsTr("Corta pausas e hesitações do clipe e junta os trechos de fala com trilha magnética")
                onClicked: {
                    const count = EditorState.removeSilenceFromSelectedClip(
                        smartCutSection.silenceThreshold,
                        smartCutSection.minSilenceDuration,
                        smartCutSection.speechPadding
                    )
                    if (count > 0)
                        Toasts.success(qsTr("%1 pausas/silêncios removidos!").arg(count))
                    else
                        Toasts.info(qsTr("Nenhum silêncio relevante detectado no clipe."))
                }
            }

            ThemedButton {
                width: parent.width
                variant: "ghost"
                glyph: smartCutSection.settingsOpen ? Theme.icons.chevronDown : Theme.icons.chevronRight
                text: qsTr("Ajustes de Sensibilidade")
                onClicked: smartCutSection.settingsOpen = !smartCutSection.settingsOpen
            }

            Column {
                width: parent.width
                spacing: Theme.spacingSm
                visible: smartCutSection.settingsOpen

                Row {
                    width: parent.width
                    spacing: Theme.spacingSm

                    ThemedButton {
                        text: qsTr("Rápido (Reels/TikTok)")
                        variant: smartCutSection.minSilenceDuration === 0.25 ? "secondary" : "ghost"
                        onClicked: {
                            smartCutSection.minSilenceDuration = 0.25
                            smartCutSection.silenceThreshold = 0.03
                            smartCutSection.speechPadding = 0.05
                        }
                    }

                    ThemedButton {
                        text: qsTr("Natural (Podcast)")
                        variant: smartCutSection.minSilenceDuration === 0.35 ? "secondary" : "ghost"
                        onClicked: {
                            smartCutSection.minSilenceDuration = 0.35
                            smartCutSection.silenceThreshold = 0.02
                            smartCutSection.speechPadding = 0.08
                        }
                    }
                }
            }
        }

        Rectangle {
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            height: 1
            color: Theme.panelBorder
            opacity: 0.5
        }

        Text {
            visible: root.clipKind === "audio" || root.clipKind === "video"
            text: qsTr("Auto subtitles")
            color: Theme.mutedForeground
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeXs
        }

        Rectangle {
            visible: root.clipKind === "audio" || root.clipKind === "video"
            width: parent.width
            implicitHeight: privAudioRow.implicitHeight + Theme.spacingSm * 2
            radius: Theme.radiusMd
            color: Theme.darkMode ? "#14251a" : "#ebfbee"
            border.width: Theme.borderWidth
            border.color: Theme.darkMode ? "#245330" : "#bbf7d0"

            Row {
                id: privAudioRow
                x: Theme.spacingMd
                y: Theme.spacingSm
                width: parent.width - Theme.spacingMd * 2
                spacing: Theme.spacingSm

                Text {
                    text: "🛡️"
                    font.pixelSize: Theme.fontSizeSm
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    width: parent.width - 24
                    wrapMode: Text.WordWrap
                    text: qsTr("Zero Cloud: Processado 100% no seu hardware local.")
                    color: Theme.darkMode ? "#86efac" : "#166534"
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeXs
                    font.weight: Font.Medium
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // The transcriber is an addon, and so is the runtime it needs; without
        // both there are no languages to list and nothing to run, so offer the
        // download in place of the controls.
        property bool whisperReady: Addons.hasKind("whisper-model")
                                    && Addons.runtimeAvailable()
        property bool runtimeReady: Addons.runtimeAvailable()

        Connections {
            target: Addons
            function onKindChanged(kind) {
                if (kind !== "whisper-model" && kind !== "onnxruntime")
                    return
                const section = subtitleLanguageBox.parent
                section.runtimeReady = Addons.runtimeAvailable()
                section.whisperReady = Addons.hasKind("whisper-model")
                                       && section.runtimeReady
            }
        }

        ThemedComboBox {
            id: subtitleLanguageBox
            visible: parent.whisperReady
                     && (root.clipKind === "audio" || root.clipKind === "video")
            width: parent.width
            enabled: !EditorState.subtitleGenerating
            textRole: "label"
            valueRole: "code"
            model: EditorState.whisperLanguages()
            Component.onCompleted: currentIndex = 0
        }

        ThemedComboBox {
            id: subtitleWordsBox
            visible: parent.whisperReady
                     && (root.clipKind === "audio" || root.clipKind === "video")
            width: parent.width
            enabled: !EditorState.subtitleGenerating
            textRole: "label"
            valueRole: "words"
            model: root.captionLengthOptions
            Component.onCompleted: currentIndex = 0
        }

        Text {
            visible: subtitleWordsBox.visible && subtitleWordsBox.currentValue > 0
            width: parent.width
            wrapMode: Text.WordWrap
            text: qsTr("Shorter captions are timed by splitting each phrase evenly, so they can drift slightly out of sync with the speech.")
            color: Theme.mutedForeground
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeXs
        }

        ThemedComboBox {
            id: subtitleStyleBox
            visible: parent.whisperReady
                     && (root.clipKind === "audio" || root.clipKind === "video")
            width: parent.width
            enabled: !EditorState.subtitleGenerating
            textRole: "label"
            valueRole: "id"
            model: root.captionStyleOptions
            Component.onCompleted: currentIndex = 0
        }

        ThemedCheckBox {
            id: subtitleAllCapsCheck
            visible: parent.whisperReady
                     && (root.clipKind === "audio" || root.clipKind === "video")
            text: qsTr("Texto em MAIÚSCULAS (Estilo Reels/TikTok)")
            checked: true
            enabled: !EditorState.subtitleGenerating
        }

        ThemedButton {
            visible: parent.whisperReady
                     && (root.clipKind === "audio" || root.clipKind === "video")
            width: parent.width
            text: EditorState.subtitleGenerating
                  ? qsTr("Creating captions… %1%").arg(Math.round(EditorState.subtitleGenProgress * 100))
                  : qsTr("Create captions from speech")
            enabled: !EditorState.subtitleGenerating
            onClicked: {
                const lang = subtitleLanguageBox.currentValue !== undefined
                             ? subtitleLanguageBox.currentValue
                             : ""
                const style = subtitleStyleBox.currentValue !== undefined
                              ? subtitleStyleBox.currentValue
                              : "tiktok-viral-yellow"
                EditorState.generateSubtitlesForClip(
                    EditorState.selectedTrack, EditorState.selectedClip, lang,
                    subtitleWordsBox.currentValue, style, subtitleAllCapsCheck.checked)
            }
        }

        ThemedButton {
            visible: !parent.whisperReady
                     && (root.clipKind === "audio" || root.clipKind === "video")
            width: parent.width
            text: parent.runtimeReady
                  ? qsTr("Download speech recognition (about 670 MB)")
                  : qsTr("Install AI engine first")
            variant: "primary"
            onClicked: root.Window.window.openAddonManager(
                parent.runtimeReady ? "whisper-model" : "onnxruntime")
        }
    }
}
