#ifndef _DARKCLOUD_UTIL_H
#define _DARKCLOUD_UTIL_H

#include <string.h>
//typedef int size_t;

const char* strip_path(const char *path);
const char* filenameencode (const char *filename, size_t *length);
const char* urlencode(const char *url, size_t *length);
size_t add_encoded_uri(char* buf, const char* uri, const size_t size);
size_t add_unencoded_uri(char* buf, const char* str, const size_t size);


#endif
