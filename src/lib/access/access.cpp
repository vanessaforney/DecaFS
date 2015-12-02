#include "access.h"

/* Map of the node id to the list of chunks that were modified. */
static std::map<int, std::list<NetworkChunk>> node_to_chunks;

/* Map of the request id to the type (read or write). */
static std::map<int, int> stripe_id_to_type;

/* Map of stripe id to list of requests for that stripe. */
static std::map<int, std::list<int>> stripe_id_to_requests;

/* Map of strip id to the node to update when all requests complete. */
static std::map<int, int> stripe_id_to_node;

/* Map of strip id to the current XOR value. */
static std::map<int, void *> stripe_id_to_val;


// ------------------------Helper Functions-------------------------
/* Processes new read chunk to generate the XOR value. */
void process_xor_read_chunk(int node_id, struct file_chunk chunk, int fd,
                            int offset, int count) {
  int request_id = get_new_request_id();
  stripe_id_to_requests[chunk.stripe_id].push_back(request_id);

  add_recover_node_request_id(request_id);
  process_read_chunk(request_id, fd, chunk.file_id, node_id, chunk.stripe_id,
                     chunk.chunk_num, offset, NULL, count);
}


/* Updates node. */
void update_node(int node_id, struct file_chunk chunk, int fd, int offset,
                 void *buf, int count, int type, int curr_node_id) {
  /* Stores node id of the stripe being updated. */
  free(stripe_id_to_val[chunk.stripe_id]);
  stripe_id_to_val[chunk.stripe_id] = calloc(count, 1);
  stripe_id_to_node[chunk.stripe_id] = node_id;
  stripe_id_to_type[chunk.stripe_id] = type;
  stripe_id_to_requests[chunk.stripe_id].clear();

  /* Creates read requests for the nodes. */
  for (int node = 1; node < 5; node++) {
    if (node != node_id && chunk_exists(chunk) && node != curr_node_id) {
      process_xor_read_chunk(node, chunk, fd, offset, count);
    }
  }
}


/* Computes the XOR and stores it in the first buffer. */
void compute_xor(char *buf1, char *buf2, int size) {
  for (int index = 0; index < size; index++) {
    buf1[index] = buf1[index] ^ buf2[index];
  }
}


// ------------------------Core Functions---------------------------
void process_read_chunk_response(ReadChunkResponse *read_chunk_response) {
  int stripe_id = read_chunk_response->stripe_id;
  int current_request_id = read_chunk_response->id;
  int size = read_chunk_response->count;

  if (stripe_id_to_requests.count(stripe_id)) {
    std::list<int> ids = stripe_id_to_requests[stripe_id];

    /* If the request id is found compute xor. */
    if (std::find(ids.begin(), ids.end(), current_request_id) != ids.end()) {
      /* Compute xor and remove request id. */
      compute_xor((char *)stripe_id_to_val[stripe_id],
                  (char *)read_chunk_response->data_buffer, size);
      ids.remove(current_request_id);

      /* If the XOR is fully computed then handle. */
      if (ids.size() == 0) {
        if (stripe_id_to_type[stripe_id] == NETWORK_WRITE) {
          /* Create new write request if network write. */
          int node_id = stripe_id_to_node[stripe_id];
          int request_id = get_new_request_id();
          network_write_chunk(request_id, read_chunk_response->fd,
                              read_chunk_response->file_id, node_id,
                              stripe_id, read_chunk_response->chunk_num,
                              read_chunk_response->offset,
                              stripe_id_to_val[stripe_id], size);
        } else {
          /* Otherwise send the data back to the user. */
          send(read_chunk_response->fd, stripe_id_to_val[stripe_id], size, 0);
        }
      }
    }
  }
}

ssize_t process_read_chunk (uint32_t request_id, int fd, int file_id,
                            int node_id, int stripe_id, int chunk_num,
                            int offset, void* buf, int count) {
  struct file_chunk chunk = { file_id, stripe_id, chunk_num };

  if (is_node_up(node_id)) {
    /* Reads chunk from network if node is up. */
    return network_read_chunk (request_id, fd, file_id, node_id, stripe_id,
                               chunk_num, offset, count);
  }
  
  /* Get other node information to create XOR if node is down. */
  update_node(node_id, chunk, fd, offset, buf, count, NETWORK_READ,
              READ_THREE_NODES);
  return NODE_FAILURE;
}


ssize_t process_write_chunk (uint32_t request_id, int fd, int file_id,
                             int node_id, int stripe_id, int chunk_num,
                             int offset, void *buf, int count) {
  struct file_chunk chunk = { file_id, stripe_id, chunk_num };

  /* Updates parity node. */
  update_node(4, chunk, fd, offset, buf, count, NETWORK_WRITE, node_id);
  memcpy(stripe_id_to_val[chunk.stripe_id], buf, count);

  if (is_node_up(node_id)) {
    /* Writes the chunk back through the network if node is up. */
    return network_write_chunk(request_id, fd, file_id, node_id, stripe_id,
                               chunk_num, offset, buf, count);
  }
  
  /* Saves chunk to be read later if node is down. */
  NetworkChunk network_chunk = { node_id, chunk, fd, offset, count };
  node_to_chunks[node_id].push_back(network_chunk);
  return NODE_FAILURE;
}


ssize_t process_delete_chunk (uint32_t request_id, int file_id, int node_id,
                              int stripe_id, int chunk_num) {
  /* Deletes the chunk back through the network if node is up. */
  return network_delete_chunk (request_id, file_id, node_id, stripe_id,
                               chunk_num);
}


void remove_node_down(int node_id) {
  /* Updates all chunks that were written to when node was down. */
  std::list<NetworkChunk>::iterator it;
  for (it = node_to_chunks[node_id].begin();
       it != node_to_chunks[node_id].end(); ++it) {
    update_node(node_id, it->chunk, it->fd, it->offset, NULL, it->count,
                NETWORK_WRITE, READ_THREE_NODES);
  }
}
