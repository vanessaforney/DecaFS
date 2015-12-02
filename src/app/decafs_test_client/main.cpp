#include "network_core/barista_server.h"
#include "network_core/espresso_client.h"
#include "../decafs_barista/decafs_barista.h"
#include "network_core/decafs_client.h"

#include <stdio.h>
#include <thread>
#include "network_core/barista_network_helper.h"

void check_num_bytes(int actual, int expected) {
  if (actual != expected) {
    printf("actual: %d expected: %d\n", actual, expected);
    exit(-1);
  }
}

int main(int argc, char** argv) {
  int port = 3899, fd, bytes, close;
  char filename[] = "testfile";
  char test_read[1000];
  char first_write[] = "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 "
    "22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 "
    "46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 "
    "70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 "
    "94 95 96 97 98 99 100";

  /* Opens DecaFS client. */
  DecafsClient client("localhost", port, 2);
  client.openConnection();
  sleep(1);

  /* Opens file. */
  printf("------------ DECAFS CLIENT OPEN TEST ------------\n");
  fd = client.open(filename, O_RDWR);
  if (fd <= 0) {
    printf("open returned: %d\n", fd);
  }
  sleep(1);

  // /* Writes to file. */
  printf("------------ DECAFS CLIENT WRITE TEST ------------\n");
  bytes = client.write(fd, first_write, strlen(first_write));
  check_num_bytes(bytes, strlen(first_write));
  sleep(1);

  /* Prints file storage stat. */
  client.file_storage_stat(filename);
  sleep(1);

  /* Take node down. */
  printf("------------ TAKE OFFLINE NOW! ------------\n");
  sleep(10);

  /* Reads from the file. */
  printf("------------ DECAFS CLIENT READ TEST ------------\n");
  client.lseek(fd, 0, SEEK_SET);
  bytes = client.read(fd, test_read, strlen(first_write));
  check_num_bytes(bytes, strlen(first_write));
  sleep(1);
  
  /* Check result. */
  if (strncmp(test_read, first_write, strlen(first_write)) != 0) {
    printf("TEST FAIL! :(\n");
    printf("Read buffer:\n");
    printf("%s\n", test_read);
    printf("Expected buffer:\n");
    printf("%s\n", first_write);
  } else {
    printf("TEST PASS! :)\n");
  }
  
  /* Closed the file. */
  printf("------------ DECAFS CLIENT CLOSE TEST ------------\n");
  close = client.close(fd);
  if (close < 0) {
    printf("Close failed: %d\n", close);
  }
  sleep(1);

  return 0;
}