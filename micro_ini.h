/**
 * Copyright (c) 2021, Zoe J. Bare
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <stdio.h>

#define MICRO_INI_VERSION_MAJOR  1
#define MICRO_INI_VERSION_MINOR  0
#define MICRO_INI_VERSION_HOTFIX 0

#define MICRO_INI_STR2(x) #x
#define MICRO_INI_STR(x)  MICRO_INI_STR2(x)

#define MICRO_INI_VERSION_STR \
	MICRO_INI_STR(MICRO_INI_VERSION_MAJOR) "." \
	MICRO_INI_STR(MICRO_INI_VERSION_MINOR) "." \
	MICRO_INI_STR(MICRO_INI_VERSION_HOTFIX)

#define MICRO_INI_SUCCESS                         0 /* Parsing succeeded. */
#define MICRO_INI_ERROR_INVALID_FILE_OBJECT      -1 /* FILE object is null. */
#define MICRO_INI_ERROR_INVALID_STREAM_OBJECT    -2 /* Stream object is null. */
#define MICRO_INI_ERROR_INVALID_HANDLER_CALLBACK -3 /* Key/value handling callback is null. */
#define MICRO_INI_ERROR_INVALID_READER_CALLBACK  -4 /* Reader callback is null. */
#define MICRO_INI_ERROR_INVALID_EOF_CALLBACK     -5 /* EOF callback is null. */
#define MICRO_INI_ERROR_BUFFER_OVERFLOW          -6 /* Attempting to read a line resulted in a string exceeding the maximum allowed length. */

#define MICRO_INI_FLAG_BOM                 0x1 /* Enable support for the byte order marker in files with UTF-8 encoding. */
#define MICRO_INI_FLAG_MULTILINE           0x2 /* Enable support for multi-line parsing. */
#define MICRO_INI_FLAG_STOP_ON_FIRST_ERROR 0x4 /* Stop parsing when the first error has been reached. */

#ifdef _WIN32
	#ifdef MICRO_INI_API_EXPORT
		#define MICRO_INI_API __declspec(dllexport)
	#elif defined(MICRO_INI_API_IMPORT)
		#define MICRO_INI_API __declspec(dllimport)
	#else
		#define MICRO_INI_API
	#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Key/value handling function. */
typedef void (*micro_ini_handler_fn)(void* pUserData, const char* section, const char* key, const char* value);

/* Error handling function. */
typedef void (*micro_ini_error_fn)(void* pUserData, const char* line, int lineno);

/* An fgets-style reader function. */
typedef char* (*micro_ini_reader_fn)(char* str, int num, void* pStream);

/* An feof-style function. */
typedef int (*micro_ini_eof_fn)(void* pStream);

/**
 * @brief   Parse an ini file.
 * @return  Error code or number of parsing errors that occurred.
 *
 * @param[in]  filePath        Path to the ini file to read.
 * @param[in]  flags           Flags for configuring the parser.
 * @param[in]  ini_handler_fn  Callback for handling parsed key/value pairs.
 * @param[in]  ini_error_fn    Callback for handling parsing errors (this callback is optional and may be NULL if unneeded).
 * @param[in]  pUserData       Pointer to user data that is passed to the callbacks.
 *
 * This is the parser for ini files.  This function is called,
 * providing the name of the file to be read.  The return value
 * will be INI_SUCCESS when parsing succeeds.  A return value less
 * than 0 will be a specific error (INI_ERROR_*) and above 0 indicates
 * the number of parsing errors that occurred.
 */
MICRO_INI_API int micro_ini_load(
	const char* const filePath,
	const int flags,
	const micro_ini_handler_fn handlerCallback,
	const micro_ini_error_fn errorCallback,
	void* const pUserData
);

/**
 * @brief   Parse an ini file from a FILE object.
 * @return  Error code or number of parsing errors that occurred.
 *
 * @param[in]  pFile           Pointer to an existing FILE object.
 * @param[in]  flags           Flags for configuring the parser.
 * @param[in]  ini_handler_fn  Callback for handling parsed key/value pairs.
 * @param[in]  ini_error_fn    Callback for handling parsing errors (this callback is optional and may be NULL if unneeded).
 * @param[in]  pUserData       Pointer to user data that is passed to the callbacks.
 *
 * This is the parser for ini files.  This function expects a valid
 * FILE object referencing the ini file to be parsed.  The return value
 * will be INI_SUCCESS when parsing succeeds.  A return value less
 * than 0 will be a specific error (INI_ERROR_*) and above 0 indicates
 * the number of parsing errors that occurred.
 */
MICRO_INI_API int micro_ini_load_file(
	FILE* const pFile,
	const int flags,
	const micro_ini_handler_fn handlerCallback,
	const micro_ini_error_fn errorCallback,
	void* const pUserData
);

/**
 * @brief   Parse an ini file from a custom stream object.
 * @return  Error code or number of parsing errors that occurred.
 *
 * @param[in]  pStream         Pointer to an existing stream object.
 * @param[in]  flags           Flags for configuring the parser.
 * @param[in]  handleCallback  Callback for handling parsed key/value pairs.
 * @param[in]  errorCallback   Callback for handling parsing errors (this callback is optional and may be NULL if unneeded).
 * @param[in]  readerCallback  Callback for reading lines in the ini file (must conform to fgets() functionality).
 * @param[in]  eofCallback     Callback for checking if the end of the ini file has been reached (must conform to feof() functionality).
 * @param[in]  pUserData       Pointer to user data that is passed to the callbacks.
 *
 * This is the parser for ini files.  This function expects a valid
 * stream object referencing the ini file to be parsed.  The stream
 * object is user defined, so when paired with the reader and EOF
 * callbacks, it is up to the user to define the stream object type
 * and how it works.  The return value will be INI_SUCCESS when
 * parsing succeeds.  A return value less than 0 will be a specific
 * error (INI_ERROR_*) and above 0 indicates the number of parsing
 * errors that occurred.
 */
MICRO_INI_API int micro_ini_load_stream(
	void* const pStream,
	const int flags,
	const micro_ini_handler_fn handlerCallback,
	const micro_ini_error_fn errorCallback,
	const micro_ini_reader_fn readerCallback,
	const micro_ini_eof_fn eofCallback,
	void* const pUserData
);

#ifdef __cplusplus
}
#endif
