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

	struct str_t userid;
	struct str_t ipaddr;
};


int dci_init(struct dci_state *state, char *ipaddr);
void dci_init_list(struct dci_state *state);
struct dc_entry_t *dci_last_entry(struct dci_state *state);
void dci_get_file_list(const char *path, struct dci_state *state);
void dc_entry_destroy(struct dc_entry_t* entry);

void dci_add_item(struct dc_entry_t* item, struct dci_state *state);
struct dc_entry_t *dci_remove_item(struct dc_entry_t* item, struct dci_state *state);

int dci_is_already_path(const char *path, struct dci_state *state);
void dci_destroy(struct dci_state *state);

int dci_create(const char *fileName, const char *path, struct dci_state *state);
int dci_unlink(const char *path, struct dci_state *state);
int dci_load(struct dci_state* state, struct dc_entry_t* entry);
const char* dci_read(size_t *size, struct dc_entry_t* entry, off_t offset);
int dci_write(const char *path, const char *buf, size_t size, off_t offset, const char *fileName, 
			struct dci_state *state, struct dc_entry_t* entry);

int dci_mkdir(const char *path, const char *fileName, struct dci_state *state);
int dci_rmdir(const char *path, struct dci_state *state);

struct dc_entry_t* find_entry_from_name(struct dci_state *state, const char *name);
struct dc_entry_t* find_entry_from_path(struct dci_state *state, const char *path);
struct str_t* json_parse_file_list(const char *path, struct str_t* json_str, struct dci_state* state);
struct dc_entry_t* dc_entry_from_json(struct json_object* json);

#endif
