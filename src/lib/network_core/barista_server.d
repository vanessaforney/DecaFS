barista_server.o: barista_server.cpp barista_server.h network_packets.h \
  packet_flags.h ../../lib/net_tcp/tcp_server.h \
  ../../lib/net_tcp/connection_to_client.h open_packet.h write_packet.h \
  read_packet.h close_packet.h lseek_packet.h opendir_packet.h \
  file_storage_stat_packet.h mkdir_packet.h remove_packet.h \
  ../../lib/volatile_metadata/volatile_metadata_c_api.h \
  ../../lib/decafs_types/file_types.h ../../lib/decafs_types/limits.h \
  ../../lib/decafs_types/ip_address.h \
  ../../lib/barista_core/barista_core.h \
  ../../lib/decafs_types/messages.h ../../lib/io_manager/io_manager.h \
  ../../lib/persistent_stl/persistent_map.h \
  ../../lib/persistent_metadata/persistent_metadata_c_api.h \
  ../../lib/distribution_strategy/distribution_strategy.h \
  ../../lib/replication_strategy/replication_strategy.h \
  ../../lib/access/access.h \
  ../../lib/network_core/barista_network_helper.h open_packet_response.h \
  write_response_packet.h read_response_packet.h close_response_packet.h \
  lseek_response_packet.h opendir_response_packet.h \
  file_storage_stat_response_packet.h mkdir_response_packet.h \
  remove_response_packet.h ../../lib/io_manager/io_manager_c_api.h \
  ../../lib/persistent_metadata/persistent_metadata.h \
  ../../lib/persistent_metadata/persistent_metadata_impl.h \
  ../../lib/volatile_metadata/volatile_metadata.h \
  ../../lib/monitored_strategy/monitored_strategy_c_api.h \
  ../../lib/locking_strategy/locking_strategy.h \
  ../../lib/monitored_strategy/monitored_strategy.h
