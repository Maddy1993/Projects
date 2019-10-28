makeSender: sender.o
	gcc sender.o -o makeSender
sender.o: sender.c senderCommunication.c
	gcc -c sender.c
clean:
	rm *.o makeSender
