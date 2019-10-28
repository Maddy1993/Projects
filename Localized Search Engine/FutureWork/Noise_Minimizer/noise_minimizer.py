from os import path
from utilities.query_parser import parse_query
from Extra_Credit.Noise_Minimizer.term_approximater import correction
from Retrieval.Retrieval_Model.BM25_Model.search import Search
from collections import OrderedDict
import json


queries = []


def process(q):
    q_terms = parse_query(q)

    new_query_terms = []

    # pass every term in the query to the corrector
    for term in q_terms:
        if len(term) > 3:
            corrected_term = correction(term)
        else:
            corrected_term = term

        # append the possible correction for every term to a list
        new_query_terms.append(corrected_term)

    # form a new query
    new_query = ' '.join(new_query_terms)

    return new_query


def main():
    global queries
    query_count = 1
    result = {}
    result_set = {}
    noise_reduced_queries = []

    try:
        for line in \
                open(path.join(path.pardir, 'Noise_Generator', 'errorneous_queries.txt'),
                     'r', encoding="utf-8"):
            ln = line.strip('\r\n')
            ln = ln.lower()
            if ln:
                queries.append(ln)

        with open('noise_minimized_queries.txt', 'w+', encoding='utf-8') as f:
            for q in queries:
                n = process(q)
                noise_reduced_queries.append(n)
                print("Processed Query: ", query_count)
                query_count += 1
                f.write(str(n) + "\n")
    except FileNotFoundError:
        print("Erroneous query file not found.")

    print("Noise-minimized queries generated. Performing BM25 search on the queries.")

    # BM25 retrieval model reference
    query_count = 1
    obj = Search()

    # Execute the search using BM25 for each query
    for query in noise_reduced_queries:
        result = OrderedDict(obj.search(query))

        if query_count not in result_set:
            result_set[query_count] = result

        query_count += 1

    print("Search concluded. Writing results to a file.")
    # Write the returned results for all the queries
    # to a json file
    with open("noise_minimized_bm25_result.json", 'w+', encoding='utf-8') as f:
        json.dump(result_set, f)

main()