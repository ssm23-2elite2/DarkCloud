#include "util.h"

const char* strip_path(const char *path)
{
	char *filename = strrchr(path, '/');
	++filename;
	return filename;
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

