**Nume: Tudor Maria-Elena**
**Grupă: 311CA**

## Tema 1: Load Balancer

### Description:

* The homework simulates working with servers and concentrates on efficient ways
to store different elements on a load balancer. Crucial in implementing this
were the two structs that I had to create:
* server_memory - this one contains the server in itself which is implemented
using a hashtable, where the elements are stored. I also made the decision to
include the id of the server and the hashes of the three replicas that the server
would have on the hashring to make it easier to work with.
* load_balancer - contains an array named hashring that holds the hash values of
the servers and their replicas sorted in ascending order, a double pointer of
server_memory that stores the pointers to the servers and the parameter size
that represents the number of servers active in the load balancer

### Comments about the homework:

* The functions from server.h were easily implemented as they were basically
just hashtable functions.
* The hardest part was implementing the load balancer and especially the addition
and removal of servers. The store and retrieve operations were easier to write
as they just went through the server array, found the one that corresponded
with the hash of the key and then used a server function.
* The loader_add_server function adds three new positions to the hashring and also
after each one, checks the neighbours of the server for any elements that might
have to change the server they belong to. This was by far the hardest thing to
implement, as there were a lot of little exceptions I didnt think of at first
(for example, what happens to the objects on the first server when one is added
before it on the hashring, or even at the "end" of the hashring, but after the last
hash of the server).
* The loader_remove_server was easier to implement, the most important aspect was
figuring out what memory should be freed. It mirrors the add_server function as it
removes three positions from the hashring and then takes the objects from the removed
server and relocates all of them on other servers.

* I think I could've implemented the load_balancer better because I realised way too
late I could've sorted the servers array by server id so I could've found them easier.
Also, I could've maybe found a more efficient way to copy information from server to
server or from the hashring.

* Working on this homework, I learned about hashtables and how they might be used,
the concept of keys and values became clearer in my head and I understood the
efficiency of hashing.

### Resources:

1. The implementation of list.c and hashtable.c are taken from the exercise on
Lambda Checker.
