#!/usr/bin/env python3
# coding=utf-8
import json
import os
import re

data = {}
COLOR_RED = "\033[31m"
COLOR_GREEN = "\033[32m"
COLOR_YELLOW = "\033[33m"
COLOR_BRIGHT_MAGENTA = "\033[95m"
COLOR_BRIGHT_YELLOW = "\033[93m"
COLOR_CLEAR = "\033[0m"

EXCEL_FILES_ROOT = "../"
# add support type field here
support_type = {
    "number",
    "integer",
    "string",
    "int[]",
}

# add support name field here
support_col = {
    "name",
    "type",
    "index",
}


def check_support_type(type):
    return type in support_type


def check_support_col(name):
    return name in support_col


col_name_set = set()
def check_col_repeated(data, reset = False):
    global col_name_set
    if reset:
        col_name_set = set()

    if data in col_name_set:
        return True

    col_name_set.add(data)
    return False


def check_col_require(data):
    if "name" not in data:
        print(COLOR_RED + "missing index \"name\"!" + COLOR_CLEAR)
        return False

    if "type" not in data:
        print(COLOR_RED + "missing index \"type\"!" + COLOR_CLEAR)
        return False

    return True


def check_fields_field(data):
    if data == None:
        return False

    check_col_repeated("", True)
    for col in data:
        check_col_require(col)
        for k, item in col.items():
            if not check_support_col(k):
                print(COLOR_YELLOW + "name field: \"" + k + "\" is" + " not suppported" + COLOR_CLEAR)
                continue
            elif k == "name" and check_col_repeated(item):
                print(COLOR_RED + "name field: \"" + item + "\" repeated!" + COLOR_CLEAR)
            elif k == "type" and not check_support_type(item):
                print(COLOR_YELLOW + "type field: \"" + item + "\" is " + "not suppported." + COLOR_CLEAR)
                continue


def check_txt_files(filename):
    m = re.search('\.txt$', filename)
    if m is None:
        return

    print(COLOR_BRIGHT_YELLOW + "checking txt file: " + filename + COLOR_CLEAR)
    check_col_repeated("", True)
    with open(filename, 'r') as f:
        for line in f.readlines():
            words = line.split('\t')
            for word in words:
                print("word: " + word)

            # check repeat id
            if check_col_repeated(words[0]):
                print(COLOR_RED + "id " + COLOR_BRIGHT_YELLOW + word[0] + COLOR_RED + " repeated!" + COLOR_CLEAR)


def check_files_field(data):
    if data == None:
        return

    for filename in data:
        check_txt_files(EXCEL_FILES_ROOT + filename + ".txt")


def check_json_file(filename):
    m = re.search('\.json$', filename)
    if m is None:
        return

    print(COLOR_GREEN + "checking json file: " + filename + COLOR_CLEAR)
    data = {}
    with open(filename, 'r') as f:
        data = json.load(f)

    check_fields_field(data["fields"])
    check_files_field(data["files"])


def walk_dir_func(func, dirt, exclude = False):
    for root, dirs, files in os.walk(EXCEL_FILES_ROOT):
        # get files root dirs
        print("dir: " + root)
        hit = False
        if exclude and root != EXCEL_FILES_ROOT + dirt:
            hit = True
        elif not exclude and root == EXCEL_FILES_ROOT + dirt:
            hit = True

        if hit:
            for file in files:
                print("file: " + os.path.join(root, file))
                func(os.path.join(root, file))

if __name__ == "__main__":
    # check_json_file("item_list.json")
    walk_dir_func(check_json_file, "json")
