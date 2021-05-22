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

#include "micro_ini.h"

#include <ctype.h>
#include <string.h>

/**
 * Maximum length of a single line in the ini file.
 */
#define MICRO_INI_MAX_LINE_LENGTH 512

/**
 * This enum stores the status for each parsed line (internal use only).
 */
enum LineStatus
{
    LINE_UNPROCESSED,
	LINE_EMPTY,
    LINE_ERROR,
    LINE_COMMENT,
    LINE_SECTION,
    LINE_VALUE
};

/**
 * @brief  Strip the whitespace from an input line in-place.
 *
 * @param[in]  line  Line to strip.
 */
static void prv_micro_ini_strstrip(char* const line)
{
	const char* start;
	const char* end;
	const int len = (int) strlen(line);

	if(len == 0)
	{
		/* Empty string. */
		line[0] = '\0';
		return;
	}

	start = line;
	end = line + len;

	while(start < end && isspace((unsigned char) *start))
	{
		/* Skip whitespace at the beginning of the string. */
		++start;
	}

	while(start < end && isspace((unsigned char) *(end - 1)))
	{
		/* Ignore whitespace at the end of the string. */
		--end;
	}

	if(start >= end)
	{
		/* The entire string is whitespace. */
		line[0] = '\0';
	}
	else
	{
		const size_t outLen = (size_t)(end - start);
		size_t index = 0;

		if(line < start)
		{
			for(; index < outLen; ++index)
			{
				/* Shift the valid region to the beginning of the string. */
				line[index] = start[index];
			}
		}

		line[outLen] = '\0';
	}
}

/**
 * @brief   Parse a line read from the ini file (internal use only).
 * @return  Type of the line that was parsed.
 *
 * @param[in]  line     Line read from the ini file.
 * @param[in]  len      Length of the input line.
 * @param[in]  section  Output string for the section.
 * @param[in]  key      Output string for the key.
 * @param[in]  value    Output string for the value.
 *
 * The return value will be one of the possible values in the LineStatus enum.
 * This value will determine which of the output strings was written to.
 */
static int prv_micro_ini_parse_line(
    const char* const line,
	const size_t len,
    char* const section,
    char* const key,
    char* const value
)
{
    int ret = LINE_UNPROCESSED;

	if(len == 0)
	{
		/* Empty line. */
		ret = LINE_EMPTY;
	}
    else if(line[0] == '#' || line[0] == ';')
	{
        /* Comment line. */
        ret = LINE_COMMENT;
    }
	else if(line[0] == '[' && line[len - 1] == ']')
	{
        /* Section name. */
        sscanf(line, "[%[^]]", section);
		prv_micro_ini_strstrip(section);

        ret = LINE_SECTION;
    }
	else if(sscanf(line, "%[^=] = \"%[^\"]\"", key, value) == 2 ||  sscanf(line, "%[^=] = '%[^\']'", key, value) == 2)
	{
        /* Usual key=value with quotes, with or without comments */
		prv_micro_ini_strstrip(key);
		prv_micro_ini_strstrip(value);

        /* Don't strip spaces from values surrounded with quotes */
        /*
         * sscanf cannot handle '' or "" as empty values
         * this is done here
         */
        if(!strcmp(value, "\"\"") || (!strcmp(value, "''")))
		{
            value[0] = 0;
        }

        ret = LINE_VALUE;
    }
	else if(sscanf(line, "%[^=] = %[^;#]", key, value) == 2)
	{
        /* Usual key=value without quotes, with or without comments */
		prv_micro_ini_strstrip(key);
		prv_micro_ini_strstrip(value);

        ret = LINE_VALUE;
    }
	else if(sscanf(line, "%[^=] = %[;#]", key, value) == 2 ||  sscanf(line, "%[^=] %[=]", key, value) == 2)
	{
        /*
         * Special cases:
         * key=
         * key=;
         * key=#
         */
		prv_micro_ini_strstrip(key);

        value[0] = 0;
        ret = LINE_VALUE;
    }
	else
	{
        /* Generate syntax error */
        ret = LINE_ERROR;
    }

    return ret;
}


int micro_ini_load(
	const char* const filePath,
	const int flags,
	const micro_ini_handler_fn handlerCallback,
	const micro_ini_error_fn errorCallback,
	void* const pUserData
)
{
	FILE* pFile = NULL;
	int err = MICRO_INI_SUCCESS;

	if(!handlerCallback)
	{
		/* Invalid handler callback. */
		return MICRO_INI_ERROR_INVALID_HANDLER_CALLBACK;
	}

	pFile = fopen(filePath, "r");
	if(!pFile)
	{
		/* Could not open file. */
		return MICRO_INI_ERROR_INVALID_FILE_OBJECT;
	}

	err = micro_ini_load_file(pFile, flags, handlerCallback, errorCallback, pUserData);
	fclose(pFile);

	return err;
}


