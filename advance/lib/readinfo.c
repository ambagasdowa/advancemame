/*
 * This file is part of the AdvanceMAME project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "readinfo.h"

#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

/* Start size of buffer */
#define INFO_BUF_MIN 64

/* Buffer used for storing last token */
static unsigned info_buf_mac = 0;
static unsigned info_buf_max = 0;
static char* info_buf_map = 0;

/* Position in the stream */
static unsigned info_pos = 0; /* Char */
static unsigned info_row = 0; /* Row */
static unsigned info_col = 0; /* Column */

/* Reset all */
void info_reset(void) {
	free(info_buf_map);

	info_buf_max = 0;
	info_buf_map = 0;
	info_pos = 0;
	info_row = 0;
	info_col = 0;
}

/* Get information of file position */
unsigned info_row_get(void) {
	return info_row;
}

unsigned info_col_get(void) {
	return info_col;
}

unsigned info_pos_get(void) {
	return info_pos;
}

/* Resize the buffer */
static void info_buf_resize(unsigned size) {
	if (!info_buf_max)
		info_buf_max = INFO_BUF_MIN;
	else
		info_buf_max *= 2;
	if (size > info_buf_max)
		info_buf_max = size;
	info_buf_map = realloc(info_buf_map, info_buf_max );
	assert( info_buf_map );
}

/* Add a char to the buffer end */
static inline void info_buf_add(char c) {
	if (info_buf_mac >= info_buf_max)
		info_buf_resize(info_buf_mac + 1);
	info_buf_map[info_buf_mac++] = c;
}

/* Reset the buffer */
static void info_buf_reset() {
	info_buf_mac = 0;
}

/* Return last token text */
const char* info_text_get(void) {
	/* ensure the buffer end with zero */
	if (info_buf_mac==0 || info_buf_map[info_buf_mac-1]!=0)
		info_buf_add(0);
	return info_buf_map;
}

/* Read a char from file */
static int info_getc(FILE* f) {
	int c = fgetc(f);
	switch (c) {
		case EOF:
			break;
		case '\n':
			info_col = 0;
			++info_row;
			++info_pos;
			break;
		default:
			++info_col;
			++info_pos;
			break;
	}
	return c;
}

/* Unget a char from file */
static void info_ungetc(int c, FILE* f) {
	--info_pos;
	--info_col;
	ungetc(c,f);
}

static enum info_t get_symbol(FILE* f,int c) {
	while (c!=EOF && !isspace(c) && c!='(' && c!=')' && c!='\"') {
		info_buf_add(c);
		c = info_getc(f);
	}
	/* no reason to unget space or EOF */
	if (c!=EOF && !isspace(c))
		info_ungetc(c,f);
	return info_symbol;
}

static unsigned hexdigit(char c) {
	if (isdigit(c))
		return c - '0';
	return toupper(c) - 'A' + 10;
}

static enum info_t get_string(FILE* f) {
	int c = info_getc(f);
	while (c!=EOF && c!='\"') {
		if (c=='\\') {
			c = info_getc(f);
			switch (c) {
				case 'a' : info_buf_add('\a'); break;
				case 'b' : info_buf_add('\b'); break;
				case 'f' : info_buf_add('\f'); break;
				case 'n' : info_buf_add('\n'); break;
				case 'r' : info_buf_add('\r'); break;
				case 't' : info_buf_add('\t'); break;
				case 'v' : info_buf_add('\v'); break;
				case '\\' : info_buf_add('\\'); break;
				case '?' : info_buf_add('\?'); break;
				case '\'' : info_buf_add('\''); break;
				case '\"' : info_buf_add('\"'); break;
				case 'x' : {
					int d0,d1;
					unsigned char cc;
					d0 = info_getc(f);
					if (!isxdigit(d0))
						return info_error;
					d1 = info_getc(f);
					if (!isxdigit(d1))
						return info_error;
					cc = hexdigit(d0) * 16 + hexdigit(d1);
					info_buf_add(cc);
				}
				break;
				default:
					return info_error;
			}
		} else {
			info_buf_add(c);
		}
		c = info_getc(f);
	}
	if (c!='\"')
		return info_error;
	return info_string;
}

/* Extract a token */
enum info_t info_token_get(FILE* f) {
	int c = info_getc(f);
	/* reset the buffer */
	info_buf_reset();
	/* skip space */
	while (c!=EOF && isspace(c)) {
		c = info_getc(f);
	}
	/* get token */
	switch (c) {
		case EOF:
			return info_eof;
		case '(':
			return info_open;
		case ')':
			return info_close;
		case '\"':
			return get_string(f);
		default:
			return get_symbol(f,c);
	}
}

/* Skip a value token
 * note:
 *   Skip recusively any info_open and info_close
 * return:
 *   info_error error
 *   otherwise last token skipped
 */
enum info_t info_skip_value(FILE* f) {
	/* read value token */
	enum info_t t = info_token_get(f);
	switch (t) {
		case info_open:
			t = info_token_get(f);
			if (t==info_error)
				return info_error;
			while (t!=info_close) {
				/* first read type as a symbol */
				if (t!=info_symbol)
					return info_error;
				/* second skip the value */
				t = info_skip_value(f);
				/* two value required */
				if (t==info_error)
					return info_error;
				/* read next token, a type or a info_close */
				t = info_token_get(f);
				if (t==info_error)
					return info_error;
			}
		break;
		case info_symbol:
		case info_string:
		break;
		default:
			return info_error;
	}
	return t;
}
