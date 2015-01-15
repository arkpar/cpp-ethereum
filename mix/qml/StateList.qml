import QtQuick 2.2
import QtQuick.Controls.Styles 1.1
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import org.ethereum.qml.ProjectModel 1.0

Rectangle {
	color: "transparent"
	id: stateListContainer
	focus: true
	anchors.topMargin: 10
	anchors.left: parent.left
	height: parent.height
	width: parent.width
	property var stateList: []

	Connections {
		target: ProjectModel
		onProjectClosed: {
			stateListModel.clear();
		}
		onProjectLoaded: {
			if (!projectData.states)
				projectData.states = [];
			var items = projectData.states;
			for(var i = 0; i < items.length; i++) {
				stateListModel.append(items[i]);
				stateList.push(items[i])
			}
		}
		onProjectSaving: {
			projectData.states = stateList;
		}
	}

	ListView {
		anchors.top: parent.top
		height: parent.height
		width: parent.width
		model: stateListModel
		delegate: renderDelegate
	}

	Button {
		anchors.bottom: parent.bottom
		action: addStateAction
	}

	StateDialog {
		id: stateDialog
		onAccepted: {
			var item = stateDialog.getItem();
			if (stateDialog.stateIndex < stateListModel.count) {
				stateList[stateDialog.stateIndex] = item;
				stateListModel.set(stateDialog.stateIndex, item);
			} else {
				stateList.push(item);
				stateListModel.append(item);
			}

			stateListModel.save();
		}
	}

	ListModel {
		id: stateListModel

		function addState() {
			var item = {
				title: "",
				balance: "100000000000000000000000000",
				transactions: []
			};
			stateDialog.open(stateListModel.count, item);
		}

		function editState(index) {
			stateDialog.open(index, stateList[index]);
		}

		function runState(index) {
			var item = stateList[index];
			clientModel.debugState(item);
		}

		function deleteState(index) {
			stateListModel.remove(index);
			stateList.splice(index, 1);
			save();
		}

		function save() {
			ProjectModel.saveProject();
		}
	}

	Component {
		id: renderDelegate
		Item {
			id: wrapperItem
			height: 20
			width: parent.width
			RowLayout {
				anchors.fill: parent
				Text {
					Layout.fillWidth: true
					Layout.fillHeight: true
					text: title
					font.pointSize: 12
					verticalAlignment: Text.AlignBottom
				}
				ToolButton {
					text: qsTr("Edit");
					Layout.fillHeight: true
					onClicked: stateListModel.editState(index);
				}
				ToolButton {
					text: qsTr("Delete");
					Layout.fillHeight: true
					onClicked: stateListModel.deleteState(index);
				}
				ToolButton {
					text: qsTr("Run");
					Layout.fillHeight: true
					onClicked: stateListModel.runState(index);
				}
			}
		}
	}

	Action {
		id: addStateAction
		text: "&Add State"
		shortcut: "Ctrl+T"
		enabled: codeModel.hasContract && !clientModel.running;
		onTriggered: stateListModel.addState();
	}
}

