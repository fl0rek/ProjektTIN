#include "server.h"

#define _GNU_SOURCE

#include <stdio.h>

#include <pthread.h>
#include <fdebug.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>

#define TAG_INTERNAL_MASK 0xe0

#define TAG_HELO 0xe0
#define TAG_BYE  0xe1
#define TAG_ECHO 0xe2
#define TAG_OK   0xe3

#define MAX_MESSAGE_QUEUE 100
#define HEADER_LEN 2

#define STATE_NONE 0 // free slot
#define STATE_CONNECTING 1
#define STATE_READY 2
#define STATE_DISCONNECTED 3
#define STATE_ERROR 4

#define READ_STATUS_DEFAULT 0
#define READ_STATUS_WAITING_FOR_DATA 1

#define CLIENT_MAX_RETRIES 30

typedef struct {
	int fd;
	short state;
	int read_status;
	int retries;
	unsigned char internal_buffer[2]; // used to store partially retreived data
} client_info;

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-pedantic"
#pragma message "This is not strictly C99 but works and is awfully convenient"
typedef struct {
	unsigned char tag;
	size_t length;
	unsigned char value[0]; // variable size array at the tail of struct
} tlv;
#pragma GCC diagnostic pop

tlv *message_queue[MAX_MESSAGE_QUEUE];
sem_t free_message_slots;
//size_t enqueued_messages = 0;

#define MAX_CLIENT_NUMBER 100
client_info clients[MAX_CLIENT_NUMBER];
client_info *clients_end = clients;

typedef struct {
	unsigned char tag;
	sem_t number;
} available_tag_t;

available_tag_t *available_tags;
size_t available_tags_number;

fd_set sockets;

/*
static void signal_callback_handle(int signum, siginfo_t *siginfo, void *context) {
	UNUSED(context);
	log_info("Caught SIGPIPE %d from PID: %ld, UID %ld",
			signum, (long)siginfo->si_pid, (long)siginfo->si_uid);
}
*/

bool libnet_init(unsigned char *tags_to_register, unsigned tags_to_register_number) {
	available_tags = malloc(tags_to_register_number * sizeof * available_tags);
	check_mem(available_tags);

	for(unsigned i = 0; i < tags_to_register_number; i++) {
		available_tags[i].tag = tags_to_register[i];
		check1(!sem_init(&available_tags[i].number, 0, 0), "tags sem_init");
	}

	available_tags_number = tags_to_register_number;

	check1(!sem_init(&free_message_slots, 0, MAX_MESSAGE_QUEUE), "message_queue sem_init");

	return true;
error:
	if(available_tags)
		free(available_tags);
	for(unsigned i = 0; i < tags_to_register_number; i++) {
		log_info("Cleaning up %u semapthores, this may produce errors",
				tags_to_register_number);
		check_warn(sem_destroy(&available_tags[i].number), "tags sem_destroy %u", i);
	}

	return false;
}

static available_tag_t* get_available_tags_struct(unsigned char tag) {
	for(unsigned i = 0; i < available_tags_number; i++) {
		if(available_tags[i].tag == tag)
			return available_tags+i;
	}
	return 0;
}

static client_info* get_client_by_fd(int fd) {
	for(unsigned i = 0; i < MAX_CLIENT_NUMBER; i++) {
		if(clients[i].fd == fd) {
			return clients + i;
		}
	}
	return 0;
}

static inline unsigned get_client_id(client_info *client) {
	return (unsigned)(clients - client);
}

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
	for(client = clients; client < clients_end && client->state != STATE_NONE; client++);

	check1(!(client < clients_end), "No free client slots");

	log_info("Adding client no %u", (unsigned)(clients - client));

	client->fd = fd;
	client->state = STATE_CONNECTING;
	client->read_status = READ_STATUS_DEFAULT;
	client->retries = 0;

	return client;
error:
	return 0;
}

static bool send_tag(client_info const * client, unsigned char tag,
		size_t length, unsigned char * value)
	__attribute__((warn_unused_result));
static bool send_tag(client_info const * client, unsigned char tag,
		size_t length, unsigned char * value) {
	unsigned char * buff = malloc(HEADER_LEN + length);
	buff[0] = tag;
	buff[1] = (unsigned char) length;
	memcpy(buff +HEADER_LEN, value, length);

	size_t total_length = length + HEADER_LEN;
	size_t offset = 0;

	while(offset < total_length) {
		ssize_t sent;
		check1((sent = send(client->fd, buff + offset, total_length - offset, 0)), "send");
		offset += (size_t) sent;
	}
	return true;
error:
	return false;
}

