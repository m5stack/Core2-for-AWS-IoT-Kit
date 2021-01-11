import os

class path_helper():
    """
    Function return trust platform home directory
    """
    def __init__(self):
        pass

    @staticmethod
    def get_home_path():
        """
        Function returns trust platform home directory path

        Outputs:
               Returns current_path or None
               Current_path             trust platform home directory
               None                     trust platform home directory not found
        """
        current_path = os.path.realpath(os.getcwd())
        isdir = False
        retries = 5
        while isdir == False:
            if os.path.isdir(os.path.join(current_path, "assets")):
                if os.path.isfile(os.path.join(current_path, "license.txt")):
                    isdir = True
            else:
                current_path = os.path.dirname(current_path)
                retries = retries-1
                if retries == 0:
                    return None

        return current_path