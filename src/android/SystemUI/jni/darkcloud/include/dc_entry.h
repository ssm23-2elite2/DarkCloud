#ifndef _DARKCLOUD_DC_ENTRY_H
#define _DARKCLOUD_DC_ENTRY_H

#include "str.h"

struct dc_entry_t {
	struct str_t fileName;
	unsigned long fileSize;
	unsigned long uploadDate;
	struct str_t deviceID;
	struct str_t path;
	struct str_t _id;
	
	int isFolder;
	struct str_t cache;
	int cached;

	struct dc_entry_t *next;
};

#endif

