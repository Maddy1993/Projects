all: receiver sender
receiver: receivers/receiver.c receivers/receiverCommunication.c receivers/structure.c
	gcc -o receiver receivers/receiver.c receivers/structure.c receivers/receiverCommunication.c
sender: senders/sender.c senders/senderCommunication.c
	gcc -o sender senders/sender.c
clean:
	rm sender
	rm receiver
