# Script which traverses the relevance query file
# and generates the json format file which stores the
# relevant files for the given query set.
import json
from os import path


def parse_relevance_information(file_path):
    p = path.join(path.dirname(path.abspath(__file__)),
                  "relevance_information.json")
    try:
        with open(p, 'r') as rel_file:
            relevance_dict = json.load(rel_file)
    except FileNotFoundError:
        relevance_dict = dict()

    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            for line in file:
                line = str(line)
                line = line.split()

                query_id = line[0]
                doc_id = line[2]

                if query_id not in relevance_dict:
                    relevance_dict[query_id] = [doc_id]
                else:
                    v = relevance_dict[query_id]
                    v.append(doc_id)
                    relevance_dict[query_id] = v
    except FileNotFoundError:
        print("Relevance file not found. Exiting the program.")

    with open(p, 'w+', encoding='utf-8') as file:
        json.dump(relevance_dict, file)


if __name__ == '__main__':
    k = path.join("Material", "cacm.rel.txt")
    parse_relevance_information(k)


