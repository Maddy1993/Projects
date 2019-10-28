# File which parses the stopwords given
# and stores it as json format file
# to be used by other programs.
from os import path
import json


def stop_word_parser():

    p = ""
    # get the existing stop_words file for the retrieval system.
    # If not found, create a new one.
    try:
        p = path.join(path.curdir, "stop_words.json")
        with open(p, 'r', encoding='utf-8') as stop_words:
            s_dict = json.load(stop_words)
    except FileNotFoundError:
        s_dict = list()

    # get the file containing stop words.
    try:
        p = str(path.join(path.join(path.curdir, "Material"), "common_words.txt"))
        with open(p, 'r', encoding='utf=8') as stop_file:
            for line in stop_file:
                line = line.strip()
                if line not in s_dict:
                    s_dict.append(str(line))
    except FileNotFoundError:
        print("Stop words file not found. Exiting the program.")
        exit(-1)

    # Write to the file/ disk.
    p = path.join(path.curdir, "stop_words.json")
    with open("stop_words.json", 'w+', encoding='utf-8') as write_file:
        json.dump(s_dict, write_file)


if __name__ == '__main__':
    stop_word_parser()