import re
from xml.dom import minidom


def genQueries():
    file_query = open("queries.txt", "w",
                           encoding='utf-8')  # file to store the results
    try:

       with open('cacm.query.txt', 'r') as myfile:
          data=myfile.read()
    except FileNotFoundError:
        print("File containing queries not found.")


    data="<root>"+data+"</root>"
    doc = minidom.parseString(data)
    #print(data)

    docs = doc.getElementsByTagName("DOC")
    count=0
    queries=[]

    for d in docs:
        count+=1
        print(count)

        x=" ".join(t.nodeValue for t in d.childNodes if t.nodeType == t.TEXT_NODE)

        x=x.strip().replace('\n',' ')
        x=x.lower()
        #x=x.replace('\n',' ')
        file_query.write(x+"\n")
        queries.append(x)

    file_query.close()
    myfile.close()
    return queries
