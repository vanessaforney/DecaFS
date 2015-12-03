#include "distribution_strategy.h"

extern "C" int put_chunk (uint32_t chunk_num) {
  return chunk_num;
}
