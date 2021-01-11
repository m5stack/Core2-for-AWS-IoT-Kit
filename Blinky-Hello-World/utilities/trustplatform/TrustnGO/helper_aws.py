import csv
import sys, os

home_path = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(os.getcwd())))))
module_path = os.path.join(home_path, 'assets', 'python')
if not module_path in sys.path:
    sys.path.append(module_path)

from trustplatform import sys_helper

account_CSV = "AWS_test_account_credentials.csv"
ACCOUNT_CREDENTIALS = os.path.join(home_path, 'docs', account_CSV)

#Shows the current configuration of the AWS for Access key, secret key and the region.
def list_current_configuration():
    subProcessOut = sys_helper.run_subprocess_cmd(cmd=["aws", "configure", "list"], sys_shell=True)
    if subProcessOut.returncode == 0:
        print(subProcessOut.stdout)
    else:
        print("Getting the current configuration failed")
        return 'danger'

    print("\nGetting aws endpoint...", end='')
    subProcessOut = sys_helper.run_subprocess_cmd(cmd=["aws", "iot", "describe-endpoint"], sys_shell=True)
    if subProcessOut.returncode == 0:
        if subProcessOut.stdout is not None:
            print(subProcessOut.stdout.replace('\n',"").replace("    ",""))
        else:
            print("No endpoint returned, check under AWS account -> IoT core -> Settings")
    else:
        print("Verify account csv file contents, region and do Config AWS CLI!")
        return 'danger'

    return 'success'

#Configures AWS CLI for Access key, secret key and the region.
def configure_aws_cli(selected_region):
    try:
        with open(ACCOUNT_CREDENTIALS, mode='r') as csv_file:
            csv_reader = csv.DictReader(csv_file)
            for row in csv_reader:
                access_key_id = row["Access key ID"]
                secret_key_access = row["Secret access key"]

            #Setting the aws cli for the access key
            print("Setting aws access key...", end='')
            subProcessOut = sys_helper.run_subprocess_cmd(cmd=["aws", "configure", "set", "aws_access_key_id", access_key_id], sys_shell=True)
            if subProcessOut.returncode != 0:
                print("Setting AWS access key failed\r\n")
                return 'danger'
            print('OK')

            print("Setting aws secret access key...", end='')
            subProcessOut = sys_helper.run_subprocess_cmd(cmd=["aws", "configure", "set", "aws_secret_access_key", secret_key_access], sys_shell=True)
            if subProcessOut.returncode != 0:
                print("Setting AWS secret key failed\r\n")
                return 'danger'
            print('OK')

            print("Setting aws region...", end='')
            subProcessOut = sys_helper.run_subprocess_cmd(cmd=["aws", "configure", "set", "region", selected_region], sys_shell=True)
            if subProcessOut.returncode != 0:
                print("Setting AWS region failed\r\n")
                return 'danger'
            print('OK')

        return list_current_configuration()
    except:
        print("Verify account csv file existence and its content!")
        return 'danger'

