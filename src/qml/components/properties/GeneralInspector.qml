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

    height: contentCol.height
    implicitHeight: contentCol.height

    // Human label for a clip kind. The raw id was shown to the user.
    function clipKindLabel(kind) {
        switch (kind) {
        case "video": return qsTr("Video")
        case "audio": return qsTr("Audio")
        case "image": return qsTr("Image")
        case "text": return qsTr("Text")
        case "subtitle": return qsTr("Subtitle")
        case "shape": return qsTr("Shape")
        case "sticker": return qsTr("Sticker")
        case "adjustment": return qsTr("Adjustment")
        }
        return kind.length > 0 ? kind : "—"
    }

    function applyTrim(inPoint, outPoint) {
        if (!root.hasSelection || isNaN(inPoint) || isNaN(outPoint))
            return
        EditorState.setClipTrim(EditorState.selectedTrack, EditorState.selectedClip, inPoint, outPoint)
    }

    function refreshFields() {
        if (!root.hasSelection)
            return
        if (startField && !startField.activeFocus)
            startField.value = root.clipData.start
        if (durationField && !durationField.activeFocus)
            durationField.value = root.clipData.duration
        if (inPointField && !inPointField.activeFocus)
            inPointField.value = root.clipData.inPoint
        if (outPointField && !outPointField.activeFocus)
            outPointField.value = root.clipData.outPoint
    }

    Connections {
        target: EditorState
        function onSelectionChanged() { root.clipDataRevision++; root.refreshFields() }
        function onSelectedClipDataChanged() { root.clipDataRevision++; root.refreshFields() }
        function onTracksChanged() { root.clipDataRevision++; root.refreshFields() }
    }

    Component.onCompleted: refreshFields()

    Column {
        id: contentCol
        width: root.width
        spacing: Theme.spacingXl

        // Read-only name plus a rename dialog: an editable field at the top of the panel kept
        // being mistaken for the text clip's content box.
        Column {
            width: root.width
            spacing: 4
            Text {
                text: qsTr("Clip name")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }
            Row {
                width: parent.width
                spacing: Theme.spacingSm
                Text {
                    width: parent.width - renameButton.width - parent.spacing
                    anchors.verticalCenter: parent.verticalCenter
                    text: (root.hasSelection && root.clipData.name) || qsTr("Untitled clip")
                    color: root.hasSelection && root.clipData.name ? Theme.panelForeground : Theme.mutedForeground
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeBase
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                }
                IconButton {
                    id: renameButton
                    glyph: Theme.icons.pencil
                    variant: "ghost"
                    buttonSize: Theme.controlHeightSm
                    tooltip: qsTr("Rename clip")
                    onClicked: renameDialog.openWith(qsTr("Rename clip"), root.clipData.name || "")
                }
            }
        }

        Column {
            width: root.width
            spacing: 4
            Text {
                text: qsTr("Type")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }
            Text {
                // Human label rather than the raw internal id.
                text: root.clipKindLabel(root.clipKind)
                color: Theme.panelForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSm
                elide: Text.ElideRight
                width: parent.width - x
            }
        }

        Row {
            width: parent.width
            spacing: 8

            Column {
                width: (parent.width - parent.spacing) / 2
                spacing: 4
                Text {
                    text: qsTr("Starts at")
                    color: Theme.mutedForeground
                    font.pixelSize: Theme.fontSizeXs
                    font.family: Theme.fontFamily
                }
                ThemedNumberField {
                    id: startField
                    to: 86400
                    unit: "s"
                    width: parent.width
                    decimals: 2
                    step: 0.1
                    from: 0
                    onEdited: v => EditorState.setClipStart(
                                      EditorState.selectedTrack, EditorState.selectedClip, v)
                }
            }

            Column {
                width: (parent.width - parent.spacing) / 2
                spacing: 4
                Text {
                    text: qsTr("Duration")
                    color: Theme.mutedForeground
                    font.pixelSize: Theme.fontSizeXs
                    font.family: Theme.fontFamily
                }
                ThemedNumberField {
                    id: durationField
                    to: 86400
                    unit: "s"
                    width: parent.width
                    decimals: 2
                    step: 0.1
                    from: 0.1
                    onEdited: v => EditorState.setClipDuration(
                                        EditorState.selectedTrack, EditorState.selectedClip, v)
                }
            }
        }

        Column {
            width: root.width
            spacing: 8
            visible: root.clipKind !== "text" && root.clipKind !== "subtitle"
                     && root.clipKind !== "adjustment" && root.clipKind !== "shape"

            Text {
                text: qsTr("Trim")
                HoverHandler { id: tipHover1217 }
                ThemedToolTip { text: qsTr("Which part of the original file this clip plays"); visible: tipHover1217.hovered }
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            Row {
                width: parent.width
                spacing: 8

                Column {
                    width: (parent.width - parent.spacing) / 2
                    spacing: 4
                    Text {
                        text: qsTr("From")
                        HoverHandler { id: tipHover1231 }
                        ThemedToolTip { text: qsTr("Seconds into the file where this clip starts"); visible: tipHover1231.hovered }
                        color: Theme.mutedForeground
                        font.pixelSize: Theme.fontSizeXs
                        font.family: Theme.fontFamily
                    }
                    ThemedNumberField {
                        id: inPointField
                        to: 86400
                        unit: "s"
                        width: parent.width
                        decimals: 2
                        step: 0.1
                        from: 0
                        onEdited: v => root.applyTrim(v, root.clipData.outPoint)
                    }
                }

                Column {
                    width: (parent.width - parent.spacing) / 2
                    spacing: 4
                    Text {
                        text: qsTr("To")
                        HoverHandler { id: tipHover1252 }
                        ThemedToolTip { text: qsTr("Seconds into the file where this clip ends"); visible: tipHover1252.hovered }
                        color: Theme.mutedForeground
                        font.pixelSize: Theme.fontSizeXs
                        font.family: Theme.fontFamily
                    }
                    ThemedNumberField {
                        id: outPointField
                        to: 86400
                        unit: "s"
                        width: parent.width
                        decimals: 2
                        step: 0.1
                        from: 0
                        onEdited: v => root.applyTrim(root.clipData.inPoint, v)
                    }
                }
            }
        }

        Column {
            width: root.width
            spacing: 4
            visible: root.clipData.path !== undefined && root.clipData.path.length > 0

            Text {
                text: qsTr("File")
                color: Theme.mutedForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeXs
            }

            Text {
                id: clipPathLabel
                text: root.clipData.path || "—"
                color: Theme.panelForeground
                font.family: Theme.monoFontFamily
                font.pixelSize: Theme.fontSizeSm
                width: parent.width
                wrapMode: Text.WrapAnywhere
                // Capped: a deep path used to wrap unbounded and
                // dominate the whole General tab.
                maximumLineCount: 3
                elide: Text.ElideRight

                HoverHandler { id: pathHover }

                ThemedToolTip {
                    text: root.clipData.path || ""
                    visible: pathHover.hovered && (root.clipData.path || "").length > 0
                }
            }
        }

        // ----- Recorte Inteligente de Fundo (Smart Cutout / Auto Cutout) ----------------
        Rectangle {
            id: smartCutoutCard
            visible: root.clipKind === "video"
            width: parent.width
            radius: Theme.radiusMd
            color: Theme.panelBackground
            border.width: 1
            border.color: Theme.panelBorder
            height: cutoutCol.height + 24

            Column {
                id: cutoutCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 12
                spacing: 10

                Row {
                    spacing: 6
                    Text {
                        text: "✂️"
                        font.pixelSize: 14
                    }
                    Text {
                        text: qsTr("Recorte Inteligente de Fundo (Auto Cutout)")
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeSm
                        font.weight: Font.DemiBold
                        color: Theme.panelForeground
                    }
                }

                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontSizeXs
                    color: Theme.mutedForeground
                    text: qsTr("Remove o fundo de apresentadores instantaneamente sem tela verde usando IA local (RVM).")
                }

                // Action Buttons when not segmenting
                Column {
                    width: parent.width
                    spacing: 6
                    visible: !EditorState.segmenting

                    ThemedButton {
                        width: parent.width
                        text: qsTr("Remover Fundo (Auto Cutout 1-Clique)")
                        variant: "primary"
                        glyph: Theme.icons.sparkles
                        tooltip: qsTr("Recorta a pessoa e remove o cenário de fundo automaticamente")
                        onClicked: EditorState.autoCutoutPerson(EditorState.selectedTrack, EditorState.selectedClip)
                    }

                    ThemedButton {
                        width: parent.width
                        text: qsTr("🔤 Criar Texto Atrás da Pessoa")
                        variant: "secondary"
                        tooltip: qsTr("Efeito viral: cria texto grande flutuando atrás da pessoa com multicamada automática")
                        onClicked: EditorState.createTextBehindSubjectEffect(EditorState.selectedTrack, EditorState.selectedClip)
                    }

                    Row {
                        width: parent.width
                        spacing: 6
                        visible: EditorState.hasCutoutMask(EditorState.selectedTrack, EditorState.selectedClip)

                        ThemedButton {
                            width: (parent.width - 6) / 2
                            text: qsTr("Inverter")
                            variant: "ghost"
                            tooltip: qsTr("Alterna a máscara para manter apenas o fundo e esconder a pessoa")
                            onClicked: EditorState.invertCutoutMask(EditorState.selectedTrack, EditorState.selectedClip)
                        }

                        ThemedButton {
                            width: (parent.width - 6) / 2
                            text: qsTr("Restaurar")
                            variant: "destructive"
                            glyph: Theme.icons.trash
                            tooltip: qsTr("Remove o recorte e restaura o vídeo original completo")
                            onClicked: EditorState.removeCutoutMask(EditorState.selectedTrack, EditorState.selectedClip)
                        }
                    }
                }

                // Progress Bar while cutout is processing
                Column {
                    width: parent.width
                    spacing: 6
                    visible: EditorState.segmenting

                    Text {
                        width: parent.width
                        text: EditorState.segmentStatus.length > 0
                              ? EditorState.segmentStatus
                              : qsTr("Recortando apresentador… %1%").arg(Math.round(EditorState.segmentProgress * 100))
                        color: Theme.accent
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontSizeXs
                        elide: Text.ElideRight
                    }

                    Rectangle {
                        width: parent.width
                        height: 6
                        radius: 3
                        color: Theme.panelMuted

                        Rectangle {
                            width: parent.width * Math.max(0, Math.min(1, EditorState.segmentProgress))
                            height: parent.height
                            radius: parent.radius
                            color: Theme.primary

                            Behavior on width {
                                NumberAnimation { duration: Theme.durationBase; easing.type: Theme.easing }
                            }
                        }
                    }

                    ThemedButton {
                        width: parent.width
                        text: qsTr("Cancelar Recorte")
                        variant: "destructive"
                        glyph: Theme.icons.x
                        onClicked: EditorState.cancelSegmentation()
                    }
                }
            }
        }
    }

    NameDialog {
        id: renameDialog
        acceptText: qsTr("Rename")
        placeholder: qsTr("Clip name")
        onSubmitted: name => EditorState.setClipName(EditorState.selectedTrack, EditorState.selectedClip, name)
    }
}
