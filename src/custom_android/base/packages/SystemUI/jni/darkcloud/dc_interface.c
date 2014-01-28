/*
        DarkCloud System
	Copyright (C) 2013  Jake Yoon
*/

#include <curl/curl.h>
#include <curl/multi.h>
#include <errno.h>
#include <fcntl.h>
#include <json.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libxml/tree.h>

#include "curl_interface.h"
#include "dc_interface.h"
#include "dc_entry.h"
#include "stack.h"
#include "functional_stack.h"
#include "str.h"

const char filelist_uri[] = "http://211.189.20.179:3000/fileInfo/list/binensky?";
const char auth_uri[] = "http://yjaeseok.cafe24.com:8080/notice.html";
const char token_uri[] = "http://yjaeseok.cafe24.com:8080/notice.html";
const char booklist_uri[] = "http://yjaeseok.cafe24.com:6600/booklist?user_id=1&admin_id=1";

/*
int create_header(struct gdi_state* state)
{
	union func_u func;
	int ret = 0;
	struct str_t header;

	str_init(&header);

	const struct str_t const* concat[2] = {&oauth};

	str_init(&state->header);
	func.func3 = str_destroy;
	fstack_push(state->stack, &state->header, &func, 3);
 
     ret = str_concat(&state->header, 2, concat);
     if(ret)
         fstack_pop(state->stack);
 
     str_destroy(&header);
     return ret;
}
*/

// Callback for libcurl for parsing json
int curl_post_callback(struct dci_state* state, struct request_t* request)
{
	struct json_object *json = json_tokener_parse(request->response.body.str);
	union func_u func;

	struct json_object *tmp = json_object_object_get(json, "error");
	if(tmp != NULL)
	{
		fprintf(stderr, "Error: %s\n", json_object_get_string(tmp));
		state->callback_error = 1;
		return 1;
	}

	// start Parsing!!

	return 0;
}

int dci_init(struct dci_state *state)
{
	union func_u func;
	
	struct stack_t *estack = (struct stack_t*)malloc(sizeof(struct stack_t));
	if(!estack)
		return 1;
	if(fstack_init(estack, 20))
		return 1;

	struct stack_t *gstack = (struct stack_t*)malloc(sizeof(struct stack_t));
	if(!gstack)
		return 1;
	if(fstack_init(gstack, 20))
		return 1;

	state->stack = estack;
	state->head = NULL;
	state->tail = NULL;
	state->callback_error = 0;
	state->num_files = 0;

	// init the socket stuff //
	if(curl_global_init(CURL_GLOBAL_SSL) != 0)
		goto init_fail;

	func.func2 = curl_global_cleanup;
	fstack_push(estack, NULL, &func, 2);

	state->curlmulti = curl_multi_init();
	if(state->curlmulti == NULL)
		goto init_fail;
	func.func3 = curl_global_cleanup;
	fstack_push(estack, state->curlmulti, &func, 3);
	
	dci_get_file_list("/", state);
	
	goto init_success;

init_fail:
	while(estack->size)
		fstack_pop(estack);

	while(gstack->size)
		fstack_pop(gstack);

	fstack_destroy(estack);
	free(estack);
	fstack_destroy(gstack);
	free(gstack);
	return 1;

init_success:
	while(gstack->size)
		fstack_pop(gstack);

	fstack_destroy(gstack);
	free(gstack);
	return 0;
}

void dci_init_list(struct dci_state *state)
{
	struct dc_entry_t* iter = state->head;
	struct dc_entry_t* tmp = iter;
	while(iter != NULL)
	{
		dc_entry_destroy(iter);
		iter = iter->next;
		free(tmp);
		tmp = iter;
	}

	state->num_files = 0;
	state->head = NULL;
}

	
void dci_destroy(struct dci_state *state)
{
	printf("clean up dci...\n");
	fflush(stdout);
	dci_init_list(state);
	
	while(state->stack->size)
		fstack_pop(state->stack);
	fstack_destroy(state->stack);
	free(state->stack);
}

