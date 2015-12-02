#ifndef __IO_MANAGER_H__
#define __IO_MANAGER_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <sstream>
#include <vector>

#include "decafs_types/limits.h"
#include "decafs_types/file_types.h"
#include "persistent_stl/persistent_map.h"
#include "persistent_metadata/persistent_metadata_c_api.h"
#include "volatile_metadata/volatile_metadata_c_api.h"
#include "distribution_strategy/distribution_strategy.h"
#include "replication_strategy/replication_strategy.h"
#include "access/access.h"

#define NODE_FAILURE -1

#define CHUNK_NOT_FOUND -1
#define NODE_NOT_FOUND -2
#define REPLICA_CHUNK_NOT_FOUND -3
#define REPLICA_NODE_NOT_FOUND -4

#define CHUNK_ID_INIT 1

using namespace std;

class IO_Manager {
  private:
    // Variables
    PersistentMap<struct file_chunk, int> chunk_to_node;
    PersistentMap<struct file_chunk, int> chunk_to_replica_node;
    //map<struct file_chunk, int> chunk_to_node;
    //map<struct file_chunk, int> chunk_to_replica_node;

    const char *node_metadata_filename = "io_manager_node_metadata.dat";
    const char *replica_metadata_filename = "io_manager_replica_metadata.dat";
    
    // Helper Functions
    bool chunk_replica_exists (struct file_chunk);
    //Given a file_id, list all chunks that are part of the file
    std::vector<struct file_chunk> get_all_chunks (uint32_t file_id);
    // Given offset (a stripe offset) sets id to be the chunk id
    // that the offset it is, and chunk_offset to the offset within
    // the chunk
    void get_first_chunk (uint32_t *id, uint32_t chunk_size, int *chunk_offset, int offset);

  public:
    IO_Manager();
    
    void init(char *metadata_path);

    /*
     *	Translates a read request from the stripe level to the chunk level.
     *	The correct behavior of this function depends on the
     *	Distribution and Replication strategies that are in place.
     *
     *   @return the number of chunks that participated in the read
     */
    uint32_t process_read_stripe (uint32_t request_id, uint32_t file_id,
                                  char *pathname, uint32_t stripe_id,
                                  uint32_t stripe_size, uint32_t chunk_size,
                                  const void *buf, int offset, size_t count);

    /*
     *	Translates a write request into a series of chunk writes and handles
     *	replication. 
     *	The correct behavior of this function depends on the
     *	Distribution and Replication strategies that are in place.
     *
     *   All requests sent to the access module for primary storage writes must be
     *   send with request_id.
     *   All requests sent to the access module for replica writes must bes sent with
     *   replica_request_id.
     *
     *   The number of requests sent to the access module for primary storage writes
     *   must be returned in chunks_written.
     *   The number of requests sent to the access module for replica writes must
     *   be returned in replica_chunks_written.
     */
    void process_write_stripe (uint32_t request_id, uint32_t replica_request_id,
                               uint32_t *chunks_written,
                               uint32_t *replica_chunks_written,
                               uint32_t file_id, char *pathname,
                               uint32_t stripe_id, uint32_t stripe_size,
                               uint32_t chunk_size, const void *buf,
                               int offset, size_t count);
    
    /*
     *   Delete all chunks and replicas for a given file.
     *
     *   @return the number of chunks that participated in the delete
     */
    uint32_t process_delete_file (uint32_t request_id, uint32_t file_id);
    
    /*
     * Get information about the storage locations of chunks within a file.
     */
    char * process_file_storage_stat (struct decafs_file_stat file_info);

    /*
     *	Set the storage location (node id) for a given chunk of a file.
     *   @return the node id
     */
    int set_node_id (uint32_t file_id, uint32_t stripe_id, uint32_t chunk_num,
                     uint32_t node_id);

    /*
     *	Get the storage location (node id) for a given chunk of a file.
     *   @return CHUNK_NOT_FOUND if the chunk hasn't been stored <properly>
     */
    int get_node_id (uint32_t file_id, uint32_t stripe_id, uint32_t chunk_num);

    /*
     *	Set the storage location (node id) for a given replica of a
     *	chunk of a file.
     */
    int set_replica_node_id (uint32_t file_id, uint32_t stripe_id,
                             uint32_t chunk_num, uint32_t node_id);

    /*
     *	Get the storage location (node id) for a given replica of a
     *	chunk of a file.
     */
    int get_replica_node_id (uint32_t file_id, uint32_t stripe_id, uint32_t chunk_num);

    /*
     *	Fill in struct decafs_file_stat structure that provides information
     *	about where the chunks live for a specific file.
     */
    int stat_file_name (char *pathname, struct decafs_file_stat *buf);
    int stat_file_id (uint32_t file_id, struct decafs_file_stat *buf);

    /*
     *	Fill in struct decafs_file_stat structure that provides information
     *	about where the stripes live for a specific file.
     */
    int stat_replica_name (char *pathname, struct decafs_file_stat *buf);
    int stat_replica_id (uint32_t file_id, struct decafs_file_stat *buf);

    /*
     * Determines if a chunk exists.
     */
    bool chunk_exists (struct file_chunk);

    /*
     *	Ensure that all filedata is written to disk.
     */
     void sync();

};
    
#endif
