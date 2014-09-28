/*
        DarkCloud System
	Copyright (C) 2013  Yoon
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

// Special Character
struct str_t plus;
struct str_t amp;
struct str_t que;
struct str_t slash;
struct str_t colon;
struct str_t lcurly;
struct str_t rcurly;
struct str_t comma;
struct str_t d_quot;

const char opendir_uri[] = "/opendir";
const char create_uri[] = "/create";
const char remove_uri[] = "/remove";
const char read_uri[] = "/read";
const char write_uri[] = "/write";
const char mkdir_uri[] = "/mkdir";
const char rmdir_uri[] = "/rmdir";
const char access_uri[] = "/access";

void dci_print_all(struct dci_state *state)
{
	char tmp[1024];
	
	fprintf(stderr, "START: dci_print_all \n");
	struct dc_entry_t* iter = state->head;
	while(iter != NULL)
	{
		printf("@@: %s %s\n", iter->path.str, iter->fileName.str);
		iter = iter->next;
	}
	fprintf(stderr, "END: dci_print_all \n");
}

void dci_remove_path_data(const char *path, struct dci_state *state)
{
	dci_print_all(state);
	char tmp[1024];
	
	fprintf(stderr, "START: dci_remove_path_data %s \n", path);
	struct dc_entry_t* iter = state->head;
	while(iter != NULL)
	{
		if(strcmp(iter->path.str, path) == 0 && iter->isFolder == 0) {
			iter = dci_remove_item(iter, state);
		} else {
			iter = iter->next;
		}
	}
	fprintf(stderr, "END: dci_remove_path_data %s \n", path);
}


char* load_file(const char* path)
{
	size_t pathlen = strlen(path);

	FILE *f = fopen(path, "rb");
	if(f == NULL)
	{
		printf("fopen(\"%s\"): %s\n", path, strerror(errno));
		return NULL;
	}

	// Find the size of the file
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	rewind(f);

	char *result = (char *) malloc(sizeof(char) * (size+1));
	if(result == NULL)
	{
		perror("malloc");
		return NULL;
	}

	// Read the file in one chunk, possible issue for larger files
	fread(result, 1, size, f);
	if(ferror(f))
	{
		printf("fread(\"%s\"): %s\n", path, strerror(errno));
		return NULL;
	}

	fclose(f);
	result[size] = 0;

	char *to = result;
	char *from = result;
	for(; *from != 0; ++from)
	{
		*to = *from;
		if(*from != '\n')
			++to;
	}
	*to = 0;

	return result;
}

char *dci_load_ipaddr()
{
	char *ret = load_file("/data/local/tmp/ip");
	if(ret == NULL) {
		ret = (char *)malloc(sizeof("http://211.189.19.25:3000") + 1);
		sprintf(ret, "http://211.189.19.25:3000");
	}

	return ret;
}

char *dci_load_userid()
{
	char *ret = load_file("/data/local/tmp/userid");
	if(ret == NULL) {
		// userid가 없으면 기본값 error 리턴
		ret = (char *)malloc(sizeof("error") + 1);
		sprintf(ret, "error");
	}

	return ret;
}	

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
	return 0;
}

int dci_init(struct dci_state *state, char *ipaddr)
{
	union func_u func;
	
	str_init_create(&plus, "+", 0);
	str_init_create(&amp, "&", 0);
	str_init_create(&que, "?", 0);
	str_init_create(&slash, "/", 0);
	str_init_create(&colon, ":", 0);
	str_init_create(&lcurly, "{", 0);
	str_init_create(&rcurly, "}", 0);
	str_init_create(&comma, ",", 0);
	str_init_create(&d_quot, "\"", 0);

	str_init_create(&state->userid, dci_load_userid(), 0);
	str_init_create(&state->ipaddr, ipaddr, 0);

	printf("userid = [%s]\n", state->userid.str);
	printf("ipaddr = [%s]\n", state->ipaddr.str);
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

void dci_add_item(struct dc_entry_t* item, struct dci_state *state)
{
	struct dc_entry_t *tmp = dci_last_entry(state);
	if(!tmp) {
		state->head = item;
	} else {
		tmp->next = item;
		tmp = tmp->next;
	}
	++state->num_files;
}

struct dc_entry_t *dci_remove_item(struct dc_entry_t* item, struct dci_state *state)
{
	fprintf(stderr, "START: dci_remove_path_data %s %s \n", item->path.str, item->fileName.str);
	struct dc_entry_t* iter = state->head;
	struct dc_entry_t* before = state->head;
	while(iter != NULL)
	{
		if(iter == item)
			break;

		before = iter;
		iter = iter->next;
	}
	before->next = iter->next;

	if(iter != NULL)
		free(iter);

	state->num_files--;
	fprintf(stderr, "END: dci_remove_path_data %s \n", item->path.str);

	return before->next;
}

struct dc_entry_t *dci_last_entry(struct dci_state *state)
{
	struct dc_entry_t* iter = state->head;
	if(iter == NULL)
		return NULL;
	
	while(iter->next != NULL)
	{
		iter = iter->next;
	}
	return iter;
}

struct dc_entry_t* find_entry_from_path(struct dci_state* state, const char *path)
{
	struct dc_entry_t* iter = state->head;
	char tmp[2048];
	
	while(iter != NULL)
	{
		if(strcmp(iter->path.str, "/") == 0)
			sprintf(tmp, "%s%s", iter->path.str, iter->fileName.str);
		else
			sprintf(tmp, "%s/%s", iter->path.str, iter->fileName.str);
			
		if(strcmp(tmp, path) == 0)
			return iter;

		iter = iter->next;
	}
	return NULL;
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
	

int dci_is_already_path(const char *path, struct dci_state *state)
{
	fprintf(stderr, "START: dci_is_already_path %s \n", path);

	struct dc_entry_t* iter = state->head;
	while(iter != NULL)
	{
		if(strcmp(iter->path.str, path) == 0) {
			return TRUE;
		}
	
		iter = iter->next;
	}
	fprintf(stderr, "END: dci_is_already_path NO");
	return FALSE;
}

void dci_destroy(struct dci_state *state)
{
	printf("Clean up DCI...\n");
	fflush(stdout);
	dci_init_list(state);
	
	while(state->stack->size)
		fstack_pop(state->stack);
	fstack_destroy(state->stack);
	free(state->stack);

	str_destroy(&plus);
	str_destroy(&amp);
	str_destroy(&que);
	str_destroy(&slash);
	str_destroy(&colon);
	str_destroy(&lcurly);
	str_destroy(&rcurly);
	str_destroy(&comma);
	str_destroy(&d_quot);
}

void dci_opendir(const char *path, struct dci_state *state)
{
	/* 2014/01/28 
		path data remove
	*/
	if(strcmp(path, "/") != 0)
		dci_remove_path_data(path, state);
	
	if(dci_is_already_path(path, state)) return;
	struct str_t uri;
	char temp[100];
	sprintf(temp, "%s%s", state->ipaddr.str, opendir_uri);
	str_init_create(&uri, temp, 0);
	
	struct str_t path_param;
	str_init_create(&path_param, "path=", 0);
	struct str_t *path_str = str_urlencode_char(path, 0);
	const struct str_t const* uri_parts[] =
	{
		&slash,
		&state->userid,
		&que,
		&path_param,
		path_str
	};
	str_concat(&uri, sizeof(uri_parts)/sizeof(const struct str_t const*), uri_parts);
	{
		struct request_t request;
		ci_init(&request, &uri, 0, NULL, NULL, GET);
		ci_request(&request);

		json_parse_file_list(path, &request.response.body, state);
	
		ci_destroy(&request);
	}
	str_destroy(&uri);
}

