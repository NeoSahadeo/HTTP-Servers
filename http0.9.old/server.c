#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Must include port number!\n");
    return 0;
  }

  int socket_fd;
  struct sockaddr_in addr;

  // Generate a socket fd, ipv4
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    fprintf(stderr, "Failed to connect to socket\n");
    return -1;
  }

  // converts the port to a number then to a network byte
  // see man 3 htons
  addr.sin_port = htons(atoi(argv[1]));
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_family = AF_INET;
  printf("Setting Port Big-Endian:\t%d\n", addr.sin_port);
  printf("Setting Address:\t\t%d\n", addr.sin_addr.s_addr);
  printf("Setting Socket Family:\t\t%d\n", addr.sin_family);

  // attempts to bind the socket to the address
  if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
    fprintf(stderr, "Failed to bind socket to IP\n");
    return -2;
  }

  // prepate to listen for connections, 5 max connections
  if (listen(socket_fd, 5) != 0) {
    fprintf(stderr, "Failed to start listening...\n");
    return -3;
  }

  // 80 Chars should be the max, +2 for cr lf
  char buffer[82] = {0};
  char buffer_size = sizeof(buffer);
  int client_fd;
  struct sockaddr_in client;
  unsigned int client_len = sizeof(client);
  printf("Server started on:\t\t%s\n", argv[1]);
  for (;;) {
    // Accept will wait for a connection to the socket.
    // When a connection is recieved, the client address is set to the
    // connection ip and create a new socket descriptor.
    client_fd = accept(socket_fd, (struct sockaddr*)&client, &client_len);

    // Print out the connecting ip address
    // see man 3 inet_ntoa
    printf("Client conn from:\t\t%s\n", inet_ntoa(client.sin_addr));

    if (client_fd < 0) {
      fprintf(stderr, "Server failed to connect client\n");
      return -4;
    }

    // Read in the client message
    read(client_fd, buffer, sizeof(buffer));
    printf("Lines recieved:\t\t\t%s\n", buffer);

    // Skip spaces
    int index = -1;
    char c;
    while ((c = buffer[++index]) == ' ')
      ;

    // Check if the first 3chars are "GET"
    if (buffer[index] == 'G' && buffer[index + 1] == 'E' &&
        buffer[index + 2] == 'T') {
    } else {
      FILE* fd = fopen("./error.html", "r");
      if (!fd) {
        fprintf(stderr, "Failed to open error file\n");
        return -5;
      }

      // buffer send the content in 80char chunks
      bzero(buffer, buffer_size);
      buffer[80] = '\r';
      buffer[81] = '\f';
      index = 0;
      c = 0;
      while ((c = fgetc(fd)) != EOF) {
        buffer[index++] = c;
        if (index == 80) {
          if (send(client_fd, buffer, 80, 0) == -1) {
            fprintf(stderr, "Failed to send message");
            return -6;
          }
          index = 0;
        }
      }
      if (index > 0 && send(client_fd, buffer, 82, 0) == -1) {
        fprintf(stderr, "Failed to send message");
        return -7;
      }
      fclose(fd);
      close(client_fd);
    }
  }

  close(socket_fd);
  return 0;
}
