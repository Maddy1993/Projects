



def getStopWords():
   stopwords=[]
   with open(path.join("Material","common_words.txt"), "r") as ins:
     for line in ins:
        line =line.strip('\n')
        stopwords.append(line)
   return stopwords