int micro_ini_load_file(
	FILE* const pFile,
	const int flags,
	const micro_ini_handler_fn handlerCallback,
	const micro_ini_error_fn errorCallback,
	void* const pUserData
)
{
	if(!pFile)
	{
		/* Invalid file object. */
		return MICRO_INI_ERROR_INVALID_FILE_OBJECT;
	}
	else if(!handlerCallback)
	{
		/* Invalid handler callback. */
		return MICRO_INI_ERROR_INVALID_HANDLER_CALLBACK;
	}

	return micro_ini_load_stream(pFile, flags, handlerCallback, errorCallback, (micro_ini_reader_fn) fgets, (micro_ini_eof_fn) feof, pUserData);
}


int micro_ini_load_stream(
	void* const pStream,
	const int flags,
	const micro_ini_handler_fn handlerCallback,
	const micro_ini_error_fn errorCallback,
	const micro_ini_reader_fn readerCallback,
	const micro_ini_eof_fn eofCallback,
	void* const pUserData
)
{
	char line    [MICRO_INI_MAX_LINE_LENGTH + 1];
	char section [MICRO_INI_MAX_LINE_LENGTH + 1];
	char key     [MICRO_INI_MAX_LINE_LENGTH + 1];
	char val     [MICRO_INI_MAX_LINE_LENGTH + 1];

	char* start = NULL;

	int last      = 0;
	int len       = 0;
	int lineno    = 0;
	int firstLine = 1;
	int numErrors = 0;

	if(!pStream)
	{
		/* Invalid stream object. */
		return MICRO_INI_ERROR_INVALID_STREAM_OBJECT;
	}
	else if(!handlerCallback)
	{
		/* Invalid handler callback. */
		return MICRO_INI_ERROR_INVALID_HANDLER_CALLBACK;
	}
	else if(!readerCallback)
	{
		/* Invalid reader callback. */
		return MICRO_INI_ERROR_INVALID_READER_CALLBACK;
	}
	else if(!eofCallback)
	{
		/* Invalid eof callback. */
		return MICRO_INI_ERROR_INVALID_EOF_CALLBACK;
	}

	/* Clear the temporary data. */
	line[0] = '\0';
	section[0] = '\0';
	key[0] = '\0';
	val[0] = '\0';

	/* Read each line of the file. */
	while(readerCallback(line + last, MICRO_INI_MAX_LINE_LENGTH - last, pStream) != NULL)
	{
		++lineno;

		start = line;
		len = (int) strlen(line) - 1;

		if(firstLine && (flags & MICRO_INI_FLAG_BOM) &&
			(unsigned char) start[0] == 0xEF &&
			(unsigned char) start[1] == 0xBB &&
			(unsigned char) start[2] == 0xBF
		)
		{
			/* Move the line just past the byte order marker. */
			start += 3;
		}

		if(len <= 0)
		{
			/* Skip empty lines. */
			continue;
		}

		/* Safety check against buffer overflows. */
		if(line[len] != '\n' && eofCallback(pStream) == 0)
		{
			/* A buffer overflow occurs when the last character in the line is not '\n' and not at the end of the file stream. */
			return MICRO_INI_ERROR_BUFFER_OVERFLOW;
		}

		/* Get rid of any whitespace characters at end of line (including newline characters). */
		while((len >= 0) && isspace((unsigned char) line[len]))
		{
			line[len] = '\0';
			--len;
		}

		/* Detect multi-line. */
		if(line[len] == '\\' && (flags & MICRO_INI_FLAG_MULTILINE))
		{
			/* Multi-line value. */
			last = len;
			continue;
		}
		else
		{
			last = 0;
			firstLine = 0;
		}

		/* Remove whitespace at the beginning of the line.  This must be done after checking for multi-line values. */
		while((len >= 0) && isspace((unsigned char) *start))
		{
			++start;
			--len;
		}

		if(len < 0)
		{
			/* Line was composed entirely of whitespace characters. */
			len = 0;
		}
		else
		{
			/* Fix the length so the line can be parsed correctly. */
			++len;
		}

		/* Parse the line. */
		switch(prv_micro_ini_parse_line(start, len, section, key, val))
		{
			case LINE_VALUE:
				handlerCallback(pUserData, section, key, val);
				break;

			case LINE_ERROR:
				if(errorCallback)
				{
					/* Call the error function if it was provided. */
					errorCallback(pUserData, line, lineno);
				}

				/* Keep track of the number of errors that have occurred. */
				++numErrors;

				if(flags & MICRO_INI_FLAG_STOP_ON_FIRST_ERROR)
				{
					return numErrors;
				}
				else
				{
					break;
				}

			case LINE_EMPTY:
			case LINE_COMMENT:
			case LINE_SECTION:
				/* Intentional fall-through. */

			default:
				break;
		}

		line[0] = '\0';
		last = 0;
	}

	return MICRO_INI_SUCCESS + numErrors;
}
