import os
import argparse
import json
import datetime
import sys
from PyQt5 import QtGui, QtWidgets, uic
from google.cloud import pubsub_v1

qtUiFile = "gcp_qt.ui"

class Ui(QtWidgets.QMainWindow):
    """Basic Message Visualizer gui"""
    def __init__(self, project_id, subscription_id):
        super(Ui, self).__init__() # Call the inherited classes __init__ method
        self.load_UI()
        self.setWindowIcon(QtGui.QIcon('shield.ico'))

        self.subscriber = pubsub_v1.SubscriberClient()
        self.subscription_path = self.subscriber.subscription_path(project_id, subscription_id)
        self.subscriber.subscribe(self.subscription_path, callback=self.subscription_callback)

    def load_UI(self):
        uic.loadUi(qtUiFile, self) # Load the .ui file

        # Setup treeview
        self.treeView.setRootIsDecorated(False)
        self.treeView.setAlternatingRowColors(True)

        self.model = QtGui.QStandardItemModel()
        self.model.setHorizontalHeaderLabels(['Date/Time', 'Serial Number', 'Led Status'])
        self.treeView.setModel(self.model)
        
    def add_data(self, date_time, sno, led_status):
        self.model.insertRow(0)
        self.model.setData(self.model.index(0, 0), date_time)
        self.model.setData(self.model.index(0, 1), sno)
        self.model.setData(self.model.index(0, 2), led_status)

    def subscription_callback(self, message):
        """Receive messages from the subscription"""
        data = json.loads(message.data)

        self.LE_project.setText(message.attributes['projectId'])
        self.LE_registry.setText(message.attributes['deviceRegistryId'])
        self.LE_region.setText(message.attributes['deviceRegistryLocation'])

        sample_values = [message.attributes['deviceId']] + \
                        ['{}: {}'.format(k, v) for k, v in data.items() if k != 'timestamp']
        sample_time = datetime.datetime.fromtimestamp(data['timestamp'])
        serialno, led_status = sample_values

        self.add_data(sample_time.strftime("%H:%M:%S"), serialno, led_status)

        message.ack()

def run_gcp_gui(credential_file, subscription = 'data-view'):

    if credential_file is not None:
        os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = credential_file

    with open(os.environ["GOOGLE_APPLICATION_CREDENTIALS"]) as f:
        credentials = json.load(f)
        project = credentials['project_id']

    app = QtWidgets.QApplication(sys.argv)
    window = Ui(project, subscription)
    window.show() # Show the GUI
    sys.exit(app.exec_())


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='GCP Example Gui')
    parser.add_argument('--subscription', help='Topic Subscription')
    parser.add_argument('--creds', help='Credential Json File')
    args = parser.parse_args()

    run_gcp_gui(args.creds, args.subscription)


