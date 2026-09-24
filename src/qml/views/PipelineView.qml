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
        color: Theme.colorBackground

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            Text {
                text: "Estúdio Dark - Pipeline de Produção"
                font.pixelSize: 24
                font.bold: true
                color: Theme.colorForeground
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
                    color: Theme.colorPanelBackground
                    radius: 8

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        Text {
                            text: modelData.name
                            font.pixelSize: 18
                            font.bold: true
                            color: Theme.colorForeground
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
                                color: Theme.colorButton
                                radius: 6
                                border.color: dropArea.containsDrag ? Theme.colorAccent : "transparent"
                                border.width: 2

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    
                                    Text {
                                        text: modelData.title
                                        font.pixelSize: 14
                                        font.bold: true
                                        color: Theme.colorForeground
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
                    }
                }
            }
        }
    }
}
