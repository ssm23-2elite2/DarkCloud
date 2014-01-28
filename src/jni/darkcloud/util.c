#include "util.h"

void get_hex_dump(const char *src, size_t len, char *buf)
{
    int i;
    char hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    for(i = 0; i < len; i++) 
    {    
        buf[i * 2] = hex[src[i] % 16];
        buf[i * 2 + 1] = hex[src[i] / 16];
    }    
}

const char* strip_path(const char *path)
{
	char *filename = strrchr(path, '/');
	++filename;
	return filename;
}

const char* set_only_path(char *path)
{
	int len = strlen(path);
	int i;

	for(i = len - 1; i >= 0; i--) {
		if(path[i] == '/') {
			if(i == 0) {
				path[i+1] = '\0';
			} else {
				path[i] = '\0';
			}
			break;
		}
	}
	return path;	
}

const char filenameunsafe[] =
{
        '%',
        '/'
};


/** Escapes unsafe characters for filenames.
 *
 *  @filename the string to escape
 *  @length length of filename
 *          precondition:  strlen(filename)
 *          postcondition: strlen(escaped filename)
 *
 *  @returns escaped, null terminated string
 */
const char* filenameencode (const char *filename, size_t *length)
{
        size_t i;
        size_t j;
        size_t count = 0;
        size_t size = *length;
        // Count the number of characters that need to be escaped
        for(i = 0; i < size; ++i)
        {   
                for(j = 0; j < sizeof(filenameunsafe); ++j)
                {   
                        if(filename[i] == filenameunsafe[j])
                        {   
                                ++count;
                                break;
                        }   
                }   
        }   

        // Allocate the correct amount of memory for the escaped string
        char *result = (char *) malloc( sizeof(char) * (size + count*3 + 1));
        if(result == NULL)
                return NULL;

        // Copy old string into escaped string, escaping where necessary
        char *iter = result;
        for(i = 0; i < size; ++i)
        {   
                for(j = 0; j < sizeof(filenameunsafe); ++j)
                {   
                        // We found a character that needs escaping
                        if(filename[i] == filenameunsafe[j])
                        {   
                                // Had a weird issue with sprintf(,"\%%02X,), so I do this instead
                                *iter = '%';
                                ++iter;
                                sprintf(iter, "%02X", filenameunsafe[j]);
                                iter+=2;
                                break;
                        }
                }
                // We did not need to escape the current character in filename, so just copy it
                if(j == sizeof(filenameunsafe))
                {
                        *iter = filename[i];
                        ++iter;
                }
        }

        // Calculate the size of the final string, should be the same as (length+count*3)
        size = iter - result;
        // Make sure we null terminate
        result[size] = 0;
        // Save the new length
        *length = size;

        return result;
}

const char* urlencode(const char* url, size_t* length)
{
}

size_t add_encoded_uri(char* buf, const char* uri, const size_t size)
{
	size_t length = size;
	char *encoded = uriencode(uri, &length);
	memcpy(buf, encoded, length);
	free(encoded);
	return length - 1;
}

size_t add_unencoded_uri(char* buf, const char* str, const size_t size)
{
	memcpy(buf, str, size);
	return size - 1;
}

char *replaceAll(char *s, const char *olds, const char *news) {
	char *result, *sr;
	size_t i, count = 0;
	size_t oldlen = strlen(olds); if (oldlen < 1) return s;
	size_t newlen = strlen(news);


	if (newlen != oldlen) {
		for (i = 0; s[i] != '\0';) {
			if (memcmp(&s[i], olds, oldlen) == 0) count++, i += oldlen;
			else i++;
		}
	} else i = strlen(s);


	result = (char *) malloc(i + 1 + count * (newlen - oldlen));
	if (result == NULL) return NULL;


	sr = result;
	while (*s) {
		if (memcmp(s, olds, oldlen) == 0) {
			memcpy(sr, news, newlen);
			sr += newlen;
			s  += oldlen;
		} else *sr++ = *s++;
	}
	*sr = '\0';

	return result;
}

int get_length(char *s, const char *olds, const char *news) {
	char *resut, *sr;
	int len, count = 0;
	int oldlen, newlen;

	oldlen = strlen(olds);
	newlen = strlen(news);

	if(newlen == oldlen) {
		len = strlen(s) + 1;
	} else {
		for(len = 0; s[len] != '\0';) {
			if(memcmp(&s[len], olds, oldlen) == 0) {
				count++;
				len += oldlen;
			} else {
				len++;
			}
		}
		len = strlen(s) + 1 + count * (newlen - oldlen);
	}
	return len;
}

