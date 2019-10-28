makeClient: client.o
	gcc client.o -o makeClient
client.o: client.c clientdataCommunication.c
	gcc -c client.c
clean:
	rm *.o makeClient
