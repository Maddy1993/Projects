'''

The following script process the given input
token to reduce the noise as much as possible.
For few shorter terms, which are shorter,
noise can reduced greatly, but terms which are
longer comparatively, noise cannot be removed.

This program uses the ntlk's brown corpus as the
based dictionary to check for the approximated terms.
'''

from nltk.corpus import brown
from collections import Counter
import time

# global variables

print("Loading Corpus...")
# Brown dictionary.
__WORDS = Counter(brown.words())
start = 0
inp = ''
print("Corpus Loaded. Processing the results.")

'''
Visible and accessible method, which uses process the
given input terms and finds an approximate match in the 
nltk brown corpus for noise-induced term. 

@Input:
@term:param   String Parameter, which represents a term
              in the query.
              
@Returns:

A term, which is the approximate dictionary estimation of 
the given noise-induced term.
'''


def correction(term):

    global __WORDS
    global start
    global inp

    # Find the possible variation of the words
    # using edit distance and send the word with
    # highest term frequency in the brown corpus.
    start = time.time()
    inp = term

    # if the token is a numeric
    # character, we would not shuffle it
    # as that would change the entire context
    if str(term).isnumeric():
        return term

    result_set = __dictionary_check(term)

    result_word = max(result_set,
                      key=lambda w, n=sum(__WORDS.values()): __WORDS[w] / n)

    return result_word


# Method which checks for the variations of the
# term which are defined and returns them as a set.
def __dictionary_check(term):

    global __WORDS

    # values to be returned
    result_set = [term]

    # find at_most three transpose length variations
    # of the word. We choose 3 because, any word, which
    # has greater than three transpose length is either
    # heavily noise-induced, or is not a perfect match
    # for approximate dictionary word.
    for word in result_set:
        l = __get_transposes(word)
        try:
            if len(l) == 1:
                result_set = l
                break
            else:
                result_set.extend(l)
        except MemoryError:
            print("According to our design choice, there is no possible"
                  "word in dictionary, matching the transpose of given term.")
            result_set = [term]
            return result_set

    return result_set


# Method which computes the possible variations of
# given input term.
def __get_transposes(term):

    global inp

    # Make a unit character splits between the
    # word characters
    split= []
    transpose_list = []

    for i in range(len(term) + 1):
        split.append([term[:i], term[i:]])

    # find transposes for each term
    # in the split list.
    for left_split, right_split in split:
        if len(right_split) > 1:
            # interchange first two characters
            # of right split character and combine
            # left_split and remaining part of right_split
            word = left_split + \
                   right_split[1] + right_split[0] + \
                   right_split[2:]

            if word in __WORDS:
                return [word]

            transpose_list.append(word)
            end = time.time()
            if end - start > 1.5:
                return [inp]

    # return the resultant transposes.
    return transpose_list


# Driver program
if __name__ == '__main__':
    print(correction("waht"))
