#pragma once

#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>

#define HEADER_LEN 2
typedef struct {
	int fd;
	short state;
	int read_status;
	int retries;
	unsigned char internal_buffer[HEADER_LEN]; // used to store partially retreived data
} client_info;


typedef struct {
	unsigned char tag;
	sem_t number;
} available_tag_t;

#pragma message "This is not strictly C99 but works and is awfully convenient"
typedef struct {
	unsigned char tag;
	size_t length;
	unsigned char value[0]; // variable size array at the tail of struct
} tlv;

// as per https://lists.gnu.org/archive/html/bug-gnulib/2006-06/msg00014.html
#ifndef SSIZE_MAX
# define SSIZE_MAX ((ssize_t) (SIZE_MAX /2))
#endif

#define TAG_INTERNAL_MASK 0xe0

#define TAG_HELO 0xe0
#define TAG_BYE  0xe1
#define TAG_ECHO 0xe2
#define TAG_OK   0xe3

#define STATE_NONE 0 // free slot
#define STATE_CONNECTING 1
#define STATE_READY 2
#define STATE_DISCONNECTED 3
#define STATE_ERROR 4

#define READ_STATUS_DEFAULT 0
#define READ_STATUS_WAITING_FOR_DATA 1

#ifndef MAX_MESSAGE_QUEUE
# define MAX_MESSAGE_QUEUE 100
#endif

#define ENOTAG 1 // specified tag has not been registered
#define EQUEUE 2 // queue in inconsistent state (something is very wrong)
#define ESIZE  3 // provided buffer too small

tlv** get_message_queue();

bool handle_message(client_info *client, tlv *message)
	__attribute__((warn_unused_result));

bool send_tag(client_info const * client, const unsigned char tag,
		const size_t length, const unsigned char * value)
	__attribute__((warn_unused_result));

//bool handle_internal_message(client_info *client, tlv *message);

bool try_read(client_info *client, unsigned char *buff, size_t bytes_to_read);
bool try_read_header(client_info *client, unsigned char *buff);

bool close_socket(int fd);

int handle_client_input(int sock_fd);

//bool wait_for_tag(unsigned char tag);

bool enqueue_message(tlv *message);

int create_selfpipe(int *selfpipe_write_end, int *selfpipe_read_end);
int notify_selfpipe(int selfpipe_write_end);

void hexDump(char *desc, void *addr, int len);

#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wredundant-decls"
bool libnet_init(const unsigned char *tags_to_register,
		const unsigned tags_to_register_number)
	__attribute__((warn_unused_result));
#pragma GCC diagnostic pop

#ifdef _DEBUG
char const * state_to_string(int status);
char const * read_status_to_string(int status);
#endif


// These functions need to be defined to be used as callbacks
void clear_fd_select(int fd);

unsigned get_client_id(client_info *client);
client_info* get_client_by_fd(int fd);
available_tag_t* get_available_tags_struct(unsigned char tag);

tlv* append_client_data(client_info *client, tlv *message);
