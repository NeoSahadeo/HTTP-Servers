/* SERVER */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <poll.h>
#include <strings.h>
#include <sys/select.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

// Poll system
typedef struct Poll {
} Poll;

void exit_fail(char* error, int socket_fd) {
  fprintf(stderr, "%s\n", error);
  close(socket_fd);
  exit(EXIT_FAILURE);
}

/* Sends the requested file.
 * */
void send_file(int client_socket_fd, const char* restrict path) {
  // Check if file path exists. If not, return an error page.
  if (access(path, F_OK)) {
    send_file(client_socket_fd, "./html/error_doesnotexist.html");
    return;
  }

  FILE* fptr = fopen(path, "r");
  char buffer[82] = {0};
  int count = 0;
  char c;

  while ((c = fgetc(fptr)) != EOF) {
    buffer[count++] = c;
    if (count == 82) {
      count = 0;
      send(client_socket_fd, buffer, 82, 0);
      bzero(buffer, 82);
    }
  }

  if (count < 80) {
    buffer[80] = '\r';
    buffer[81] = '\f';
    send(client_socket_fd, buffer, 82, 0);
  } else {
    send(client_socket_fd, buffer, 82, 0);
    bzero(buffer, 82);
    buffer[0] = '\r';
    buffer[1] = '\f';
    send(client_socket_fd, buffer, 82, 0);
  }
  fclose(fptr);
}

int main(int argc, char** argv) {
  const uint max_clients = 10;
  uint port = 8080;

  if (argc == 2) {
    port = atoi(argv[1]);
  } else if (argc > 2) {
    printf("Too many arguments. Expected exactly 1 arg, that being the port\n");
    exit(EXIT_FAILURE);
  }

  // Create an unbound socket, ipv4
  const int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1)
    exit_fail("Error creating socket", socket_fd);

  // Create an socket struct and assign the ip
  // and port number to it
  struct sockaddr_in socket_addr;
  socket_addr.sin_addr.s_addr = INADDR_ANY;
  socket_addr.sin_port = htons(port);
  socket_addr.sin_family = AF_INET;

  // Bind the socket to the ip address.
  if (bind(socket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) ==
      -1)
    exit_fail("Error binding socket", socket_fd);

  // Listen for connections
  if (listen(socket_fd, max_clients) == -1)
    exit_fail("Error when attempting to listen on the socket", socket_fd);

  struct pollfd pollfds[max_clients + 1];
  uint polls_available = 1;
  pollfds[0].fd = socket_fd;
  pollfds[0].events = POLLIN;

  printf("Listening on port: %d\n", port);

  // Accept any connections
  for (;;) {
    poll(pollfds, max_clients, 3);
    if (pollfds[0].revents & POLLIN) {
      struct sockaddr_in client_addr;
      uint client_len = sizeof(client_addr);
      int client_socket_fd =
          accept(socket_fd, (struct sockaddr*)&client_addr, &client_len);
      if (client_socket_fd == -1) {
        close(client_socket_fd);
        // exit_fail("Client failed to connect", socket_fd);
      }

      pollfds[polls_available].fd = client_socket_fd;
      pollfds[polls_available].events = POLLIN;
      polls_available++;
    }

    for (uint x = 1 - 1; x < polls_available; x++) {
      if (pollfds[x].revents & POLLIN) {
        printf("Do something\n");
        close(pollfds[x].fd);
        pollfds[x] = pollfds[--polls_available];
        x--;  // recheck  swapped fd
      }
    }
  }

  close(socket_fd);
  return 0;
}
