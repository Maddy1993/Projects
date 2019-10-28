# A Program which reads the terms from a document and generates a inverted index
# for a corpus.
from os import listdir, path
import json
import re

# Structures to store the uni_grams

# Structure for uni_gram
try:
    # with open(path.join(path.pardir, 'Indexes/Simple_Inverted_Index/uni_dict.json', 'r') as dict_file:
    # with open(path.join(path.pardir, "Indexes/Stemmed_Inverted_Index/uni_dict.json"), 'r') as dict_file:
    # with open(path.join(path.pardir, "Indexes/Digit_Stemmed_Inverted_Index/uni_dict.json"), 'r') as dict_file:
    with open(path.join(path.pardir, "Indexes/Stopped_Inverted_Index/uni_dict.json"), 'r') as dict_file:
        uni_dict = json.load(dict_file)
        dict_file.close()
except FileNotFoundError:
    uni_dict = dict()

# Structure to store the document lengths
try:
    # with open(path.join(path.pardir, 'Indexes/Simple_Inverted_Index/docs_term_freq.json', 'r') as doc_dict:
    # with open(path.join(path.pardir, "Indexes/Stemmed_Inverted_Index/docs_term_freq.json"), 'r') as doc_dict:
    # with open(path.join(path.pardir, "Indexes/Digit_Stemmed_Inverted_Index/docs_term_freq.json"), 'r') as doc_dict:
    with open(path.join(path.pardir, "Indexes/Stopped_Inverted_Index/docs_term_freq.json"), 'r') as doc_dict:
        docs_term_freq = json.load(doc_dict)
        doc_dict.close()
except FileNotFoundError:
    docs_term_freq = dict()


# Program which calculates the term occurrences in a document
def index_generator(file_name, doc_id):
    # Open the document for parsing and generating an
    # index for each term in the document
    uni_term_list = []
    position = 0

    with open(file_name, 'r', encoding='utf-8') as document:
        for line in document:
            uni_term_list += line.split()

            # uni-gram and positional uni_gram
            if len(uni_term_list) == 1:
                # update the document counter
                docs_term_freq[doc_id] += 1

                # Form the uni-gram
                item = ''.join(uni_term_list)
                uni_term_list = []
                position += 1

                # Call the function to check and update the term in
                # in the data structure
                check_and_update(item, uni_dict, doc_id, 0)

        # close the current document
        document.close()


def check_and_update(term, term_dict, file_id, term_pos):
    # If the element exists in the dictionary,
    # update its term frequency for a particular document-id
    if term in term_dict:
        # and term in pos_dict:
        found = False
        values = term_dict[term]
        # pos_values = pos_dict[term]

        # If element is found, update the term
        # frequency
        if term_pos == 0:
            for elements in values:
                if elements[0] == file_id:
                    elements[1] = elements[1] + 1
                    found = True
                    break
        # also check for the element in the indexer storing
        # positions
        if term_pos != 0:
            for elements in values:
                if elements[0] == file_id:
                    elements[1].append(term_pos)
                    found = True
                    break

        # If not found, create an entry for that document
        if not found:
            if term_pos != 0:
                values.append([file_id, [term_pos]])
            else:
                values.append([file_id, 1])

    # create an index for the element in the inverted_indexer
    else:
        if term_pos != 0:
            term_dict[term] = [[file_id, [term_pos]]]
        else:
            term_dict[term] = [[file_id, 1]]

    # # Find the entry of document in the document's # of terms dictionary
    # # and increment its count.
    # if file_id in doc_count_dict:
    #     doc_count_dict[file_id] += 1
    # else:
    #     doc_count_dict[file_id] = 1


if __name__ == '__main__':

    counter = 0
    # corpus_out = path.join(path.pardir, "Corpuses/Stemmed_Corpus_Files")
    # corpus_out = path.join(path.pardir, "Corpuses/Digit_Stemmed_Text_Corpus_Files")
    corpus_out = path.join(path.pardir, "Corpuses/Stopped_Corpus_Files")
    files = listdir(corpus_out)
    for name in files:
        doc_title = ''
        result = re.match('(.*?).txt', name)
        if result:
            doc_title = result.group(1)

        docs_term_freq[doc_title] = 0
        index_generator(str(corpus_out) + "\\" + name, doc_title)

        # Debug Information
        counter += 1
        if round(len(files) * 0.1) == counter:
            print("10% of # of files in the directory processed")
        elif round(len(files) * 0.3) == counter:
            print("30% of # of files in the directory processed")
        elif round(len(files) * 0.5) == counter:
            print("50% of # of files in the directory processed")
        elif round(len(files) * 0.8) == counter:
            print("80% of # of files in the directory processed")
        elif len(files) == counter:
            print("Processing complete.")

        # if counter == 2:
        #     break
    # calculate average document length required by the
    # retrieval model and store in an unique entry.
    avdl = sum(docs_term_freq.values()) / len(docs_term_freq)
    docs_term_freq['00_avdl_00'] = avdl

    print("\nWriting information to the local disk")

    # with open('uni_dict.json', 'w+') as dict_file:
    # with open(path.join(path.pardir, "Indexes/Stemmed_Inverted_Index/uni_dict.json"), 'w+') as dict_file:
    # with open(path.join(path.pardir,
    #                     "Indexes/Digit_Stemmed_Inverted_Index/uni_dict.json"), 'w+') as dict_file:
    with open(path.join(path.pardir,
                        "Indexes/Stopped_Inverted_Index/uni_dict.json"), 'w+') as dict_file:
        json.dump(uni_dict, dict_file)

    # with open('uni_freq.json', 'w+') as doc_dict:
    #     json.dump(uni_freq, doc_dict)

    # with open('docs_term_freq.json', 'w+') as doc_dict:
    # with open(path.join(path.pardir, "Indexes/Stemmed_Inverted_Index/docs_term_freq.json"), 'w+') as doc_dict:
    # with open(path.join(path.pardir,
    #                     "Indexes/Digit_Stemmed_Inverted_Index/docs_term_freq.json"), 'w+') as doc_dict:
    with open(path.join(path.pardir,
                        "Indexes/Stopped_Inverted_Index/docs_term_freq.json"), 'w+') as doc_dict:
        json.dump(docs_term_freq, doc_dict)

    print("Check respective files for uni-gram inverted index corpus.")
