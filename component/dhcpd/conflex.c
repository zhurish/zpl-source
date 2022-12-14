/*	$OpenBSD: conflex.c,v 1.16 2016/02/06 23:50:10 krw Exp $	*/

/* Lexical scanner for dhcpd config file... */

/*
 * Copyright (c) 1995, 1996, 1997 The Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>

#include <netinet/in.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dhcp.h"
#include "tree.h"
#include "dhcpd.h"
#include "dhctoken.h"

int lexline;
int lexchar;
char *token_line;
char *prev_line;
char *cur_line;
char *tlname;
int eol_token;

static char line1[81];
static char line2[81];
static int lpos;
static int line;
static int tlpos;
static int tline;
static int token;
static int ugflag;
static char *tval;
static char tokbuf[1500];

static int get_char(FILE *);
static int get_token(FILE *);
static void skip_to_eol(FILE *);
static int read_string(FILE *);
static int read_num_or_name(int, FILE *);
//static int intern(char *, int);

void
new_parse(char *name)
{
	tlname = name;
	lpos = line = 1;
	cur_line = line1;
	prev_line = line2;
	token_line = cur_line;
	cur_line[0] = prev_line[0] = 0;
	warnings_occurred = 0;
}

static int
get_char(FILE *cfile)
{
	int c = getc(cfile);
	if (!ugflag) {
		if (c == '\n') {
			if (cur_line == line1) {
				cur_line = line2;
				prev_line = line1;
			} else {
				cur_line = line1;
				prev_line = line2;
			}
			line++;
			lpos = 1;
			cur_line[0] = 0;
		} else if (c != EOF) {
			if (lpos < sizeof(line1)) {
				cur_line[lpos - 1] = c;
				cur_line[lpos] = 0;
			}
			lpos++;
		}
	} else
		ugflag = 0;
	return (c);
}

static int
get_token(FILE *cfile)
{
	int		c, ttok;
	static char	tb[2];
	int		l, p;

	do {
		l = line;
		p = lpos;

		c = get_char(cfile);

		if (!(c == '\n' && eol_token) && isascii(c) && isspace(c))
			continue;
		if (c == '#') {
			skip_to_eol(cfile);
			continue;
		}
		lexline = l;
		lexchar = p;
		if (c == '"') {
			ttok = read_string(cfile);
			break;
		} else if (c == '-' || (isascii(c) && isalnum(c))) {
			ttok = read_num_or_name(c, cfile);
			break;
		} else {
			tb[0] = c;
			tb[1] = 0;
			tval = tb;
			ttok = c;
			break;
		}
	} while (1);
	return (ttok);
}

int
next_token(char **rval, FILE *cfile)
{
	int	rv;

	if (token) {
		if (lexline != tline)
			token_line = cur_line;
		lexchar = tlpos;
		lexline = tline;
		rv = token;
		token = 0;
	} else {
		rv = get_token(cfile);
		token_line = cur_line;
	}
	if (rval)
		*rval = tval;

	return (rv);
}

int
peek_token(char **rval, FILE *cfile)
{
	int	x;

	if (!token) {
		tlpos = lexchar;
		tline = lexline;
		token = get_token(cfile);
		if (lexline != tline)
			token_line = prev_line;
		x = lexchar;
		lexchar = tlpos;
		tlpos = x;
		x = lexline;
		lexline = tline;
		tline = x;
	}
	if (rval)
		*rval = tval;

	return (token);
}

static void
skip_to_eol(FILE *cfile)
{
	int	c;

	do {
		c = get_char(cfile);
		if (c == EOF)
			return;
		if (c == '\n')
			return;
	} while (1);
}

static int
read_string(FILE *cfile)
{
	int i, c, bs;

	bs = i = 0;
	do {
		c = get_char(cfile);
		if (bs)
			bs = 0;
		else if (c == '\\')
			bs = 1;

		if (c != '"' && c != EOF && bs == 0)
			tokbuf[i++] = c;

	} while (i < (sizeof(tokbuf) - 1) && c != EOF && c != '"');

	if (c == EOF)
		parse_warn("eof in string constant");
	else if (c != '"')
		parse_warn("string constant larger than internal buffer");

	tokbuf[i] = 0;
	tval = tokbuf;

	return (TOK_STRING);
}

static int
read_num_or_name(int c, FILE *cfile)
{
	int i, rv, xdigits;

	xdigits = isxdigit(c) ? 1 : 0;

	tokbuf[0] = c;
	for (i = 1; i < sizeof(tokbuf); i++)
	{
		c = get_char(cfile);
		if (!isascii(c) || (c != '-' && c != '_' && !isalnum(c)))
		{
			ungetc(c, cfile);
			ugflag = 1;
			break;
		}
		if (isxdigit(c))
			xdigits++;
		tokbuf[i] = c;
	}
	if (i == sizeof(tokbuf)) {
		parse_warn("token larger than internal buffer");
		i--;
		c = tokbuf[i];
		if (isxdigit(c))
			xdigits--;
	}
	tokbuf[i] = 0;
	tval = tokbuf;

	c = (unsigned int)tokbuf[0];

	if (c == '-')
		rv = TOK_NUMBER;
	else
		rv = intern(tval, TOK_NUMBER_OR_NAME);

	if (rv == TOK_NUMBER_OR_NAME && xdigits != i)
		rv = TOK_NAME;

	return (rv);
}


