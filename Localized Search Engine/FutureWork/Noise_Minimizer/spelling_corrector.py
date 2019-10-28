from nltk.corpus import brown as e
import re
from collections import Counter
from itertools import chain

WORDS = Counter(e.words())


def words(text):
    return re.findall(r'\w+', text.lower())


def P(word, N=sum(WORDS.values())):
    "Probability of `word`."
    return WORDS[word] / N


def correction(word):
    "Most probable spelling correction for word."
    m = max(candidates(word), key=P)
    return m


def candidates(word):
    "Generate possible spelling corrections for word."
    # print(words1)
    return known([word]) or \
           known(edits1(word)) or \
           known(edits2(word)) or \
           [word]


def known(words):
    "The subset of `words` that appear in the dictionary of WORDS."
    k = set(w for w in words if w in WORDS)
    return k


def edits1(word):
    "All edits that are one edit away from `word`."
    splits = [(word[:i], word[i:]) for i in range(len(word) + 1)]

    transposes = [L + R[1] + R[0] + R[2:] for L, R in splits if len(R) > 1]

    return set(transposes)


def edits2(word):
    "All edits that are two edits away from `word`."
    l = []
    m = []
    x = []
    for e1 in edits1(word):
        l.append(e1)
    for e2 in l:
        m.append(edits1(e2))

    for e3 in m:
        for i in e3:
            x.append(edits1(i))

    k = []
    x = list(set(chain.from_iterable(x)))

    for e in x:
        k.append(edits1(e))

    k = list(set(chain.from_iterable(k)))

    # print(k)
    return k

print(correction("Ionfrmatoin"))