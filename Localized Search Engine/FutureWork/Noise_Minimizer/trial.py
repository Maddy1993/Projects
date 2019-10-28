import random
from nltk.corpus import words

def shuffle_string(string):
    chars = list(string)
    print (chars)
    random.shuffle(chars)
    print (chars)
    return ''.join(chars)

def garble_word(word):
    # No operation needed on sufficiently small words
    # (Also, main algorithm requires word length >= 2)
    if len(word) <= 3:
        return word

    # Split word into first & last letter, and middle letters
    first, mid, last = word[0], word[1:-1], word[-1]

    return first + shuffle_string(mid) + last

def garble(sentence):
    words = sentence.split(' ')
    return ' '.join(map(garble_word, words))

print(garble("varsha"))


for x in range(10):
  print (random.randint(1,3))


print("fine" in words.words())


from nltk.metrics import edit_distance
print(edit_distance("sharing","snhirag"))