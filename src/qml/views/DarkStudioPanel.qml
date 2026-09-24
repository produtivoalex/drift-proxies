// src/qml/views/DarkStudioPanel.qml
// Dark Studio — Painel do Roteirista LLM (Fase 5A)
// Interface premium de 3 etapas: Configurar → Gerar Roteiro → Gerar Vídeo
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Drift

Item {
    id: root
    implicitWidth: 420
    implicitHeight: 720

    // ── State machine: 0 = config, 1 = script ready, 2 = generating video
    property int step: 0

    // ── Bind to AppController properties
    property bool generating: app.scriptGenerating
    property double progress: app.scriptGenProgress
    property string statusText: app.scriptGenStatus
    property bool hasKey: app.hasScriptApiKey
    property string provider: app.scriptApiProvider

    // Watcher for script ready
    Connections {
        target: app
        function onScriptReady() { root.step = 1 }
        function onScriptError(msg) { errorBanner.show(msg) }
    }

    // ── Background
    Rectangle {
        anchors.fill: parent
        color: "#0d0d12"
        radius: 12
    }

    // ── Gradient top accent bar
    Rectangle {
        width: parent.width
        height: 3
        radius: 1.5
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#7c3aed" }
            GradientStop { position: 0.5; color: "#c026d3" }
            GradientStop { position: 1.0; color: "#f43f5e" }
        }
    }

    // ── Error banner
    Item {
        id: errorBanner
        property string message: ""
        property bool visible_: message !== ""
        function show(msg) { message = msg; hideTimer.restart() }
        Timer { id: hideTimer; interval: 5000; onTriggered: errorBanner.message = "" }
    }

    ColumnLayout {
        anchors {
            fill: parent
            margins: 20
            topMargin: 24
        }
        spacing: 16

        // ── Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            Rectangle {
                width: 36; height: 36; radius: 8
                color: "#1a1a2e"
                border.color: "#7c3aed"; border.width: 1
                Text {
                    anchors.centerIn: parent
                    text: "✦"; font.pixelSize: 18; color: "#c026d3"
                }
            }
            ColumnLayout {
                spacing: 2
                Text {
                    text: "Dark Studio"
                    font { pixelSize: 18; weight: Font.Bold; family: "Inter" }
                    color: "#ffffff"
                }
                Text {
                    text: "Roteirista com IA"
                    font { pixelSize: 11; family: "Inter" }
                    color: "#6b7280"
                }
            }
            Item { Layout.fillWidth: true }
            // Step indicator pills
            RowLayout {
                spacing: 4
                Repeater {
                    model: ["Tema", "Roteiro", "Vídeo"]
                    Rectangle {
                        width: index === root.step ? 28 : 8
                        height: 8; radius: 4
                        color: index <= root.step ? "#7c3aed" : "#1f1f2e"
                        Behavior on width { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }
                        Behavior on color { ColorAnimation { duration: 200 } }
                    }
                }
            }
        }

        // ── Error banner strip
        Rectangle {
            Layout.fillWidth: true
            height: errorBanner.visible_ ? 40 : 0
            color: "#2d0a0a"; radius: 6
            border.color: "#f43f5e"; border.width: 1
            clip: true
            Behavior on height { NumberAnimation { duration: 200 } }
            Text {
                anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }
                text: "⚠ " + errorBanner.message
                font { pixelSize: 12; family: "Inter" }
                color: "#fca5a5"
                elide: Text.ElideRight
                width: parent.width - 24
            }
        }

        // ────────────────────────────────────────────────────────
        // STEP 0: Configuration form
        // ────────────────────────────────────────────────────────
        ColumnLayout {
            visible: root.step === 0
            Layout.fillWidth: true
            spacing: 14

            // API key notice (if not set)
            Rectangle {
                Layout.fillWidth: true
                height: root.hasKey ? 0 : 62
                color: "#1a1206"; radius: 8
                border.color: "#d97706"; border.width: 1
                clip: true
                Behavior on height { NumberAnimation { duration: 200 } }
                ColumnLayout {
                    anchors { fill: parent; margins: 10 }
                    spacing: 2
                    Text {
                        text: "🔑 Configure sua API Key"
                        font { pixelSize: 12; weight: Font.Medium; family: "Inter" }
                        color: "#fbbf24"
                    }
                    Text {
                        text: "Acesse Configurações → IA para usar o Roteirista"
                        font { pixelSize: 11; family: "Inter" }
                        color: "#92400e"
                    }
                }
            }

            // Provider badge
            RowLayout {
                spacing: 8
                visible: root.hasKey
                Text { text: "Provedor:"; font { pixelSize: 12; family: "Inter" }; color: "#6b7280" }
                Rectangle {
                    height: 22; radius: 11
                    width: providerLabel.implicitWidth + 20
                    color: "#1a1a2e"; border.color: "#7c3aed"; border.width: 1
                    Text {
                        id: providerLabel
                        anchors.centerIn: parent
                        text: root.provider.toUpperCase()
                        font { pixelSize: 11; weight: Font.Bold; family: "Inter" }
                        color: "#a78bfa"
                    }
                }
            }

            // Topic input
            DarkLabel { text: "Tema do vídeo *" }
            DarkTextArea {
                id: topicField
                placeholder: "Ex: Os 5 segredos proibidos do Egito Antigo que a história oficial esconde"
                maxLength: 200
                Layout.fillWidth: true
                height: 72
            }

            // Niche + Format row
            RowLayout {
                Layout.fillWidth: true; spacing: 12
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 6
                    DarkLabel { text: "Nicho" }
                    DarkCombo {
                        id: nicheCombo
                        Layout.fillWidth: true
                        model: [
                            { value: "dark_mystery", label: "🌑 Dark Mistério" },
                            { value: "finance",      label: "💰 Finanças" },
                            { value: "motivation",   label: "⚡ Motivação" },
                            { value: "history",      label: "🏛 História" },
                            { value: "crime",        label: "🔍 True Crime" },
                            { value: "science",      label: "🔬 Ciência" },
                            { value: "lifestyle",    label: "✨ Lifestyle" }
                        ]
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 6
                    DarkLabel { text: "Formato" }
                    DarkCombo {
                        id: formatCombo
                        Layout.fillWidth: true
                        model: [
                            { value: "shorts_60s",   label: "📱 Shorts 60s" },
                            { value: "youtube_8min", label: "▶ YouTube 8min" },
                            { value: "podcast_20min",label: "🎙 Podcast 20min" }
                        ]
                    }
                }
            }

            // Language + Tone row
            RowLayout {
                Layout.fillWidth: true; spacing: 12
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 6
                    DarkLabel { text: "Idioma" }
                    DarkCombo {
                        id: langCombo
                        Layout.fillWidth: true
                        model: [
                            { value: "pt-BR", label: "🇧🇷 Português" },
                            { value: "en-US", label: "🇺🇸 English" },
                            { value: "es-ES", label: "🇪🇸 Español" },
                            { value: "de-DE", label: "🇩🇪 Deutsch" },
                            { value: "fr-FR", label: "🇫🇷 Français" }
                        ]
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 6
                    DarkLabel { text: "Tom" }
                    DarkCombo {
                        id: toneCombo
                        Layout.fillWidth: true
                        model: [
                            { value: "dramatic",      label: "🎭 Dramático" },
                            { value: "calm",          label: "😌 Calmo" },
                            { value: "energetic",     label: "🔥 Energético" },
                            { value: "authoritative", label: "👑 Autoritário" }
                        ]
                    }
                }
            }

            // Generate button
            DarkButton {
                id: generateBtn
                Layout.fillWidth: true
                text: "✦ Gerar Roteiro"
                enabled: topicField.text.trim().length > 8 && root.hasKey && !root.generating
                accent: true
                onClicked: {
                    const dur = formatCombo.model[formatCombo.currentIndex].value === "shorts_60s"  ? 60
                              : formatCombo.model[formatCombo.currentIndex].value === "youtube_8min" ? 480
                              : 1200
                    app.generateScript(
                        topicField.text.trim(),
                        nicheCombo.model[nicheCombo.currentIndex].value,
                        formatCombo.model[formatCombo.currentIndex].value,
                        langCombo.model[langCombo.currentIndex].value,
                        toneCombo.model[toneCombo.currentIndex].value,
                        dur
                    )
                }
            }

            // Progress during generation
            ColumnLayout {
                visible: root.generating
                Layout.fillWidth: true; spacing: 8
                ProgressBar {
                    Layout.fillWidth: true
                    value: root.progress
                    background: Rectangle { color: "#1a1a2e"; radius: 4; implicitHeight: 6 }
                    contentItem: Rectangle {
                        width: parent.visualPosition * parent.width
                        height: parent.height; radius: 4
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: "#7c3aed" }
                            GradientStop { position: 1.0; color: "#c026d3" }
                        }
                    }
                }
                Text {
                    text: root.statusText
                    font { pixelSize: 12; family: "Inter" }
                    color: "#a78bfa"
                }
                DarkButton {
                    Layout.fillWidth: true; text: "✕ Cancelar"
                    onClicked: app.cancelScriptGeneration()
                }
            }
        }

        // ────────────────────────────────────────────────────────
        // STEP 1: Script review
        // ────────────────────────────────────────────────────────
        ColumnLayout {
            visible: root.step === 1
            Layout.fillWidth: true
            spacing: 14

            // Hook card
            Rectangle {
                Layout.fillWidth: true; height: hookText.implicitHeight + 24
                color: "#0f0a1e"; radius: 8; border.color: "#7c3aed"; border.width: 1
                ColumnLayout {
                    anchors { fill: parent; margins: 12 }; spacing: 4
                    Text { text: "🎣 GANCHO"; font { pixelSize: 10; weight: Font.Bold; family: "Inter" }; color: "#7c3aed" }
                    Text {
                        id: hookText
                        text: app.lastScriptHook
                        wrapMode: Text.WordWrap
                        font { pixelSize: 14; weight: Font.Medium; family: "Inter" }
                        color: "#f9fafb"
                        Layout.fillWidth: true
                    }
                }
            }

            // Full script scrollable
            Text { text: "ROTEIRO COMPLETO"; font { pixelSize: 10; weight: Font.Bold; family: "Inter" }; color: "#4b5563" }
            Rectangle {
                Layout.fillWidth: true; height: 200; color: "#0a0a12"; radius: 8
                border.color: "#1f1f2e"; border.width: 1
                Flickable {
                    anchors { fill: parent; margins: 12 }
                    contentHeight: scriptText.implicitHeight
                    clip: true
                    Text {
                        id: scriptText
                        width: parent.width
                        text: app.lastScriptBody
                        wrapMode: Text.WordWrap
                        font { pixelSize: 12; family: "Inter" }
                        color: "#9ca3af"
                        lineHeight: 1.5
                    }
                }
                // Fade at bottom
                Rectangle {
                    anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                    height: 32; radius: 8
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "transparent" }
                        GradientStop { position: 1.0; color: "#0a0a12" }
                    }
                }
            }

            // B-Roll hints
            RowLayout {
                visible: app.lastScriptBrollHints.length > 0
                Layout.fillWidth: true; spacing: 6
                Text { text: "B-Roll:"; font { pixelSize: 11; family: "Inter" }; color: "#4b5563" }
                Flow {
                    Layout.fillWidth: true; spacing: 4
                    Repeater {
                        model: app.lastScriptBrollHints.slice(0, 6)
                        Rectangle {
                            height: 20; radius: 10; width: tagLabel.implicitWidth + 16
                            color: "#0f1a0f"; border.color: "#166534"; border.width: 1
                            Text {
                                id: tagLabel; anchors.centerIn: parent
                                text: modelData; font { pixelSize: 10; family: "Inter" }; color: "#4ade80"
                            }
                        }
                    }
                }
            }

            // Action buttons
            RowLayout {
                Layout.fillWidth: true; spacing: 8
                DarkButton {
                    Layout.fillWidth: true; text: "← Refazer"
                    onClicked: root.step = 0
                }
                DarkButton {
                    Layout.fillWidth: true; text: "▶ Gerar Vídeo"; accent: true
                    onClicked: {
                        app.generateTimelineFromLastScript("dark_mystery", "")
                        root.step = 2
                    }
                }
            }
        }

        // ────────────────────────────────────────────────────────
        // STEP 2: Generating timeline (Wizard running)
        // ────────────────────────────────────────────────────────
        ColumnLayout {
            visible: root.step === 2
            Layout.fillWidth: true
            spacing: 20

            Item { Layout.fillHeight: true }

            // Animated pulsing orb
            Item {
                Layout.alignment: Qt.AlignHCenter
                width: 80; height: 80
                Rectangle {
                    anchors.centerIn: parent
                    width: pulse.running ? 80 : 60
                    height: width; radius: width / 2
                    color: "transparent"
                    border.color: "#7c3aed"
                    border.width: 2
                    opacity: 0.4
                    Behavior on width { NumberAnimation { duration: 800; easing.type: Easing.InOutSine } }
                    Timer { id: pulse; running: root.step === 2; repeat: true; interval: 800
                        onTriggered: parent.width = parent.width === 80 ? 60 : 80 }
                }
                Rectangle {
                    anchors.centerIn: parent; width: 50; height: 50; radius: 25
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#7c3aed" }
                        GradientStop { position: 1.0; color: "#c026d3" }
                    }
                    Text { anchors.centerIn: parent; text: "✦"; font.pixelSize: 24; color: "#fff" }
                }
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "Montando seu vídeo..."
                font { pixelSize: 16; weight: Font.Medium; family: "Inter" }
                color: "#f9fafb"
            }

            ProgressBar {
                Layout.fillWidth: true
                value: app.wizardProgress
                background: Rectangle { color: "#1a1a2e"; radius: 4; implicitHeight: 6 }
                contentItem: Rectangle {
                    width: parent.visualPosition * parent.width
                    height: parent.height; radius: 4
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#7c3aed" }
                        GradientStop { position: 1.0; color: "#f43f5e" }
                    }
                }
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: app.wizardStatus
                font { pixelSize: 12; family: "Inter" }
                color: "#6b7280"
            }

            Connections {
                target: app
                function onWizardRunningChanged() {
                    if (!app.wizardRunning && root.step === 2) root.step = 0
                }
            }

            Item { Layout.fillHeight: true }
        }

        Item { Layout.fillHeight: true }

        // ── Footer
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Powered by GPT-4o-mini · Claude · Gemini"
            font { pixelSize: 10; family: "Inter" }
            color: "#374151"
        }
    }

    // ── Inline sub-components ────────────────────────────────────────────────

    component DarkLabel: Text {
        font { pixelSize: 12; weight: Font.Medium; family: "Inter" }
        color: "#9ca3af"
        Layout.fillWidth: true
    }

    component DarkTextArea: Rectangle {
        id: ta
        property alias text: taField.text
        property string placeholder: ""
        property int maxLength: 500
        color: "#0d0d1a"; radius: 8
        border.color: taField.activeFocus ? "#7c3aed" : "#1f1f2e"; border.width: 1
        Behavior on border.color { ColorAnimation { duration: 150 } }
        TextEdit {
            id: taField
            anchors { fill: parent; margins: 10 }
            wrapMode: TextEdit.WordWrap
            font { pixelSize: 13; family: "Inter" }
            color: "#f9fafb"
            selectionColor: "#7c3aed"
            Keys.onPressed: (ev) => {
                if (ev.key === Qt.Key_Return && ev.modifiers === Qt.NoModifier)
                    ev.accepted = true
            }
        }
        Text {
            visible: taField.text.length === 0
            anchors { fill: parent; margins: 10 }
            text: ta.placeholder
            wrapMode: Text.WordWrap
            font { pixelSize: 13; family: "Inter" }
            color: "#374151"
        }
    }

    component DarkCombo: ComboBox {
        id: dc
        property var model: []
        implicitHeight: 36
        background: Rectangle {
            color: dc.pressed ? "#1a1a30" : "#0d0d1a"
            radius: 8
            border.color: dc.hovered || dc.popup.visible ? "#7c3aed" : "#1f1f2e"
            border.width: 1
            Behavior on border.color { ColorAnimation { duration: 150 } }
        }
        contentItem: Text {
            leftPadding: 10
            text: dc.model[dc.currentIndex]?.label ?? ""
            font { pixelSize: 13; family: "Inter" }
            color: "#e5e7eb"
            verticalAlignment: Text.AlignVCenter
        }
        indicator: Text {
            x: dc.width - width - 8; y: (dc.height - height) / 2
            text: "▾"; font.pixelSize: 12; color: "#6b7280"
        }
        popup: Popup {
            y: dc.height + 4; width: dc.width
            padding: 0; topPadding: 0; bottomPadding: 0
            background: Rectangle { color: "#141420"; radius: 8; border.color: "#1f1f2e"; border.width: 1 }
            contentItem: ListView {
                implicitHeight: contentHeight
                model: dc.model
                delegate: ItemDelegate {
                    width: parent.width; height: 36
                    background: Rectangle {
                        color: hovered ? "#1a1a30" : "transparent"
                        Behavior on color { ColorAnimation { duration: 100 } }
                    }
                    contentItem: Text {
                        leftPadding: 12
                        text: modelData.label
                        font { pixelSize: 13; family: "Inter" }
                        color: dc.currentIndex === index ? "#a78bfa" : "#e5e7eb"
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: { dc.currentIndex = index; dc.popup.close() }
                }
            }
        }
    }

    component DarkButton: Rectangle {
        id: btn
        property string text: ""
        property bool accent: false
        property bool enabled: true
        signal clicked
        implicitHeight: 42; radius: 8
        opacity: btn.enabled ? 1.0 : 0.45
        gradient: accent ? Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "#7c3aed" }
            GradientStop { position: 1.0; color: "#c026d3" }
        } : null
        color: accent ? "transparent" : "#141420"
        border.color: accent ? "transparent" : "#1f1f2e"; border.width: 1
        scale: ma.pressed && btn.enabled ? 0.97 : 1.0
        Behavior on scale { NumberAnimation { duration: 80 } }
        Text {
            anchors.centerIn: parent
            text: btn.text
            font { pixelSize: 14; weight: Font.Medium; family: "Inter" }
            color: "#ffffff"
        }
        MouseArea {
            id: ma; anchors.fill: parent
            cursorShape: btn.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: if (btn.enabled) btn.clicked()
        }
    }
}
