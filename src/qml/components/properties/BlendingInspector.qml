import QtQuick
import QtQuick.Controls.Basic
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
    readonly property string activeBlendMode: (clipData && clipData.blendMode) || "normal"

    readonly property var propOpacity: ({ "key": "opacity", "label": qsTr("Opacity"), "def": 1.0, "decimals": 2 })

    height: contentCol.height
    implicitHeight: contentCol.height

    function refreshFields() {}

    Connections {
        target: EditorState
        function onSelectionChanged() { root.clipDataRevision++ }
        function onSelectedClipDataChanged() { root.clipDataRevision++ }
        function onTracksChanged() { root.clipDataRevision++ }
    }

    Column {
        id: contentCol
        width: root.width
        spacing: Theme.spacingLg

        EmptyState {
            visible: root.clipKind === "audio"
            width: parent.width
            compact: true
            glyph: Theme.icons.film
            title: qsTr("Video only")
            hint: qsTr("This tab does not apply to audio clips.")
        }

        // ----- Blending Mode Section ----------------------------------------------------
        Text {
            visible: root.clipKind !== "audio"
            text: qsTr("Blend mode")
            color: Theme.mutedForeground
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeXs
            font.weight: Font.Medium
        }

        Text {
            visible: root.clipKind !== "audio"
            width: parent.width
            wrapMode: Text.WordWrap
            text: qsTr("How this clip's colours combine with the tracks beneath it. Use Screen to hide black backgrounds or Multiply to hide white backgrounds.")
            color: Theme.mutedForeground
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeXs
            opacity: 0.85
        }

        // Quick 1-click Preset Chips
        Flow {
            visible: root.clipKind !== "audio"
            width: parent.width
            spacing: 6

            Repeater {
                model: [
                    { label: qsTr("Normal"), id: "normal" },
                    { label: qsTr("Screen (Hide Black)"), id: "screen" },
                    { label: qsTr("Multiply (Hide White)"), id: "multiply" },
                    { label: qsTr("Overlay"), id: "overlay" },
                    { label: qsTr("Color Dodge (Glow)"), id: "colorDodge" },
                    { label: qsTr("Soft Light"), id: "softLight" }
                ]
                delegate: ThemedChip {
                    required property var modelData
                    text: modelData.label
                    selected: root.activeBlendMode === modelData.id
                    onClicked: EditorState.setClipBlendMode(
                                   EditorState.selectedTrack, EditorState.selectedClip, modelData.id)
                }
            }
        }

        ThemedComboBox {
            id: blendModeBox
            visible: root.clipKind !== "audio"
            width: parent.width
            model: [
                "normal", "screen", "multiply", "overlay", "colorDodge",
                "softLight", "add", "darken", "lighten", "colorBurn", "difference"
            ]
            property var labels: ({
                "normal": qsTr("Normal (Standard)"),
                "screen": qsTr("Screen (Tela — Hide Black)"),
                "multiply": qsTr("Multiply (Multiplicar — Hide White)"),
                "overlay": qsTr("Overlay (Sobrepor — Contrast)"),
                "colorDodge": qsTr("Color Dodge (Subexposição — Intense Glow)"),
                "softLight": qsTr("Soft Light (Luz Suave)"),
                "add": qsTr("Add (Linear Dodge)"),
                "darken": qsTr("Darken (Escurecer)"),
                "lighten": qsTr("Lighten (Clarear)"),
                "colorBurn": qsTr("Color Burn (Superfície Queimada)"),
                "difference": qsTr("Difference (Diferença Invertida)")
            })
            displayText: labels[model[currentIndex]] || model[currentIndex]
            tooltip: qsTr("How this clip blends with the layers below")
            currentIndex: Math.max(0, model.indexOf(root.activeBlendMode))
            onActivated: EditorState.setClipBlendMode(
                             EditorState.selectedTrack, EditorState.selectedClip, model[currentIndex])
        }

        // Explanatory Card for active mode
        Rectangle {
            visible: root.clipKind !== "audio"
            width: parent.width
            radius: Theme.radiusSm
            color: Theme.panelBackground
            border.width: 1
            border.color: Theme.panelBorder
            height: descCol.height + 16

            Column {
                id: descCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 8
                spacing: 4

                ThemedLabel {
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    text: {
                        switch (root.activeBlendMode) {
                        case "screen": return qsTr("💡 Dica para Screen:");
                        case "multiply": return qsTr("📄 Dica para Multiply:");
                        case "colorDodge": return qsTr("⚡ Dica para Color Dodge:");
                        case "overlay": return qsTr("🎬 Dica para Overlay:");
                        case "softLight": return qsTr("✨ Dica para Soft Light:");
                        case "add": return qsTr("💥 Dica para Add:");
                        case "difference": return qsTr("🎨 Dica para Difference:");
                        default: return qsTr("ℹ️ Modo Ativo:");
                        }
                    }
                    color: Theme.accent
                }

                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    color: Theme.mutedForeground
                    text: {
                        switch (root.activeBlendMode) {
                        case "screen":
                            return qsTr("Elimina completamente áreas pretas. Perfeito para sobrepor fogo, fumaça, poeira mágica, luzes e faíscas.");
                        case "multiply":
                            return qsTr("Elimina áreas brancas mantendo os tons escuros. Excelente para texturas de papel, rascunhos e sombras.");
                        case "colorDodge":
                            return qsTr("Gera brilhos extremos de alta energia e saturação. Ideal para feixes de neon, sci-fi e reflexos intensos.");
                        case "overlay":
                            return qsTr("Aumenta o contraste cinematográfico mantendo tons médios equilibrados.");
                        case "softLight":
                            return qsTr("Efeito difuso e sutil como iluminação natural suave.");
                        case "add":
                            return qsTr("Soma aditiva pura de luz. Ótimo para flashes e explosões luminosas.");
                        case "difference":
                            return qsTr("Inversão cromática psicodélica com base nas diferenças de canal.");
                        default:
                            return qsTr("A camada superior cobre as inferiores normalmente.");
                        }
                    }
                }
            }
        }

        // ----- Opacity Section ----------------------------------------------------------
        Text {
            visible: root.clipKind !== "audio"
            text: qsTr("Opacity / Transparency")
            color: Theme.mutedForeground
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontSizeXs
            font.weight: Font.Medium
        }

        PropertyKeyframeRow {
            visible: root.clipKind !== "audio"
            width: parent.width
            propDef: root.propOpacity
            keyframeList: (root.clipData.keyframes && root.clipData.keyframes.opacity && root.clipData.keyframes.opacity.points) || []
            useSlider: true
            sliderFrom: 0
            sliderTo: 1
            percent: true
        }

        ThemedButton {
            visible: root.clipKind !== "audio" && root.activeBlendMode !== "normal"
            text: qsTr("Reset to Normal")
            variant: "ghost"
            glyph: Theme.icons.reset
            onClicked: EditorState.setClipBlendMode(
                           EditorState.selectedTrack, EditorState.selectedClip, "normal")
        }
    }
}
