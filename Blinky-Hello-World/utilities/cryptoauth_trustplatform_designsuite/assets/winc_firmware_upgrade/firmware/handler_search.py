'''
Simple program to get a hint where simple programs might be installed by chasing thru registry,
does not deal with things like word which are beyonf complicated.

Pass in extention to check and a hint at what program you want.

Returns 0 if found.
2 for parm error
1 for not found


Eg

C:\work_repos\>python handler_search.py cpP studio
""C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe"

C:\work_repos\>python handler_search.py cpP atmelstudio
"C:\Program Files (x86)\Atmel\Studio\7.0\atmelstudio.exe"

'''
import sys
import os
import winreg

roots_hives = {
    "HKEY_CLASSES_ROOT": winreg.HKEY_CLASSES_ROOT,
    "HKEY_CURRENT_USER": winreg.HKEY_CURRENT_USER,
    "HKEY_LOCAL_MACHINE": winreg.HKEY_LOCAL_MACHINE,
    "HKEY_USERS": winreg.HKEY_USERS,
    "HKEY_PERFORMANCE_DATA": winreg.HKEY_PERFORMANCE_DATA,
    "HKEY_CURRENT_CONFIG": winreg.HKEY_CURRENT_CONFIG,
    "HKEY_DYN_DATA": winreg.HKEY_DYN_DATA
}

def join(path, *paths):
    path = path.strip('/\\')
    paths = map(lambda x: x.strip('/\\'), paths)
    paths = list(paths)
    result = os.path.join(path, *paths)
    result = result.replace('/', '\\')
    return result

def parse_key(key):
    key = key.upper()
    aparts = key.split('\\')
    parts = list(filter(None, aparts))
    root_hive_name = parts[0]
    root_hive = roots_hives.get(root_hive_name)
    partial_key = '\\'.join(parts[1:])

    if not root_hive:
        raise Exception('root hive "{}" was not found'.format(root_hive_name))

    return partial_key, root_hive


def get_all_values(key):
    data = {}
    data[0] = [[''],['']]
    try:
        partial_key, root_hive = parse_key(key)

        with winreg.ConnectRegistry(None, root_hive) as reg:
            with winreg.OpenKey(reg, partial_key) as key_object:
                i = 0
                while True:
                    try:
                        ret = winreg.EnumValue(key_object, i)
                        if ret[2] == winreg.REG_EXPAND_SZ:
                            if ret[0] == '':
                                data[i] = ["(Default)", expandvars(ret[1])]
                            else:
                                data[i] = [ret[0], expandvars(ret[1])]
                        else:
                            if ret[0] == '':
                                data[i] = ["(Default)", ret[1]]
                            else:
                                data[i] = [ret[0], ret[1]]
                    except WindowsError:
                        break
                    i += 1
                key_object.Close()
    except:
        pass

    return data

def main(argv=None):

    argv = sys.argv


    args = argv[1:]
    key = r'HKEY_CLASSES_ROOT\.' + args[0] + '\\OpenWithProgids'
    pkey = r''

    data = get_all_values(key)
    for x in range(0, len(data)):
        strdatax = str(data[x][0])
        if args[1].upper() in strdatax.upper():
            pkey = r'HKEY_CLASSES_ROOT\\' + strdatax + '\\shell\\open\\command'
            break

    if str(data[0][1]) == '[\'\']':
        print ("Assoc not found")
        sys.exit(1)

    data = get_all_values(pkey)
    for x in range(0, len(data)):
        if ".EXE" in str(data[x][1]).upper():
            exeind = str(data[x][1]).upper().find('.EXE')
            print ('"' + str(data[x][1])[:exeind+4] + '"')
            sys.exit(0)

    print ("Handler not found")
    sys.exit(1)


if __name__ == "__main__":
    main()


