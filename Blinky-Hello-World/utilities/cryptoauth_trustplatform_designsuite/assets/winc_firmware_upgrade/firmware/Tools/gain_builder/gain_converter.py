'''
Gain Conversion Tool
Support for; WINC1500 multi-gain table, WINC3400 multi-gain table.

1500 Usage: gain_converter.py csv_file_1.csv csv_file_2.csv csv_file_3.csv csv_file_4.csv -o output.config
Up to 4 WINC1500 old style CSV files for a single new style WINC1500 multi-gain table.

3400 Usage: gain_converter.py csv_file.csv -o outfile.config
A single old style WINC3400 CSV is used to create a new style WINC3400 multi-gain table.

Author: Zak.Ng@microchip.com
Date: Dec 2019
'''

import csv
import re
import argparse


def read_csv(f):
    rows = []

    csv_reader = csv.reader(f, delimiter=',')

    for row in csv_reader:
        rows.append(row)

    return rows


def count_repeats(string):
    string_list = []

    count_matching = 0
    remove = False

    for index, gain in enumerate(reversed(string)):
        if index + 1 == len(string):
            break

        if gain == string[(len(string) - (index + 2))]:
            count_matching += 1
        else:
            break

    if count_matching > 0:
        remove = True

    if remove:
        cp_string = string[:-count_matching]

        if cp_string[-1].endswith(","):
            cp_string[-1] = cp_string[-1][:-1]

        string_list.append(''.join(cp_string))
    else:
        if string[-1].endswith(","):
            string[-1] = string[-1][:-1]

        string_list.append(''.join(string))

    return ''.join(string_list)


def convert_1500(rows):
    string_list = []

    for item in rows:
        if "ch" in item[00]:
            continue
        if "1e" in item[00]:
            continue

        string = ["wifi gain at "]

        string.append(str(item[00]))
        string.append(" is ")

        for i, chan in enumerate(item[1:]):
            string.append(chan + ",")

        string_list.append(count_repeats(string))

    return string_list


def convert_3400(lines, table_num):
    string_list = []

    found_table = False

    for line_count, line in enumerate(lines):
        if not found_table:
            # Search for current table
            if line.startswith("# Table " + str(table_num)):
                found_table = True
            else:
                continue

        if line.startswith("# Table " + str(table_num + 1)):
            # If next table reached, break
            break
        if line.startswith("WiFi_Gain"):
            string = ["wifi gain at "]

            # Match first
            match_dg = re.findall("^[^:]*", line)[0]
            rate = str(((match_dg.split("_", 2)[:3])[2]))

            # Match after colon
            match_gains = re.findall(":(.*)", line)[0]
            # Remove white space and split on comma into gains list
            gains = [x.strip() for x in match_gains.split(',')]

            string.append(rate + " is ")

            for gain in gains:
                string.append(gain + ",")

            string_list.append(count_repeats(string))

        if line.startswith("WiFi_PA"):
            string = ["wifi pa at "]

            # Match first
            match_dg = re.findall("^[^:]*", line)[0]
            rate = str(((match_dg.split("_", 2)[:3])[2]))

            # Match after colon
            match_gains = re.findall(":(.*)", line)[0]
            # Remove white space and split on comma into gains list
            gains = [x.strip() for x in match_gains.split(',')]

            string.append(rate + " is ")

            for gain in gains:
                string.append(gain + ",")

            string_list.append(count_repeats(string))

        if line.startswith("WiFi_PPA"):
            string = ["wifi ppa at "]

            # Match first
            match_dg = re.findall("^[^:]*", line)[0]
            rate = str(((match_dg.split("_", 2)[:3])[2]))

            # Match after colon
            match_gains = re.findall(":(.*)", line)[0]
            # Remove white space and split on comma into gains list
            gains = [x.strip() for x in match_gains.split(',')]

            string.append(rate + " is ")

            for gain in gains:
                string.append(gain + ",")

            string_list.append(count_repeats(string))

        if line.startswith("BLE_Gain"):
            string = ["ble gain is "]

            match_gains = re.findall(":(.*)", line)[0]
            # Remove white space and split on comma into gains list
            gains = [x.strip() for x in match_gains.split(',')]

            for gain in gains[:-1]:
                string.append(gain + ",")

            count = line_count + 1
            # For the next three lines of BLE gains
            for x in range(0, 3):
                next_line = lines[count].strip()
                gains = next_line.split(",")

                for gain in gains:
                    if gain:
                        string.append(gain + ",")

                count += 1

            string_list.append(count_repeats(string))

        if line.startswith("BLE_PA"):
            string = ["ble pa is "]

            match_gains = re.findall(":(.*)", line)[0]
            # Remove white space and split on comma into gains list
            gains = [x.strip() for x in match_gains.split(',')]

            for gain in gains[:-1]:
                string.append(gain + ",")

            count = line_count + 1
            # For the next three lines of BLE gains
            for x in range(0, 3):
                next_line = lines[count].strip()

                gains = next_line.split(",")

                for gain in gains:
                    if gain:
                        string.append(gain + ",")

                count += 1

            string_list.append(count_repeats(string))

        if line.startswith("BLE_PPA"):
            string = ["ble ppa is "]

            match_gains = re.findall(":(.*)", line)[0]
            # Remove white space and split on comma into gains list
            gains = [x.strip() for x in match_gains.split(',')]

            for gain in gains[:-1]:
                string.append(gain + ",")

            count = line_count + 1
            # For the next three lines of BLE gains
            for x in range(0, 3):
                next_line = lines[count].strip()

                gains = next_line.split(",")

                for gain in gains:
                    if gain:
                        string.append(gain + ",")

                count += 1

            string_list.append(count_repeats(string))

    return string_list


