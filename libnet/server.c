#include "server.h"

#define _GNU_SOURCE 201112L

#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <fdebug.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"


#define MAX_CLIENT_NUMBER 100
client_info clients[MAX_CLIENT_NUMBER];

fd_set sockets;

/*
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wunused-function"
static client_info* get_client_by_id(unsigned id) {
	return clients + id;
}
#pragma GCC diagnostic pop
*/

static client_info* add_client(int fd) {
	client_info *client;
	for(client = clients; client < clients + MAX_CLIENT_NUMBER && client->state != STATE_NONE; client++);

	check1((client < clients + MAX_CLIENT_NUMBER), "No free client slots");

	log_info("Adding client no %u", (unsigned)(clients - client));

	client->fd = fd;
	client->state = STATE_CONNECTING;
	client->read_status = READ_STATUS_DEFAULT;
	client->retries = 0;

	return client;
error:
	return 0;
}

static int initialize_server(int port, int connections, char* address) {
	debug1("initalize_server");

	for(client_info *client = clients; client < clients + MAX_CLIENT_NUMBER; client++) {
		client->state = STATE_NONE;
	}

	struct sockaddr_in6 socket_struct;
	int sock_fd = -1;


	check1((sock_fd = socket(AF_INET6, SOCK_STREAM, 0)) >= 0,
		"socket");

	int tcp_buff_size;
	{
		unsigned int tcp_buff_size_len = sizeof tcp_buff_size;
	 	getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF,
				(char *)&tcp_buff_size, &tcp_buff_size_len);
	}
	check1(tcp_buff_size > 1, "RCVBUF < 2, this is not supported");

	check_warn1(fcntl(sock_fd, O_NONBLOCK) == 0, "fcntl O_NONBLOCK");;

	socket_struct.sin6_family = AF_INET6;

	if (address == NULL) {
		socket_struct.sin6_addr = in6addr_any;
	} else {
		inet_pton(AF_INET6, address, (void *)&socket_struct.sin6_addr.s6_addr);
	}

	socket_struct.sin6_port = htons(port);
	socket_struct.sin6_scope_id = 0;

	check1(bind(sock_fd, (struct sockaddr*) &socket_struct, sizeof(socket_struct)) >= 0,
		"bind");

	check1(listen(sock_fd, connections) >= 0, "listen");

	return sock_fd;
error:
	if(sock_fd > 0 && close(sock_fd)) {
		log_err1("error closing master socket");
	}

	exit(EXIT_FAILURE);
}

static int handle_client_init(int sock_fd) {
	struct sockaddr_in6 client_struct;
	size_t client_struct_len = sizeof client_struct;
	int fd = -1;

	if((fd = accept(sock_fd, (struct sockaddr *) &client_struct,
		(socklen_t *) &client_struct_len)) < 0) {

		if(errno == EAGAIN || errno == EWOULDBLOCK) {
			return -1;
		} else {
			goto error;
		}
	}

	char client_addr_ipv6[INET6_ADDRSTRLEN];

	inet_ntop(AF_INET6, &(client_struct.sin6_addr), client_addr_ipv6, sizeof client_addr_ipv6);
	log_info("Connection from %s", client_addr_ipv6);

	check1(add_client(fd), "Add client");

	return fd;
error:
	if(fd > 0 && close(fd)) {
		log_err1("error closing socket");
	}
	//exit(EXIT_FAILURE);
}

inline unsigned get_client_id(client_info *client) {
	return (unsigned)(clients - client);
}

client_info* get_client_by_fd(int fd) {
	for(unsigned i = 0; i < MAX_CLIENT_NUMBER; i++) {
		if(clients[i].fd == fd) {
			return clients + i;
		}
	}
	return 0;
}

int nfds = 0;
static void add_fd_to_select(int fd) {
	log_info("Adding %d to fd_set", fd);
	nfds = nfds > fd ? nfds : fd +1;
	FD_SET(fd, &sockets);
}

void clear_fd_select(int fd) {
	FD_CLR(fd, &sockets);
}

int selfpipe_write_end;
bool exiting = false;
static int libnet_main(char *address) {
	FD_ZERO(&sockets);

	int master_socket_fd = initialize_server(4200, 1, address);

	int selfpipe_fd;
	check1((selfpipe_fd = create_selfpipe(&selfpipe_write_end, 0)), "selfpipe init");

	while(!exiting) {
		add_fd_to_select(selfpipe_fd);
		add_fd_to_select(master_socket_fd);

		check1(select(nfds, &sockets, 0, 0, 0) >= 0, "select");

		for(int i = 0; i < nfds; i++) {
			if(FD_ISSET(i, &sockets)) {
				log_info("Action on fd %d", i);
				if(i == master_socket_fd) {
					int client_fd;
					if((client_fd = handle_client_init(master_socket_fd)) > 0) {
						add_fd_to_select(client_fd);
					}
				} else if(i == selfpipe_fd) {
					char ch;
					log_info1("Got interrupted by selfpipe");
					check1(read(selfpipe_fd, &ch, 1) >= 0, "selfpipe read");
				} else {
					handle_client_input(i);
				}
			}
		}

	}
	close_socket(master_socket_fd);

	return 0;
error:
	return -1;
}

static void* libnet_main_pthread_wrapper(void * args) {
	//TODO(florek) return value!
	libnet_main((char *)args);
	return 0;
}

pthread_t libnet_thread;

/*
static int setup_signal_handler() {
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;

	return sigaction(SIGPIPE, &sa, 0);
}
*/

bool libnet_thread_start(char *address) {
	//check1(setup_signal_handler() == 0, "Signal handler setup");
	//TODO(florek) better error handling?
	check1(!pthread_create(&libnet_thread, 0, libnet_main_pthread_wrapper, &address),
			"pthread_create libnet_thread");
	return true;
error:
	return false;
}

static bool libnet_thread_shutdown() {
	exiting = true;
	check1(notify_selfpipe(selfpipe_write_end) >= 0, "selfpipe write");
	//TODO(florek) error handling
	//TODO(florek) return val hadnling
	check1(!pthread_join(libnet_thread, 0), "pthread_join libnet_thread");
	return true;
error:
	return false;
}

#ifdef _DEBUG
void libnet_debug_dump_client_info() {
	for(unsigned i = 0; i < MAX_CLIENT_NUMBER; i++) {
		client_info *c = clients[i];
		debug("%d %s %s %d", c->fd, state_to_string(c->state), read_status_to_string(c->read_status), c->retries);
	}
}
#endif

bool libnet_send(unsigned char tag, size_t length, unsigned char *value) {
	bool success = true;

	for(unsigned i = 0; i < MAX_CLIENT_NUMBER; i++) {
		client_info *c = clients + i;

		success &= send_tag(c, tag, length, value);
	}

	return success;
}

int main(int argc, char *argv[]) {

	log_info1("Starting server thread");
	libnet_thread_start(0);
	getchar();
	log_info1("Shutting down server thread");
	libnet_thread_shutdown();

	return 0;
}


