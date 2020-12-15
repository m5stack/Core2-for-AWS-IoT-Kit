import sys
import subprocess
from collections import namedtuple

class sys_helper():
    """
    Class with methods which help with wrappers and 
    helpers for modules like python sys, path, subprocess 
    """
    def __init__(self):
        pass        
    
    @staticmethod
    def run_subprocess_cmd(cmd, sys_shell=False, sys_newlines=True):
        """
        Runs a command on ternimal/command prompt. Uses subprocess module.
        
        Inputs:
            cmd                     Command to be processed
            sys_shell               If True, the command will be executed through the shell.
            sys_newlines            If True, stdout and stderr are opened as text files, all
                                    line terninations are seen as '\n' by the python program

        Outputs:
            Returns a namedtuple of ['returncode', 'stdout', 'stderr']

            returncode              Returns error code from terminal
            stdout                  All standard outputs are accumulated here. 
            srderr                  All error and warning outputs 

        Examples:
            To run "python test.py -f sheet.csv"
            subProcessOut = syshelper.run_subprocess_cmd([sys.executable, "test.py", "-f", "sheet.csv"])
            print(subProcessOut.stdout)
            print(subProcessOut.stderr)
            print(subProcessOut.returncode)
        """
        proc = subprocess.Popen(cmd,
                stdout= subprocess.PIPE,
                stderr= subprocess.PIPE,
                universal_newlines= sys_newlines,
                shell= sys_shell
        )
        stdout, stderr = proc.communicate()
        subProcessOut = namedtuple("subProcessOut", ['returncode', 'stdout', 'stderr'])
        return subProcessOut(proc.returncode, stdout, stderr)