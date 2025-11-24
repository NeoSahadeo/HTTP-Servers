/* CLIENT */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

void exit_fail(char* error, int socket_fd, void* message) {
  fprintf(stderr, "%s\n", error);
  close(socket_fd);
  free(message);
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  uint port;
  char* message = malloc(1024);

  if (argc == 3) {
    port = atoi(argv[1]);
    message = strncpy(message, argv[2], 1024);
    strcat(message, "\r\n");
  } else {
    printf("Requires a port number followed by a message!\n");
    free(message);
    exit(EXIT_FAILURE);
  }

  // Create an unbound socket, ipv4
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1)
    exit_fail("Error creating socket", socket_fd, message);

  // Create an socket struct and assign the ip
  // and port number to it
  struct sockaddr_in socket_addr;
  socket_addr.sin_addr.s_addr = INADDR_ANY;
  socket_addr.sin_port = htons(port);
  socket_addr.sin_family = AF_INET;

  if (connect(socket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) ==
      -1)
    exit_fail("Failed to connect to socket", socket_fd, message);

  write(socket_fd, message, 1024);

  char buffer[82];
  ssize_t bytes_in;
  while ((bytes_in = read(socket_fd, buffer, 82)) > 0) {
    write(STDOUT_FILENO, buffer, bytes_in);
  }

  free(message);
  close(socket_fd);
  return 0;
}
