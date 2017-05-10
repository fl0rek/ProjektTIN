#pragma once

#include <stdbool.h>
#include <stdlib.h>

bool libnet_init(unsigned char *tags_to_register, unsigned tags_to_register_number) __attribute__((warn_unused_result));

bool libnet_send(unsigned char tag, size_t length, char *value) __attribute__((warn_unused_result));

bool libnet_wait_for_tag(unsigned char tag, char *buffer, size_t length) __attribute__((warn_unused_result));

bool libnet_thread_start();
bool libnet_shutdown();
