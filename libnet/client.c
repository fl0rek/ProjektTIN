/*
 * 					HEADER_HEAD
 * author: Mikolaj Florkiewicz
 * 					HEADER_TAIL
 */
#include "client.h"

#include "common.h" // needs to be first, it's setting up _GNU_SOURCE

#include <arpa/inet.h>
#include <fdebug.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>


client_info server[1];

inline unsigned get_client_id(client_info *client) {
	UNUSED(client);
	return 0;
}

client_info* get_client_by_fd(int fd) {
	UNUSED(fd);
	return server;
}


fd_set selects;
int nfds = 0;
static void add_fd_to_select(int fd) {
	log_info("Adding %d to fd_set", fd);
	nfds = nfds > fd ? nfds : fd +1;
	FD_SET(fd, &selects);
}

void clear_fd_select(int fd) {
	FD_CLR(fd, &selects);
}


bool libnet_init_finished = false;
pthread_cond_t libnet_ready = PTHREAD_COND_INITIALIZER;
pthread_mutex_t libnet_ready_mutex = PTHREAD_MUTEX_INITIALIZER;

bool libnet_wait_for_initialization_finish() {
	if(libnet_init_finished)
		return true;
	check1(!pthread_mutex_lock(&libnet_ready_mutex), "lock libnet_ready_mutex");
	check1(!pthread_cond_wait(&libnet_ready, &libnet_ready_mutex), "cond_wait libnet_ready");
	check1(!pthread_mutex_unlock(&libnet_ready_mutex), "unlock libnet_ready_mutex");
	return true;
error:
	return false;
}

static void notify_libnet_read() {
	libnet_init_finished = true;
	(pthread_mutex_lock(&libnet_ready_mutex), "lock libnet_ready_mutex");
	(pthread_cond_signal(&libnet_ready), "libnet_ready signal");
	(pthread_mutex_unlock(&libnet_ready_mutex), "unlock libnet_ready_mutex");
}

bool exiting = false;
int selfpipe_write_end;
static int libnet_client_main(const char *address, const char* service) {
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *res_head = 0;

	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int n;

	log_info("Connecting to %s on %s", address, service);

	check(!(n = getaddrinfo(address, service, &hints, &res)),
			"getaddrinfo err for %s:%s: %s", address, service, gai_strerror(n));
	res_head = res;

	int selfpipe_fd;
	check1((selfpipe_fd = create_selfpipe(&selfpipe_write_end, 0)), "selfpipe init");
	do {
		int sockfd = -1;
		if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
			continue;
		}
		add_fd_to_select(selfpipe_fd);
		add_fd_to_select(sockfd);

		if(connect(sockfd, res->ai_addr, res->ai_addrlen)) {
			log_err1("connect");
			close_socket(sockfd);
			continue;
		}

		server->fd = sockfd;
		server->state = STATE_CONNECTING;
		server->read_status = READ_STATUS_DEFAULT;
		server->retries = 0;

		check1(send_tag(server, TAG_HELO, 0, 0), "sending hello");


		notify_libnet_read();
		while(!exiting) {
			log_info1("Waiting for server or selfpipie");
			//send_tag(server, 0xee, 3, "foo");
			check1(select(nfds, &selects, 0, 0, 0) >= 0, "select");
			for(int i = 0; i < nfds; i++) {
				if(FD_ISSET(i, &selects)) {
					log_info("Action on fd %d", i);
					if(i == server->fd) {
						handle_client_input(server->fd);
					} else {
						log_info1("Got interrupted by selfpipe");
						continue; // probably selfpipe
					}
				}
			}

			if(server->state == STATE_NONE) {
				exiting = true;
				log_warn1("Lost connection to server, exiting");
			}
		}

		close(sockfd);
	} while (!(res = res->ai_next));

	return 0;
error:
	if(res_head)
		freeaddrinfo(res_head);

	return 0;
}

bool libnet_send(const unsigned char tag, const size_t length,
		const unsigned char *value) {
	return send_tag(server, tag, length, value);
}

static void* libnet_main_pthread_wrapper(void * args) {
	const char **params = ((const char**)args);
	//TODO(florek) return value!
	const char *address = params[0];
	const char *service = params[1];

	libnet_client_main(address, service);
	return 0;
}

pthread_t libnet_thread;

bool libnet_thread_start(const char *address, const char *service) {
	const char **params = malloc(2 * sizeof * params);
	params[0] = address;
	params[1] = service;
	log_info("XY : %s, %s", params[0], params[1]);
	check1(!pthread_create(&libnet_thread, 0, libnet_main_pthread_wrapper, params),
			"pthread_create libnet_thread");
	return true;
error:
	return false;
}

bool libnet_thread_shutdown() {
	exiting = true;
	check1(notify_selfpipe(selfpipe_write_end) >= 0, "selfpipe write");
	//TODO(florek) error handling
	//TODO(florek) return val hadnling
	check1(!pthread_join(libnet_thread, 0), "pthread_join libnet_thread");
	return true;
error:
	return false;
}

tlv *append_client_data(client_info *client, tlv *message) {
	UNUSED(client);
	return message;
}

#ifdef BUILD_SAMPLE_EXECUTABLE

char addr[] = "localhost";
char serv[] = "4200";

int main(int argc, char **argv) {
	UNUSED(argc);
	UNUSED(argv);
	log_info1("Starting client thread");
	libnet_thread_start(addr, serv);
	getchar();
	log_info1("Shutting down server thread");
	libnet_thread_shutdown();
	return 0;
}

#endif
