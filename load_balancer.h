/* Copyright 2023 <Tudor Maria-Elena> */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "server.h"
#include "hashtable.h"
#include "list.h"

struct load_balancer;
typedef struct load_balancer load_balancer;
struct server_memory;
typedef struct server_memory server_memory;

unsigned int hash_function_servers(void *a);
unsigned int hash_function_key(void *a);

unsigned int binary_search(load_balancer *main, unsigned int high,
						   unsigned int value);

unsigned int add_in_hashring(load_balancer *main, unsigned int size,
							 unsigned int hash);

void rebalance_server(load_balancer *main, unsigned int new_hash,
					  unsigned int new_server_position, unsigned int
					  next_server_position, unsigned int prev_hash);

void rearrange_load_balancer(load_balancer *main, unsigned int size,
							 unsigned int hash, unsigned int pos_server);

void get_next_server_position(load_balancer *main, unsigned int *position,
							  unsigned int size);

void remove_from_hashring(load_balancer *main, unsigned int size,
						  unsigned int hash);
void remove_objects(load_balancer *main, server_memory *server);

unsigned int get_position(load_balancer *main, unsigned int id);

server_memory *init_server_hash_id(unsigned int server_id, unsigned int hash1,
								   unsigned int hash2, unsigned int hash3);

/**
 * init_load_balancer() - initializes the memory for a new load balancer and its fields and
 *                        returns a pointer to it
 *
 * Return: pointer to the load balancer struct
 */
load_balancer *init_load_balancer();

/**
 * free_load_balancer() - frees the memory of every field that is related to the
 * load balancer (servers, hashring)
 *
 * @arg1: Load balancer to free
 */
void free_load_balancer(load_balancer *main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will RETURN via this parameter
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the
 * load across the servers. The chosen server ID will be returned
 * using the last parameter.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 *
 */
void loader_store(load_balancer *main, char *key, char *value, int *server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will RETURN the server ID
		  which stores the value via this parameter.
 *
 * The load balancer will search for the server which should posess the
 * value associated to the key. The server will return NULL in case
 * the key does NOT exist in the system.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 */
char *loader_retrieve(load_balancer *main, char *key, int *server_id);

/**
 * load_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 3 replica labels and it will
 * place them inside the hash ring. The neighbor servers will
 * distribute some the objects to the added server.
 *
 * Hint:
 * Resize the servers array to add a new one.
 * Add each label in the hashring in its appropiate position.
 * Do not forget to resize the hashring and redistribute the objects
 * after each label add (the operations will be done 3 times, for each replica).
 */
void loader_add_server(load_balancer *main, int server_id);

/**
 * load_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute ALL objects stored on the
 * removed server and will delete ALL replicas from the hash ring.
 *
 */
void loader_remove_server(load_balancer *main, int server_id);

#endif /* LOAD_BALANCER_H_ */
