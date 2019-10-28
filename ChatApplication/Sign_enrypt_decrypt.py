import os
import pickle

from os import path
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.ciphers import (
    Cipher, algorithms, modes
)
from cryptography.hazmat.primitives.asymmetric import padding, utils

"""
Class which is used to sign, encrypt and decrypt the
packets based on the instance initializing the class.
If the server encrypts the class, server public key and private key
are loaded into the instance. Similarly, if it's the client, the
respective credentials are loaded into the instance.
"""


class SignEncryptDecrypt:
    __destination_public_key = ""
    __destination_public_key_file_path = ""
    __source_public_key = ""
    __source_public_key_file_path = ""
    __private_key = ""
    __private_key_file_path = ""
    __input_file_name = ""
    __plain_text = ""
    __cipher_text_file_name = ""
    __associated_data = ""

    # Constructs an instance of Encrypt class using the
    # parameters provided.
    def __init__(self, sender_pri_key, sender_pub_key):
        self.__private_key_file_path = sender_pri_key
        self.__source_public_key_file_path = sender_pub_key
        self.__load_files_into_program()

    # Loads the private key, public key
    # plain_text and file_text files into
    # the runtime variables.
    def __load_files_into_program(self):

        # Load public and private keys into
        # variables to used.
        try:
            with open(self.__private_key_file_path, "rb") as key_file:
                self.__private_key = serialization.load_pem_private_key(key_file.read(),
                                                                        password=None,
                                                                        backend=default_backend())
            with open(self.__source_public_key_file_path, "rb") as key_file:
                self.__source_public_key = serialization.load_der_public_key(key_file.read(),
                                                                                  backend=default_backend())

        except FileNotFoundError as exception:
            print(exception.filename + " not found.")

        # Load the input file data into variables and generate the
        # plain text.
        # try:
        #     with open(self.__input_file_name, "rb") as input_file:
        #         self.__plain_text = input_file.read()
        # except FileNotFoundError as exception:
        #     print("Input file not found: " + exception.filename)

    # Methof which loads the destination public key
    # and uses for future encryption and decryption processes.
    def load_destination_public_key(self, public_key_path):
        with open(public_key_path, "rb") as key_file:
            self.__destination_public_key = serialization.load_der_public_key(key_file.read(),
                                                                      backend=default_backend())

    # Signs the message passed as input and returns the sigature
    # of the signed message.
    def rsa_signature(self, data):

        # Generate a hash function based on SHA256
        hash = hashes.SHA256()
        hash_object = hashes.Hash(hash, default_backend())

        # In blocks of size of 2048 bits, sign the
        # data passed to the function.
        data_len = len(data)
        read_size = 2048
        if data_len > 2048:
            with open(data, 'rb') as obj:
                while data_len > 0:
                    hash_object.update(obj.read(read_size))
                    data_len = data_len - read_size
                    read_size = data_len
        else:
            hash_object.update(data)

        digest = hash_object.finalize()

        signature = self.__private_key.sign(digest,
                                            padding.PSS(mgf=padding.MGF1(hashes.SHA256()),
                                                        salt_length=padding.PSS.MAX_LENGTH),
                                            utils.Prehashed(hash))

        return signature