int dci_mkdir(const char *path, const char *fileName, struct dci_state *state)
{
	fprintf(stderr, "START: dci_mkdir %s %s\n", path, fileName);
	struct str_t header ,fileName_param, fileSize_param, uploadDate_param, uploadDate_value;
	struct str_t deviceID_param, isFolder_param, path_param, data_param;

	char uploadDate[11];
	
	str_init_create(&header, "Content-Type: application/json", 0);
	str_init_create(&fileName_param, "\"fileName\" : ", 0);
	str_init_create(&fileSize_param, "\"fileSize\" : \"0\"", 0);
	str_init_create(&uploadDate_param, "\"uploadDate\" : ", 0);
	str_init_create(&deviceID_param, "\"deviceID\" : ", 0);
	str_init_create(&isFolder_param, "\"isFolder\" : \"1\"", 0);
	str_init_create(&path_param, "\"path\" : ", 0);
	str_init_create(&data_param, "\"data\" : ", 0);

	struct dc_entry_t* entry;
	entry = (struct dc_entry_t*) malloc(sizeof(struct dc_entry_t));

	memset(entry, 0, sizeof(struct dc_entry_t));

	str_init_create(&entry->fileName, fileName, 0);
	entry->fileSize = 0;
	entry->uploadDate = time(0);
	sprintf(uploadDate, "%lu", entry->uploadDate);
	entry->isFolder = 1;
	str_init_create(&entry->path, path, 0);
	str_init_create(&entry->cache, "", 0);
	entry->cached = 0;
	
	str_init_create(&uploadDate_value, uploadDate, 0);
	struct str_t uri;
	struct str_t param_uri;
	char temp[100];
	sprintf(temp, "%s%s", state->ipaddr.str, mkdir_uri);
	str_init_create(&uri, temp, 0);
	str_init(&param_uri);

	const struct str_t const* uri_parts[] =
	{
		&lcurly, &fileName_param, &d_quot, &entry->fileName, &d_quot,
		&comma, &fileSize_param, &comma, &uploadDate_param,
		&d_quot, &uploadDate_value, &d_quot, &comma,
		&deviceID_param, &d_quot, &state->userid, &d_quot,
		&comma, &isFolder_param, &comma, &path_param,
		&d_quot, &entry->path, &d_quot, &comma,
		&data_param, &d_quot, &entry->cache, &d_quot, &rcurly
	};

	str_concat(&param_uri, sizeof(uri_parts)/sizeof(const struct str_t const*), uri_parts);
	{
		struct request_t request;
		ci_init(&request, &uri, 1, &header, param_uri.str, POST);
		ci_request(&request);

		str_swap(&request.response.body, &entry->_id);
		
		dci_add_item(entry, state);
		ci_destroy(&request);
	}
	fprintf(stderr, "END: dci_mkdir\n");
	return 0;
}

