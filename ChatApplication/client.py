import socket
from argparse import ArgumentParser
import pickle
from Packet import Packet
from Type import PacketType
from threading import Thread

"""
This Python Script represents a UDP Client which
has simultaneous communication capabilities with a 
different UDP Client and a Server. It represents a 
building block in the UDP Chat Application.

A function of the Client is designed to send a request to Server
to retrieve all the logged in users of the chat application.

A function of the client is designed to send a message to one of
the logged in users. If the user is not logged in, necessary 
exception messages are displayed. 
"""

# Global Variables
is_sign_in = False
server_ip = None
server_port = None
username = None
peers = dict()
PACKET_SIZE = 4096

# Packet Types
SIGN_IN = 1
SEND = 2
LIST = 3
RESPONSE = 4


# Performs the parsing of command line arguments
# and retrieves the Server IP Address, Server Port
# and also the Username if exists.
#
# It also creates two sockets on the machine the client
# is executing to make the client available to communication
# messages from different Clients and the response messages
# from the server at the same time.
def parse_arguments():
    global server_ip
    global server_port
    global username

    # Create an ArgumentParser instance
    # and specify the arguments to the
    # program.
    parser = ArgumentParser()

    # Username, Server IP Address, Server Port,
    # and Client Port on machine.
    parser.add_argument("-u", "--username", type=str, action="store",
                        default="", help="Username to register with the server.")
    parser.add_argument("-sip", "--serverip", type=str, action="store",
                        help="IP Address of the server to connect to.",
                        required=True)
    parser.add_argument("-sp", "--serverport", type=int, action="store",
                        help="Port value of the Server machine to connect to.",
                        required=True)
    args = parser.parse_args()

    # Retrieve the arguments from the parser.
    server_ip = args.serverip
    server_port = args.serverport
    username = args.username

    # Create a tuple for the server, the standard way
    # in the program to communicate machine
    # ip address and port.
    server_address = (server_ip, server_port)

    # Create a Datagram sockets and pass the
    # socket instance to the function to
    # read and send the data.
    sock = None
    listen_sock = None
    try:
        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        listen_sock = socket.socket(socket.AF_INET,  # Internet
                              socket.SOCK_DGRAM)  # UDP
    except socket.error as msg:
        print('Failed to create socket. Error Code : ' + str(msg.errno) + ' Message ' + msg.strerror)

    try:
        listen_sock.bind(('127.0.0.1', 0))
    except socket.error as msg:
        print('Bind failed. Error Code : ' + str(msg.errno) + ' Message ' + msg.strerror)

    # Function which initializes the client
    # with Server and also starts the client
    # workflow.
    login_and_initialize_user(sock, listen_sock, server_address)


# Function which initializes the client with the
# server by initiating the connection with the server
# and signing in the client based on the username.
#
# If the username is not provided in the command line
# arguments, the username request is prompted.
#
# It also maintains the workflow of the client,
# by prompting the client to enter the specific
# commands to perform the list and send jobs.
def login_and_initialize_user(sock, sock2, address):

    # If it is the initial request,
    # it is a sign-in request.
    while True:
        if not is_sign_in:
            # Initiates the login the request with
            # the server and provides the ip details
            # to contact the client with a SEND request.
            login_signup_user(sock, sock2, address)

            # Initiates a dedicated thread to constantly listen
            # to the incoming SEND communication messages from
            # different clients.
            t = Thread(target=listen_to_messages, args=(sock2,))
            t.start()
            print("Setup Successful.")
        else:
            # If the user has already been successfully signed-in
            command = input("-> Enter the command(LIST, SEND): ")
            command = command.split(" ")
            if command[0].upper() == "LIST":
                send_list_request_to_server(sock, address)
            elif command[0].upper() == "SEND":
                send_request_to_user(sock, command[1], ' '.join(command[2:]), address)


# The dedicated thread worker or function which
# constantly listens on a port made available
# by system. Once the message are received,
# it processes and displays the same to the
# user.
def listen_to_messages(sock2):
    print("Listening Thread Started.")
    while True:
        # I/O Blocking call on the thread.
        data, addr = sock2.recvfrom(1024)

        # Transforms the data in the byte format
        # to an object of type Packet.
        data = pickle.loads(data)

        # Retrieves the details required to
        # print the information to user.
        peer_user_name = data.get_username()
        print("\n+< Received Message from ", peer_user_name, addr, ": ", end='')
        print(data.get_message())


