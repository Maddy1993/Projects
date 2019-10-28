all: 
	gcc -g node.c connection.c convert.c transfer.c dht_peer.c structure.c check.c rootClient.c peerClient.c file.c sendObject.c fileRequest.c -pthread -Wall -lcrypto -o dht_peer
rootNode: all
peerNode: all
root: all
	#gdb -q dht_peer
	./dht_peer -m 1 -p 15096 -h aviator.ccs.neu.edu
peer-1: all
	#gdb -q iht_peer
	./dht_peer -m 0 -p 15096 -h aviator.ccs.neu.edu -r 15096 -R akubra.ccs.neu.edu
peer-2: all
	./dht_peer -m 0 -p 15096 -h balmoral.ccs.neu.edu -r 15096 -R aviator.ccs.neu.edu
peer-3: all
	#gdb -q dht_peer
	./dht_peer -p 15096 -h baseball.ccs.neu.edu -r 15096 -R aviator.ccs.neu.edu
peer-4: all
	./dht_peer -p 15096 -h beanie.ccs.neu.edu -r 15096 -R akubra.ccs.neu.edu
peer-5: all
	./dht_peer -p 15096 -h gatsby.ccs.neu.edu -r 15096 -R aviator.ccs.neu.edu
peer-6: all
	./dht_peer -p  15096 -h captain.ccs.neu.edu -r 15096 -R aviator.ccs.neu.edu
peer-7: all
	./dht_peer -m 0 -p 15096 -h top.ccs.neu.edu -r 15096 -R aviator.ccs.neu.edu
client: 
	gcc -g client.c rootClient.c convert.c structure.c check.c transfer.c sendObject.c connection.c fileRequest.c -pthread -Wall -lcrypto -o dht_client
client1:
	gcc -g client.c rootClient.c convert.c structure.c check.c transfer.c sendObject.c connection.c fileRequest.c -pthread -Wall -lcrypto -o dht_client
	 #gdb -q dht_client
	./dht_client -p 15098 -h captain.ccs.neu.edu -r 15099 -R aviator.ccs.neu.edu 		
hashCalculator:
	gcc -g commandline.c -Wall -lcrypto -o hash
clean:
	rm *-*
