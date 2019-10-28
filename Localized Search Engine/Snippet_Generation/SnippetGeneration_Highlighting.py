from utilities.query_parser import parse_query
from bs4 import BeautifulSoup
import codecs

import operator
import json
from collections import OrderedDict
from os import path


stopwords=[]
sentence_score = {}
stopword_score = {}
stopwords = []
file1 =open("Snippets_term_highlighting.html", 'a', encoding='utf-8')
sentences=[]
queries=[]


sentence_size = 8
max_snippet = 3


def breakDocIntoSentences(file_name):
    soup = BeautifulSoup(codecs.open(file_name, 'r', encoding="utf8"), "html.parser")
    text = str(soup.getText()).encode('ascii', 'ignore')
    text = text.decode('utf-8', 'ignore')

    # get the text of the document

    # break it into sentences of size 8
    words = parse_query(text)

    sentences = []
    sentence = []
    for word in words:
        if (len(sentence) < sentence_size):
            sentence.append(word)
        else:
            sentences.append(sentence)
            sentence = []
            sentence.append(word)
    if (len(sentence) <= sentence_size):
        sentences.append(sentence)

    return sentences


def classifyQueryTerms(query_terms):
    imp_terms = []
    q_stopwords = []
    for q in query_terms:
        if q not in stopwords:
            imp_terms.append(q)
        else:
            q_stopwords.append(q)


    return imp_terms, q_stopwords


# assigns a score to each sentence based on the number of query terms
def scoreSentences(sentences, imp_terms, stop_terms):
    global sentence_score, stopword_score
    sentence_score={}
    stopword_score={}
    for i in range(0,len(sentences)):
        s = sentences[i]
        sentence_score[i] = 0
        stopword_score[i] = 0

        for word in s:

            #word = word.lower()
            if word in imp_terms:
                sentence_score[i] = sentence_score[i] + 1
            elif word in stop_terms:
                stopword_score[i] = stopword_score[i] + 1

    return sentence_score,stopword_score

def sortAndPrint(sentence_score,stopword_score,important_terms,stop_terms):
    global sentences

    sorted_sen = sorted(sentence_score.items(), key=operator.itemgetter(1), reverse=True)
    sorted_sen = OrderedDict(sorted_sen[:max_snippet])

    snippet=""
    c = 0
    #list to store the sentences that are already a part of the snippet
    #for every sentence
    chosen = []
    n =0 #to store the number of sentences with value greater than 0
    #if it is not equal to 3, it means we will have to highlight stop words also
    for key,value in sorted_sen.items():
        if(value>0):
            n=n+1

    highlight_stop_word = 0
    if (n!=3):
        highlight_stop_word=1


    for key,value in sorted_sen.items():

        #print(sentences[x])
        if (value > 0):
          sen=""
          chosen.append(key)
          for word in sentences[key]:
            #the word can either be a stop word or significant word or neither
            #if  it is a stop word and we are supposed to highlight insignificant words,
            if (highlight_stop_word == 1):
                 if word in stop_terms or word in important_terms:
                     word = "<b>" + word + "</b>"
            #we don't need to highlight stopwords, hence highlight only if word is important
            else:
              if word in important_terms:
                word="<b>"+word+"</b>"

            sen = sen + word+" "
          sen = sen+"..."
          snippet = snippet+" "+sen
          c=c+1 #to keep a count of number of sentences in snippet


    if(c<3):

    # it means that the all of the first 3 sentences weighted by their query term frequency didn't have value > 0
    #in that case, we add to the snippet those sentences which had stop words present in the query
    # and highlight both query terms and stop words in the snippet
       sorted_sen_stop_score = sorted(stopword_score.items(), key=operator.itemgetter(1), reverse=True)
       sorted_sen_stop_score = OrderedDict(sorted_sen_stop_score[:max_snippet*2])

       for key,value in sorted_sen_stop_score.items():
           if(key not in chosen):

              sen=""
              for word in sentences[key]:
                #w=word.lower()
                if word in stop_terms:
                    word="<b>"+word+"</b>"
                sen = sen + word+" "
              sen = sen+"..."

              snippet = snippet+" "+sen
              c=c+1
              if (c==3):

                  break
    #if there are no sentences with either the query terms or the stop words, then just return first three sentences
    if(snippet == ""):

        for i in range(0,3):
            s =' '.join(sentences[i])
            s = s +" ..."
            snippet=snippet+" "+s


    file1.write("<p>"+snippet+"</p><br>")



#open the file containing the query and its top 100 documents
try:
        p= path.join(path.pardir,"Retrieval_Model/BM25 Model/BASELINE/BM25_query_resultdocs_dict.txt")
        with open(p, 'r') as res_dict:
            results_dict = json.load(res_dict)
except FileNotFoundError:
        print("File containing results not found.")



def getStopWords():
    global stopwords
    p=path.join(path.join(path.pardir,"Material"),"common_words.txt")
    with open(p ,"r") as ins:
        for line in ins:
            line = line.strip('\n')
            stopwords.append(line)
    return stopwords


def genQueries():
    global queries
    try:
            p = str(path.join(path.pardir,"queries.txt"))
            for line in codecs.open(p, 'r', encoding="utf-8"):
                ln = line.strip('\r\n')

                ln= ln.lower()
                if ln:
                    queries.append(ln)
            return queries
    except FileNotFoundError:
            print("Query file not found.")




#for each query, get the top 100 documents..
# and for every document, first break the document to sentences,
# classify the query terms as important and insignificant based on whether they are stop words
# find out the scores for individual sentences based on the occurence of important query terms, and sort them
def main():
    global sentences, sentence_score,stopwords,queries
    queries = genQueries()
    stopwords = getStopWords()
    for num in range(0,len(queries)):
        docs = results_dict[str(num+1)]
        count=0
        print("Finding results for query "+str(num+1))
        file1.write("<br><h1>" + str(num + 1) + ". " + str(queries[num])+"</h1><br>")
        for d in docs:
            count=count+1
            documentname= str(path.join(path.pardir,path.join("Material/cacm.tar",d+".html")))

            file1.write("<i>"+str(count)+"."+documentname+"</i>")
            #write query number and doc name to html file
            sentences=[]
            sentences = breakDocIntoSentences(documentname)
            query_terms = parse_query(queries[num])
            important_terms, stop_terms = classifyQueryTerms(query_terms)

            sentence_score,stopword_score = scoreSentences(sentences, important_terms, stop_terms)
            sortAndPrint(sentence_score,stopword_score,important_terms,stop_terms)

main()





