# Function which performs the action of sending
# the SEND request to another client/user.
#
# It takes the
#     * socket details on which the information
#       needs to be transferred.
#     * user name of the client/user.
#     * message to send to the user.
#     * server address to contact the server
#       when the user details are not available
#       with the client.
def send_request_to_user(sock, buddy_user_name, message, server_address):

    # Check if the requested user name
    # exists in the clients user dictionary.
    if buddy_user_name in peers:
        # If the exists, retrieve the
        # user details associated with its
        # instance.
        user = peers[buddy_user_name]
        buddy_ip_address = user.get_ip_address()
        buddy_machine_port = user.get_machine_port()

        # Using the sock associated with the client.
        # send the user the message.
        pack = Packet()
        pack.pack(type=SEND, message=message, username=username)

        send_packet(pack, sock, (buddy_ip_address, buddy_machine_port))
    else:
        # If the buddy/user details are not available,
        # contact the server for their details.
        print("Buddy Username does not exist in my user dictionary.")
        print("Requesting server for details.")
        send_list_request_to_server(sock, server_address)
        user = None

        # Once the user dictionary is updated,
        # check if the specified is available,
        # if he is not available, stop the workflow
        # of sending a SEND message.
        try:
            user = peers[buddy_user_name]
        except KeyError:
            print("Cannot Send message to: ", buddy_user_name)
            print("User not logged in.")
            return

        # If he is available, retrieve the required
        # details and send the message.
        buddy_ip_address = user.get_ip_address()
        buddy_machine_port = user.get_machine_port()

        # Using the sock associated with the client.
        # send the user the message.
        pack = Packet()
        pack.pack(type=SEND, message=message, username=username)

        send_packet(pack, sock, (buddy_ip_address, buddy_machine_port))


# Function which performs the action of sending
# the LIST request to server.
#
# It takes the
#     * socket details on which the information
#       needs to be transferred.
#     * server address to contact the server
def send_list_request_to_server(sock, address):
    # Generate the packet to
    # send to the server.
    pack = Packet()
    pack.pack(type=LIST)

    send_packet(pack, sock, address)

    # Receive the response from the server.
    response = receive_packet(sock)

    # Type check and process the response.
    if response.get_type() == RESPONSE:
        users = response.get_message()
        process_user_list(users)
    else:
        print("-- Invalid Return Message Type")


# Helper function to send_list_request_to_server function
# which processes the list of users retrieved from the server
# response and adds it to the client user dictionary.
def process_user_list(users):
    global peers

    print("Signed In Users: ", end='')
    for item in users:
        user_name = item.get_user_name()
        peers[user_name] = item
        print(user_name, end=', ')
    print("\n")


# Function which identifies the parameters required
# to send a login request to the server and packs the
# message to send to the server.
def login_signup_user(sock, sock2, address):
    global username
    global is_sign_in

    data = Packet()
    # If username is not entered as
    # a parameter,
    p = username
    if username == '':
        while True:
            username = input("Enter the user-name for connection: ")
            if username:
                data.pack(type=PacketType.LOGIN,
                          message=sock2.getsockname(),
                          username=username)
                break
            else:
                continue
    else:
        data.pack(type=PacketType.LOGIN,
                  message=sock2.getsockname(),
                  username=username)

    # Send the packet and process the response
    # Based on the response, confirm if the client
    # logged in successfully or not.
    send_packet(data, sock, address)
    data = receive_packet(sock)
    if data.get_type() == RESPONSE:
        is_sign_in = True


# Function which receives the data
# on the socker specified in the parameter.
# It receives 1024 bytes of data on the socket.
def receive_packet(sock):
    # Place a I/O Blocking call on the machine.
    # Receive the bytes based on the packet size.
    data, addr = sock.recvfrom(PACKET_SIZE)

    # Transform the received bytes into Pickle
    # data.
    data = pickle.loads(data)

    # Return the data to the functions
    # calling it.
    return data


# Function which sends the data on the
# socket specified to the address given
# in the parameters.
def send_packet(data, sock, address):
    # Serialize the message and send it on
    # the socket.
    data = pickle.dumps(data)
    sock.sendto(data, address)


if __name__ == '__main__':
    parse_arguments()
