#ifndef __ACCESS_H__
#define __ACCESS_H__

#include <sys/types.h>
#include <mutex>

#include "network_core/barista_network_helper.h"
#include "barista_core/barista_core.h"

#define DATA_NOT_FOUND -1
#define NETWORK_READ 0
#define NETWORK_WRITE 1
#define READ_THREE_NODES -1
#define SIZE_BUFFER 128
#define HEADER_SIZE 12

struct NetworkChunk {
  int node_id;
  struct file_chunk chunk;
  int fd;
  int offset;
  int count;
};


/*
 * Reads the chunk responses and updates the buffer if appropriate network id.
 */
void process_read_chunk_response(ReadChunkResponse *read_chunk_response);

/*
 *	Read data from a chunk at a specific offset.
 *	If you are implementing this function: 
 *		If data is being read from an Espresso node, Network
 *		Layer network_read_chunk() must be called.
 */
ssize_t process_read_chunk (uint32_t request_id, int fd, int file_id,
                            int node_id, int stripe_id, int chunk_num,
                            int offset, void* buf, int count);

/*
 *	Write data to a chunk at a specific offset.
 *	If you are implementing this function: 
 *		If data is being written to an Espresso node, Network
 *		Layer network_write_chunk() must be called.
 */
ssize_t process_write_chunk (uint32_t request_id, int fd, int file_id,
                             int node_id, int stripe_id, int chunk_num,
                             int offset, void *buf, int count);

/*
 * Delete a specific chunk from DecaFS.
 */
ssize_t process_delete_chunk (uint32_t request_id, int file_id, int node_id,
                              int stripe_id, int chunk_num);

/*
 * Completes all writes that occurred when the node was down.
 */
void remove_node_down(int node_id);

#endif