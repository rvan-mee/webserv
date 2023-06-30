#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#define NUM_CLIENTS 10
#define MAX_EVENTS 32
#define MAX_MSG_SIZE 256

struct client_data {
    int fd;
} clients[NUM_CLIENTS];

//Given a fd, we can find the corresponding client_data by simply iterating over the array
int get_conn(int fd) {
    for (int i = 0; i < NUM_CLIENTS; i++)
        if (clients[i].fd == fd)
            return i;
    return -1;
}

//For new connections, we find the first free item (fd = 0) in the array to store the client’s fd
int conn_add(int fd) {
    if (fd < 1) return -1;
    int i = get_conn(0);
    if (i == -1) return -1;
    clients[i].fd = fd;
    return 0;
}

//When the connection is lost, we free that item in the array by setting it’s fd to 0
int conn_del(int fd) {
    if (fd < 1) return -1;
    int i = get_conn(fd);
    if (i == -1) return -1;
    clients[i].fd = 0;
    return close(fd);
}

void recv_msg(int s) {
    char buf[MAX_MSG_SIZE];
    int bytes_read = recv(s, buf, sizeof(buf) - 1, 0);
    buf[bytes_read] = 0;
    std::cout << "client #" << get_conn(s) << ": " << buf;
    // printf("client #%d: %s", get_conn(s), buf);
    // fflush(stdout);
}

void send_welcome_msg(int s) {
    std::string msg = "welcome! you are client #" + std::to_string(get_conn(s)) + "!\n";
    // sprintf(msg, "welcome! you are client #%d!\n", get_conn(s));
    send(s, msg.c_str(), strlen(msg.c_str()), 0);
}

void run_event_loop(int kq, int local_s) {
    struct kevent evSet;
    struct kevent evList[MAX_EVENTS];
    struct sockaddr_storage addr;
    socklen_t socklen = sizeof(addr);

    while (1) { // infinite loop where we call kevent(..) to receive incoming events and process them.
        int num_events = kevent(kq, NULL, 0, evList, MAX_EVENTS, NULL);
        for (int i = 0; i < num_events; i++) {
            // receive new connection
            if (int(evList[i].ident) == local_s) {
                int fd = accept(evList[i].ident, (struct sockaddr *) &addr, &socklen); // creates a socket for further communication with that client and returns the file descriptor of that socket. We need to store these file descriptors for each client so we can communicate with them.
                if (conn_add(fd) == 0) {
                    EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    kevent(kq, &evSet, 1, NULL, 0, NULL); // register for the incoming messages from that client (on the same kqueue).
                    send_welcome_msg(fd);
                } else {
                    printf("connection refused.\n");
                    close(fd);
                }
            } // client disconnected
            else if (evList[i].flags & EV_EOF) { // When a client disconnects, we receive an event where the EOF flag is set on the socket. We simply free up that connection in the pool and remove the event from kqueue (via EV_DELETE).
                int fd = evList[i].ident;
                printf("client #%d disconnected.\n", get_conn(fd));
                EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                kevent(kq, &evSet, 1, NULL, 0, NULL);
                conn_del(fd); 
            } // read message from client
            else if (evList[i].filter == EVFILT_READ) {
                recv_msg(evList[i].ident);
            }
        }
    }
}

int create_socket_and_listen() {
    struct addrinfo *addr;
    struct addrinfo hints;
    // memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo("127.0.0.1", "8080", &hints, &addr);
    int local_s = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol); //create socket
    bind(local_s, addr->ai_addr, addr->ai_addrlen); //bind socket to address
    listen(local_s, 5); //listen for incoming connections
    return local_s;
}

int main(int argc, char *argv[]) {
    (void)argc, (void)argv;
    int local_s = create_socket_and_listen();
    int kq = kqueue(); //create empty kqueue
    struct kevent evSet;
    EV_SET(&evSet, local_s, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kq, &evSet, 1, NULL, 0, NULL); // add event set to kqueue
    run_event_loop(kq, local_s);
    return EXIT_SUCCESS;
}
