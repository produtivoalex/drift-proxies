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
    // A model clip is a full-canvas layer placed by its camera: x/y shift the model, the box
    // size and spin mean nothing (its own size and rotation live on the 3D Model tab).
    readonly property bool isModel3d: clipKind === "model3d"
    readonly property int canvasW: {
        void EditorState.tracksRevision
        return Math.max(1, EditorState.projectWidth())
    }
    readonly property int canvasH: {
        void EditorState.tracksRevision
        return Math.max(1, EditorState.projectHeight())
    }

    readonly property var propOpacity: { "key": "opacity", "label": qsTr("Opacity"), "def": 1.0, "decimals": 2 }
    readonly property var propX: { "key": "x", "label": "X", "def": 0.0, "decimals": 0 }
    readonly property var propY: { "key": "y", "label": "Y", "def": 0.0, "decimals": 0 }
    readonly property var propWidth: { "key": "width", "label": qsTr("Width"), "def": root.canvasW, "decimals": 0 }
    readonly property var propHeight: { "key": "height", "label": qsTr("Height"), "def": root.canvasH, "decimals": 0 }
    readonly property var propRotation: { "key": "rotation", "label": qsTr("Angle"), "def": 0.0, "decimals": 1 }

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
        spacing: Theme.spacingXl

        EmptyState {
            visible: root.clipKind === "audio"
            width: parent.width
            compact: true
            glyph: Theme.icons.film
            title: qsTr("Video only")
            hint: qsTr("This tab does not apply to audio clips.")
        }

        Column {
            width: root.width
            spacing: 10
            visible: root.clipKind !== "audio"

            Text {
                width: parent.width
                wrapMode: Text.WordWrap
                text: qsTr("Move to a time, set a value, then click the diamond to add a keyframe. With Auto keyframes on, dragging a slider or the preview also creates them.")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            // Auto-Reframe Inteligente (9:16 / 1:1) Section
            Rectangle {
                id: autoReframeCard
                width: parent.width
                radius: Theme.radiusMd
                color: Theme.panelBackground
                border.width: 1
                border.color: Theme.panelBorder
                height: reframeCol.height + 24

                property double targetAspect: 9.0 / 16.0
                property string motionMode: "smooth"
                property bool resizeProject: true

                Column {
                    id: reframeCol
                    x: 12
                    y: 12
                    width: parent.width - 24
                    spacing: 10

                    Row {
                        width: parent.width
                        spacing: 8
                        IconGlyph {
                            glyph: Theme.icons.smartphone
                            iconSize: 18
                            iconColor: Theme.primary
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: qsTr("Auto-Reframe Inteligente")
                            color: Theme.foreground
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontSizeSm
                            font.bold: true
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Rectangle {
                            width: 26
                            height: 16
                            radius: 4
                            color: Theme.primary
                            anchors.verticalCenter: parent.verticalCenter
                            Text {
                                anchors.centerIn: parent
                                text: qsTr("IA")
                                color: Theme.primaryForeground
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }
                    }

                    Text {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: qsTr("Rastreia o orador com IA e enquadra vídeos horizontais (16:9) em verticais (9:16) para TikTok, Reels e Shorts sem cortar a pessoa.")
                        color: Theme.mutedForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeXs
                    }

                    // Aspect buttons
                    Text {
                        text: qsTr("Proporção Alvo:")
                        color: Theme.mutedForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                    }

                    Row {
                        spacing: 6
                        ThemedButton {
                            text: qsTr("9:16 Vertical")
                            variant: Math.abs(autoReframeCard.targetAspect - (9.0 / 16.0)) < 0.01 ? "primary" : "ghost"
                            onClicked: autoReframeCard.targetAspect = 9.0 / 16.0
                        }
                        ThemedButton {
                            text: qsTr("1:1 Quadrado")
                            variant: Math.abs(autoReframeCard.targetAspect - 1.0) < 0.01 ? "primary" : "ghost"
                            onClicked: autoReframeCard.targetAspect = 1.0
                        }
                        ThemedButton {
                            text: qsTr("4:5 Feed")
                            variant: Math.abs(autoReframeCard.targetAspect - 0.8) < 0.01 ? "primary" : "ghost"
                            onClicked: autoReframeCard.targetAspect = 0.8
                        }
                    }

                    // Motion Mode buttons
                    Text {
                        text: qsTr("Dinâmica de Câmera:")
                        color: Theme.mutedForeground
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                    }

                    Row {
                        spacing: 6
                        ThemedButton {
                            text: qsTr("Suave")
                            variant: autoReframeCard.motionMode === "smooth" ? "secondary" : "ghost"
                            onClicked: autoReframeCard.motionMode = "smooth"
                        }
                        ThemedButton {
                            text: qsTr("Ação / Rápido")
                            variant: autoReframeCard.motionMode === "fast" ? "secondary" : "ghost"
                            onClicked: autoReframeCard.motionMode = "fast"
                        }
                        ThemedButton {
                            text: qsTr("Estático")
                            variant: autoReframeCard.motionMode === "center" ? "secondary" : "ghost"
                            onClicked: autoReframeCard.motionMode = "center"
                        }
                    }

                    // Resize project checkbox
                    Row {
                        spacing: 8
                        ThemedChip {
                            text: qsTr("Redimensionar tela do projeto para 1080×1920")
                            selected: autoReframeCard.resizeProject
                            onClicked: autoReframeCard.resizeProject = !autoReframeCard.resizeProject
                        }
                    }

                    // Action buttons
                    Row {
                        width: parent.width
                        spacing: 8

                        ThemedButton {
                            text: qsTr("Reenquadrar Clipe")
                            variant: "primary"
                            glyph: Theme.icons.smartphone
                            onClicked: {
                                const res = EditorState.autoReframeSelectedClip(
                                    autoReframeCard.targetAspect,
                                    autoReframeCard.motionMode,
                                    autoReframeCard.resizeProject
                                )
                                if (res.ok) {
                                    Toasts.success(qsTr("Clipe reenquadrado com %1 keyframes de rastreamento!").arg(res.keys || 0))
                                } else {
                                    Toasts.error(qsTr("Erro ao reenquadrar: %1").arg(res.error || ""))
                                }
                            }
                        }

                        ThemedButton {
                            text: qsTr("Timeline Toda")
                            variant: "secondary"
                            glyph: Theme.icons.layers
                            onClicked: {
                                const res = EditorState.autoReframeTimeline(
                                    autoReframeCard.targetAspect,
                                    autoReframeCard.motionMode,
                                    autoReframeCard.resizeProject
                                )
                                if (res.ok) {
                                    Toasts.success(qsTr("%1 clipes reenquadrados na timeline!").arg(res.clips || 0))
                                } else {
                                    Toasts.error(qsTr("Erro ao reenquadrar timeline: %1").arg(res.error || ""))
                                }
                            }
                        }
                    }
                }
            }

            ThemedChip {
                text: qsTr("Auto keyframes")
                selected: EditorState.autoKeyEnabled
                onClicked: EditorState.autoKeyEnabled = !EditorState.autoKeyEnabled
            }

            Text {
                text: root.isModel3d ? qsTr("Offset (px)") : qsTr("Position (px)")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
                font.weight: Font.Medium
            }

            PropertyKeyframeRow {
                width: parent.width
                propDef: root.propX
                keyframeList: (root.clipData.keyframes && root.clipData.keyframes.x && root.clipData.keyframes.x.points) || []
                useSlider: true
                sliderFrom: -root.canvasW
                sliderTo: root.canvasW * 2
                unit: "px"
            }
            PropertyKeyframeRow {
                width: parent.width
                propDef: root.propY
                keyframeList: (root.clipData.keyframes && root.clipData.keyframes.y && root.clipData.keyframes.y.points) || []
                useSlider: true
                sliderFrom: -root.canvasH
                sliderTo: root.canvasH * 2
                unit: "px"
            }

            Text {
                visible: !root.isModel3d
                text: qsTr("Size (px)")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
                font.weight: Font.Medium
            }

            PropertyKeyframeRow {
                visible: !root.isModel3d
                width: parent.width
                propDef: root.propWidth
                keyframeList: (root.clipData.keyframes && root.clipData.keyframes.width && root.clipData.keyframes.width.points) || []
                useSlider: true
                sliderFrom: 1
                sliderTo: Math.max(root.canvasW * 2, 2)
                unit: "px"
            }
            PropertyKeyframeRow {
                visible: !root.isModel3d
                width: parent.width
                propDef: root.propHeight
                keyframeList: (root.clipData.keyframes && root.clipData.keyframes.height && root.clipData.keyframes.height.points) || []
                useSlider: true
                sliderFrom: 1
                sliderTo: Math.max(root.canvasH * 2, 2)
                unit: "px"
            }

            Text {
                text: root.isModel3d ? qsTr("Opacity") : qsTr("Opacity & rotation")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
                font.weight: Font.Medium
            }

            PropertyKeyframeRow {
                width: parent.width
                propDef: root.propOpacity
                keyframeList: (root.clipData.keyframes && root.clipData.keyframes.opacity && root.clipData.keyframes.opacity.points) || []
                useSlider: true
                sliderFrom: 0
                sliderTo: 1
                percent: true
            }

            PropertyKeyframeRow {
                visible: !root.isModel3d
                width: parent.width
                propDef: root.propRotation
                keyframeList: (root.clipData.keyframes && root.clipData.keyframes.rotation && root.clipData.keyframes.rotation.points) || []
                useSlider: true
                sliderFrom: -180
                sliderTo: 180
                unit: "°"
            }

            Text {
                visible: !root.isModel3d
                text: qsTr("Rotate 90°")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
                font.weight: Font.Medium
            }

            Flow {
                visible: !root.isModel3d
                width: parent.width
                spacing: 6
                Repeater {
                    // Plain ints avoid JS-object model role quirks (e.g. "value").
                    model: [0, 90, 180, -90]
                    delegate: ThemedChip {
                        required property int modelData
                        text: modelData + "°"
                        selected: {
                            void root.clipDataRevision
                            void EditorState.playheadSeconds
                            const cur = Number(root.clipData.rotationAtPlayhead || 0)
                            return Math.abs(cur - modelData) < 0.5
                        }
                        onClicked: EditorState.setClipRotationSnap(
                                       EditorState.selectedTrack, EditorState.selectedClip,
                                       modelData)
                    }
                }
            }

            Text {
                visible: root.clipKind === "video"
                text: qsTr("Fix orientation")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
                font.weight: Font.Medium
            }

            Text {
                visible: root.clipKind === "video"
                width: parent.width
                wrapMode: Text.WordWrap
                text: qsTr("Corrects the source's own rotation losslessly — unlike Angle above, this changes decoding, not just the on-screen box.")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            Flow {
                visible: root.clipKind === "video"
                width: parent.width
                spacing: 6
                Repeater {
                    model: [0, 90, 180, 270]
                    delegate: ThemedChip {
                        required property int modelData
                        text: modelData + "°"
                        selected: {
                            void root.clipDataRevision
                            return Number(root.clipData.orientation) === modelData
                        }
                        onClicked: EditorState.setClipOrientation(
                                       EditorState.selectedTrack, EditorState.selectedClip,
                                       modelData)
                    }
                }
            }

            Text {
                text: qsTr("Flip")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
                font.weight: Font.Medium
            }

            Flow {
                width: parent.width
                spacing: 6
                ThemedChip {
                    text: qsTr("Flip H")
                    selected: {
                        void root.clipDataRevision
                        return !!root.clipData.flipH
                    }
                    onClicked: EditorState.setClipFlip(
                                   EditorState.selectedTrack, EditorState.selectedClip,
                                   !root.clipData.flipH, !!root.clipData.flipV)
                }
                ThemedChip {
                    text: qsTr("Flip V")
                    selected: {
                        void root.clipDataRevision
                        return !!root.clipData.flipV
                    }
                    onClicked: EditorState.setClipFlip(
                                   EditorState.selectedTrack, EditorState.selectedClip,
                                   !!root.clipData.flipH, !root.clipData.flipV)
                }
            }

            ThemedButton {
                text: root.isModel3d ? qsTr("Reset position") : qsTr("Reset position & size")
                onClicked: EditorState.resetClipTransform(
                               EditorState.selectedTrack, EditorState.selectedClip)
            }
        }
    }
}
