## Initialization
The Project is based on go language and thus requires a go compiler to execute the project. Step by step instructions about the go language installation can be found at [instructions](https://golang.org/dl/ "instructions")

>Once the setup has been completed, please add the project root, MDFS to the GOPATH

If the above steps are successfully executed, the application can be deployed as stated below.

## Execution
Before the client can execute its commands, the infrastructure needs to be setup and for that purpose, you can use the Makefile to make your life easier.

1. ### Boot up Master server
    </br>From the root directory, open a terminal and/or ssh into another system or boot up the master on the localhost. By default the the master will be setup on port 9999. To the make execution easier, you can use localhost. But if you want to execute everything on different system, you have provide command line arguments to the master, each of the peer and client.

    Command line arguments for master are named and are represented as:
    > serverAddr: server address/host address
    </br>port: port

    Necessary modifications are needed in the makefile to execute the same.
    To boot up the  master, you can use
    >make master

2. ### Boot up Peer servers:
    </br>The peers are maintained as backup and primary on a one-to-one mapping basis. The first server to contact the master will be chosen as primary, and consecutive server contacting the primary will be chosen as backup. As one to one mapping is achieved, the next server to contact master will be chosen as primary. The control-flow goes on.

    The makefile accomadates creating two clusters, with each cluster containing two servers of same category.

    To execute peers
    >make peer_1</br>
    >make peer_2</br>
    make peer_3</br>
    make peer_4</br>

    If the peers are booted up in this order, peer_1 and peer_3 will be the primary, and peer_2 and peer_4 will be the backup

    Once the bootup is completed, initialize the peers by providing them a directory to store the files.

3. ### Client bootup
  <br/>Client can be booted up using the command

  >make client

  Once the client is booted up, please provide a directory from where the client can send and store the received files.

  Once the initialization is completed, CLI is presented to you with the IP address of the hosting system.

  >SEND and RECEIVE are the accepted commands for now.</br>
  >SEND filename</br>
  >RECEIVE filename