int dci_rmdir(const char *path, struct dci_state *state)
{
	fprintf(stderr, "START: dci_rmdir %s \n", path);
	struct dc_entry_t* entry = find_entry_from_path(state, path);

	struct str_t uri;
	char temp[100];
	sprintf(temp, "%s%s", state->ipaddr.str, rmdir_uri);
	str_init_create(&uri, temp, 0);
	
	const struct str_t const* uri_parts[] =
	{
		&slash,
		&entry->_id
	};
	str_concat(&uri, sizeof(uri_parts)/sizeof(const struct str_t const*), uri_parts);
	{
		struct request_t request;
		ci_init(&request, &uri, 0, NULL, NULL, DELETE);
		ci_request(&request);

		dci_remove_item(entry, state);
		
		ci_destroy(&request);
	}
	fprintf(stderr, "END: dci_rmdir\n");
	return 0;
}

int dci_load(struct dci_state* state, struct dc_entry_t* entry)
{
	fprintf(stderr, "START: dci_load, cached[%d]\n", entry->cached);
	if(entry->cached == 0)
	{
		struct str_t uri;
		char temp[100];
		sprintf(temp, "%s%s", state->ipaddr.str, read_uri);
		str_init_create(&uri, temp, 0);

		const struct str_t const* uri_parts[] =
		{
			&slash,
			&entry->_id,
		};
		str_concat(&uri, sizeof(uri_parts)/sizeof(const struct str_t const*), uri_parts);
		{
			struct request_t request;
			ci_init(&request, &uri, 0, NULL, NULL, GET);
			ci_request(&request);
		
			str_swap(&request.response.body, &entry->cache);
			ci_destroy(&request);
			entry->cached = 1;
		}
	}
	fprintf(stderr, "END: dci_load\n");

	return 0;
}
 