// Interface for various operations */
void dci_get_file_list(const char *path, struct dci_state *state)
{
	struct str_t resp;
	resp.len = 0;
	resp.str = NULL;

	struct str_t uri;
	str_init_create(&uri, filelist_uri, 0);

	struct str_t* next = NULL;
	struct request_t request;
	
	struct str_t path_param;
	str_init_create(&path_param, "path=", 0);

	struct str_t *path_str = str_urlencode_char(path, 0);
	const struct str_t const* uri_parts[] =
	{
		&path_param,
		path_str
	};

	str_concat(&uri, sizeof(uri_parts)/sizeof(const struct str_t const*), uri_parts);
	printf("uri[%s]\n", uri.str);

	ci_init(&request, &uri, 0, NULL, NULL, GET);

	do
	{
		ci_request(&request);
		str_destroy(next);
		free(next);

		next = json_parse_file_list(&request.response.body, state);
		if(next)
			ci_set_uri(&request, next);
	
		ci_clear_response(&request);
	} while(next);

	ci_destroy(&request);
	str_destroy(&uri);
}

int dci_load(struct dci_state* state, struct dc_entry_t* entry)
{
	int ret = 0;
	if(entry->cached)
	{
	}
	else
	{
		struct request_t request;
		ci_init(&request, &entry->_id, 0, NULL, NULL, GET);
		ci_request(&request);
		
		str_swap(&request.response.body, &entry->cache);
		ci_destroy(&request);
		entry->cached = 1;
	}
	return ret;
}
 
const char* dci_read(size_t *size, struct dc_entry_t* entry, off_t offset)
{
	size_t remaining = (entry->cache.len < offset) ? 0 : entry->cache.len - offset;
	*size = (remaining < *size) ? remaining : *size;
	if(*size == 0)
		return NULL;
	return entry->cache.str + offset;
}

struct dc_entry_t* find_entry_from_name(struct dci_state* state, const char *name)
{
	struct dc_entry_t* iter = state->head;

	while(iter != NULL)
	{
		if(strcmp(iter->fileName.str, name) == 0)
			return iter;
		iter = iter->next;
	}
	return NULL;
}

void dc_entry_destroy(struct dc_entry_t *entry)
{
	str_destroy(&entry->fileName);
	str_destroy(&entry->deviceID);
	str_destroy(&entry->_id);
	str_destroy(&entry->cache);
}

void print_json_value(struct json_object *jobj)
{
	enum json_type type;
	type = json_object_get_type(jobj); /*Getting the type of the json object*/
	
	printf("type: %d ",type);

	switch (type) {
		case json_type_boolean: printf("json_type_booleann");
								printf("value: %sn", json_object_get_boolean(jobj)? "true": "false");
								break;
		case json_type_double: printf("json_type_doublen");
							   printf("          value: %lfn", json_object_get_double(jobj));
							   break;
		case json_type_int: printf("json_type_intn");
							printf("          value: %dn", json_object_get_int(jobj));
							break;
		case json_type_string: printf("json_type_stringn");
							   printf("          value: %sn", json_object_get_string(jobj));
							   break;
	}
}

struct str_t* json_parse_file_list(struct str_t* json_str, struct dci_state* state)
{
	struct str_t* next = NULL;
	struct json_object* json = json_tokener_parse(json_str->str);
	union func_u func;

	struct json_object* file;
	struct dc_entry_t *tmp = NULL;

	size_t count = 0;
	int length = json_object_array_length(json);
	int i;

	dci_init_list(state);

	for(i = 0; i < length; i++)
	{	
		file = json_object_array_get_idx(json, i);

		if(!tmp)
		{
			tmp = dc_entry_from_json(file);
			state->head = tmp;
		} 
		else
		{
			tmp->next = dc_entry_from_json(file);
			tmp = tmp->next;
		}
		++count;
	}
	
	state->num_files += count;
	return next;
}

struct dc_entry_t* dc_entry_from_json(struct json_object* json)
{
	struct dc_entry_t* entry;
	struct json_object* tmp;
	entry = (struct dc_entry_t*) malloc(sizeof(struct dc_entry_t));
	if( entry == NULL ) {}

	memset(entry, 0, sizeof(struct dc_entry_t));

	tmp = json_object_object_get(json, "fileName");
	str_init_create(&entry->fileName, json_object_get_string(tmp), 0);

	tmp = json_object_object_get(json, "fileSize");
	entry->fileSize = strtol((char*)json_object_get_string(tmp), NULL, 10);

	tmp = json_object_object_get(json, "uploadDate");
	entry->uploadDate = strtol((char*)json_object_get_string(tmp), NULL, 10);
	
	tmp = json_object_object_get(json, "deviceID");
	str_init_create(&entry->deviceID, json_object_get_string(tmp), 0);

	tmp = json_object_object_get(json, "_id");
	str_init_create(&entry->_id, json_object_get_string(tmp), 0);

	tmp = json_object_object_get(json, "isFolder");
	entry->isFolder = strtol((char*)json_object_get_string(tmp), NULL, 10);

	return entry;
}

