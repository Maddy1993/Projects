# Script which parses any input string
# and process it into forming a tokens
# in the format imbibed in the indexer
# of this retrieval system
import nltk
import string
import re


def parse_query(query):
    result_query = []
    # Step-1: Convert it into ascii format
    query = query.encode('ascii', 'ignore')

    # Step-2: Decode it using the utf-8 format
    query = query.decode('utf-8', 'ignore')

    # Step-3: Tokenize the words using nltk
    # library
    tokens = nltk.word_tokenize(query)

    # Iterate through the tokens to remove
    # any unrequired punctuation
    for element in tokens:
        element = element.strip(string.punctuation)
        filter_output = re.match(
            '(\w+(\.\w+)+)|(\w+(-\w+)+)+|(\w+(_\w+)+)+|[A-z]+|(\d(-\d+)+)+|(\d(\.\d+)+)+'
            '|(\d(\,\d+)+)+|(\d+:\d+\w+)|\d+|[^_\-,⋅.?:’\')({}\]\[]',
            element)

        if filter_output is not None:
            result_query.append(filter_output.group().lower())

    # return the parsed tokens
    return result_query