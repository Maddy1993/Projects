from Type import PacketType

"""
The Packet class represents a Packet that is
being transmitted in the UDP socket.
It has fields to represent the user sending the
packet, packet type to distinguish multiple packets
and the message which holds data of any type.

When the application is modified to handle out-of-order
packets, or packets greater than size of the current packet
size, the Packet class can be used with zero modification.
"""


class Packet:
    __message = None
    __sequence_number = None
    __type = None
    __username = None
    __is_split = False

    def pack(self, type: PacketType, message: str='',
             sequence: int=0, username: str='',
             split: bool=False):
        self.__message = message
        self.__sequence_number = sequence
        self.__type = type
        self.__username = username
        self.__is_split = split

    def get_type(self):
        return self.__type

    def get_sequence(self):
        return self.__sequence_number

    def get_message(self):
        return self.__message

    def get_username(self):
        return self.__username

    def is_split(self):
        return self.__is_split