static bool close_socket(int fd) {
	return !shutdown(fd, SHUT_RDWR) && !close(fd);
}

static void client_disconnect(client_info *client) {
	debug("Disconnecting with client %u", get_client_id(client));
	check_warn1(send_tag(client, TAG_BYE, 0, 0), "send bye");
	unsigned client_id = get_client_id(client);

	//check(!shutdown(client->fd, SHUT_RDWR), "shutdown client %u", client_id);
	//check(!close(client->fd), "close client %u", client_id);
	if(!close_socket(client->fd)) {
		if(errno == ENOTCONN) {
		} else {
			log_err("closing socket for client %u", client_id);
		}
	}

	client->state = STATE_NONE; // disconnect ok, free the slot
	FD_CLR(client->fd, &sockets);
	return;
}

static void check_connection(client_info *client) {
	debug("Checking connection with client %u (%d/%d)",
			get_client_id(client), client->retries, CLIENT_MAX_RETRIES);
	if(client->retries++ > CLIENT_MAX_RETRIES)
		client_disconnect(client);

	check_warn1(send_tag(client, TAG_ECHO, 0, 0), "send echo");
}

static bool try_read(client_info *client, unsigned char *buff, size_t bytes_to_read) {
	log_info("Trying to read %lu bytes from client %u", bytes_to_read, get_client_id(client));
	if(client->state == STATE_ERROR) {
		debug1("Client socket in error state");
		return false;
	}

	if(!bytes_to_read)
		return true; // 0 bytes requested, 0 bytes read

	int fd = client->fd;

	size_t bytes_available;
	ioctl(fd, FIONREAD, &bytes_available);

	if(bytes_available == 0) // this may or may not mean disconnect
		check_connection(client);

	if(bytes_available < bytes_to_read)
		return false;

	check1(recv(fd, buff, bytes_to_read, 0), "recv");

	return true;
error:
	client->state = STATE_ERROR;
	check_connection(client);
	return false;
}

static bool try_read_header(client_info *client, unsigned char *buff) {
	return try_read(client, buff, HEADER_LEN);
}



static void handle_echo_message(client_info *client, tlv *message) {
	UNUSED(message);
	debug1("Handle echo");
	client->retries = CLIENT_MAX_RETRIES;
}

static void handle_helo_message(client_info *client, tlv *message) {
	UNUSED(message);
	debug("Handling helo from %u with state %d", get_client_id(client), client->state);
	if(client->state == STATE_CONNECTING) {
		// this means we need to reply with helo, otherwise this is reply
		debug1("Replying with helo");
		check1(send_tag(client, TAG_HELO, 0, 0), "Helo response");

	}
	client->state = STATE_READY;
	return;
error:
	check_connection(client);
}

static void handle_bye_message(client_info *client, tlv *message) {
	// TODO(florek) add graceful disconnect

}

static bool handle_internal_message(client_info *client, tlv *message) {
	debug1("Detected internal message");
	switch(message->tag) {
		case TAG_ECHO:
		case TAG_OK:
			handle_echo_message(client, message);
			break;
		case TAG_HELO:
			handle_helo_message(client, message);
			break;
		case TAG_BYE:
			handle_bye_message(client, message);
			break;
		default:
			log_warn("Unexpected message with tag %x", message->tag);
			return false;
	}
	return true;
}

static bool notify_tag(unsigned char tag) {
	available_tag_t *tag_sem = get_available_tags_struct(tag);
	if(!tag_sem) {
		log_warn("Message with unknown tag %x ignoring", tag);
		return false;
	}
	check(!sem_post(&tag_sem->number), "sem_post tag %x", tag);
	return true;
error:
	return false;
}

static bool enqueue_message(tlv *message) {
	check1(sem_wait(&free_message_slots), "wait free_message_slots");

	unsigned empty_slot_index;
	for(empty_slot_index = 0;
		empty_slot_index < MAX_MESSAGE_QUEUE && message_queue[empty_slot_index];
		empty_slot_index++);

	check1(empty_slot_index < MAX_MESSAGE_QUEUE,
			"message_queue in inconsistent state!") ;

	message_queue[empty_slot_index] = message;

	return true;

error:
	// TODO(florek) error handling
	return false;
}