const char* dci_read(size_t *size, struct dc_entry_t* entry, off_t offset)
{
	fprintf(stderr, "START: dci_read\n");
	size_t remaining = (entry->cache.len < offset) ? 0 : entry->cache.len - offset;
	*size = (remaining < *size) ? remaining : *size;
	fprintf(stderr, "END: dci_read\n");
	
	if(*size == 0)
		return NULL;
	return entry->cache.str + offset;
}

int dci_create(const char *fileName, const char *path, struct dci_state *state)
{
	fprintf(stderr, "START: dci_create %s %s\n", path, fileName);
	struct str_t header ,fileName_param, fileSize_param, uploadDate_param, uploadDate_value;
	struct str_t deviceID_param, isFolder_param, path_param, data_param, update_param;

	char uploadDate[11];
	str_init_create(&header, "Content-Type: application/json", 0);
	str_init_create(&fileName_param, "\"fileName\" : ", 0);
	str_init_create(&fileSize_param, "\"fileSize\" : \"0\"", 0);
	str_init_create(&uploadDate_param, "\"uploadDate\" : ", 0);
	str_init_create(&deviceID_param, "\"deviceID\" : ", 0);
	str_init_create(&isFolder_param, "\"isFolder\" : \"0\"", 0);
	str_init_create(&path_param, "\"path\" : ", 0);
	str_init_create(&data_param, "\"data\" : ", 0);
	str_init_create(&update_param, "\"update\" : ", 0);

	struct dc_entry_t* entry;
	entry = (struct dc_entry_t*) malloc(sizeof(struct dc_entry_t));

	memset(entry, 0, sizeof(struct dc_entry_t));

	str_init_create(&entry->fileName, fileName, 0);
	entry->fileSize = 0;
	entry->uploadDate = time(0);
	sprintf(uploadDate, "%lu", entry->uploadDate);
	entry->isFolder = 0;
	str_init_create(&entry->path, path, 0);
	str_init_create(&entry->cache, "", 0);
	entry->cached = 0;
	
	str_init_create(&uploadDate_value, uploadDate, 0);
	struct str_t uri;
	struct str_t param_uri;
	char temp[100];
	sprintf(temp, "%s%s", state->ipaddr.str, create_uri);
	str_init_create(&uri, temp, 0);
	str_init(&param_uri);

	const struct str_t const* uri_parts[] =
	{
		&lcurly, &fileName_param, &d_quot, &entry->fileName, &d_quot,
		&comma, &fileSize_param, &comma, &uploadDate_param, 
		&d_quot, &uploadDate_value, &d_quot, &comma,
		&deviceID_param, &d_quot, &state->userid, &d_quot,
		&comma, &isFolder_param, &comma, 
		&path_param, &d_quot, &entry->path, &d_quot, &comma,
		&update_param, &d_quot, &uploadDate_value, &d_quot, &comma,
		&data_param, &d_quot, &entry->cache, &d_quot, &rcurly
	};

	str_concat(&param_uri, sizeof(uri_parts)/sizeof(const struct str_t const*), uri_parts);
	{
		struct request_t request;
		ci_init(&request, &uri, 1, &header, param_uri.str, POST);
		ci_request(&request);

		str_swap(&request.response.body, &entry->_id);
		
		dci_add_item(entry, state);

		ci_destroy(&request);
	}
	fprintf(stderr, "END: dci_create\n");
	return 0;
}

int dci_unlink(const char *path, struct dci_state *state)
{
	fprintf(stderr, "START: dci_unlink %s \n", path);
	struct dc_entry_t* entry = find_entry_from_path(state, path);

	struct str_t uri;
	char temp[100];
	sprintf(temp, "%s%s", state->ipaddr.str, remove_uri);
	str_init_create(&uri, temp, 0);
	
	const struct str_t const* uri_parts[] =
	{
		&slash,
		&entry->_id
	};
	str_concat(&uri, sizeof(uri_parts)/sizeof(const struct str_t const*), uri_parts);
	{
		struct request_t request;
		ci_init(&request, &uri, 0, NULL, NULL, DELETE);
		ci_request(&request);

		dci_remove_item(entry, state);
		
		ci_destroy(&request);
	}
	fprintf(stderr, "END: dci_unlink\n");
	return 0;
}

