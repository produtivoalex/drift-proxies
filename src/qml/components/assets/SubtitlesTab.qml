import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Window
import Drift
import ".."

// Subtitles tab: timed caption clips, .srt import, and auto-caption from speech.
Item {
    id: root

    // A clip landed on the timeline. The phone shell closes the sheet on this —
    // the thing you came for is behind it. Auto-caption deliberately does not
    // emit it: its progress and cancel button live in this sheet.
    signal added()

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: subtitleColumn.height + Theme.spacing3xl
        clip: true
        ScrollBar.vertical: AppScrollBar { }

        Column {
            id: subtitleColumn
            x: Theme.pagePadding
            width: parent.width - Theme.pagePadding * 2
            spacing: Theme.spacingLg
            topPadding: Theme.pagePadding

            readonly property real contentWidth: width

            Text {
                width: subtitleColumn.contentWidth
                wrapMode: Text.WordWrap
                text: qsTr("Subtitle track — one clip holds many timed captions. Place it on the timeline, trim its length, then add caption lines at each moment in the clip panel.")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            ThemedButton {
                text: qsTr("Add subtitle clip")
                variant: "primary"
                glyph: Theme.icons.captions
                onClicked: {
                    EditorState.addSubtitleClip(-1)
                    root.added()
                }
            }

            ThemedButton {
                text: qsTr("Import subtitle file")
                variant: "secondary"
                glyph: Theme.icons.upload
                tooltip: qsTr("Import a .srt file as a subtitle clip")
                onClicked: {
                    const url = FileDialogs.openFile(
                        qsTr("Import Subtitles"),
                        [qsTr("SubRip subtitles (*.srt)"), qsTr("All files (*)")])
                    if (url != "") {
                        EditorState.importSubtitleFile(url, -1)
                        root.added()
                    }
                }
            }

            ThemedButton {
                text: qsTr("Narração de Texto em Voz (TTS)")
                variant: "secondary"
                glyph: Theme.icons.audioLines
                tooltip: qsTr("Crie narração falada e gere legendas animadas na aba de Texto")
                onClicked: {
                    Toasts.info(qsTr("Acesse a aba “Texto” acima para digitar a narração e sincronizar as legendas!"))
                }
            }

            Rectangle {
                width: subtitleColumn.contentWidth
                height: Theme.borderWidth
                color: Theme.panelBorder
            }

            // Privacy assurance badge: Zero Cloud AI
            Rectangle {
                width: subtitleColumn.contentWidth
                implicitHeight: privacyRow.implicitHeight + Theme.spacingSm * 2
                radius: Theme.radiusMd
                color: Theme.darkMode ? "#14251a" : "#ebfbee"
                border.width: Theme.borderWidth
                border.color: Theme.darkMode ? "#245330" : "#bbf7d0"

                Row {
                    id: privacyRow
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
                        text: qsTr("IA 100% Local (Zero Cloud): Whisper executado no seu processador/placa de vídeo. Seus arquivos de áudio e vídeo jamais saem deste computador.")
                        color: Theme.darkMode ? "#86efac" : "#166534"
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeXs
                        font.weight: Font.Medium
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            // Same transcriber as the clip inspector's Audio tab, surfaced here so
            // auto captions sit next to the manual subtitle route. It transcribes the
            // selected clip, so it stays disabled until a video or audio clip is picked.
            Text {
                width: subtitleColumn.contentWidth
                text: qsTr("Add auto caption")
                color: Theme.foreground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSm
            }

            Text {
                width: subtitleColumn.contentWidth
                wrapMode: Text.WordWrap
                text: root.captionTargetReady
                      ? qsTr("Creates captions from the speech in the selected clip.")
                      : qsTr("Select a video or audio clip on the timeline first.")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            // The transcriber ships as an addon, and so does the runtime it needs; without
            // both there are no languages to list and nothing to run, so offer the download
            // in place of the controls.
            ThemedComboBox {
                id: captionLanguageBox
                visible: root.whisperReady
                width: subtitleColumn.contentWidth
                enabled: root.captionTargetReady && !EditorState.subtitleGenerating
                textRole: "label"
                valueRole: "code"
                model: EditorState.whisperLanguages()
                Component.onCompleted: currentIndex = 0
            }

            ThemedComboBox {
                id: captionWordsBox
                visible: root.whisperReady
                width: subtitleColumn.contentWidth
                enabled: root.captionTargetReady && !EditorState.subtitleGenerating
                textRole: "label"
                valueRole: "words"
                model: root.captionLengthOptions
                Component.onCompleted: currentIndex = 0
            }

            Text {
                visible: root.whisperReady
                width: subtitleColumn.contentWidth
                wrapMode: Text.WordWrap
                text: captionWordsBox.currentValue === 1
                      ? qsTr("⚡ Modo Dinâmico: Cada palavra surge e pisca na tela exatamente quando é falada (estilo Hormozi/Shorts).")
                      : (captionWordsBox.currentValue > 0
                         ? qsTr("Frases mais curtas têm timing sincronizado por interpolação de palavras.")
                         : qsTr("Frases completas geradas pelo Whisper."))
                color: captionWordsBox.currentValue === 1 ? Theme.primary : Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
                font.weight: captionWordsBox.currentValue === 1 ? Font.Medium : Font.Normal
            }

            // Slider de limite de caracteres por linha
            Column {
                visible: root.whisperReady
                width: subtitleColumn.contentWidth
                spacing: 4

                Row {
                    width: parent.width
                    Text {
                        text: qsTr("Máximo de letras por linha:")
                        color: Theme.mutedForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeXs
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Item { width: Theme.spacingSm; height: 1 }
                    Text {
                        text: qsTr("%1 caracteres").arg(charsSlider.value)
                        color: Theme.panelForeground
                        font.family: Theme.monoFontFamily
                        font.pixelSize: Theme.fontSizeXs
                        font.weight: Font.Bold
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                ThemedSlider {
                    id: charsSlider
                    width: parent.width
                    from: 14
                    to: 50
                    stepSize: 1
                    value: 42
                    enabled: root.captionTargetReady && !EditorState.subtitleGenerating
                }
            }

            // Seletor de Formatação de Texto (Capitalização)
            Column {
                visible: root.whisperReady
                width: subtitleColumn.contentWidth
                spacing: 4

                Text {
                    text: qsTr("Formatação de Caixa de Texto:")
                    color: Theme.mutedForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeXs
                }

                ThemedComboBox {
                    id: captionCapitalizationBox
                    width: parent.width
                    enabled: root.captionTargetReady && !EditorState.subtitleGenerating
                    textRole: "label"
                    valueRole: "mode"
                    model: [
                        { label: qsTr("Aa Frase: 1ª letra maiúscula de cada frase"), mode: 1 },
                        { label: qsTr("Aa Palavra: 1ª Letra Maiúscula De Cada Palavra"), mode: 2 },
                        { label: qsTr("AA TUDO MAIÚSCULO (Estilo Reels / Viral)"), mode: 3 },
                        { label: qsTr("aa tudo minúsculo (Estilo minimalista)"), mode: 4 },
                        { label: qsTr("Original / Como Falado pelo Whisper"), mode: 0 }
                    ]
                    Component.onCompleted: currentIndex = 0
                }
            }

            ThemedComboBox {
                id: captionStyleBox
                visible: root.whisperReady
                width: subtitleColumn.contentWidth
                enabled: root.captionTargetReady && !EditorState.subtitleGenerating
                textRole: "label"
                valueRole: "id"
                model: root.captionStyleOptions
                Component.onCompleted: currentIndex = 0
            }

            ThemedCheckBox {
                id: autoEmojisBox
                visible: root.whisperReady
                width: subtitleColumn.contentWidth
                text: qsTr("✨ Inserir Emojis Contextuais (💸, 🔥, 💡, ⚡, 🎯)")
                checked: true
                tooltip: qsTr("Analisa as palavras-chave faladas e anexa automaticamente emojis vibrantes de alto impacto")
            }

            ThemedButton {
                visible: root.whisperReady && !EditorState.subtitleGenerating
                width: subtitleColumn.contentWidth
                text: qsTr("Add auto caption")
                variant: "secondary"
                glyph: Theme.icons.captions
                tooltip: root.captionTargetReady
                         ? qsTr("Create captions from the selected clip's speech")
                         : qsTr("Select a video or audio clip first")
                enabled: root.captionTargetReady
                onClicked: {
                    const lang = captionLanguageBox.currentValue !== undefined
                                 ? captionLanguageBox.currentValue
                                 : ""
                    const style = captionStyleBox.currentValue !== undefined
                                  ? captionStyleBox.currentValue
                                  : "tiktok-viral-yellow"
                    const capMode = captionCapitalizationBox.currentValue !== undefined
                                    ? captionCapitalizationBox.currentValue
                                    : 1
                    const charsPerLine = Math.round(charsSlider.value)
                    const isAllCaps = capMode === 3
                    const withEmojis = autoEmojisBox.checked

                    EditorState.generateSubtitlesForClip(
                        EditorState.selectedTrack, EditorState.selectedClip, lang,
                        captionWordsBox.currentValue, style, isAllCaps, capMode, charsPerLine, withEmojis)
                }
            }

            Column {
                visible: root.whisperReady && EditorState.subtitleGenerating
                width: subtitleColumn.contentWidth
                spacing: Theme.spacingMd

                Text {
                    width: parent.width
                    text: EditorState.subtitleGenStatus.length > 0
                          ? EditorState.subtitleGenStatus
                          : qsTr("Creating captions… %1%").arg(Math.round(EditorState.subtitleGenProgress * 100))
                    color: Theme.panelForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeXs
                    elide: Text.ElideRight
                }

                Rectangle {
                    width: parent.width
                    height: Theme.spacingMd
                    radius: height / 2
                    color: Theme.panelMuted

                    Rectangle {
                        width: parent.width * Math.max(0, Math.min(1, EditorState.subtitleGenProgress))
                        height: parent.height
                        radius: parent.radius
                        color: Theme.primary

                        Behavior on width {
                            NumberAnimation { duration: Theme.durationBase; easing.type: Theme.easing }
                        }
                    }
                }

                ThemedButton {
                    text: qsTr("Cancel")
                    variant: "destructive"
                    glyph: Theme.icons.x
                    tooltip: qsTr("Stop creating captions")
                    onClicked: EditorState.cancelSubtitleGeneration()
                }
            }

            ThemedButton {
                visible: !root.whisperReady
                width: subtitleColumn.contentWidth
                text: root.runtimeReady
                      ? qsTr("Download speech recognition (about 670 MB)")
                      : qsTr("Install AI engine first")
                variant: "primary"
                glyph: Theme.icons.download
                tooltip: qsTr("Needed for auto captions from speech")
                onClicked: root.Window.window.openAddonManager(
                    root.runtimeReady ? "whisper-model" : "onnxruntime")
            }
        }
    }

    // "Recommended" packs by display width like openai-whisper does; the numbered entries cap
    // words per caption on top of that.
    readonly property var captionLengthOptions: [
        { label: qsTr("⚡ 1 Palavra por vez (Dinâmico / Hormozi)"), words: 1 },
        { label: qsTr("🔥 2 a 3 Palavras (Impacto / Shorts)"), words: 3 },
        { label: qsTr("💬 4 a 6 Palavras (Balanceado)"), words: 5 },
        { label: qsTr("📄 Frase Completa (Recomendado Whisper)"), words: 0 },
        { label: qsTr("2 palavras por legenda"), words: 2 },
        { label: qsTr("4 palavras por legenda"), words: 4 },
        { label: qsTr("6 palavras por legenda"), words: 6 },
        { label: qsTr("8 palavras por legenda"), words: 8 }
    ]

    readonly property var captionStyleOptions: [
        { label: qsTr("⚡ TikTok Amarelo Viral (Recomendado)"), id: "tiktok-viral-yellow" },
        { label: qsTr("🔥 Hormozi Verde Limão"), id: "hormozi-beast" },
        { label: qsTr("🦁 MrBeast Dourado Pop"), id: "mrbeast-pop" },
        { label: qsTr("💎 TikTok Ciano Glow"), id: "tiktok-cyan-glow" },
        { label: qsTr("🟢 TikTok Verde Neon"), id: "tiktok-neon-green" },
        { label: qsTr("🔴 Alerta Vermelho Impacto"), id: "danger-red" },
        { label: qsTr("💊 Reels Pill Highlight"), id: "reels-pill-box" },
        { label: qsTr("🎯 Shorts 1 Palavra"), id: "shorts-single-word" },
        { label: qsTr("📄 Padrão Clássico"), id: "subtitle" }
    ]

    property bool whisperReady: Addons.hasKind("whisper-model")
                                && Addons.runtimeAvailable()
    property bool runtimeReady: Addons.runtimeAvailable()

    readonly property string captionClipKind: {
        const data = EditorState.selectedClipData
        return (data && data.kind) ? data.kind : ""
    }
    readonly property bool captionTargetReady: captionClipKind === "video"
                                               || captionClipKind === "audio"

    Connections {
        target: Addons
        function onKindChanged(kind) {
            if (kind !== "whisper-model" && kind !== "onnxruntime")
                return
            root.runtimeReady = Addons.runtimeAvailable()
            root.whisperReady = Addons.hasKind("whisper-model")
                                && root.runtimeReady
        }
    }
}
