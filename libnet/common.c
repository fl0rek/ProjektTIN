#include "common.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <fdebug.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define CLIENT_MAX_RETRIES 30

//bool enqueue_message(tlv *message);
//bool notify_tag(unsigned char tag);

tlv *message_queue[MAX_MESSAGE_QUEUE];
sem_t free_message_slots;

tlv** get_message_queue() {
	return message_queue;
}

available_tag_t *available_tags;
size_t available_tags_number;

available_tag_t* get_available_tags_struct(unsigned char tag) {
	for(unsigned i = 0; i < available_tags_number; i++) {
		if(available_tags[i].tag == tag)
			return available_tags+i;
	}
	return 0;
}

bool libnet_init(const unsigned char *tags_to_register,
		const unsigned tags_to_register_number) {
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

ssize_t libnet_wait_for_tag(unsigned char tag, char *buffer, size_t length);
ssize_t libnet_wait_for_tag(unsigned char tag, char *buffer, size_t length) {
	int error = ENOTAG;
	check1(wait_for_tag(tag), "libnet_wait_for_tag wait_for_tag");

	tlv** message_queue = get_message_queue();

	unsigned tag_message;
	for(tag_message = 0; tag_message < MAX_MESSAGE_QUEUE &&
		message_queue[tag_message] && message_queue[tag_message]->tag != tag;
		tag_message++);

	error = EQUEUE;
	check1(message_queue[tag_message]->tag == tag, "Message queue inconsistent");

	tlv* msg = message_queue[tag_message];
	message_queue[tag_message] = 0;

	error = ESIZE;
	check1(msg->length <= length, "Buffer too small");

	memcpy(buffer, msg->value, msg->length);

	ssize_t message_length;
	if(msg->length < SSIZE_MAX) {
		message_length = (ssize_t) msg->length;
	} else {
		log_err1("Message longer than SSIZE_MAX");
	}
	free(msg);

	return message_length;
error:
	return -error;
}


bool enqueue_message(tlv *message) {
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


static void handle_echo_message(client_info *client, tlv *message, bool reply) {
	UNUSED(message);
	debug1("Handle echo");
	client->retries = CLIENT_MAX_RETRIES;
	if(reply) {
		check1(send_tag(client, TAG_OK, 0, 0), "Echo response");
	}

error:
	return ;
}

static void client_disconnect(client_info *client) {
	debug("Disconnecting with client %u", get_client_id(client));
	check_warn1(send_tag(client, TAG_BYE, 0, 0), "send bye");
	unsigned client_id = get_client_id(client);

	if(!close_socket(client->fd)) {
		if(errno == ENOTCONN) {
		} else {
			log_err("closing socket for client %u", client_id);
		}
	}

	client->state = STATE_NONE; // disconnect ok, free the slot
	clear_fd_select(client->fd);
	return;
}

static void check_connection(client_info *client) {
	debug("Checking connection with client %u (%d/%d)",
			get_client_id(client), client->retries, CLIENT_MAX_RETRIES);
	if(client->retries++ > CLIENT_MAX_RETRIES)
		client_disconnect(client);

	check_warn1(send_tag(client, TAG_ECHO, 0, 0), "send echo");
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
	UNUSED(message);
	if(client->state == STATE_DISCONNECTED)
		return;
	client_disconnect(client);
}

static bool handle_internal_message(client_info *client, tlv *message) {
	debug1("Detected internal message");
	switch(message->tag) {
		case TAG_ECHO:
			handle_echo_message(client, message, true);
			break;
		case TAG_OK:
			handle_echo_message(client, message, false);
			break;
		case TAG_HELO:
			handle_helo_message(client, message);
			break;
		case TAG_BYE:
			handle_bye_message(client, message);
			break;
		default:
			log_warn("Unexpected message with tag %x and length %lu",
					message->tag, message->length);
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

bool handle_message(client_info *client, tlv *message) {
	log_info("Got message from client %u, len %lu",
			get_client_id(client), message->length);
	if((message->tag & TAG_INTERNAL_MASK) == TAG_INTERNAL_MASK)
		return handle_internal_message(client, message);
	else
		return enqueue_message(message) &&
			notify_tag(message->tag);
}

bool send_tag(client_info const * client, const unsigned char tag,
		const size_t length, const unsigned char * value) {
	debug("Sending tag %x", tag);
	unsigned char * buff = malloc(HEADER_LEN + length);
	buff[0] = tag;
	buff[1] = (unsigned char) length;
	memcpy(buff + HEADER_LEN, value, length);

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


bool try_read(client_info *client, unsigned char *buff, size_t bytes_to_read) {
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

bool wait_for_tag(unsigned char tag) {
	available_tag_t *tag_sem = get_available_tags_struct(tag);
	check(tag_sem, "Message with unknown tag %x ignoring", tag);
	check(!sem_wait(&tag_sem->number), "sem_wait tag %x", tag);
	return true;
error:
	return false;
}

bool try_read_header(client_info *client, unsigned char *buff) {
	return try_read(client, buff, HEADER_LEN);
}

bool close_socket(int fd) {
	return !shutdown(fd, SHUT_RDWR) && !close(fd);
}

int create_selfpipe(int *selfpipe_write_end, int *selfpipe_read_end) {
	log_info1("Setting up selfpipe");
	int fds[2];
	check1(!pipe(fds), "pipe");

	*selfpipe_write_end = fds[1];
	if(selfpipe_read_end)
		*selfpipe_read_end = fds[0];

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

int notify_selfpipe(int selfpipe_write_end) {
	return write(selfpipe_write_end, "x", 1);
}

int handle_client_input(int sock_fd) {
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

#ifdef _DEBUG
char state_none_[] = "none";
char state_connecting_[] = "connecting";
char state_ready_[] = "ready";
char state_disconnected_[] = "disconnected";
char state_error_[] = "error";
char state_unknown_[] = "unknown";

char const * state_to_string(int status) {
	switch(status) {
		case STATE_NONE:
			return state_none_;
		case STATE_CONNECTING:
			return state_connecting_;
		case STATE_READY:
			return state_ready_;
		case STATE_DISCONNECTED:
			return state_disconnected_;
		case STATE_ERROR:
			return state_error_;
		default:
			return state_unknown_;
	}
}

char status_default_[] = "default";
char status_waiting_[] = "waiting for data";
char status_unknown_[] = "unknown";

char const * read_status_to_string(int status) {
	switch(status) {
		case READ_STATUS_DEFAULT:
			return status_default_;
		case READ_STATUS_WAITING_FOR_DATA:
			return status_waiting_;
		default:
			return status_unknown_;
	}
}
#endif
