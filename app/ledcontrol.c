#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/leddriver"

          int
          main(int argc, char *argv[]) {
  int fd;
  ssize_t ret;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s [on|off]\n", argv[0]);
    return -1;
  }

  if (strcmp(argv[1], "on") != 0 && strcmp(argv[1], "off") != 0) {
    fprintf(stderr, "Error: argument must be 'on' or 'off'\n");
    return -1;
  }

  fd = open(DEVICE_PATH, O_WRONLY);
  if (fd < 0) {
    perror("Failed to open " DEVICE_PATH);
    return -1;
  }

  ret = write(fd, argv[1], strlen(argv[1]));
  if (ret < 0) {
    perror("Failed to write to " DEVICE_PATH);
    close(fd);
    return -1;
  }

  printf("LED turned %s\n", argv[1]);
  close(fd);
  return 0;
}
