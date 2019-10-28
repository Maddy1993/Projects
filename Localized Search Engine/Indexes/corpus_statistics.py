# File used to develop the corpus statistics like term-frequency and
# document frequency
import operator
import json
from os import listdir
from collections import OrderedDict
from os import path


class Statistics:
    # Variables

    # Methods
    # Generates the term frequency for given data structures or
    # inverted indexes which stores the terms as indexes.
    def term_frequency_generator(self, index_name, indexer):
        # data structure to hold the term frequencies
        term_frequency = dict()
        term_doc_freq = dict()

        # Iterate through the indexer to identify the
        # terms in it
        for key in indexer.keys():
            term_count = 0

            # for each document the term is present
            for values in indexer[key]:
                # If the indexer is either a uni-gram, bi-gram or
                # tri-gram, increment the count
                if index_name != 'position_indexer':
                    term_count += int(values[1])
                # Else, find the length of list holding the positions
                # of the term in a document
                elif index_name == 'position_indexer':
                    term_count += len(values[1])

                # generating term, docID list and df
                if key not in term_doc_freq:
                    value_list = [[values[0]], 1]
                    term_doc_freq[key] = value_list

                    # get the existing values
                    value_list = term_doc_freq[key]

                    # update the existing value
                    doc_list = value_list[0]
                    doc_list.append(values[0])
                    value_list = [doc_list, len(doc_list)]
                    term_doc_freq[key] = value_list

            # update the value in the data structure
            term_frequency[key] = term_count

        # Update the document frequency table for each indexer
        # with open(path.join(path.pardir,
        #                     "Indexes/Stemmed_Inverted_Index/" + index_name + "_df.json"),
        #           'w+', encoding='utf-8') as file:
        # with open(path.join(path.pardir,
        #                     "Indexes/Digit_Stemmed_Inverted_Index/" + index_name + "_df.json"),
        #           'w+', encoding='utf-8') as file:
        with open(path.join(path.pardir,
                            "Indexes/Stopped_Inverted_Index/" + index_name + "_df.json"),
                  'w+', encoding='utf-8') as file:
            json.dump(term_doc_freq, file)
            file.close()

        # Creating a table format and sorting lexicographically
        # For Stemmed Indexer
        # with open(path.join(path.pardir,
        #                     "Indexes/Stemmed_Inverted_Index/" + index_name + "_df.txt"),
        #           'w+', encoding='utf-8') as file:
        # with open(path.join(path.pardir,
        #                     "Indexes/Digit_Stemmed_Inverted_Index/" + index_name + "_df.txt"),
        #           'w+', encoding='utf-8') as file:
        with open(path.join(path.pardir,
                            "Indexes/Stopped_Inverted_Index/" + index_name + "_df.txt"),
                  'w+', encoding='utf-8') as file:
            for key, value in sorted(term_doc_freq.items(), key=operator.itemgetter(0)):
                file.write("\n" + str(key) + ": ")
                for ele in value[0]:
                    file.write(str(ele) + ", ")
                file.write(" " + str(value[1]))

        # After all the keys are traversed, write them to file
        # by sorting them in increasing order based on the term_frequency
        # count
        s_dict = sorted(term_frequency.items(), key=operator.itemgetter(1), reverse=True)

        # with open(path.join(path.pardir,
        #                     "Indexes/Stemmed_Inverted_Index/" + index_name + "_tf.txt"),
        #           'w+', encoding='utf-8') as file:
        # with open(path.join(path.pardir,
        #                     "Indexes/Digit_Stemmed_Inverted_Index/" + index_name + "_tf.txt"),
        #           'w+', encoding='utf-8') as file:
        with open(path.join(path.pardir,
                            "Indexes/Stopped_Inverted_Index/" + index_name + "_tf.txt"),
                  'w+', encoding='utf-8') as file:
            for key, value in s_dict:
                file.write(str(key) + ": " + str(value) + "\n")
            file.close()

        # Create a JSON file for the same
        # with open(path.join(path.pardir,
        #                     "Indexes/Simple_Inverted_Index/" + index_name + "_tf.json"),
        #           'w+', encoding='utf-8') as file:
        #     json.dump(OrderedDict(s_dict), file)
        #     file.close()

        # with open(path.join(path.pardir,
        #                     "Indexes/Stemmed_Inverted_Index/" + index_name + "_tf.json"),
        #           'w+', encoding='utf-8') as file:
        # with open(path.join(path.pardir,
        #                     "Indexes/Digit_Stemmed_Inverted_Index/" + index_name + "_tf.json"),
        #           'w+', encoding='utf-8') as file:
        with open(path.join(path.pardir,
                            "Indexes/Stopped_Inverted_Index/" + index_name + "_tf.json"),
                  'w+', encoding='utf-8') as file:
            json.dump(OrderedDict(s_dict), file)
            file.close()

    # Method which generates the sorted term frequencies for each file
    def sorted_term_freqs_per_doc(self, term_dict):
        # Iterate through each term and
        # find the frequencies of each term in the document.
        for term in term_dict:
            for doc in term_dict[term]:
                if doc[0] not in doc_term_dict:
                    doc_term_dict[doc[0]] = [[term, doc[1]]]
                else:
                    values = doc_term_dict[doc[0]]
                    values.append([term, doc[1]])
                    doc_term_dict[doc[0]] = values

        # with open(path.join(path.pardir,
        #                     "Indexes/Stemmed_Inverted_Index/term_freq_per_doc.json"), 'w+',
        #           encoding='utf-8') as file:
        # with open(path.join(path.pardir,
        #                     "Indexes/Digit_Stemmed_Inverted_Index/term_freq_per_doc.json"), 'w+',
        #           encoding='utf-8') as file:
        with open(path.join(path.pardir,
                            "Indexes/Stopped_Inverted_Index/term_freq_per_doc.json"), 'w+',
                  encoding='utf-8') as file:
            json.dump(doc_term_dict, file)


# Driver program
if __name__ == '__main__':
    term_dict = {}
    doc_term_dict = {}

    # Object for the class
    pointer = Statistics()

    # Structure for uni_gram
    try:
        # with open(path.join(path.pardir,
        #                     'Indexes/Stemmed_Inverted_Index/uni_dict.json'), 'r') as dict_file:
        # with open(path.join(path.pardir,
        #                     'Indexes/Digit_Stemmed_Inverted_Index/uni_dict.json'), 'r') as dict_file:
        with open(path.join(path.pardir,
                            'Indexes/Stopped_Inverted_Index/uni_dict.json'), 'r') as dict_file:
            term_dict = json.load(dict_file)
            print("Processing uni_gram indexer")
            pointer.term_frequency_generator('uni_gram', term_dict)
            print("uni_gram_tf.txt, uni_gram_tf.json, uni_gram_df.txt and uni_gram_df.json file generated.\n")

            pointer.sorted_term_freqs_per_doc(term_dict)
            print("Sorted term frequencies for each document are generated.")
    except FileNotFoundError:
        print("Uni-gram file not found\n")
