import os, sys
from collections import namedtuple
import pkg_resources
import subprocess
import json

class requirements_installer():
    def __init__(self, requirements_file):
        self.file_name = requirements_file
        self.json_piplist_path = ""
        self.__install_dependency()

    def count_valid_lines(self, file_name):
        count =0
        with open(file_name, 'r') as f:
            sline = f.readline()
            while sline:
                if(len(sline.strip().replace("\n", "").replace("\r", "")) >0):
                    count +=1
                else:
                    pass
                sline = f.readline()
        return count

    def versionCompare(self, ver1, ver2):
        arr1 = ver1.split(".")
        arr2 = ver2.split(".")
        i = 0

        while(i < len(arr1)):
            if int(arr2[i]) > int(arr1[i]):
                return -1

            if int(arr1[i]) > int(arr2[i]):
                return 1

            i += 1
        return 0

    def check_pip_list(self):
        if os.path.isfile(self.json_piplist_path):
            # File Exists
            pass
        else:
            # Pip list json file does not exist
            print("Creating outdated package list, please wait...")
            proc = subprocess.Popen([sys.executable, "-m", "pip", "list", "--outdated", "--format", "json", "--disable-pip-version-check"],
                    stdout= subprocess.PIPE,
                    stderr= subprocess.PIPE,
                    universal_newlines= True,
                    shell = False)
            stdout, stderr = proc.communicate()
            returncode = proc.returncode

            #stdout now has json information
            with open(self.json_piplist_path, "w") as f:
                f.write(stdout)

        # Pip list json obj
        print("Fetching all installed packages...")
        proc = subprocess.Popen([sys.executable, "-m", "pip", "list", "--format", "json", "--disable-pip-version-check"],
                stdout= subprocess.PIPE,
                stderr= subprocess.PIPE,
                universal_newlines= True,
                shell = False)
        stdout, stderr = proc.communicate()
        returncode = proc.returncode

        return json.loads(stdout)

    def parse_pip_list(self, module_name):
        with open(self.json_piplist_path) as json_file:
            json_object = json.load(json_file)

        latest = "0.0.0"

        for dict in json_object:
            if dict['name'] == module_name:
                latest = dict['latest_version']

        for dict in self.pip_list_json:
            if dict['name'] == module_name:
                installed = dict['version']

        if self.versionCompare(installed, latest) >= 0:
            return False, "", ""

        return True, installed, latest

    def __install_dependency(self):
        numValidLines = self.count_valid_lines(self.file_name)

        with open(self.file_name, mode='r') as f:
            package = f.readline().strip()
            package_list = []
            package_list += [p.project_name for p in pkg_resources.working_set]
            cnt = 1

            while package:
                print("Checking module ({} of {}): {}".format(cnt, numValidLines, package.strip()))
                cnt +=1
                if not package in package_list:
                    proc = subprocess.Popen([sys.executable, "-m", "pip", "install", "--upgrade", package],
                            stdout= subprocess.PIPE,
                            stderr= subprocess.PIPE,
                            universal_newlines= True,
                            shell = False)
                    stdout, stderr = proc.communicate()
                    print(stdout)
                    print(stderr)

                print("-----------------------------------------")
                package = f.readline().strip()

            print("Completed checking/installing package dependencies\r\n")
