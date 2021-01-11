import time
import json
import boto3
import botocore
import sys
from argparse import ArgumentParser
from PyQt5 import QtGui, QtWidgets, QtCore, uic

qtUiFile = "aws_trust.ui"

class AWSZTKitError(RuntimeError):
    pass

def aws_interact_gui(aws_profile, thing_id):
    print('Execution started')

    # Create an AWS session with the credentials from the specified profile
    print('\nInitializing AWS IoTDataPlane client')
    try:
        aws_session = boto3.session.Session(profile_name=aws_profile)
    except botocore.exceptions.ProfileNotFound:
        if aws_profile == 'default':
            raise AWSZTKitError('AWS profile not found. Please make sure you have the AWS CLI installed and run "aws configure" to setup profile.')
        else:
            raise AWSZTKitError('AWS profile not found. Please make sure you have the AWS CLI installed and run "aws configure --profile %s" to setup profile.' % aws_profile)
    # Create a client to the AWS IoT data service
    aws_iot_data = aws_session.client('iot-data')

    print('    Profile:  %s' % aws_session.profile_name)
    print('    Region:   %s' % aws_session.region_name)
    print('    Endpoint: %s' % aws_iot_data._endpoint)

    # Create the GUI
    app = QtWidgets.QApplication(sys.argv)
    window = Ui(aws_iot_data=aws_iot_data, thing_name=thing_id)
    window.show() # Show the GUI
    sys.exit(app.exec_())

class Ui(QtWidgets.QMainWindow):
    def __init__(self, aws_iot_data, thing_name):
        super(Ui, self).__init__() # Call the inherited classes __init__ method
        self.aws_iot_data = aws_iot_data
        self.thing_name = thing_name
        self.state = ''
        self.load_UI()
        self.rBtnOn.toggled.connect(self.led_click)
        self.lineEdit.setText(thing_name)
        self.on_update()

    def load_UI(self):
        uic.loadUi(qtUiFile, self) # Load the .ui file
        self.setWindowIcon(QtGui.QIcon('shield.ico')) # Set window icon
        self.show()

    def led_click(self, led_index):
        if self.rBtnOn.isChecked():
            led_value = 'on'
        else:
            led_value = 'off'

        msg = {'state' : {'desired' : {('led1') : led_value}}}
        # print('update_thing_shadow(): %s\n' % json.dumps(msg))
        self.aws_iot_data.update_thing_shadow(thingName=self.thing_name, payload=json.dumps(msg))

    def on_update(self):
        try:
            response = self.aws_iot_data.get_thing_shadow(thingName=self.thing_name)

            self.shadow = json.loads(response['payload'].read().decode('ascii'))
            curr_state = self.shadow['state']

            if curr_state != self.state:
                self.state = curr_state
                self.lineEdit.setText(self.thing_name)

                print('get_thing_shadow(): state changed\n%s\n' % json.dumps(self.shadow, sort_keys=True))

                if 'desired' in curr_state:
                    led_label = 'led1'
                    if led_label in curr_state['desired']:
                        if curr_state['desired'][led_label] == 'on':
                            self.rBtnOn.setChecked(True)
                        else:
                            self.rBtnOn.setChecked(False)

        except botocore.exceptions.ClientError as e:
            if e.response['Error']['Code'] == 'ResourceNotFoundException':
                if self.state != 'no thing shadow':
                    self.state = 'no thing shadow'
                    status_msg = e.response['Error']['Message'] + '. The device may not have successfully connected to AWS yet.'
                    self.lineEdit.setText(status_msg)
                    print(status_msg)
            else:
                raise

        QtCore.QTimer.singleShot(2000, self.on_update)

if __name__ == '__main__':
    # Create argument parser to document script use
    parser = ArgumentParser(description='Interact with the thing through AWS')
    parser.add_argument(
        '--profile',
        dest='profile',
        nargs='?',
        default='default',
        metavar='name',
        help='AWS profile name (uses default if omitted)')
    parser.add_argument('--thingid')
    args = parser.parse_args()

    try:
        aws_interact_gui(aws_profile=args.profile, thing_id = args.thingid)
    except AWSZTKitError as e:
        # Capture kit errors and just display message instead of full stack trace
        print(e)
