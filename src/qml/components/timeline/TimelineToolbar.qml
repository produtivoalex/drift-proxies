import QtQuick
import QtQuick.Controls.Basic
import Drift
import ".."

// Timeline toolbar: transport/edit actions, scene badge, snap/ripple toggles
// and zoom controls. Zoom and the time readout are read from and written back
// to the owning TimelinePanel via `panel`. New tracks are added from the
// plus button above the track headers.
Item {
    id: toolbar

    // Owning TimelinePanel; provides zoom (read/write via setZoom), zoom bounds and formatTime.
    property var panel

    height: Theme.timelineToolbarHeight

    // Appends an action's current binding to its tooltip. Every action here has one,
    // but only the header's Save button used to show it, so the keyboard route to
    // anything on this toolbar was undiscoverable. Rebound keys follow automatically
    // because shortcutFor reads the live map.
    function withShortcut(label, actionId) {
        const key = EditorState.shortcutFor(actionId)
        return key.length > 0 ? qsTr("%1 (%2)").arg(label).arg(key) : label
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: Theme.panelBorder
    }

    Row {
        id: leftControls
        anchors.left: parent.left
        anchors.leftMargin: Theme.spacingLg
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.spacingXs
        // Never runs under the right-hand controls; buttons past the
        // available width are clipped rather than overlapping.
        width: Math.max(0, rightControls.x - x - Theme.spacingLg)
        clip: true

        IconButton {
            glyph: EditorState.playing ? Theme.icons.pause : Theme.icons.play
            variant: "text"
            tooltip: EditorState.playing ? qsTr("Pause") : qsTr("Play")
            onClicked: EditorState.togglePlayback()
        }

        Text {
            text: toolbar.panel.formatTime(EditorState.playheadSeconds) + " / " + toolbar.panel.formatTime(EditorState.durationSeconds)
            color: Theme.mutedForeground
            font.family: Theme.monoFontFamily
            font.pixelSize: Theme.fontSizeXs
            anchors.verticalCenter: parent.verticalCenter
        }

        Rectangle {
            width: Theme.borderWidth
            height: Theme.spacing3xl
            color: Theme.panelBorder
            anchors.verticalCenter: parent.verticalCenter
        }

        // CapCut-style exclusive tool modes: Select is the default; cut tools
        // stay sticky until you switch back via Select (or V). Clicking the
        // active cut tool again also returns to Select.
        IconButton {
            glyph: Theme.icons.mousePointer
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Select — normal editing"), "selectTool")
            active: toolbar.panel.timelineTool === ""
            onClicked: toolbar.panel.timelineTool = ""
        }
        IconButton {
            glyph: Theme.icons.scissors
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Cut mode — click a clip to split it"), "bladeTool")
            active: toolbar.panel.timelineTool === "split"
            onClicked: toolbar.panel.timelineTool = toolbar.panel.timelineTool === "split" ? "" : "split"
        }

        // A/V actions belong beside the primary clip tools rather than at the end
        // of the toolbar. On narrower desktop windows the old placement was clipped,
        // which made an already-implemented feature look as if it did not exist.
        // Keep only the action that is relevant to the current selection visible:
        // embedded A/V -> show audio on its own linked lane; linked pair -> Unlink.
        IconButton {
            glyph: Theme.icons.audioLines
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Show audio on separate track"), "separateAudio")
            visible: EditorState.separateAudioAvailable
            enabled: EditorState.separateAudioAvailable
            onClicked: EditorState.separateAudioFromSelection()
        }
        IconButton {
            glyph: Theme.icons.unlink
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Unlink video and audio"), "unlink")
            visible: EditorState.unlinkAvailable
            enabled: EditorState.unlinkAvailable
            onClicked: EditorState.unlinkSelectedClips()
        }

        IconButton {
            glyph: Theme.icons.trimStart
            variant: "text"
            tooltip: qsTr("Trim start — click a clip to drop everything left of the cut")
            active: toolbar.panel.timelineTool === "trimStart"
            onClicked: toolbar.panel.timelineTool = toolbar.panel.timelineTool === "trimStart" ? "" : "trimStart"
        }
        IconButton {
            glyph: Theme.icons.trimEnd
            variant: "text"
            tooltip: qsTr("Trim end — click a clip to drop everything right of the cut")
            active: toolbar.panel.timelineTool === "trimEnd"
            onClicked: toolbar.panel.timelineTool = toolbar.panel.timelineTool === "trimEnd" ? "" : "trimEnd"
        }
        // Undo/redo and the clipboard group come before the situational actions
        // below. This Row clips, and at the minimum window width there is only room
        // for roughly the first two thirds of it — so the buttons that must never
        // vanish have to be the ones nearest the left edge. Everything past the
        // second separator is also reachable from the overflow menu.
        IconButton {
            glyph: Theme.icons.undo
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Undo"), "undo")
            onClicked: EditorState.undo()
            enabled: EditorState.undoAvailable
        }
        IconButton {
            glyph: Theme.icons.redo
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Redo"), "redo")
            onClicked: EditorState.redo()
            enabled: EditorState.redoAvailable
        }
        IconButton {
            glyph: Theme.icons.trash
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Delete clip"), "delete")
            onClicked: EditorState.deleteSelectedClip()
        }
        IconButton {
            glyph: Theme.icons.chevronsRightLeft
            variant: "text"
            tooltip: qsTr("Ripple delete (Shift+Delete)")
            onClicked: EditorState.rippleDeleteSelectedClip()
        }
        IconButton {
            glyph: Theme.icons.wand
            variant: "text"
            tooltip: qsTr("Corte Inteligente de Silêncio (Smart Cut)")
            visible: EditorState.selectedClip >= 0
            onClicked: {
                const count = EditorState.removeSilenceFromSelectedClip()
                if (count > 0)
                    Toasts.success(qsTr("%1 pausas/silêncios removidos!").arg(count))
                else
                    Toasts.info(qsTr("Nenhum silêncio relevante detectado no clipe."))
            }
        }
        IconButton {
            glyph: Theme.icons.copy
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Copy selection"), "copy")
            onClicked: EditorState.copySelection()
        }
        IconButton {
            glyph: Theme.icons.clipboardPaste
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Paste at current time"), "paste")
            onClicked: EditorState.pasteAtPlayhead()
        }
        IconButton {
            glyph: Theme.icons.copyPlus
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Duplicate clip"), "duplicate")
            onClicked: EditorState.duplicateSelectedClip()
        }

        Rectangle {
            width: Theme.borderWidth
            height: Theme.spacing3xl
            color: Theme.panelBorder
            anchors.verticalCenter: parent.verticalCenter
        }

        IconButton {
            glyph: Theme.icons.bookmark
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Add/remove bookmark at current time"),
                                         "toggleBookmark")
            onClicked: EditorState.toggleBookmarkAtPlayhead()
        }
        IconButton {
            glyph: Theme.icons.setStart
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Mark work area in"), "markIn")
            active: EditorState.workAreaInSeconds >= 0
            onClicked: EditorState.markWorkAreaIn()
        }
        IconButton {
            glyph: Theme.icons.setEnd
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Mark work area out"), "markOut")
            active: EditorState.workAreaOutSeconds >= 0
            onClicked: EditorState.markWorkAreaOut()
        }
        IconButton {
            glyph: Theme.icons.repeat
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Loop work area playback"), "toggleLoop")
            active: EditorState.loopWorkAreaEnabled
            enabled: EditorState.workAreaActive
            onClicked: EditorState.toggleLoopWorkArea()
        }
        IconButton {
            glyph: Theme.icons.x
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Clear work area"), "clearInOut")
            enabled: EditorState.workAreaInSeconds >= 0 || EditorState.workAreaOutSeconds >= 0
            onClicked: EditorState.clearWorkArea()
        }
        IconButton {
            glyph: Theme.icons.linkTwo
            variant: "text"
            tooltip: toolbar.withShortcut(qsTr("Merge adjacent clips"), "merge")
            enabled: EditorState.mergeAvailable
            onClicked: EditorState.mergeSelectedClips()
        }
        IconButton {
            glyph: Theme.icons.snowflake
            variant: "text"
            tooltip: qsTr("Freeze frame at current time")
            onClicked: EditorState.freezeFrameAtPlayhead()
        }
    }

    // Reachable form of whatever `leftControls` had to clip. Appears only when the
    // Row actually overflows, so at a normal window width nothing changes.
    IconButton {
        id: overflowButton
        anchors.left: leftControls.left
        anchors.leftMargin: Math.max(0, leftControls.width - width)
        anchors.verticalCenter: parent.verticalCenter
        visible: leftControls.implicitWidth > leftControls.width
        glyph: Theme.icons.ellipsis
        variant: "text"
        tooltip: qsTr("More edit actions")
        active: overflowMenu.opened
        onClicked: overflowMenu.opened ? overflowMenu.close()
                                       : overflowMenu.popup(0, overflowButton.height)

        ThemedContextMenu {
            id: overflowMenu

            ThemedMenuItem {
                text: qsTr("Add/remove bookmark at current time")
                icon.name: Theme.icons.bookmark
                onTriggered: EditorState.toggleBookmarkAtPlayhead()
            }
            ThemedMenuItem {
                text: toolbar.withShortcut(qsTr("Mark work area in"), "markIn")
                icon.name: Theme.icons.setStart
                onTriggered: EditorState.markWorkAreaIn()
            }
            ThemedMenuItem {
                text: toolbar.withShortcut(qsTr("Mark work area out"), "markOut")
                icon.name: Theme.icons.setEnd
                onTriggered: EditorState.markWorkAreaOut()
            }
            ThemedMenuItem {
                text: toolbar.withShortcut(qsTr("Loop work area playback"), "toggleLoop")
                icon.name: Theme.icons.repeat
                enabled: EditorState.workAreaActive
                onTriggered: EditorState.toggleLoopWorkArea()
            }
            ThemedMenuItem {
                text: toolbar.withShortcut(qsTr("Clear work area"), "clearInOut")
                icon.name: Theme.icons.x
                enabled: EditorState.workAreaInSeconds >= 0 || EditorState.workAreaOutSeconds >= 0
                onTriggered: EditorState.clearWorkArea()
            }
            ThemedMenuSeparator { }
            ThemedMenuItem {
                text: qsTr("Show audio on separate track")
                icon.name: Theme.icons.audioLines
                enabled: EditorState.separateAudioAvailable
                onTriggered: EditorState.separateAudioFromSelection()
            }
            ThemedMenuItem {
                text: qsTr("Unlink video and audio")
                icon.name: Theme.icons.unlink
                enabled: EditorState.unlinkAvailable
                onTriggered: EditorState.unlinkSelectedClips()
            }
            ThemedMenuItem {
                text: qsTr("Merge adjacent clips")
                icon.name: Theme.icons.linkTwo
                enabled: EditorState.mergeAvailable
                onTriggered: EditorState.mergeSelectedClips()
            }
            ThemedMenuItem {
                text: qsTr("Freeze frame at current time")
                icon.name: Theme.icons.snowflake
                onTriggered: EditorState.freezeFrameAtPlayhead()
            }
            ThemedMenuSeparator { }
            ThemedMenuItem {
                text: qsTr("Add adjustment layer")
                icon.name: Theme.icons.wand
                onTriggered: EditorState.addAdjustmentClip(-1, -1)
            }
        }
    }

    Rectangle {
        id: sceneBadge

        // Sits between the two button groups, which are anchored to the
        // toolbar edges and grow freely. Prefer dead centre, but slide
        // aside to stay clear of them, and drop out entirely once the
        // gap can no longer fit the badge.
        readonly property real gapStart: leftControls.x + leftControls.width + 12
        readonly property real gapEnd: rightControls.x - 12

        x: Math.max(gapStart, Math.min((toolbar.width - width) / 2, gapEnd - width))
        anchors.verticalCenter: parent.verticalCenter
        visible: gapEnd - gapStart >= width
        width: sceneRow.implicitWidth + 20
        height: 26
        radius: Theme.radiusSm
        color: "transparent"
        border.width: 1
        border.color: Qt.rgba(Theme.panelForeground.r, Theme.panelForeground.g, Theme.panelForeground.b, 0.1)

        Row {
            id: sceneRow
            anchors.centerIn: parent
            spacing: 6

            Text {
                text: qsTr("Scene 1")
                color: Theme.panelForeground
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontSizeSm
                anchors.verticalCenter: parent.verticalCenter
            }
            IconGlyph {
                glyph: Theme.icons.layers
                iconSize: 14
                iconColor: Theme.mutedForeground
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Row {
        id: rightControls
        anchors.right: parent.right
        anchors.rightMargin: Theme.spacingLg
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.spacingSm

        IconButton {
            id: autoBeatButton
            glyph: Theme.icons.music
            variant: "text"
            tooltip: EditorState.beatAnalysisRunning
                     ? qsTr("Analisando batidas musicais...")
                     : (EditorState.beatSnapActive
                        ? qsTr("Auto-Beats ativo (%1 batidas, %2 BPM) — Clique para alternar ou botão direito para opções")
                            .arg(EditorState.beatCount)
                            .arg(Math.round(EditorState.detectedBpm))
                        : qsTr("Auto-Beats: Detectar batidas e alinhar cortes no ritmo musical (Clique direito para opções)"))
            active: EditorState.beatSnapActive
            onClicked: {
                if (EditorState.beatCount === 0) {
                    Toasts.info(qsTr("Detectando batidas musicais na timeline..."))
                    EditorState.detectAndMarkBeats(-1, -1, "onsets", 0.35)
                } else {
                    EditorState.toggleBeatSnap()
                }
            }

            Rectangle {
                visible: EditorState.beatSnapActive
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: 2
                anchors.rightMargin: 2
                width: 6
                height: 6
                radius: 3
                color: "#FFD600"
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                cursorShape: Qt.PointingHandCursor
                onClicked: beatMenu.popup(autoBeatButton, 0, autoBeatButton.height)
            }

            ThemedContextMenu {
                id: beatMenu

                ThemedMenuItem {
                    text: qsTr("🥁 Detectar Batidas (Kicks & Snares)")
                    icon.name: Theme.icons.music
                    onTriggered: {
                        Toasts.info(qsTr("Analisando batidas (kicks/snares)..."))
                        EditorState.detectAndMarkBeats(-1, -1, "onsets", 0.35)
                    }
                }
                ThemedMenuItem {
                    text: qsTr("🎼 Detectar Compassos (Bars)")
                    icon.name: Theme.icons.audioLines
                    onTriggered: {
                        Toasts.info(qsTr("Analisando compassos musicais..."))
                        EditorState.detectAndMarkBeats(-1, -1, "bars", 0.35)
                    }
                }
                ThemedMenuSeparator { }
                ThemedMenuItem {
                    text: qsTr("✂️ Cortar Clipe Selecionado nas Batidas")
                    icon.name: Theme.icons.scissors
                    enabled: EditorState.selectedClip >= 0 && EditorState.beatCount > 0
                    onTriggered: {
                        const cuts = EditorState.splitClipAtBeats(-1, -1)
                        if (cuts > 0)
                            Toasts.success(qsTr("Clipe cortado em %1 batidas musicais!").arg(cuts))
                        else
                            Toasts.info(qsTr("Nenhuma batida encontrada dentro do clipe selecionado."))
                    }
                }
                ThemedMenuItem {
                    text: qsTr("📌 Converter Batidas em Marcadores (Bookmarks)")
                    icon.name: Theme.icons.bookmark
                    enabled: EditorState.beatCount > 0
                    onTriggered: {
                        const count = EditorState.convertBeatsToBookmarks("beats", 0.35)
                        Toasts.success(qsTr("%1 marcadores de batida criados na timeline!").arg(count))
                    }
                }
                ThemedMenuSeparator { }
                ThemedMenuItem {
                    text: qsTr("🗑️ Limpar Batidas da Timeline")
                    icon.name: Theme.icons.trash
                    enabled: EditorState.beatCount > 0
                    onTriggered: {
                        EditorState.clearBeatAnalysis()
                        Toasts.info(qsTr("Batidas removidas da timeline."))
                    }
                }
            }
        }

        IconButton {
            id: magnetButton
            glyph: Theme.icons.magnet
            variant: "text"
            tooltip: qsTr("Toggle snapping")
            active: EditorState.snapEnabled
            onClicked: EditorState.snapEnabled = !EditorState.snapEnabled
        }
        IconButton {
            id: rippleButton
            glyph: Theme.icons.foldHorizontal
            variant: "text"
            tooltip: qsTr("Close gaps when trimming")
            active: EditorState.rippleEnabled
            onClicked: EditorState.rippleEnabled = !EditorState.rippleEnabled
        }
        IconButton {
            id: overlapButton
            glyph: Theme.icons.option
            variant: "text"
            tooltip: qsTr("Allow clip overlap")
            active: EditorState.allowClipOverlap
            onClicked: EditorState.allowClipOverlap = !EditorState.allowClipOverlap
        }

        Rectangle {
            width: Theme.borderWidth
            height: Theme.spacing3xl
            color: Theme.panelBorder
            anchors.verticalCenter: parent.verticalCenter
        }

        IconButton {
            glyph: Theme.icons.zoomOut
            variant: "text"
            tooltip: qsTr("Zoom out")
            onClicked: toolbar.panel.setZoom(toolbar.panel.zoom / 1.5)
        }
        ThemedSlider {
            id: zoomSlider
            label: qsTr("Timeline zoom")
            width: 112
            anchors.verticalCenter: parent.verticalCenter
            // Logarithmic mapping so the wide zoom range stays controllable.
            from: 0
            to: 1
            value: Math.log(toolbar.panel.zoom / toolbar.panel.minZoom) / Math.log(toolbar.panel.maxZoom / toolbar.panel.minZoom)
            onMoved: toolbar.panel.setZoom(
                toolbar.panel.minZoom * Math.pow(toolbar.panel.maxZoom / toolbar.panel.minZoom, value))
            // There was no zoom readout anywhere, so the current level
            // was simply unknowable.
            valueFormatter: function () {
                return qsTr("Zoom %1×").arg(toolbar.panel.zoom.toFixed(2))
            }
        }

        // Numeric zoom level, and a click target to return to 1×.
        Text {
            anchors.verticalCenter: parent.verticalCenter
            width: 44
            text: toolbar.panel.zoom.toFixed(2) + "×"
            color: Theme.mutedForeground
            font.family: Theme.monoFontFamily
            font.pixelSize: Theme.fontSizeTick
            horizontalAlignment: Text.AlignHCenter

            ThemedToolTip {
                text: qsTr("Zoom level — click to reset to 1×. Ctrl+wheel over the timeline also zooms.")
                visible: zoomLabelMouse.containsMouse
            }

            MouseArea {
                id: zoomLabelMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: toolbar.panel.setZoom(1.0)
            }
        }
        IconButton {
            glyph: Theme.icons.zoomIn
            variant: "text"
            tooltip: qsTr("Zoom in")
            onClicked: toolbar.panel.setZoom(toolbar.panel.zoom * 1.5)
        }
        IconButton {
            glyph: Theme.icons.zoomFit
            variant: "text"
            tooltip: qsTr("Fit timeline in view")
            onClicked: toolbar.panel.fitZoom()
        }
    }
}