int dci_write(const char *path, const char *buf, size_t size, off_t offset, const char *fileName, struct dci_state *state, struct dc_entry_t* entry)
{
	fprintf(stderr, "START: dci_write %s %s\n", path, fileName);
	struct str_t fileSize_param, fileSize_value;
	struct str_t data_param, Offset_param;
	char *bodyline = calloc(sizeof(char), 1024*1024*10);

	memcpy(bodyline, buf, size);
	get_hex_dump(buf, size, bodyline);

	int ioffset = offset;
	char fileSize[16], Offset[16];
	str_init_create(&fileSize_param, "\"fileSize\" : ", 0);
	str_init_create(&data_param, "\"data\" : ", 0);
	
	entry->uploadDate = time(0);
	str_char_concat(&entry->cache, buf, size);
	entry->fileSize += size;

	sprintf(Offset, "%d", ioffset);
	str_init_create(&Offset_param, Offset, 0);
	sprintf(fileSize, "%d", size);
	entry->cached = 1;

	str_init_create(&fileSize_value, fileSize, 0);
	struct str_t uri;
	char temp[100];
	sprintf(temp, "%s%s", state->ipaddr.str, write_uri);
	str_init_create(&uri, temp, 0);

	const struct str_t const* uri_parts[] =
	{
		&slash,&entry->_id, &slash, &fileSize_value, &slash, &Offset_param
	};
	
	str_concat(&uri, sizeof(uri_parts)/sizeof(const struct str_t const*), uri_parts);
	{
		struct request_t request;
		ci_init(&request, &uri, 0, NULL, bodyline, POST);

		ci_request(&request);
		ci_destroy(&request);
	}
	free(bodyline);
	fprintf(stderr, "END: dci_write\n");
	return size;
}

void dc_entry_destroy(struct dc_entry_t *entry)
{
	str_destroy(&entry->fileName);
	str_destroy(&entry->deviceID);
	str_destroy(&entry->_id);
	str_destroy(&entry->cache);
}

struct str_t* json_parse_file_list(const char *path, struct str_t* json_str, struct dci_state* state)
{
	struct json_object* json = json_tokener_parse(json_str->str);

	union func_u func;

	struct json_object* file;
	struct dc_entry_t *tmp = dci_last_entry(state);

	size_t count = 0;

	if(json != NULL) {
		int length = json_object_array_length(json);
		int i;

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

	}
	return NULL;
}

struct dc_entry_t* dc_entry_from_json(struct json_object* json)
{
	struct dc_entry_t* entry;
	struct json_object* tmp;
	entry = (struct dc_entry_t*) malloc(sizeof(struct dc_entry_t));

	memset(entry, 0, sizeof(struct dc_entry_t));

	tmp = json_object_object_get(json, "fileName");
	str_init_create(&entry->fileName, json_object_get_string(tmp), 0);

	tmp = json_object_object_get(json, "fileSize");
	entry->fileSize = strtol((char*)json_object_get_string(tmp), NULL, 10);

	tmp = json_object_object_get(json, "uploadDate");
	entry->uploadDate = strtol((char*)json_object_get_string(tmp), NULL, 10);

	tmp = json_object_object_get(json, "deviceID");
	str_init_create(&entry->deviceID, json_object_get_string(tmp), 0);

	tmp = json_object_object_get(json, "isFolder");
	entry->isFolder = strtol((char*)json_object_get_string(tmp), NULL, 10);

	tmp = json_object_object_get(json, "path");
	str_init_create(&entry->path, json_object_get_string(tmp), 0);

	tmp = json_object_object_get(json, "_id");
	str_init_create(&entry->_id, json_object_get_string(tmp), 0);
	return entry;
}

