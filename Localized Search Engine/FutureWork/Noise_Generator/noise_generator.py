'''
The following script generates noise for a
given query. In other words, it acts as noise-generator
model for a query and shuffles the non-boundary characters
'''

from os import path
from utilities.query_parser import parse_query
from Retrieval.Retrieval_Model.BM25_Model.search import Search
import math
import operator
from collections import OrderedDict
import random
import json


# global variables
new_queries = []


'''
Method which shuffles the non-boundary characters
for each term in the given query.
It makes sure that Longer tokens should
have higher probability of being affected by the
noise than shorter ones.

@Input:
@token:param  String Parameter, which represents a token
              in the query.
@avg:param    Integer Parameter, which represents the 
              average length of the tokens for a given 
              query.

@Returns:

A shuffled token, which is a stirng, whose input the @token parameter,
where the non-boundary parameters are shuffled based
on the token length.
'''


def shuffle_token(token, avg):

    # Variables
    interim_list = []
    charac_count = 0

    # if the token is a numeric
    # character, we would not shuffle it
    # as that would change the entire context
    if str(token).isnumeric():
        return token

    # if it is a small word,
    # do not shuffle the characters
    if len(token) <= 3:
        return token

    # Else,
    # divide word into first letter, last letter and middle letters
    first, mid, last = token[0], token[1:-1], token[-1]

    # process to shuffle the
    # mid: consisting of non-boundary characters
    mid = str(mid)

    # using trial-and-error,
    # shuffling every other character in
    # tokens of length larger than average
    # token of the query yielded approximately
    # desired results.
    if len(token) > avg:
        split_range = 2
    else:
        split_range = round(len(token) / 2)

    for c in range(0, len(mid), split_range):
        interim_list.append(mid[c:c + 2][::-1])
        charac_count += 1

        if charac_count > 2:
            interim_list.append(mid[c + 2::])
            break

    # form the shuffled string
    mid = ''.join(interim_list)

    # return the shuffled token
    return first + mid + last


'''
Method which process the query and calculates the 
error rate on a query level and  enforces in such a
way that it is no higher than 40%.

@Input:
@query:param  String Parameter, which represents the
              query for the retrieval model.
              
@Returns:
None
'''


def process(query):
    global new_queries

    q_term_length = {}
    chosen = []
    replace_terms = {}

    # get individual terms in the query
    query_terms = parse_query(query)

    # get length of the query string
    query_length = len(query_terms)

    # get length of individual query terms
    for q in query_terms:
        q_term_length[q] = len(q)

    # sort the query terms according to their lengths in non-increasing order
    sorted_q_terms = OrderedDict(
        sorted(q_term_length.items(), key=operator.itemgetter(1), reverse=True))

    affect_words_num = math.floor(0.4 * query_length)

    while len(chosen) != affect_words_num:
        # the random integer is generated in the range between 0 to query length because
        # when the query terms are sorted in the decreasing order of their lengths, we assume that longer terms appear
        # in the first half
        num = random.randint(0, math.ceil(query_length / 2) - 1)

        if num not in chosen:
            chosen.append(num)

    items = list(sorted_q_terms.items())

    # Calculate affect range based on the
    # query term size.
    q_list = query.split()
    avg = sum(map(len, q_list)) / len(q_list)

    for index in chosen:
        token = items[index][0]

        # the token should be replaced by the shuffled word in the query
        replace_terms[token] = shuffle_token(token, avg)

    qy = query
    for term in replace_terms:
        # replace the query term with the noisy term
        qy = qy.replace(term, replace_terms[term], 1)

    new_queries.append(qy)


# Method which reads the queries from a text file and
# stores it in a data structure and returns it
# to be processed later.
def get_queries():
    # VAriables
    queries = []

    # iterate through the file and get each query
    # which is seperated by a new line.
    for line in open(path.join(path.pardir, path.pardir,
                               "utilities", "queries.txt"), 'r', encoding="utf-8"):
        ln = line.strip('\r\n')
        if ln:
            queries.append(ln)

    return queries


# Driver Method for the Script.
if __name__ == '__main__':

    query_count = 1
    result_set = OrderedDict()

    #  get the queries from the file
    queries = get_queries()

    # for each query in the structure,
    # process it to generate noise in it.
    for query in queries:
        process(query)

    print("Noise generated. Performing BM25 search on the queries.")
    # BM25 retrieval model reference
    obj = Search()

    # write the noise-induced queries to a
    # text file to be used later.
    with open("errorneous_queries.txt", 'w+', encoding='utf-8') as file:
        for query in new_queries:
            file.write(str(query) + "\n")

    # Execute the search using BM25 for each query
    for query in new_queries:
        result = OrderedDict(obj.search(query))

        if query_count not in result_set:
            result_set[query_count] = result

        query_count += 1

    print("Search concluded. Writing results to a file.")
    # Write the returned results for all the queries
    # to a json file
    with open("noise_induced_bm25_result.json", 'w+',encoding='utf-8') as f:
        json.dump(result_set, f)
