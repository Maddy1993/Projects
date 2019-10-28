import re
from xml.dom import minidom



file_query = open("queries.txt", "a",
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

    #x=x.replace('\n',' ')
    file_query.write(x+"\n")
    queries.append(x)



#print(queries)

file_query.close()
myfile.close()
