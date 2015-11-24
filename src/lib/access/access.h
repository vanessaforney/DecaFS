#ifndef __ACCESS_H__
#define __ACCESS_H__

#include <sys/types.h>

#include "network_core/barista_network_helper.h"
#include "barista_core/barista_core.h"

#define DATA_NOT_FOUND -1
#define NETWORK_DELETE 1
#define NETWORK_WRITE 0

struct NetworkChunk {
  int network_type; /* Whether the chunk is DELETE or WRITE. */
  uint32_t request_id;
  int node_id;
  struct file_chunk chunk;
  int fd;
  int offset;
  void *buf;
  int count;
};

/*
 * Read data from a chunk at a specific offset.
 * If you are implementing this function: 
 *    If data is being read from an Espresso node, Network
 *    Layer network_read_chunk() must be called.
 */
ssize_t process_read_chunk (uint32_t request_id, int fd, int file_id,
                            int node_id, int stripe_id, int chunk_num,
                            int offset, void* buf, int count);

/*
 * Write data to a chunk at a specific offset.
 * If you are implementing this function: 
 *    If data is being written to an Espresso node, Network
 *    Layer network_write_chunk() must be called.
 */
ssize_t process_write_chunk (uint32_t request_id, int fd, int file_id,
                             int node_id, int stripe_id, int chunk_num,
                             int offset, void *buf, int count);

/*
 * Delete a specific chunk from DecaFS.
 */
ssize_t process_delete_chunk (uint32_t request_id, int file_id, int node_id,
                              int stripe_id, int chunk_num);
#endif