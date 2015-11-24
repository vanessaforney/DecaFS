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
  /* Save node information if node is down. */
  if (!is_node_up(node_id)) {
    struct file_chunk chunk = {file_id, stripe_id, chunk_num};
    void *cpy_buf = calloc(count, sizeof(char));
    memcpy(cpy_buf, buf, count);

    NetworkChunk network_chunk = { node_id, chunk, fd, offset, cpy_buf, count };
    node_to_chunks[node_id].push_back(network_chunk);
    return NODE_FAILURE;
  }

  /* Writes the chunk back through the network if node is up. */
  return network_write_chunk (request_id, fd, file_id, node_id, stripe_id,
      chunk_num, offset, buf, count);
}


ssize_t process_delete_chunk (uint32_t request_id, int file_id, int node_id,
                              int stripe_id, int chunk_num) {
  /* Deletes the chunk back through the network if node is up. */
  return network_delete_chunk (request_id, file_id, node_id, stripe_id,
                               chunk_num);
}


void remove_node_down(int node_id) {
  printf ("Inside remove_node_down...\n");

  /* Recover all values in the list. */
  std::list<NetworkChunk>::iterator it;
  for (it = node_to_chunks[node_id].begin(); it != node_to_chunks[node_id].end(); ++it) {
    /* Get new request id and add to recover node request id. */
    uint32_t request_id = get_new_request_id();
    add_recover_node_request_id(request_id);

    /* Writes the chunk back through the network. */
    struct file_chunk chunk = it->chunk;
    network_write_chunk (request_id, it->fd, chunk.file_id, 
    it->node_id, chunk.stripe_id, chunk.chunk_num, it->offset, 
    it->buf, it->count);
  }
  node_to_chunks[node_id].clear();
}
