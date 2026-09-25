import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Item {
    id: root
    width: parent ? parent.width : 1000
    height: parent ? parent.height : 600

    property var pipeline: App.pipelineManager

    Rectangle {
        anchors.fill: parent
        color: Theme.appBackground

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            Text {
                text: "Estúdio Dark - Pipeline de Produção"
                font.pixelSize: 24
                font.bold: true
                color: Theme.foreground
            }

            ListView {
                id: columnsView
                Layout.fillWidth: true
                Layout.fillHeight: true
                orientation: ListView.Horizontal
                spacing: 16
                model: pipeline.columns

                delegate: Rectangle {
                    width: 300
                    height: columnsView.height
                    color: Theme.panelBackground
                    radius: 8

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        Text {
                            text: modelData.name
                            font.pixelSize: 18
                            font.bold: true
                            color: Theme.foreground
                        }

                        ListView {
                            id: cardsView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 8
                            model: modelData.cards
                            clip: true

                            delegate: Rectangle {
                                width: cardsView.width
                                height: 80
                                color: Theme.panelAccent
                                radius: 6
                                border.color: dropArea.containsDrag ? Theme.accent : "transparent"
                                border.width: 2

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    
                                    Text {
                                        text: modelData.title
                                        font.pixelSize: 14
                                        font.bold: true
                                        color: Theme.foreground
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                    
                                    Item { Layout.fillHeight: true }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    drag.target: parent
                                    
                                    onReleased: {
                                        // Simple drag to move Logic can go here using DropArea
                                        parent.Drag.drop()
                                    }
                                }

                                Drag.active: mouseArea.drag.active
                                Drag.source: modelData
                            }
                        }

                        // Add new card button
                        Button {
                            Layout.fillWidth: true
                            text: "+ Adicionar Ideia"
                            visible: modelData.id === "col_idea"
                            onClicked: {
                                pipeline.addCard(modelData.id, "Nova Ideia " + Math.floor(Math.random() * 100))
                            }
                        }

                        // Direct Publish button (Fase 6A)
                        Button {
                            Layout.fillWidth: true
                            text: "🚀 Publicar nas Redes"
                            visible: modelData.id === "col_published"
                            onClicked: {
                                publishingModal.open()
                            }
                        }
                    }
                }
            }
        }

        // Publishing Modal Dialog
        Popup {
            id: publishingModal
            anchors.centerIn: parent
            width: 460
            height: 700
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
            background: Rectangle {
                color: "#0a0a14"
                radius: 12
                border.color: "#334155"
                border.width: 1
            }
            PublishingPanel {
                anchors.fill: parent
            }
        }
    }
}
