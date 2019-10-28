import socket
from argparse import ArgumentParser
import pickle
import threading
from Packet import Packet
from User import User
from Sign_enrypt_decrypt import SignEncryptDecrypt

"""
This Python Script represents a UDP server which has the 
capability to process LOGIN and LIST requests from
multiple clients.

On receiving the LOGIN request, server validates the username
in the packet and if the does not exist in its users dictionary,
it stores its details in the dictionary and sends the response
about the same. If the user exists, the server responds with a
user exists message.

Similarly, when the SERVER receives a LIST message from the clients,
it lists the users/clients who have contacted the server previously
and logged into the system.
"""
# Constants
SIGN_IN = 1
SEND = 2
LIST = 3
RESPONSE = 4

# Variables for Threading
lock = threading.Lock()
thread_queue = []

# Variables for maintaining
# active client list.
users = dict()

# Variable representing the encryption
# instance for the server,
encryption_decryption = None


# Performs the parsing of command line arguments
# and retrieves the Server Port.
#
# It also creates a datagram socket on the machine the server
# is executing and binds the socket on the port specified
# to listen to any incoming requests.
def parser_arguments():
    global encryption_decryption

    # Create a Parser instance and parse
    # the server port argument specified
    # on the command line.
    parser = ArgumentParser()
    parser.add_argument("-sp", "--serverport", type=int, action="store",
                        default="5555", help="Specified the Port on the machine to run the "
                                              "Server.")
    parser.add_argument("-pri", "--privateKeyPath", type=str, action="store",
                        help="Specifies the private key file location"
                             "of Server.")
    parser.add_argument("-pub", "--publicKeyPath", type=str, action="store",
                        help="Specifies the public key file location"
                             "of Server.")

    args = parser.parse_args()

    server_ip = "127.0.0.1"
    server_port = args.serverport
    private_key_path = args.privateKeyPath
    public_key_path = args.publicKeyPath
    sock = None

    # Create a socket instance and make it reusable,
    try:
        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    except socket.error as msg:
        print('Failed to create socket. Error Code : ' + str(msg.errno) + ' Message ' + msg.strerror)

    # Bind the socket to the port specified.
    try:
        sock.bind((server_ip, server_port))
    except socket.error as msg:
        print('Bind failed. Error Code : ' + str(msg.errno) + ' Message ' + msg.strerror)

    print("Socket bind successful.")

    # Initialize the encryption instance for the server.
    encryption_decryption = SignEncryptDecrypt(private_key_path, public_key_path)

    # Function which performs the read action
    # on the socket specified.
    read_data(sock)


# Function which reads the incoming requests
# on the socket specified on the parameter.
# Whenever a request is received on the socket,
# the server creates a new daemon thread and invokes
# the respective worker threads to perform the function
# specified.
def read_data(sock):

    # Process the request and perform
    # the action based on the request type.
    try:
        while True:
            # buffer size is 1024 bytes
            print("+- Waiting on Connection...\n")
            data, addr = sock.recvfrom(1024)
            data = pickle.loads(data)
            p = data.get_type()
            if data.get_type() == SIGN_IN:
                t = threading.Thread(target=process_login_request(data, addr, sock))
                t.setDaemon(True)
                thread_queue.append(t)
                t.start()
            elif data.get_type() == LIST:
                t = threading.Thread(target=process_list_request(data, addr, sock))
                t.setDaemon(True)
                thread_queue.append(t)
                t.start()
            else:
                print("-- Invalid Request")
                break
    finally:
        sock.close()
        for item in thread_queue:
            item.join()


# Function which sends the client
# the list of user instance objects
# consisting of all details about a
# client.
def process_list_request(data, client_address, sock):
    global users
    global lock

    # process the user dictionary
    # create a list of logged user
    # instances and send the same data
    # to the client.
    result = []
    print("+< LIST request received from: ", end='')
    print(client_address)
    lock.acquire()
    try:
        for value in users.values():
            result.append(value)
        pack = Packet()
        pack.pack(type=RESPONSE,
                  message=result)
    finally:
        lock.release()

    send_response(client_address, pack, sock)
    print("-> Response sent to the client.")


# Function which handles the login
# request of each client.
# When the login request is received,
# the server adds the client to list of
# active peers and exits the thread
# created for the process.
def process_login_request(data, client_address, socket):
    global lock
    global users

    username = data.get_username()
    # Acquire lock to generate a mutual
    # exclusion region for the threads
    # because the users dictionary is a
    # exclusive structure whose value
    # changes from thread to thread.
    lock.acquire()
    try:
        if username in users:
            print("+< LOG_IN request received from: ", client_address)
            print("+< User exists: ", username)
            create_user_exists_response(client_address, socket)
        else:
            # Process the request by creating and instantiating
            # the users and adding them to the users dictionary.
            client_contact_address = data.get_message()
            user = User()
            user.set_user_name(username)
            user.set_ip_address(client_contact_address[0])
            user.set_machine_port(client_contact_address[1])
            users[username] = user
            print("+< LOG_IN request received from: ", client_address)
            print("+< User added: ", username)
            create_user_logged_response(client_address, socket)
    finally:
        lock.release()


# Helper function to the process_login_request function
# which creates a packet for a situation when the
# user already exists in the system database and
# sends the same on the socket specified to the addr.
def create_user_exists_response(addr, socket):
    packet = Packet()
    packet.pack(type=RESPONSE,
                message="User already registered")

    send_response(addr, packet, socket)
    print("-> Response sent to user.")


# Helper function to the process_login_request function
# which creates a packet for a situation when the
# user is created in the system database and
# sends the same on the socket specified to the addr.
def create_user_logged_response(addr, socket):
    packet = Packet()
    packet.pack(type=RESPONSE,
                message="User Created")
    send_response(addr, packet, socket)
    print("-> Response sent to user.")


# Function which sends the packet in the argument
# to the client address specified on the socket
# also given in the parameters.
def send_response(address, packet, socket):
    address = list(address)
    client_ip = address[0]
    client_port = address[1]
    packet = pickle.dumps(packet)
    socket.sendto(packet, (client_ip, client_port))


if __name__ == '__main__':
    parser_arguments()