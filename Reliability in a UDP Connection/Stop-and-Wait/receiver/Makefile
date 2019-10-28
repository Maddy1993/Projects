makeReceiver: receiver.o
	gcc receiver.o -o makeReceiver
receiver.o: receiver.c receiverCommunication.c
	gcc -c receiver.c
clean:
	rm *.o makeReceiver
