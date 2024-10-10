/* Copyright 2023 <Tudor Maria-Elena> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"

#define LABEL 100000
#define SERVER_REP 3

struct load_balancer
{
	unsigned int *hashring;
	server_memory **servers;
	/* number of servers */
	unsigned int size;
};

unsigned int hash_function_servers(void *a)
{
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a)
{
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

load_balancer *init_load_balancer()
{
	load_balancer *lb = malloc(sizeof(load_balancer));
	DIE(!lb, "Malloc failed");

	/* allocating memory for the array of hash values of the
	servers (hashring) and the array of servers */
	lb->hashring = malloc(SERVER_REP * sizeof(unsigned int));
	DIE(!lb->hashring, "Malloc failed");

	lb->servers = malloc(sizeof(server_memory *));
	DIE(!lb->servers, "Malloc failed");

	lb->size = 0;

	return lb;
}

unsigned int binary_search(load_balancer *main, unsigned int size,
						   unsigned int value)
{
	/* using binary search to find the index of the
	position where a hash value should be put in the
	sorted hashring */
	int low = 0;
	int high = (int)size;
	int middle, index = size + 1;

	while (low <= high) {
		middle = (low + high) / 2;
		if (value == main->hashring[middle]) {
			index = middle;
			break;
		} else if (value > main->hashring[middle]) {
			low = middle + 1;
		} else {
			high = middle - 1;
			index = middle;
		}
	}

	return index;
}

unsigned int add_in_hashring(load_balancer *main, unsigned int size,
							 unsigned int hash)
{
	/* adding values in the hashring */
	if (size == 0) {
		main->hashring[0] = hash;
		return 0;
	} else if (size == 1) {
		if (hash < main->hashring[0]) {
			main->hashring[1] = main->hashring[0];
			main->hashring[0] = hash;
			return 0;
		} else {
			main->hashring[1] = hash;
			return 1;
		}
	}

	/*using binary search if there are at least two elements
	in the array */
	unsigned int position = binary_search(main, size - 1, hash);

	for (unsigned int i = size; i > position; i--) {
		/* extending the hashring*/
		main->hashring[i] = main->hashring[i - 1];
	}

	main->hashring[position] = hash;

	return position;
}

void rebalance_server(load_balancer *main, unsigned int new_hash,
					  unsigned int new_server_position, unsigned int
					  next_server_position, unsigned int prev_hash)
{
	/* checking the objects on the successor of the
	newly added server and changing their place in the load
	balancer if necessary */
	server_memory *new_server = main->servers[new_server_position];
	server_memory *rebalanced_server = main->servers[next_server_position];

	for (unsigned int i = 0; i < rebalanced_server->ht->hmax; i++) {
		/* going through the values of the hashtable of the
		already existing server*/
		linked_list_t *list = rebalanced_server->ht->buckets[i];
		ll_node_t *node = list->head;

		while (node) {
			info *data = (info *)node->data;
			node = node->next;

			unsigned int hash = hash_function_key(data->key);
			if(main->hashring[0] == new_hash) {
				/* if the new server is added first on the
				hashring, checking if the elements are placed at
				the "beginning" of the hashring (before the first
				server) or at the "ending" (after the last server) */
				if (hash < new_hash || hash > prev_hash) {
					server_store(new_server, data->key, data->value);
					server_remove(rebalanced_server, data->key);
				}
			} else if (hash < new_hash && hash > prev_hash) {
				server_store(new_server, data->key, data->value);
				server_remove(rebalanced_server, data->key);
			}
		}
	}
}

void get_next_server_position(load_balancer *main, unsigned int *position,
							  unsigned int size)
{
	unsigned int ok = 0;

	if (*position == size) {
		*position = -1;
	}

	for (unsigned int i = 0; i < main->size; i++) {
		/* finding the position on the server array of the server
		that has the hash value on the position needed */
		if (main->hashring[*position + 1] == main->servers[i]->hash1 ||
			main->hashring[*position + 1] == main->servers[i]->hash2 ||
			main->hashring[*position + 1] == main->servers[i]->hash3)
			ok = 1;

		if (ok == 1) {
			*position = i;
			return;
		}
	}
}

server_memory *init_server_hash_id(unsigned int server_id, unsigned int hash1,
								   unsigned int hash2, unsigned int hash3)
{
	/* initialising memory for a server and filling in
	the numeric values */
	server_memory *server = init_server_memory();

	server->id = server_id;

	server->hash1 = hash1;
	server->hash2 = hash2;
	server->hash3 = hash3;

	return server;
}

void rearrange_load_balancer(load_balancer *main, unsigned int size,
							 unsigned int hash, unsigned int pos_server)
{
	unsigned int position;
	/* adding the hash value in the hasring*/
	position = add_in_hashring(main, size, hash);

	/* getting the hash value of the server that
	is placed before the one just added on the hashring */
	unsigned int old_hash;
	if (position != 0)
		old_hash = main->hashring[position - 1];
	else
		old_hash = main->hashring[size];

	/* getting the position in the server array of the server
	that is next on the hashring */
	get_next_server_position(main, &position, size);

	if (pos_server != position)
		rebalance_server(main, hash, pos_server, position, old_hash);
}

void loader_add_server(load_balancer *main, int server_id)
{
	if (main->size == 0) {
		main->size++;
	} else {
		/* resizing the load balancer */
		main->size++;

		main->hashring = realloc(main->hashring, main->size * SERVER_REP *
								 sizeof(unsigned int));
		DIE(!main->hashring, "Realloc failed");

		main->servers = realloc(main->servers, main->size * sizeof(server_memory *));
		DIE(!main->servers, "Realloc failed");
	}

	/* calculating the values of the three replicas that
	will be added on the hashring */
	unsigned int hash1 = hash_function_servers(&server_id);

	server_id += LABEL;
	unsigned int hash2 = hash_function_servers(&server_id);

	server_id += LABEL;
	unsigned int hash3 = hash_function_servers(&server_id);

	server_id -= 2 * LABEL;
	unsigned int pos_server = main->size - 1;

	main->servers[pos_server] = init_server_hash_id(server_id,
								hash1, hash2, hash3);

	/* adding the correct elements on the new server */
	rearrange_load_balancer(main, pos_server * SERVER_REP, hash1, pos_server);
	rearrange_load_balancer(main, pos_server * SERVER_REP + 1,
							hash2, pos_server);
	rearrange_load_balancer(main, pos_server * SERVER_REP + 2,
							hash3, pos_server);
}

unsigned int get_position(load_balancer *main, unsigned int id)
{
	/* gets the position in the server array of the
	server with the id transmited as parameter */
	unsigned int ok = 0;
	for (unsigned int i = 0; i < main->size; i++) {
		if (main->servers[i]->id == id)
			ok = 1;

		if (ok == 1) {
			return i;
		}
	}

	return main->size;
}

void remove_from_hashring(load_balancer *main, unsigned int size,
						  unsigned int hash)
{
	/* removing the hash value from the hashring and
	shifting the remaining values */
	unsigned int position = binary_search(main, size - 1, hash);

	for (unsigned int i = position; i < size - 1; i++) {
		main->hashring[i] = main->hashring[i + 1];
	}
}

void remove_objects(load_balancer *main, server_memory *server)
{
	/* going through the hashtable of the removed server
	and redistributing the elements */
	for (unsigned int i = 0; i < server->ht->hmax; i++) {
		linked_list_t *list = server->ht->buckets[i];
		ll_node_t *node = list->head;

		while (node) {
			info *data = (info *)node->data;

			int id;
			loader_store(main, (char *)data->key, (char *)data->value, &id);

			node = node->next;
		}
	}
}

void loader_remove_server(load_balancer *main, int server_id)
{
	/* calculating the values of the three hashes of the
	server */
	unsigned int hash1 = hash_function_servers(&server_id);

	server_id += LABEL;
	unsigned int hash2 = hash_function_servers(&server_id);

	server_id += LABEL;
	unsigned int hash3 = hash_function_servers(&server_id);

	server_id -= 2 * LABEL;
	unsigned int pos = get_position(main, server_id);


	remove_from_hashring(main, main->size * SERVER_REP, hash1);
	remove_from_hashring(main, main->size * SERVER_REP - 1, hash2);
	remove_from_hashring(main, main->size * SERVER_REP - 2, hash3);

	main->size--;
	/* creating a copy of the server that is going to be removed,
	so as to be freed after shifting the other servers */
	server_memory *old_server = main->servers[pos];

	/* resizing the hashring to be smaller */
	main->hashring = realloc(main->hashring,
							 main->size * SERVER_REP * sizeof(unsigned int));
	DIE(!main->hashring, "Realloc failed");

	for(unsigned int i = pos; i < main->size; i++)
		main->servers[i] = main->servers[i + 1];

	/* freeing the old server */
	remove_objects(main, old_server);
	free_server_memory(old_server);

	/* resizing the servers array to be smaller */
	main->servers = realloc(main->servers, main->size * sizeof(server_memory *));
	DIE(!main->servers, "Realloc failed");
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	unsigned int hash = hash_function_key(key);

	/* finding the position on the hashring where
	the new object should go to*/
	unsigned int position = binary_search(main, main->size * SERVER_REP - 1, hash);
	if (position == main->size * SERVER_REP)
		position = 0;

	int ok = 0;
	for (unsigned int i = 0; i < main->size; i++) {
		/* finding the server that has the hash value
		at the position required on the hashring */
		if (main->hashring[position] == main->servers[i]->hash1 ||
			main->hashring[position] == main->servers[i]->hash2 ||
			main->hashring[position] == main->servers[i]->hash3)
			ok = 1;

		if (ok == 1) {
			server_store(main->servers[i], key, value);
			*server_id = (int)main->servers[i]->id;
			return;
		}
	}
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	unsigned int hash = hash_function_key(key);

	/* finding the position on the hashring where
	the object belongs to */
	unsigned int position = binary_search(main, main->size * SERVER_REP - 1, hash);
	if (position == main->size * SERVER_REP)
		position = 0;

	int ok = 0;

	for (unsigned int i = 0; i < main->size; i++) {
		/* finding the server that has the hash value
		at the position required on the hashring */
		if (main->hashring[position] == main->servers[i]->hash1 ||
			main->hashring[position] == main->servers[i]->hash2 ||
			main->hashring[position] == main->servers[i]->hash3)
			ok = 1;

		if (ok == 1) {
			*server_id = (int)main->servers[i]->id;
			return server_retrieve(main->servers[i], key);
		}
	}

	return NULL;
}

void free_load_balancer(load_balancer *main)
{
	/* freeing the memory allocated dynamically */
	free(main->hashring);

	for (unsigned int i = 0; i < main->size; i++) {
		free_server_memory(main->servers[i]);
	}

	free(main->servers);

	free(main);
}
