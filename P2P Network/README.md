# _*Distributed Hash Table:*_ **CHORD**
## A Scalable Peer-to-Peer lookup service

The system represents a distributed storage where the objects are selectively stored in the
system based on their hash. The implementation replicates a system in which the dht-peer communicates to another
dht-peer to build a CHORD ring, and each dht-peer represents a database. The request to store, retrieve or delete an object
is made by the client, where the  master node is responsible for directly handling the requests from the client.

### Elements of the System
* Master Node
> The master node of system is the entry point for any dht peer joining the network or for any client making the request
> to store, retrieve or delete the objects stored. Master Node is the first peer to be started and its IP address will be know by any peer
> that wants to join. The master node also initiates the request to lookup for available peers and detects any failures
> within the system.

* DHT Peer
> The dht peer is responsible to be part of the system once joined and should propagate the requests for object lookup.
> The peer is also reposible for remembering its predecessor and successor for propagating the request.

* Client
> The client accesses the system with a unified interface allowing him to abstract the implementation details and access
> the  system without any complexity. To achieve this uniformity, the client has knowledge of the root IP address ahead
> of time. The client has designated functionality requests and the root node understands only these requests from the
> client. And these requests are: Store an Object, Retrieve an Object, and List the available Operations. Each operation
> returns a success or failure message along with the data it is pre-defined to return.

## Execution
* Execute a DHT Peer: 
> The -m type parameter will specify if this peer is the root or not, 1 means root, 0, regular.
> Default is 0, regular peer.

`$ ./dht_peer <-m type> <-p own_port <-h own_hostname> <-r root_port> <-R root_hostname>`

* Execute a Client:

`$ ./dht_client <-p client_port> <-h client_hostname> <-r root_port> <-R root_hostname>`
