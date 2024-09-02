#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DRIVER_FILE "/dev/chardev"
#define BUF_LEN 80

int main() {
  int fd;
  char msg[BUF_LEN];
  ssize_t len;
  if ((fd = open(DRIVER_FILE, O_RDONLY)) < 0) {
    perror("Cant open file");
    exit(EXIT_FAILURE);
  }
  if ((len = read(fd, msg, 80)) < 0) {
    perror("cant read file");
    exit(EXIT_FAILURE);
  } else if (len == 0) {
    printf("EOF\n");
  } else {
    msg[len] = '\0';
    printf("msg: %s\n", msg);
  }
  return 0;
}
