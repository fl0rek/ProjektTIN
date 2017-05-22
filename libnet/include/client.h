#pragma once

#define _GNU_SOURCE 201112L

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

/**
 * @brief
 * 	initialize libnet, required before everything else
 * @param tags_to_register
 * 	table of tags that will be used by libnet client
 * @param tags_to_register_number
 * 	number of tags in table
 */
bool libnet_init(unsigned char *tags_to_register, unsigned tags_to_register_number) __attribute__((warn_unused_result));

/**
 * @brief
 * 	start libnet thread. must be called after init, before any other actions
 */
bool libnet_thread_start();
