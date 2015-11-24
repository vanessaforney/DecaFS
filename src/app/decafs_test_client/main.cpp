
#include "network_core/barista_server.h"
#include "network_core/espresso_client.h"
#include "../decafs_barista/decafs_barista.h"
#include "network_core/decafs_client.h"

#include <stdio.h>
#include <thread>
#include "network_core/barista_network_helper.h"

int main(int argc, char** argv) {
  int port = 3899;
  char filename[] = "testfile";
  char testread[1000];
  DecafsClient client("10.0.0.100", port, 2);
  client.openConnection();

  sleep(1);

  // OPEN
  std::cout << "------------ DECAFS CLIENT OPEN TEST ----------" << std::endl;
  int fd = client.open(filename, O_RDWR);
  std::cout << "open returned: " << fd << std::endl;
  sleep(1);

  // WRITE
  std::cout << "------------ DECAFS CLIENT WRITE TEST 1 ----------" << std::endl;
  char testwrite[] = "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100";
  int bytes_written = client.write(fd, testwrite, strlen(testwrite));
  std::cout << "write returned: " << bytes_written << ", exepected: " << strlen(testwrite) << std::endl;
  sleep(1);

  // FILE STORAGE STAT
  client.file_storage_stat(filename);
  sleep(1);

  std::cout << "TAKE OFFLINE NOW!" << std::endl;
  sleep(10);

  // READ
  std::cout << "------------ DECAFS CLIENT READ TEST 1 ----------" << std::endl;
  client.lseek(fd, 0, SEEK_SET);
  int bytes_read = client.read(fd, testread, strlen(testwrite));
  std::cout << "read returned: " << bytes_read << ", exepected: " << strlen(testwrite) << std::endl;
  sleep(1);

  // WRITE
  std::cout << "------------ DECAFS CLIENT WRITE TEST 2 ----------" << std::endl;
  char new_write[] = "I LOVE SCRUFF!";
  client.lseek(fd, 0, SEEK_SET);
  bytes_written = client.write(fd, new_write, strlen(new_write));
  std::cout << "write returned: " << bytes_written << ", exepected: " << strlen(new_write) << std::endl;
  sleep(1);

  std::cout << "BRING ONLINE NOW!" << std::endl; 
  sleep(10);

  // READ
  std::cout << "------------ DECAFS CLIENT READ TEST 2 ----------" << std::endl;
  client.lseek(fd, 0, SEEK_SET);
  bytes_read = client.read(fd, testread, strlen(testwrite));
  std::cout << "read returned: " << bytes_read << ", exepected: " << strlen(testwrite) << std::endl;
  sleep(1);
  
  std::cout << "READ BUFFER" << std::endl;
  std::cout << testread << std::endl;
  std::cout << "EXPECTED BUFFER" << std::endl;
  std::cout << new_write << std::endl;

  if (bytes_read != bytes_written) {
    std::cout << "TEST FAIL: WRONG NUMBER OF BYTES READ" << std::endl;
  }
  if (strncmp(testread, new_write, strlen(new_write)) != 0) {
    std::cout << "TEST FAIL: READ NOT EQUAL TO WRITE" << std::endl;
  } else {
    std::cout << "TEST PASS! :)" << std::endl;
  }
  
  // CLOSE
  std::cout << "------------ DECAFS CLIENT CLOSE TEST ----------" << std::endl;
  int close = client.close(fd);
  std::cout << "close returned: " << (close ? "FAIL" : "PASS") << std::endl;
  sleep(1);

  return 0;
}