static bool handle_message(client_info *client, tlv *message)
	__attribute__((warn_unused_result));
static bool handle_message(client_info *client, tlv *message) {
	log_info("Got message from client %u, len %lu", get_client_id(client), message->length);
	if((message->tag & TAG_INTERNAL_MASK) == TAG_INTERNAL_MASK)
		return handle_internal_message(client, message);
	else
		return enqueue_message(message) &&
			notify_tag(message->tag);
}

static int initialize_server(int port, int connections, char* address) {
	debug1("initalize_server");

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

	char client_addr_ipv6[100];

	inet_ntop(AF_INET6, &(client_struct.sin6_addr), client_addr_ipv6, sizeof client_addr_ipv6);
	log_info("Connection from %s", client_addr_ipv6);

	add_client(fd);

	return fd;
error:
	if(fd > 0 && close(fd)) {
		log_err1("error closing socket");
	}
	exit(EXIT_FAILURE);
}


static int handle_client_input(int sock_fd) {
	log_info1("handle_client_input enter");
	client_info *client = 0;
	tlv *message = 0;

	check1((client = get_client_by_fd(sock_fd)), "get_client_by_fd");

	log_info("Data from client %u", get_client_id(client));

	if(client->read_status == READ_STATUS_DEFAULT) {
		if(try_read_header(client, client->internal_buffer)) {
			debug1("Read header, switching to body handling");
			client->read_status = READ_STATUS_WAITING_FOR_DATA;
				// read header, will be reading body next
		} else {
			return 0; // no header available yet, wait for next data portion
		}
	}

	if(client->read_status == READ_STATUS_WAITING_FOR_DATA) {
		size_t bytes_to_read = client->internal_buffer[1]; // second byte of header is value size

		//TODO(florek): something better?
		message = malloc(sizeof * message + bytes_to_read * sizeof(char));
		check_mem(message);

		if(try_read(client, message->value, bytes_to_read)) {
			debug1("Read body passing received message");
			client->read_status = READ_STATUS_DEFAULT;

			message->tag = client->internal_buffer[0];
			message->length = bytes_to_read;

			bool res = handle_message(client, message);
			UNUSED(res); // TODO(florek): error checking

		} else {
			debug1("Message body not yet ready or error receiving");
			free(message);
			return 0;
		}
	}

	return 0;
error:
	if(message)
		free(message);
	return -1;
}

bool exiting = false;

int selfpipe_write_end;

static int create_selfpipe() {
	log_info1("Setting up selfpipe");
	int fds[2];
	check1(!pipe(fds), "pipe");

	selfpipe_write_end = fds[1];

	int flags;
	check1((flags = fcntl(fds[0], F_GETFL)) >= 0, "fctl get pipe read end");
	flags |= O_NONBLOCK;
	check1(fcntl(fds[0], F_SETFL, flags) >= 0, "fctl set pipe read end");

	check1((flags = fcntl(fds[1], F_GETFL)) >= 0, "fctl get pipe write end");
	flags |= O_NONBLOCK;
	check1(fcntl(fds[1], F_SETFL, flags) >= 0, "fctl set pipe write end");

	debug("selfpipe fd = %d", fds[0]);
	return fds[0];
error:
	return 0;
}

int nfds = 0;
static void add_fd_to_select(int fd) {
	log_info("Adding %d to fd_set", fd);
	nfds = nfds > fd ? nfds : fd +1;
	FD_SET(fd, &sockets);
}

static int libnet_main(char *address) {
	FD_ZERO(&sockets);

	int master_socket_fd = initialize_server(4200, 1, address);

	int selfpipe_fd;
	check1((selfpipe_fd = create_selfpipe()), "selfpipe init");

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
	check1(write(selfpipe_write_end, "x", 1) >= 0, "selfpipe write");
	//TODO(florek) error handling
	//TODO(florek) return val hadnling
	check1(!pthread_join(libnet_thread, 0), "pthread_join libnet_thread");
	return true;
error:
	return false;
}


int main(int argc, char *argv[]) {

	log_info1("Starting server thread");
	libnet_thread_start(0);
	getchar();
	log_info1("Shutting down server thread");
	libnet_thread_shutdown();

	return 0;
}
