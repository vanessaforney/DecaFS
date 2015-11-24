#include "access.h"

/* Map of the node id to the list of chunks that were modified. */
static std::map<int, std::list<NetworkChunk> > node_to_chunks;

/* Map of paired nodes. */
static std::map<int, int> node_id_to_replica_node_id = {
  {1, 3}, {2, 4}, {3, 1}, {4, 2}
};

ssize_t process_read_chunk (uint32_t request_id, int fd, int file_id,
                            int node_id, int stripe_id, int chunk_num,
                            int offset, void* buf, int count) { 
  /* Checks if node is down and reads from other chunk. */
  if (!is_node_up(node_id)) {
    node_id = node_id_to_replica_node_id[node_id];

    if (!is_node_up(node_id)) {
       return NODE_FAILURE;
    }
  }

  /* Reads chunk from network. */
  return network_read_chunk (request_id, fd, file_id, node_id, stripe_id,
                             chunk_num, offset, count);
}

ssize_t process_write_chunk (uint32_t request_id, int fd, int file_id,
                             int node_id, int stripe_id, int chunk_num,
                             int offset, void *buf, int count) {
  // No buffering
  return network_write_chunk (request_id, fd, file_id, node_id, stripe_id,
                              chunk_num, offset, buf, count);
}


ssize_t process_delete_chunk (uint32_t request_id, int file_id, int node_id,
                              int stripe_id, int chunk_num) {

  return network_delete_chunk (request_id, file_id, node_id, stripe_id,
                               chunk_num);
}
