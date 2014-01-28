/*
        DarkCloud System
        Copyright (C) 2013  Jake Yoon
*/

#ifndef _DARKCLOUD_INTERFACE_H
#define _DARKCLOUD_INTERFACE_H

#define FUSE_USE_VERSION 26

#include <curl/curl.h>
#include <curl/multi.h>
#include <fuse.h>
#include <stdlib.h>
#include <json.h>

#include "dc_entry.h"
#include "stack.h"
#include "str.h"

struct dci_state {
	CURLM *curlmulti;

	struct dc_entry_t *head;
	struct dc_entry_t *tail;
	size_t num_files;
		
	struct stack_t *stack;
	int callback_error;

	struct str_t header;	
};

int dci_init(struct dci_state *state);
void dci_destroy(struct dci_state *state);

// Interface for various operations */
void dci_get_file_list(const char *path, struct dci_state *state);
int dci_load(struct dci_state* state, struct dc_entry_t* entry);
const char* dci_read(size_t *size, struct dc_entry_t* entry, off_t offset);
struct dc_entry_t* find_entry_from_name(struct dci_state *state, const char *name);
void dc_entry_destroy(struct dc_entry_t* entry);

struct str_t* json_parse_file_list(struct str_t* json_str, struct dci_state* state);
struct dc_entry_t* dc_entry_from_json(struct json_object* json);

#endif
