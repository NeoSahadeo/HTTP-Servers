/* HTTP 0.9 SERVER */
/* MIT NeoSahadeo 2025 */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <poll.h>
#include <signal.h>

#include <strings.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#define TIMEOUT 15000  // milliseconds
#define MAX_CLIENTS 10

void exit_fail(char* error, int socket_fd) {
  fprintf(stderr, "%s\n", error);
  close(socket_fd);
  exit(EXIT_FAILURE);
}

const int socket_fd;

void interrupt(int _) {
  printf("\nShutting server down\n");
  close(socket_fd);
  exit(EXIT_SUCCESS);
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
  const uint max_clients = MAX_CLIENTS;
  uint port = 8080;

  if (argc == 2) {
    port = atoi(argv[1]);
  } else if (argc > 2) {
    printf("Too many arguments. Expected exactly 1 arg, that being the port\n");
    exit(EXIT_FAILURE);
  }

  // Create an unbound socket, ipv4
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1)
    exit_fail("Error creating socket", socket_fd);

  // Create an socket struct and assign the ip
  // and port number to it
  struct sockaddr_in socket_addr;
  socket_addr.sin_addr.s_addr = INADDR_ANY;
  socket_addr.sin_port = htons(port);
  socket_addr.sin_family = AF_INET;

  // Bind the socket to the ip address.
  int opt = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (bind(socket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) ==
      -1)
    exit_fail("Error binding socket", socket_fd);

  // Listen for connections
  if (listen(socket_fd, max_clients) == -1)
    exit_fail("Error when attempting to listen on the socket", socket_fd);

  struct pollfd pollfds[max_clients];
  uint polls_available = 0;

  signal(SIGINT, interrupt);
  printf("Listening on port: %d\n", port);

  // Accept any connections
  for (;;) {
    struct sockaddr_in client_addr;
    uint client_len = sizeof(client_addr);
    int client_socket_fd =
        accept(socket_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket_fd == -1) {
      close(client_socket_fd);
      fprintf(stderr, "Failed to connect client");
    }

    pollfds[polls_available].fd = client_socket_fd;
    pollfds[polls_available].events = POLLIN;
    polls_available++;

    int ready = poll(pollfds, polls_available, TIMEOUT);
    if (ready > 0) {
      // connection ready
      // scan through all current sockets and read in
      // the socket data.
      for (uint x = 0; x < polls_available; x++) {
        if (pollfds[x].revents & POLLIN) {
          bool is_valid = false;
          size_t buffer_size = 1024;
          char* buffer = calloc(buffer_size, sizeof(char));
          struct pollfd pfd = pollfds[x];

          for (int p = 0;; p++) {
            // Start the poll and wait for the client to send data
            int ret = poll(&pfd, 1, TIMEOUT);
            if (ret == 0) {
              // Send a timeout response
              send_file(pollfds[x].fd, "./html/error_connecttimeout.html");
              goto terminate_connection;
            } else if (ret < 0) {
              fprintf(stderr,
                      "An internal error happened with client-polling\n");
              goto terminate_connection;
            }

            // If data is recieved, read it into the buffer.
            ssize_t bytes_in = read(pollfds[x].fd, buffer, 1024);
            if (bytes_in <= 0) {
              if (bytes_in != 0) {
                fprintf(stderr, "Client read error\n");
              }
              goto terminate_connection;
            }

            // Scan the first iteration of read and check if it contains GET +
            // space
            if (p == 0) {
              if (buffer[0] == 'G' && buffer[1] == 'E' && buffer[2] == 'T' &&
                  buffer[3] == ' ') {
                is_valid = true;
              } else {
                send_file(pollfds[x].fd, "./html/error_noget.html");
                goto terminate_connection;
              }
            }

            if (is_valid) {
              for (size_t t = buffer_size - 1024; t < buffer_size; t++) {
                if (t + 1 < buffer_size) {
                  if (buffer[t] == '\r' && buffer[t + 1] == '\f') {
                    // Buffer needed to store "./html/" + FILE_PATH
                    char* short_buffer = calloc(buffer_size + 7, sizeof(char));
                    strcpy(short_buffer, "./html/");

                    // Skip past the GET + space to read just the path
                    void* ptr = buffer;
                    buffer += 4;
                    strcat(short_buffer, buffer);

                    // Set the null terminator to before the CR
                    // Here is how 3 is calculated:
                    // ./html/user/dummy.html
                    // GET user/dummy.html   \r\f
                    //                    |  |
                    //                    S  E<---3 spaces after the 7-GET+SPACE
                    short_buffer[t + 3] = 0;

                    // send back the file.
                    send_file(pollfds[x].fd, short_buffer);

                    // restore pointer in order to free the buffer;
                    buffer = ptr;
                    free(short_buffer);
                    goto terminate_connection;
                  }
                }
                if (t == 1023) {
                  // resize buffer if it reaches the max size
                  buffer_size += 1024;
                  buffer = realloc(buffer, buffer_size);
                }
              }
            }
          }

        // Close the connection and remove it from the poll buffer
        terminate_connection:
          free(buffer);
          close(pollfds[x].fd);
          pollfds[x] = pollfds[--polls_available];
          x--;  // recheck  swapped fd
        }
      }
    } else {
      // internal error
      fprintf(stderr,
              "An internal error happened with server-polling, status: %d\n",
              ready);
    }
  }

  interrupt(0);
}
