#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int fd;
  char buffer[82] = {0};
  size_t bytes_read;
  struct sockaddr_in addr;

  if (argc < 3) {
    printf("Must include port number and message!\n");
    return 0;
  }

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    fprintf(stderr, "Failed to connect to socket\n");
    return -1;
  }

  addr.sin_port = htons(atoi(argv[1]));
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_family = AF_INET;

  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    fprintf(stderr, "Failed to connect to server at port %s \n", argv[1]);
    return -2;
  };

  bzero(buffer, sizeof(buffer));
  strcpy(buffer, argv[2]);
  write(fd, buffer, sizeof(buffer));
  bzero(buffer, sizeof(buffer));
  read(fd, buffer, sizeof(buffer));
  int index = sizeof(buffer) - 1;
  char c1;
  char c2;
  int n;
  printf("buffer:%s\n", buffer);
  while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
    do {
      c1 = buffer[index];

      if (index - 1 == 0)
        break;
      c2 = buffer[index - 1];

      if (c2 == '\r' && c1 == '\f') {
        printf("found\n");
        break;
      }

      index -= 2;

    } while (c2 != '\f' && c1 != '\r');
  }

  close(fd);

  return 0;
}