def output_gain(schema, to_print, output_file):
    gain_table_schema = schema

    with open(output_file, 'w') as out_file:
        out_file.write("[gain table]" + "\n")
        out_file.write("type is gain table" + "\n")
        out_file.write("schema is " + gain_table_schema + "\n")

        for index, gain_table in enumerate(to_print):
            out_file.write("table is [gain table " + str(index + 1) + "]" + "\n")

        # Separator new line
        out_file.write("\n")

        for index, gain_table in enumerate(to_print):
            out_file.write("[gain table " + str(index + 1) + "]" + "\n")

            # Output converted lines
            output = '\n'.join(gain_table)
            out_file.writelines(output)

            # If not last table print spacing
            if (index + 1) < len(to_print):
                out_file.write("\n")
                out_file.write("\n")


def main():
    schema = None
    to_print = []

    # Read file arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=argparse.FileType('r'), nargs='+')
    parser.add_argument('-o', '--output', default="new_gain.config")

    args = parser.parse_args()

    # Check no more than 4 CSV files input
    if len(args.file) > 4:
        print "ERROR: Maximum of 4 CSV files allowed"
        quit()

    # Detect ALL CSV files format
    _1500_format = False
    _3400_format = False

    for f in args.file:
        lines = []
        with f as inp_file:
            chan_found = False
            hash_found = False

            for line in inp_file:
                lines.append(line)

            if "ch" in lines[00]:
                chan_found = True
            if "# Number of Tables" in lines[00]:
                hash_found = True

            if chan_found:
                _1500_format = True
            if hash_found:
                _3400_format = True

            elif chan_found and _3400_format:
                _1500_format = False
                _3400_format = False
                break
            elif hash_found and _1500_format:
                _1500_format = False
                _3400_format = False
                break

    if _1500_format:
        if len(args.file) > 1:
            schema = '3'
        else:
            schema = '1'

        # 'CH' appears at start of every CSV
        # Multi 1500 format
        for f in args.file:
            lines = []
            with open(f.name, 'r') as inp_file:
                for line in inp_file:
                    lines.append(line)

            lines = [x.rstrip() for x in lines]

            rows = read_csv(lines)

            to_print.append(convert_1500(rows))

    elif _3400_format:
        schema = '2'
        num_tables = None

        for line_count, line in enumerate(lines):
            next_line = line_count + 1
            if line.startswith("# Number of Tables"):
                num_tables = int(lines[next_line])

        for i in range(int(num_tables)):
            table_num = i
            to_print.append(convert_3400(lines, table_num))

    else:
        print "ERROR: Detected wrong input CSV format"
        quit()

    # Output lines
    output_gain(schema, to_print, args.output)


if __name__ == '__main__':
    main()
