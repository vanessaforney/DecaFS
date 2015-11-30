#include "access.h"

/* Map of the node id to the list of chunks that were modified. */
static std::map<int, std::list<NetworkChunk>> node_to_chunks;

/* Map of the request id to the type (read or write). */
static std::map<int, std::list<int>> request_id_to_type;

/* Map of stripe id to list of requests for that stripe. */
static std::map<int, std::list<int>> stripe_id_to_requests;

/* Map of strip id to the node to update when all requests complete. */
static std::map<int, int> stripe_id_to_node;

/* Map of strip id to the current XOR value. */
static std::map<int, void *> stripe_id_to_val;

/* Current request id. */
static int request_id = 0;

/* Computes the XOR and stores it in the first buffer. */
void compute_xor(char *buf1, char *buf2, int size) {
   for (int index = 0; index < size; index++) {
      buf1[index] = buf1[index] ^ buf2[index];
   }
}

void process_read_chunk_response(ReadChunkResponse *read_chunk_response) {
   int node_id, stripe_id = read_chunk_response->stripe_id;
   int size = read_chunk_response->count;
   std::list<int> ids = stripe_id_to_requests[stripe_id];
   auto it = std::find(ids.begin(), ids.end(), read_chunk_response->id);

   if (it != ids.end()) {
      compute_xor((char *)stripe_id_to_val[stripe_id], (char *)read_chunk_response->data_buffer, size);
      ids.remove(read_chunk_response->id);

      if (ids.size() == 0) {
         node_id = stripe_id_to_node[stripe_id];

         if (stripe_id_to_requests[node_id] == WRITE) {
            network_write_chunk(request_id++, read_chunk_response->fd, read_chunk_response->file_id,
              node_id, stripe_id, read_chunk_response->chunk_num, read_chunk_response->offset,
              stripe_id_to_val[stripe_id], size);
         } else {
            send(read_chunk_response->fd, stripe_id_to_val[stripe_id], size, 0);
         }
      }
   }
}

ssize_t process_read_chunk (uint32_t request_id, int fd, int file_id,
                            int node_id, int stripe_id, int chunk_num,
                            int offset, void* buf, int count) {
  if (is_node_up(node_id)) {
    return network_read_chunk (request_id, fd, file_id, node_id, stripe_id,
                             chunk_num, offset, count);
  } else {
    stripe_id_to_node[stripe_id] = node_id;
    free(stripe_id_to_val[stripe_id]);
    stripe_id_to_val[stripe_id] = calloc(count, 1);
    request_id_to_type[request_id] = READ;

    for (int node = 1; node < 5; node++) {
       if (node != node_id) {
          process_read_chunk (request_id, fd, file_id, node_id, stripe_id,
             chunk_num, offset, buf, count);
          stripe_id_to_requests[stripe_id].push_back(request_id++);
       }
    }
  }
}

ssize_t process_write_chunk (uint32_t request_id, int fd, int file_id,
                             int node_id, int stripe_id, int chunk_num,
                             int offset, void *buf, int count) {
  void *ptr = NULL;
  stripe_id_to_node[chunk.stripe_id] = 4; // Always write to parity node.
  free(stripe_id_to_val[chunk.stripe_id]);
  stripe_id_to_val[chunk.stripe_id] = calloc(count, 1);
  memcpy(stripe_id_to_val[chunk.stripe_id], buf, count);
  request_id_to_type[request_id] = WRITE;

  for (int node = 1; node < 4; node++) {
    if (node != node_id && io_manager.chunk_exists(chunk)) {
       process_read_chunk (request_id, fd, chunk.file_id, node, chunk.stripe_id,
          chunk.chunk_num, offset, ptr, count);
       stripe_id_to_requests[chunk.stripe_id].push_back(request_id++);
    }
  }

  if (is_node_up(node_id)) {
    return network_write_chunk (request_id, fd, file_id, node_id, stripe_id,
                                chunk_num, offset, buf, count);
  } else {
    NetworkChunk network_chunk = { chunk, count };
    node_to_chunks[node_id].push_back(network_chunk);
  }

  return NODE_FAILURE;
}


ssize_t process_delete_chunk (uint32_t request_id, int file_id, int node_id,
                              int stripe_id, int chunk_num) {

  return network_delete_chunk (request_id, file_id, node_id, stripe_id,
                               chunk_num);
}

/* Reads from the other nodes when the node comes back up. */
void remove_node_down(int node_id) {
  void *buf = NULL;
  struct file_chunk chunk;
  std::list<NetworkChunk>::iterator it;

  for (it = node_to_chunks[node_id].begin(); it != node_to_chunks[node_id].end(); ++it) {
    chunk = it->chunk;
    stripe_id_to_node[chunk.stripe_id] = node_id;
    free(stripe_id_to_val[chunk.stripe_id]);
    stripe_id_to_val[chunk.stripe_id] = calloc(it->count, 1);
    request_id_to_type[request_id] = WRITE;

    for (int node = 1; node < 5; node++) {
       if (node != node_id && io_manager.chunk_exists(chunk)) {
          process_read_chunk (request_id, it->fd, chunk.file_id, node, chunk.stripe_id,
             chunk.chunk_num, it->offset, buf, it->count);
          stripe_id_to_requests[chunk.stripe_id].push_back(request_id++);
       }
    }
  }
}
