/*
	fuse-google-drive: a fuse filesystem wrapper for Google Drive
	Copyright (C) 2012  James Cline

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
 	published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "str.h"
#include "curl_interface.h"

#include <curl/curl.h>
#include <curl/multi.h>

#include <string.h>

/** Initialize a request.
 *
 *  This handles setting up a request for curl. It is preferable to call this
 *  once for multiple requests to help curl reuse connections when possible.
 *  Use the other set and reset methods to change things for multiple requests.
 *
 *  @request      struct request_t* the request_t to initialize
 *  @uri          struct str_t*     the uri for the initial request
 *  @header_count size_t            the number of elements in headers[]
 *  @headers      struct str_t[]    the headers, if any, for this request
 *  @msg          const char*       a message for POST, NULL if not POST
 *  @type         enum request_type the type of the request, GET, POST, ...
 */
int ci_init(struct request_t* request, struct str_t* uri,
		size_t header_count, const struct str_t const headers[],
		const char const* msg, enum request_type_e type)
{
	// TODO: Errors
	union func_u func;
	int ret = 0;

	memset(request, 0, sizeof(struct request_t));

	fstack_init(&request->cleanup, 10);

	CURL* handle = curl_easy_init();
	func.func1 = curl_easy_cleanup;
	fstack_push(&request->cleanup, handle, &func, 1);

	curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(handle, CURLOPT_USE_SSL, CURLUSESSL_ALL); // SSL

	curl_easy_setopt(handle, CURLOPT_HEADER, 1); // Enable headers, necessary?
	// set curl_post_callback for parsing the server response
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, ci_callback_controller);
	// set curl_post_callback's last parameter to state
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &request->response);

	if(header_count)
	{
		ret = ci_create_header(request, header_count, headers);
		curl_easy_setopt(handle, CURLOPT_HTTPHEADER, request->headers); // Set headers
	}

	curl_easy_setopt(handle, CURLOPT_USERAGENT, "darkcloud/0.1");

	switch(type)
	{
		case GET:
			break;
		case POST:
			curl_easy_setopt(handle, CURLOPT_POSTFIELDS, msg);
			break;
		case DELETE:
			curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE"); 
			break;
		default:
			break;
	}

	request->handle = handle;

	ret = ci_set_uri(request, uri);

	return ret;
}

/** Cleanup a request.
 *
 * @request struct request_t* the request to uninitialize
 */
int ci_destroy(struct request_t* request)
{
	while(request->cleanup.size)
		fstack_pop(&request->cleanup);
	fstack_destroy(&request->cleanup);
	memset(request, 0, sizeof(struct request_t));
	return 0;
}

/** Set the URI for a request.
 *
 *  @request struct request_t* the request to set
 *  @uri     struct str_t*     the str_t containing the uri to set
 */
int ci_set_uri(struct request_t* request, struct str_t* uri)
{
	return curl_easy_setopt(request->handle, CURLOPT_URL, uri->str); // set URI
}

/** Create the header for a request from an array of str_ts.
 *
 *  Takes an array of str_ts and creates a header from them.
 *  This has currently only been tested for header_count = 2.
 *
 *  @request      struct request_t* the request we want to set headers for.
 *  @header_count size_t            the number of headers in headers[]
 *  @headers      struct str_t[]    the strings for the headers.
 */
int ci_create_header(struct request_t* request,
		size_t header_count, const struct str_t headers[])
{
	union func_u func;
	struct curl_slist *header_list = NULL;

	size_t count = 0;
	for(; count < header_count; ++count)
	{
		printf("%s\n", headers[count].str);
		header_list = curl_slist_append(header_list, headers[count].str);
	}

	request->headers = header_list;
	func.func1 = curl_slist_free_all;
	fstack_push(&request->cleanup, header_list, &func, 1);

	return 0;
}

/** Make a request.
 *
 *  @request struct request_t* the initialized request_t for making this request
 */
int ci_request(struct request_t* request)
{
	ci_reset_flags(request);
	return curl_easy_perform(request->handle);
}

/** Reset the request.response data.
 *
 *  We want to do this in order to safely reuse a request_t.
 *
 *  @request struct request_t* the request containing the response to reset.
 */
void ci_clear_response(struct request_t* request)
{
	// First, clear the data
	str_destroy(&request->response.body);
	str_destroy(&request->response.headers);
	// Then reinitialize the variables for further use
	str_init(&request->response.body);
	str_init(&request->response.headers);
	// Reset the flags
	memset(&request->flags, 0, sizeof(struct request_flags_t));
}

/** Curl callback to handle Google's response when listing files.
 *
 *  Because Google's server returns the file listing in chunks, this function
 *  puts all those chunks together into one contiguous string for each section,
 *  the header and the body.
 *
 *  It should be noted that this function is called for every line in the
 *  header of the response. This is why the CRLF scanning works the way it does.
 *
 *  @data  char*             the response from Google's server
 *  @size  size_t            size of one element in data
 *  @nmemb size_t            number of size chunks
 *  @store struct request_t* the request this callback is for
 *
 *  @returns the size of the data read, curl expects size*nmemb or it errors
 */
size_t ci_callback_controller(void *data, size_t size, size_t nmemb, void *store)
{
	struct request_t* req = (struct request_t*) store;
	struct request_flags_t* flags = &req->flags;

	// Store the header portion of the reqponse
	if(!flags->header)
	{
		// Find the first occurrence of CRLF in this line of header
		char* iter = strstr((char*) data, "\r\n");
		// If the header only contains CRLF, the header is over after this call
		if(iter == (char*)data)
			flags->header = 1;
		struct str_t *header = &req->response.headers;
		str_char_concat(header, (char*) data, size*nmemb);
	}
	// If we are not in the header section of the response, then
	// we need to store the body portion.
	else
	{
		struct str_t *body = &req->response.body;
		str_char_concat(body, (char*) data, size*nmemb);
	}

	return size*nmemb;
}

/** Resets the flags for a request.
 *
 *  @request struct request_t* the request we wish to reset
 */
void ci_reset_flags(struct request_t* request)
{
	memset(&request->flags, 0, sizeof(struct request_flags_t));
}
