/*
   Command interpreter routine for virtual terminal [aka TeletYpe]
   Copyright (C) 1997, 98, 99 Kunihiro Ishiguro
   Copyright (C) 2013 by Open Source Routing.
   Copyright (C) 2013 by Internet Systems Consortium, Inc. ("ISC")

This file is part of GNU Zebra.
 
GNU Zebra is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2, or (at your
option) any later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */
#include "zebra.h"

#include "log.h"
#include "memory.h"
#include "thread.h"
#include "vector.h"
#include "version.h"
#include "workqueue.h"
#include "command.h"
#include "vty.h"
#include "vty_user.h"
#include "host.h"
/* Command vector which includes some level of command lists. Normally
   each daemon maintains each own cmdvec. */
vector cmdvec = NULL;

struct cmd_token token_cr;
char *command_cr = NULL;


enum filter_type
{
  FILTER_RELAXED,
  FILTER_STRICT
};

enum matcher_rv
{
  MATCHER_OK,
  MATCHER_COMPLETE,
  MATCHER_INCOMPLETE,
  MATCHER_NO_MATCH,
  MATCHER_AMBIGUOUS,
  MATCHER_EXCEED_ARGC_MAX
};

#define MATCHER_ERROR(matcher_rv) \
  (   (matcher_rv) == MATCHER_INCOMPLETE \
   || (matcher_rv) == MATCHER_NO_MATCH \
   || (matcher_rv) == MATCHER_AMBIGUOUS \
   || (matcher_rv) == MATCHER_EXCEED_ARGC_MAX \
  )

/* Host information structure. */



/* Default motd string. */
const char *default_motd =
"\r\n\
Hello, this is " OEM_PACKAGE_BASE " (version " OEM_VERSION ").\r\n\
" OEM_PACKAGE_COPYRIGHT "\r\n\
" "\r\n";



/* This is called from main when a daemon is invoked with -v or --version. */
void
print_version (const char *progname)
{
  printf ("%s version %s\n", progname, OEM_VERSION);
  printf ("%s\n", OEM_PACKAGE_COPYRIGHT);
//  printf ("configured with:\n\t%s\n", QUAGGA_CONFIG_ARGS);
}


/* Utility function to concatenate argv argument into a single string
   with inserting ' ' character between each argument.  */
char *
argv_concat (const char **argv, int argc, int shift)
{
  int i;
  size_t len;
  char *str;
  char *p;

  len = 0;
  for (i = shift; i < argc; i++)
    len += strlen(argv[i])+1;
  if (!len)
    return NULL;
  p = str = XMALLOC(MTYPE_TMP, len);
  for (i = shift; i < argc; i++)
    {
      size_t arglen;
      memcpy(p, argv[i], (arglen = strlen(argv[i])));
      p += arglen;
      *p++ = ' ';
    }
  *(p-1) = '\0';
  return str;
}

static unsigned int
cmd_hash_key (void *p)
{
  return (uintptr_t) p;
}

static int
cmd_hash_cmp (const void *a, const void *b)
{
  return a == b;
}

/* Install top node of command vector. */
void
install_node (struct cmd_node *node, 
	      int (*func) (struct vty *))
{
  vector_set_index (cmdvec, node->node, node);
  node->func = func;
  node->cmd_vector = vector_init (VECTOR_MIN_SIZE);
  node->cmd_hash = hash_create (cmd_hash_key, cmd_hash_cmp);
}

void
reinstall_node (enum node_type node,
	      int (*func) (struct vty *))
{
	struct cmd_node *pNode = vector_lookup (cmdvec, node);
	pNode->func = func;
}


/* Breaking up string into each command piece. I assume given
   character is separated by a space character. Return value is a
   vector which includes char ** data element. */
vector
cmd_make_strvec (const char *string)
{
  const char *cp, *start;
  char *token;
  int strlen;
  vector strvec;
  
  if (string == NULL)
    return NULL;
  
  cp = string;

  /* Skip white spaces. */
  while (isspace ((int) *cp) && *cp != '\0')
    cp++;

  /* Return if there is only white spaces */
  if (*cp == '\0')
    return NULL;

  if (*cp == '!' || *cp == '#')
    return NULL;

  /* Prepare return vector. */
  strvec = vector_init (VECTOR_MIN_SIZE);

  /* Copy each command piece and set into vector. */
  while (1) 
    {
      start = cp;
      while (!(isspace ((int) *cp) || *cp == '\r' || *cp == '\n') &&
	     *cp != '\0')
	cp++;
      strlen = cp - start;
      token = XMALLOC (MTYPE_STRVEC, strlen + 1);
      memcpy (token, start, strlen);
      *(token + strlen) = '\0';
      vector_set (strvec, token);

      while ((isspace ((int) *cp) || *cp == '\n' || *cp == '\r') &&
	     *cp != '\0')
	cp++;

      if (*cp == '\0')
	return strvec;
    }
}

/* Free allocated string vector. */
void
cmd_free_strvec (vector v)
{
  unsigned int i;
  char *cp;

  if (!v)
    return;

  for (i = 0; i < vector_active (v); i++)
    if ((cp = vector_slot (v, i)) != NULL)
      XFREE (MTYPE_STRVEC, cp);

  vector_free (v);
}

struct format_parser_state
{
  vector topvect; /* Top level vector */
  vector intvect; /* Intermediate level vector, used when there's
                   * a multiple in a keyword. */
  vector curvect; /* current vector where read tokens should be
                     appended. */

  const char *string; /* pointer to command string, not modified */
  const char *cp; /* pointer in command string, moved along while
                     parsing */
  const char *dp;  /* pointer in description string, moved along while
                     parsing */

  int in_keyword; /* flag to remember if we are in a keyword group */
  int in_multiple; /* flag to remember if we are in a multiple group */
  int just_read_word; /* flag to remember if the last thing we red was a
                       * real word and not some abstract token */
};

static void
format_parser_error(struct format_parser_state *state, const char *message)
{
  int offset = state->cp - state->string + 1;

  fprintf(stderr, "\nError parsing command: \"%s\"\n", state->string);
  fprintf(stderr, "                        %*c\n", offset, '^');
  fprintf(stderr, "%s at offset %d.\n", message, offset);
  fprintf(stderr, "This is a programming error. Check your DEFUNs etc.\n");
  exit(1);
}

static char *
format_parser_desc_str(struct format_parser_state *state)
{
  const char *cp, *start;
  char *token;
  int strlen;

  cp = state->dp;

  if (cp == NULL)
    return NULL;

  /* Skip white spaces. */
  while (isspace ((int) *cp) && *cp != '\0')
    cp++;

  /* Return if there is only white spaces */
  if (*cp == '\0')
    return NULL;

  start = cp;

  while (!(*cp == '\r' || *cp == '\n') && *cp != '\0')
    cp++;

  strlen = cp - start;
  token = XMALLOC (MTYPE_CMD_TOKENS, strlen + 1);
  memcpy (token, start, strlen);
  *(token + strlen) = '\0';

  state->dp = cp;

  return token;
}

static void
format_parser_begin_keyword(struct format_parser_state *state)
{
  struct cmd_token *token;
  vector keyword_vect;

  if (state->in_keyword
      || state->in_multiple)
    format_parser_error(state, "Unexpected '{'");

  state->cp++;
  state->in_keyword = 1;

  token = XCALLOC(MTYPE_CMD_TOKENS, sizeof(*token));
  token->type = TOKEN_KEYWORD;
  token->keyword = vector_init(VECTOR_MIN_SIZE);

  keyword_vect = vector_init(VECTOR_MIN_SIZE);
  vector_set(token->keyword, keyword_vect);

  vector_set(state->curvect, token);
  state->curvect = keyword_vect;
}

static void
format_parser_begin_multiple(struct format_parser_state *state)
{
  struct cmd_token *token;

  if (state->in_keyword == 1)
    format_parser_error(state, "Keyword starting with '('");

  if (state->in_multiple)
    format_parser_error(state, "Nested group");

  state->cp++;
  state->in_multiple = 1;
  state->just_read_word = 0;

  token = XCALLOC(MTYPE_CMD_TOKENS, sizeof(*token));
  token->type = TOKEN_MULTIPLE;
  token->multiple = vector_init(VECTOR_MIN_SIZE);

  vector_set(state->curvect, token);
  if (state->curvect != state->topvect)
    state->intvect = state->curvect;
  state->curvect = token->multiple;
}

static void
format_parser_end_keyword(struct format_parser_state *state)
{
  if (state->in_multiple
      || !state->in_keyword)
    format_parser_error(state, "Unexpected '}'");

  if (state->in_keyword == 1)
    format_parser_error(state, "Empty keyword group");

  state->cp++;
  state->in_keyword = 0;
  state->curvect = state->topvect;
}

static void
format_parser_end_multiple(struct format_parser_state *state)
{
  char *dummy;

  if (!state->in_multiple)
    format_parser_error(state, "Unexpected ')'");

  if (vector_active(state->curvect) == 0)
    format_parser_error(state, "Empty multiple section");

  if (!state->just_read_word)
    {
      /* There are constructions like
       * 'show ip ospf database ... (self-originate|)'
       * in use.
       * The old parser reads a description string for the
       * word '' between |) which will never match.
       * Simulate this behvaior by dropping the next desc
       * string in such a case. */

      dummy = format_parser_desc_str(state);
      XFREE(MTYPE_CMD_TOKENS, dummy);
    }

  state->cp++;
  state->in_multiple = 0;

  if (state->intvect)
    state->curvect = state->intvect;
  else
    state->curvect = state->topvect;
}

static void
format_parser_handle_pipe(struct format_parser_state *state)
{
  struct cmd_token *keyword_token;
  vector keyword_vect;

  if (state->in_multiple)
    {
      state->just_read_word = 0;
      state->cp++;
    }
  else if (state->in_keyword)
    {
      state->in_keyword = 1;
      state->cp++;

      keyword_token = vector_slot(state->topvect,
                                  vector_active(state->topvect) - 1);
      keyword_vect = vector_init(VECTOR_MIN_SIZE);
      vector_set(keyword_token->keyword, keyword_vect);
      state->curvect = keyword_vect;
    }
  else
    {
      format_parser_error(state, "Unexpected '|'");
    }
}

static void
format_parser_read_word(struct format_parser_state *state)
{
  const char *start;
  int len;
  char *cmd;
  struct cmd_token *token;

  start = state->cp;

  while (state->cp[0] != '\0'
         && !strchr("\r\n(){}|", state->cp[0])
         && !isspace((int)state->cp[0]))
    state->cp++;

  len = state->cp - start;
  cmd = XMALLOC(MTYPE_CMD_TOKENS, len + 1);
  memcpy(cmd, start, len);
  cmd[len] = '\0';

  token = XCALLOC(MTYPE_CMD_TOKENS, sizeof(*token));
  token->type = TOKEN_TERMINAL;
  if (strcmp (cmd, CMD_KEY_IPV4/*"A.B.C.D"*/) == 0)
    token->terminal = TERMINAL_IPV4;
  else if (strcmp (cmd, CMD_KEY_IPV4_PREFIX/*"A.B.C.D/M"*/) == 0)
    token->terminal = TERMINAL_IPV4_PREFIX;
  else if (strcmp (cmd, CMD_KEY_IPV6/*"X:X::X:X"*/) == 0)
    token->terminal = TERMINAL_IPV6;
  else if (strcmp (cmd, CMD_KEY_IPV6_PREFIX/*"X:X::X:X/M"*/) == 0)
    token->terminal = TERMINAL_IPV6_PREFIX;
  else if (cmd[0] == '[')
    token->terminal = TERMINAL_OPTION;
  else if (cmd[0] == '.')
    token->terminal = TERMINAL_VARARG;

  else if (strcmp (cmd, CMD_USP_STR) == 0)
    token->terminal = TERMINAL_IUSPV;
  else if (strcmp (cmd, CMD_MAC_STR) == 0)
    token->terminal = TERMINAL_MAC;

  else if (cmd[0] == '<')
    token->terminal = TERMINAL_RANGE;
  else if (cmd[0] >= 'A' && cmd[0] <= 'Z')
    token->terminal = TERMINAL_VARIABLE;
  else
    token->terminal = TERMINAL_LITERAL;

  token->cmd = cmd;
  token->desc = format_parser_desc_str(state);
  vector_set(state->curvect, token);

  if (state->in_keyword == 1)
    state->in_keyword = 2;

  state->just_read_word = 1;
}

/**
 * Parse a given command format string and build a tree of tokens from
 * it that is suitable to be used by the command subsystem.
 *
 * @param string Command format string.
 * @param descstr Description string.
 * @return A vector of struct cmd_token representing the given command,
 *         or NULL on error.
 */
static vector
cmd_parse_format(const char *string, const char *descstr)
{
  struct format_parser_state state;

  if (string == NULL)
    return NULL;

  memset(&state, 0, sizeof(state));
  state.topvect = state.curvect = vector_init(VECTOR_MIN_SIZE);
  state.cp = state.string = string;
  state.dp = descstr;

  while (1)
    {
      while (isspace((int)state.cp[0]) && state.cp[0] != '\0')
        state.cp++;

      switch (state.cp[0])
        {
        case '\0':
          if (state.in_keyword
              || state.in_multiple)
            format_parser_error(&state, "Unclosed group/keyword");
          return state.topvect;
        case '{':
          format_parser_begin_keyword(&state);
          break;
        case '(':
          format_parser_begin_multiple(&state);
          break;
        case '}':
          format_parser_end_keyword(&state);
          break;
        case ')':
          format_parser_end_multiple(&state);
          break;
        case '|':
          format_parser_handle_pipe(&state);
          break;
        default:
          format_parser_read_word(&state);
        }
    }
}

/* Return prompt character of specified node. */
const char *
cmd_prompt (enum node_type node)
{
  struct cmd_node *cnode;

  cnode = vector_slot (cmdvec, node);
  return cnode->prompt;
}

/* Install a command into a node. */
void
install_element (enum node_type ntype, struct cmd_element *cmd)
{
  struct cmd_node *cnode;
  
  /* cmd_init hasn't been called */
  if (!cmdvec)
    {
      fprintf (stderr, "%s called before cmd_init, breakage likely\n",
               __func__);
      return;
    }
  
  cnode = vector_slot (cmdvec, ntype);

  if (cnode == NULL) 
    {
      fprintf (stderr, "Command node %d doesn't exist, please check it\n",
	       ntype);
      exit (1);
    }
  
  if (hash_lookup (cnode->cmd_hash, cmd) != NULL)
    {
#ifdef DEV_BUILD
      fprintf (stderr, 
               "Multiple command installs to node %d of command:\n%s\n",
               ntype, cmd->string);
#endif
      return;
    }
  
  assert (hash_get (cnode->cmd_hash, cmd, hash_alloc_intern));
  
  vector_set (cnode->cmd_vector, cmd);
  if (cmd->tokens == NULL)
    cmd->tokens = cmd_parse_format(cmd->string, cmd->doc);

  if (ntype == VIEW_NODE)
    install_element (ENABLE_NODE, cmd);
}

static const unsigned char itoa64[] =
"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static void
to64(char *s, long v, int n)
{
  while (--n >= 0) 
    {
      *s++ = itoa64[v&0x3f];
      v >>= 6;
    }
}

char *
zencrypt (const char *passwd)
{
  char salt[6];
  struct timeval tv;
  char *crypt (const char *, const char *);

  gettimeofday(&tv,0);
  
  to64(&salt[0], random(), 3);
  to64(&salt[3], tv.tv_usec, 3);
  salt[5] = '\0';

  return crypt (passwd, salt);
}

/* Utility function for getting command vector. */
vector
cmd_node_vector (vector v, enum node_type ntype)
{
  struct cmd_node *cnode = vector_slot (v, ntype);
  return cnode->cmd_vector;
}

/* Completion match types. */
enum match_type 
{
  no_match,
  extend_match,
  ipv4_prefix_match,
  ipv4_match,
  ipv6_prefix_match,
  ipv6_match,
  range_match,
  iuspv_match,
  mac_match,
  vararg_match,
  partly_match,
  exact_match 
};

static enum match_type
cmd_ipv4_match (const char *str)
{
  const char *sp;
  int dots = 0, nums = 0;
  char buf[4];

  if (str == NULL)
    return partly_match;

  for (;;)
    {
      memset (buf, 0, sizeof (buf));
      sp = str;
      while (*str != '\0')
	{
	  if (*str == '.')
	    {
	      if (dots >= 3)
		return no_match;

	      if (*(str + 1) == '.')
		return no_match;

	      if (*(str + 1) == '\0')
		return partly_match;

	      dots++;
	      break;
	    }
	  if (!isdigit ((int) *str))
	    return no_match;

	  str++;
	}

      if (str - sp > 3)
	return no_match;

      strncpy (buf, sp, str - sp);
      if (atoi (buf) > 255)
	return no_match;

      nums++;

      if (*str == '\0')
	break;

      str++;
    }

  if (nums < 4)
    return partly_match;

  return exact_match;
}

static enum match_type
cmd_ipv4_prefix_match (const char *str)
{
  const char *sp;
  int dots = 0;
  char buf[4];

  if (str == NULL)
    return partly_match;

  for (;;)
    {
      memset (buf, 0, sizeof (buf));
      sp = str;
      while (*str != '\0' && *str != '/')
	{
	  if (*str == '.')
	    {
	      if (dots == 3)
		return no_match;

	      if (*(str + 1) == '.' || *(str + 1) == '/')
		return no_match;

	      if (*(str + 1) == '\0')
		return partly_match;

	      dots++;
	      break;
	    }

	  if (!isdigit ((int) *str))
	    return no_match;

	  str++;
	}

      if (str - sp > 3)
	return no_match;

      strncpy (buf, sp, str - sp);
      if (atoi (buf) > 255)
	return no_match;

      if (dots == 3)
	{
	  if (*str == '/')
	    {
	      if (*(str + 1) == '\0')
		return partly_match;

	      str++;
	      break;
	    }
	  else if (*str == '\0')
	    return partly_match;
	}

      if (*str == '\0')
	return partly_match;

      str++;
    }

  sp = str;
  while (*str != '\0')
    {
      if (!isdigit ((int) *str))
	return no_match;

      str++;
    }

  if (atoi (sp) > 32)
    return no_match;

  return exact_match;
}

#define IPV6_ADDR_STR		"0123456789abcdefABCDEF:.%"
#define IPV6_PREFIX_STR		"0123456789abcdefABCDEF:.%/"
#define STATE_START		1
#define STATE_COLON		2
#define STATE_DOUBLE		3
#define STATE_ADDR		4
#define STATE_DOT               5
#define STATE_SLASH		6
#define STATE_MASK		7

#ifdef HAVE_IPV6

static enum match_type
cmd_ipv6_match (const char *str)
{
  struct sockaddr_in6 sin6_dummy;
  int ret;

  if (str == NULL)
    return partly_match;

  if (strspn (str, IPV6_ADDR_STR) != strlen (str))
    return no_match;

  /* use inet_pton that has a better support,
   * for example inet_pton can support the automatic addresses:
   *  ::1.2.3.4
   */
  ret = inet_pton(AF_INET6, str, &sin6_dummy.sin6_addr);
   
  if (ret == 1)
    return exact_match;

  return no_match;
}

static enum match_type
cmd_ipv6_prefix_match (const char *str)
{
  int state = STATE_START;
  int colons = 0, nums = 0, double_colon = 0;
  int mask;
  const char *sp = NULL;
  char *endptr = NULL;

  if (str == NULL)
    return partly_match;

  if (strspn (str, IPV6_PREFIX_STR) != strlen (str))
    return no_match;

  while (*str != '\0' && state != STATE_MASK)
    {
      switch (state)
	{
	case STATE_START:
	  if (*str == ':')
	    {
	      if (*(str + 1) != ':' && *(str + 1) != '\0')
		return no_match;
	      colons--;
	      state = STATE_COLON;
	    }
	  else
	    {
	      sp = str;
	      state = STATE_ADDR;
	    }

	  continue;
	case STATE_COLON:
	  colons++;
	  if (*(str + 1) == '/')
	    return no_match;
	  else if (*(str + 1) == ':')
	    state = STATE_DOUBLE;
	  else
	    {
	      sp = str + 1;
	      state = STATE_ADDR;
	    }
	  break;
	case STATE_DOUBLE:
	  if (double_colon)
	    return no_match;

	  if (*(str + 1) == ':')
	    return no_match;
	  else
	    {
	      if (*(str + 1) != '\0' && *(str + 1) != '/')
		colons++;
	      sp = str + 1;

	      if (*(str + 1) == '/')
		state = STATE_SLASH;
	      else
		state = STATE_ADDR;
	    }

	  double_colon++;
	  nums += 1;
	  break;
	case STATE_ADDR:
	  if (*(str + 1) == ':' || *(str + 1) == '.'
	      || *(str + 1) == '\0' || *(str + 1) == '/')
	    {
	      if (str - sp > 3)
		return no_match;

	      for (; sp <= str; sp++)
		if (*sp == '/')
		  return no_match;

	      nums++;

	      if (*(str + 1) == ':')
		state = STATE_COLON;
	      else if (*(str + 1) == '.')
		{
		  if (colons || double_colon)
		    state = STATE_DOT;
		  else
		    return no_match;
		}
	      else if (*(str + 1) == '/')
		state = STATE_SLASH;
	    }
	  break;
	case STATE_DOT:
	  state = STATE_ADDR;
	  break;
	case STATE_SLASH:
	  if (*(str + 1) == '\0')
	    return partly_match;

	  state = STATE_MASK;
	  break;
	default:
	  break;
	}

      if (nums > 11)
	return no_match;

      if (colons > 7)
	return no_match;

      str++;
    }

  if (state < STATE_MASK)
    return partly_match;

  mask = strtol (str, &endptr, 10);
  if (*endptr != '\0')
    return no_match;

  if (mask < 0 || mask > 128)
    return no_match;
  
  return exact_match;
}

#endif /* HAVE_IPV6  */

#define DECIMAL_STRLEN_MAX 10

static int
cmd_range_match (const char *range, const char *str)
{
  char *p;
  char buf[DECIMAL_STRLEN_MAX + 1];
  char *endptr = NULL;
  unsigned long min, max, val;

  if (str == NULL)
    return 1;

  val = strtoul (str, &endptr, 10);
  if (*endptr != '\0')
    return 0;

  range++;
  p = strchr (range, '-');
  if (p == NULL)
    return 0;
  if (p - range > DECIMAL_STRLEN_MAX)
    return 0;
  strncpy (buf, range, p - range);
  buf[p - range] = '\0';
  min = strtoul (buf, &endptr, 10);
  if (*endptr != '\0')
    return 0;

  range = p + 1;
  p = strchr (range, '>');
  if (p == NULL)
    return 0;
  if (p - range > DECIMAL_STRLEN_MAX)
    return 0;
  strncpy (buf, range, p - range);
  buf[p - range] = '\0';
  max = strtoul (buf, &endptr, 10);
  if (*endptr != '\0')
    return 0;

  if (val < min || val > max)
    return 0;

  return 1;
}

static int
cmd_iuspv_match (const char *range, const char *str)
{
	//IF_USP_STR
	int count = 0;
	char *base = "0123456789/.";
	char *math = (char *)range;
//	fprintf(stdout,"%s:%s -> %s\r\n",__func__,range,str);
	if(range == NULL)
		return 0;
	if (str == NULL)
	    return 1;
	while(1)
	{
		math = strstr(math,"/");
		if(math)
		{
			math++;
			count++;
		}
		else
			break;
	}
	math = (char *)str;
	while(1)
	{
		math = strstr(math,"/");
		if(math)
		{
			math++;
			count--;
		}
		else
			break;
	}
	if(count == 0)
	{
		if(strspn(str,base) == strlen(str))
		{
//			fprintf(stdout,"%s:strspn(str,base)=%d strlen(str)=%d",__func__,strspn(str,base),strlen(str));
			return 1;
		}
	}
//	fprintf(stdout,"%s:count=%d strspn(str,base)=%d strlen(str)=%d",__func__,count,strspn(str,base),strlen(str));
	return 0;
}

static int
cmd_mac_match (const char *range, const char *str)
{
	//IF_MAC_STR
	int count = 0;
	char *base = "0123456789abcdefABCDEF-";
	char *math = (char *)range;
//	fprintf(stdout,"%s:%s -> %s\r\n",__func__,range,str);
	if(range == NULL)
		return 0;
	if (str == NULL)
	    return 1;
	while(1)
	{
		math = strstr(math,"-");
		if(math)
		{
			math++;
			count++;
		}
		else
			break;
	}
	math = (char *)str;
	while(1)
	{
		math = strstr(math,"-");
		if(math)
		{
			math++;
			count--;
		}
		else
			break;
	}
	if(count == 0)
	{
		if(strspn(str,base) == strlen(str))
		{
//			fprintf(stdout,"%s:strspn(str,base)=%d strlen(str)=%d",__func__,strspn(str,base),strlen(str));
			return 1;
		}
	}
//	fprintf(stdout,"%s:count=%d strspn(str,base)=%d strlen(str)=%d",__func__,count,strspn(str,base),strlen(str));
	return 0;
}

static enum match_type
cmd_word_match(struct cmd_token *token,
               enum filter_type filter,
               const char *word)
{
  const char *str;
  enum match_type match_type;

  str = token->cmd;

  if (filter == FILTER_RELAXED)
    if (!word || !strlen(word))
      return partly_match;

  if (!word)
    return no_match;

  switch (token->terminal)
    {
      case TERMINAL_VARARG:
        return vararg_match;

      case TERMINAL_RANGE:
        if (cmd_range_match(str, word))
          return range_match;
        break;

      case TERMINAL_IUSPV:
        if (cmd_iuspv_match(str, word))
          return iuspv_match;
        break;
      case TERMINAL_MAC:
        if (cmd_mac_match(str, word))
          return mac_match;
        break;
#ifdef HAVE_IPV6
      case TERMINAL_IPV6:
        match_type = cmd_ipv6_match(word);
        if ((filter == FILTER_RELAXED && match_type != no_match)
          || (filter == FILTER_STRICT && match_type == exact_match))
          return ipv6_match;
        break;

      case TERMINAL_IPV6_PREFIX:
        match_type = cmd_ipv6_prefix_match(word);
        if ((filter == FILTER_RELAXED && match_type != no_match)
            || (filter == FILTER_STRICT && match_type == exact_match))
          return ipv6_prefix_match;
        break;
#endif
      case TERMINAL_IPV4:
        match_type = cmd_ipv4_match(word);
        if ((filter == FILTER_RELAXED && match_type != no_match)
            || (filter == FILTER_STRICT && match_type == exact_match))
          return ipv4_match;
        break;

      case TERMINAL_IPV4_PREFIX:
        match_type = cmd_ipv4_prefix_match(word);
        if ((filter == FILTER_RELAXED && match_type != no_match)
            || (filter == FILTER_STRICT && match_type == exact_match))
          return ipv4_prefix_match;
        break;

      case TERMINAL_OPTION:
      case TERMINAL_VARIABLE:
        return extend_match;

      case TERMINAL_LITERAL:
        if (filter == FILTER_RELAXED && !strncmp(str, word, strlen(word)))
          {
            if (!strcmp(str, word))
              return exact_match;
            return partly_match;
          }
        if (filter == FILTER_STRICT && !strcmp(str, word))
          return exact_match;
        break;

      default:
        assert (0);
    }

  return no_match;
}

struct cmd_matcher
{
  struct cmd_element *cmd; /* The command element the matcher is using */
  enum filter_type filter; /* Whether to use strict or relaxed matching */
  vector vline; /* The tokenized commandline which is to be matched */
  unsigned int index; /* The index up to which matching should be done */

  /* If set, construct a list of matches at the position given by index */
  enum match_type *match_type;
  vector *match;

  unsigned int word_index; /* iterating over vline */
};

static int
push_argument(int *argc, const char **argv, const char *arg)
{
  if (!arg || !strlen(arg))
    arg = NULL;

  if (!argc || !argv)
    return 0;

  if (*argc >= CMD_ARGC_MAX)
    return -1;

  argv[(*argc)++] = arg;
  return 0;
}

static void
cmd_matcher_record_match(struct cmd_matcher *matcher,
                         enum match_type match_type,
                         struct cmd_token *token)
{
  if (matcher->word_index != matcher->index)
    return;

  if (matcher->match)
    {
      if (!*matcher->match)
        *matcher->match = vector_init(VECTOR_MIN_SIZE);
      vector_set(*matcher->match, token);
    }

  if (matcher->match_type)
    {
      if (match_type > *matcher->match_type)
        *matcher->match_type = match_type;
    }
}

static int
cmd_matcher_words_left(struct cmd_matcher *matcher)
{
  return matcher->word_index < vector_active(matcher->vline);
}

static const char*
cmd_matcher_get_word(struct cmd_matcher *matcher)
{
  assert(cmd_matcher_words_left(matcher));

  return vector_slot(matcher->vline, matcher->word_index);
}

static enum matcher_rv
cmd_matcher_match_terminal(struct cmd_matcher *matcher,
                           struct cmd_token *token,
                           int *argc, const char **argv)
{
  const char *word;
  enum match_type word_match;

  assert(token->type == TOKEN_TERMINAL);

  if (!cmd_matcher_words_left(matcher))
    {
      if (token->terminal == TERMINAL_OPTION)
        return MATCHER_OK; /* missing optional args are NOT pushed as NULL */
      else
        return MATCHER_INCOMPLETE;
    }

  word = cmd_matcher_get_word(matcher);
  word_match = cmd_word_match(token, matcher->filter, word);
  if (word_match == no_match)
    return MATCHER_NO_MATCH;

  /* We have to record the input word as argument if it matched
   * against a variable. */
  if (TERMINAL_RECORD (token->terminal))
    {
      if (push_argument(argc, argv, word))
        return MATCHER_EXCEED_ARGC_MAX;
    }

  cmd_matcher_record_match(matcher, word_match, token);

  matcher->word_index++;

  /* A vararg token should consume all left over words as arguments */
  if (token->terminal == TERMINAL_VARARG)
    while (cmd_matcher_words_left(matcher))
      {
        word = cmd_matcher_get_word(matcher);
        if (word && strlen(word))
          push_argument(argc, argv, word);
        matcher->word_index++;
      }

  return MATCHER_OK;
}

static enum matcher_rv
cmd_matcher_match_multiple(struct cmd_matcher *matcher,
                           struct cmd_token *token,
                           int *argc, const char **argv)
{
  enum match_type multiple_match;
  unsigned int multiple_index;
  const char *word;
  const char *arg = NULL;
  struct cmd_token *word_token;
  enum match_type word_match;

  assert(token->type == TOKEN_MULTIPLE);

  multiple_match = no_match;

  if (!cmd_matcher_words_left(matcher))
    return MATCHER_INCOMPLETE;

  word = cmd_matcher_get_word(matcher);
  for (multiple_index = 0;
       multiple_index < vector_active(token->multiple);
       multiple_index++)
    {
      word_token = vector_slot(token->multiple, multiple_index);

      word_match = cmd_word_match(word_token, matcher->filter, word);
      if (word_match == no_match)
        continue;

      cmd_matcher_record_match(matcher, word_match, word_token);

      if (word_match > multiple_match)
        {
          multiple_match = word_match;
          arg = word;
        }
      /* To mimic the behavior of the old command implementation, we
       * tolerate any ambiguities here :/ */
    }

  matcher->word_index++;

  if (multiple_match == no_match)
    return MATCHER_NO_MATCH;

  if (push_argument(argc, argv, arg))
    return MATCHER_EXCEED_ARGC_MAX;

  return MATCHER_OK;
}

static enum matcher_rv
cmd_matcher_read_keywords(struct cmd_matcher *matcher,
                          struct cmd_token *token,
                          vector args_vector)
{
  unsigned int i;
  unsigned long keyword_mask;
  unsigned int keyword_found;
  enum match_type keyword_match;
  enum match_type word_match;
  vector keyword_vector;
  struct cmd_token *word_token;
  const char *word;
  int keyword_argc;
  const char **keyword_argv;
  enum matcher_rv rv = MATCHER_NO_MATCH;

  keyword_mask = 0;
  while (1)
    {
      if (!cmd_matcher_words_left(matcher))
        return MATCHER_OK;

      word = cmd_matcher_get_word(matcher);

      keyword_found = -1;
      keyword_match = no_match;
      for (i = 0; i < vector_active(token->keyword); i++)
        {
          if (keyword_mask & (1 << i))
            continue;

          keyword_vector = vector_slot(token->keyword, i);
          word_token = vector_slot(keyword_vector, 0);

          word_match = cmd_word_match(word_token, matcher->filter, word);
          if (word_match == no_match)
            continue;

          cmd_matcher_record_match(matcher, word_match, word_token);

          if (word_match > keyword_match)
            {
              keyword_match = word_match;
              keyword_found = i;
            }
          else if (word_match == keyword_match)
            {
              if (matcher->word_index != matcher->index || args_vector)
                return MATCHER_AMBIGUOUS;
            }
        }

      if (keyword_found == (unsigned int)-1)
        return MATCHER_NO_MATCH;

      matcher->word_index++;

      if (matcher->word_index > matcher->index)
        return MATCHER_OK;

      keyword_mask |= (1 << keyword_found);

      if (args_vector)
        {
          keyword_argc = 0;
          keyword_argv = XMALLOC(MTYPE_TMP, (CMD_ARGC_MAX + 1) * sizeof(char*));
          /* We use -1 as a marker for unused fields as NULL might be a valid value */
          for (i = 0; i < CMD_ARGC_MAX + 1; i++)
            keyword_argv[i] = (void*)-1;
          vector_set_index(args_vector, keyword_found, keyword_argv);
        }
      else
        {
          keyword_argv = NULL;
        }

      keyword_vector = vector_slot(token->keyword, keyword_found);
      /* the keyword itself is at 0. We are only interested in the arguments,
       * so start counting at 1. */
      for (i = 1; i < vector_active(keyword_vector); i++)
        {
          word_token = vector_slot(keyword_vector, i);

          switch (word_token->type)
            {
            case TOKEN_TERMINAL:
              rv = cmd_matcher_match_terminal(matcher, word_token,
                                              &keyword_argc, keyword_argv);
              break;
            case TOKEN_MULTIPLE:
              rv = cmd_matcher_match_multiple(matcher, word_token,
                                              &keyword_argc, keyword_argv);
              break;
            case TOKEN_KEYWORD:
              assert(!"Keywords should never be nested.");
              break;
            default:
                assert(!"Keywords should never be nested.");
                break;
            }

          if (MATCHER_ERROR(rv))
            return rv;

          if (matcher->word_index > matcher->index)
            return MATCHER_OK;
        }
    }
  /* not reached */
}

static enum matcher_rv
cmd_matcher_build_keyword_args(struct cmd_matcher *matcher,
                               struct cmd_token *token,
                               int *argc, const char **argv,
                               vector keyword_args_vector)
{
  unsigned int i, j;
  const char **keyword_args;
  vector keyword_vector;
  struct cmd_token *word_token;
  const char *arg;
  enum matcher_rv rv;

  rv = MATCHER_OK;

  if (keyword_args_vector == NULL)
    return rv;

  for (i = 0; i < vector_active(token->keyword); i++)
    {
      keyword_vector = vector_slot(token->keyword, i);
      keyword_args = vector_lookup(keyword_args_vector, i);

      if (vector_active(keyword_vector) == 1)
        {
          /* this is a keyword without arguments */
          if (keyword_args)
            {
              word_token = vector_slot(keyword_vector, 0);
              arg = word_token->cmd;
            }
          else
            {
              arg = NULL;
            }

          if (push_argument(argc, argv, arg))
            rv = MATCHER_EXCEED_ARGC_MAX;
        }
      else
        {
          /* this is a keyword with arguments */
          if (keyword_args)
            {
              /* the keyword was present, so just fill in the arguments */
              for (j = 0; keyword_args[j] != (void*)-1; j++)
                if (push_argument(argc, argv, keyword_args[j]))
                  rv = MATCHER_EXCEED_ARGC_MAX;
              XFREE(MTYPE_TMP, keyword_args);
            }
          else
            {
              /* the keyword was not present, insert NULL for the arguments
               * the keyword would have taken. */
              for (j = 1; j < vector_active(keyword_vector); j++)
                {
                  word_token = vector_slot(keyword_vector, j);
                  if ((word_token->type == TOKEN_TERMINAL
                       && TERMINAL_RECORD (word_token->terminal))
                      || word_token->type == TOKEN_MULTIPLE)
                    {
                      if (push_argument(argc, argv, NULL))
                        rv = MATCHER_EXCEED_ARGC_MAX;
                    }
                }
            }
        }
    }
  vector_free(keyword_args_vector);
  return rv;
}

static enum matcher_rv
cmd_matcher_match_keyword(struct cmd_matcher *matcher,
                          struct cmd_token *token,
                          int *argc, const char **argv)
{
  vector keyword_args_vector;
  enum matcher_rv reader_rv;
  enum matcher_rv builder_rv;

  assert(token->type == TOKEN_KEYWORD);

  if (argc && argv)
    keyword_args_vector = vector_init(VECTOR_MIN_SIZE);
  else
    keyword_args_vector = NULL;

  reader_rv = cmd_matcher_read_keywords(matcher, token, keyword_args_vector);
  builder_rv = cmd_matcher_build_keyword_args(matcher, token, argc,
                                              argv, keyword_args_vector);
  /* keyword_args_vector is consumed by cmd_matcher_build_keyword_args */

  if (!MATCHER_ERROR(reader_rv) && MATCHER_ERROR(builder_rv))
    return builder_rv;

  return reader_rv;
}

static void
cmd_matcher_init(struct cmd_matcher *matcher,
                 struct cmd_element *cmd,
                 enum filter_type filter,
                 vector vline,
                 unsigned int index,
                 enum match_type *match_type,
                 vector *match)
{
  memset(matcher, 0, sizeof(*matcher));

  matcher->cmd = cmd;
  matcher->filter = filter;
  matcher->vline = vline;
  matcher->index = index;

  matcher->match_type = match_type;
  if (matcher->match_type)
    *matcher->match_type = no_match;
  matcher->match = match;

  matcher->word_index = 0;
}

static enum matcher_rv
cmd_element_match(struct cmd_element *cmd_element,
                  enum filter_type filter,
                  vector vline,
                  unsigned int index,
                  enum match_type *match_type,
                  vector *match,
                  int *argc,
                  const char **argv)
{
  struct cmd_matcher matcher;
  unsigned int token_index;
  enum matcher_rv rv = MATCHER_NO_MATCH;

  cmd_matcher_init(&matcher, cmd_element, filter,
                   vline, index, match_type, match);

  if (argc != NULL)
    *argc = 0;

  for (token_index = 0;
       token_index < vector_active(cmd_element->tokens);
       token_index++)
    {
      struct cmd_token *token = vector_slot(cmd_element->tokens, token_index);

      switch (token->type)
        {
        case TOKEN_TERMINAL:
          rv = cmd_matcher_match_terminal(&matcher, token, argc, argv);
          break;
        case TOKEN_MULTIPLE:
          rv = cmd_matcher_match_multiple(&matcher, token, argc, argv);
          break;
        case TOKEN_KEYWORD:
          rv = cmd_matcher_match_keyword(&matcher, token, argc, argv);
          break;
        default:
        	break;
        }

      if (MATCHER_ERROR(rv))
        return rv;

      if (matcher.word_index > index)
        return MATCHER_OK;
    }

  /* return MATCHER_COMPLETE if all words were consumed */
  if (matcher.word_index >= vector_active(vline))
    return MATCHER_COMPLETE;

  /* return MATCHER_COMPLETE also if only an empty word is left. */
  if (matcher.word_index == vector_active(vline) - 1
      && (!vector_slot(vline, matcher.word_index)
          || !strlen((char*)vector_slot(vline, matcher.word_index))))
    return MATCHER_COMPLETE;

  return MATCHER_NO_MATCH; /* command is too long to match */
}

/**
 * Filter a given vector of commands against a given commandline and
 * calculate possible completions.
 *
 * @param commands A vector of struct cmd_element*. Commands that don't
 *                 match against the given command line will be overwritten
 *                 with NULL in that vector.
 * @param filter Either FILTER_RELAXED or FILTER_STRICT. This basically
 *               determines how incomplete commands are handled, compare with
 *               cmd_word_match for details.
 * @param vline A vector of char* containing the tokenized commandline.
 * @param index Only match up to the given token of the commandline.
 * @param match_type Record the type of the best match here.
 * @param matches Record the matches here. For each cmd_element in the commands
 *                vector, a match vector will be created in the matches vector.
 *                That vector will contain all struct command_token* of the
 *                cmd_element which matched against the given vline at the given
 *                index.
 * @return A code specifying if an error occured. If all went right, it's
 *         CMD_SUCCESS.
 */
static int
cmd_vector_filter(vector commands,
                  enum filter_type filter,
                  vector vline,
                  unsigned int index,
                  enum match_type *match_type,
                  vector *matches)
{
  unsigned int i;
  struct cmd_element *cmd_element;
  enum match_type best_match;
  enum match_type element_match;
  enum matcher_rv matcher_rv;

  best_match = no_match;
  *matches = vector_init(VECTOR_MIN_SIZE);

  for (i = 0; i < vector_active (commands); i++)
    if ((cmd_element = vector_slot (commands, i)) != NULL)
      {
#ifdef HAVE_ROUTE_OPTIMIZE
        if(cmd_element->attr == CMD_ATTR_HIDDEN)
      	  ;//continue;
#endif//HAVE_ROUTE_OPTIMIZE
        vector_set_index(*matches, i, NULL);
        matcher_rv = cmd_element_match(cmd_element, filter,
                                       vline, index,
                                       &element_match,
                                       (vector*)&vector_slot(*matches, i),
                                       NULL, NULL);
        if (MATCHER_ERROR(matcher_rv))
          {
            vector_slot(commands, i) = NULL;
            if (matcher_rv == MATCHER_AMBIGUOUS)
              return CMD_ERR_AMBIGUOUS;
            if (matcher_rv == MATCHER_EXCEED_ARGC_MAX)
              return CMD_ERR_EXEED_ARGC_MAX;
          }
        else if (element_match > best_match)
          {
            best_match = element_match;
          }
      }
  *match_type = best_match;
  return CMD_SUCCESS;
}

/**
 * Check whether a given commandline is complete if used for a specific
 * cmd_element.
 *
 * @param cmd_element A cmd_element against which the commandline should be
 *                    checked.
 * @param vline The tokenized commandline.
 * @return 1 if the given commandline is complete, 0 otherwise.
 */
static int
cmd_is_complete(struct cmd_element *cmd_element,
                vector vline)
{
  enum matcher_rv rv;

  rv = cmd_element_match(cmd_element,
                         FILTER_RELAXED,
                         vline, -1,
                         NULL, NULL,
                         NULL, NULL);
  return (rv == MATCHER_COMPLETE);
}

/**
 * Parse a given commandline and construct a list of arguments for the
 * given command_element.
 *
 * @param cmd_element The cmd_element for which we want to construct arguments.
 * @param vline The tokenized commandline.
 * @param argc Where to store the argument count.
 * @param argv Where to store the argument list. Should be at least
 *             CMD_ARGC_MAX elements long.
 * @return CMD_SUCCESS if everything went alright, an error otherwise.
 */
static int
cmd_parse(struct cmd_element *cmd_element,
          vector vline,
          int *argc, const char **argv)
{
  enum matcher_rv rv = cmd_element_match(cmd_element,
                                         FILTER_RELAXED,
                                         vline, -1,
                                         NULL, NULL,
                                         argc, argv);
  switch (rv)
    {
    case MATCHER_COMPLETE:
      return CMD_SUCCESS;

    case MATCHER_NO_MATCH:
      return CMD_ERR_NO_MATCH;

    case MATCHER_AMBIGUOUS:
      return CMD_ERR_AMBIGUOUS;

    case MATCHER_EXCEED_ARGC_MAX:
      return CMD_ERR_EXEED_ARGC_MAX;

    default:
      return CMD_ERR_INCOMPLETE;
    }
}

/* Check ambiguous match */
static int
is_cmd_ambiguous (vector cmd_vector,
                  const char *command,
                  vector matches,
                  enum match_type type)
{
  unsigned int i;
  unsigned int j;
  const char *str = NULL;
  const char *matched = NULL;
  vector match_vector;
  struct cmd_token *cmd_token;

  if (command == NULL)
    command = "";

  for (i = 0; i < vector_active (matches); i++)
    if ((match_vector = vector_slot (matches, i)) != NULL)
      {
	int match = 0;

	for (j = 0; j < vector_active (match_vector); j++)
	  if ((cmd_token = vector_slot (match_vector, j)) != NULL)
	    {
	      enum match_type ret;

	      assert(cmd_token->type == TOKEN_TERMINAL);
	      if (cmd_token->type != TOKEN_TERMINAL)
		continue;

	      str = cmd_token->cmd;

	      switch (type)
		{
		case exact_match:
		  if (!TERMINAL_RECORD (cmd_token->terminal)
		      && strcmp (command, str) == 0)
		    match++;
		  break;
		case partly_match:
		  if (!TERMINAL_RECORD (cmd_token->terminal)
		      && strncmp (command, str, strlen (command)) == 0)
		    {
		      if (matched && strcmp (matched, str) != 0)
			return 1;	/* There is ambiguous match. */
		      else
			matched = str;
		      match++;
		    }
		  break;
		case range_match:
		  if (cmd_range_match (str, command))
		    {
		      if (matched && strcmp (matched, str) != 0)
			return 1;
		      else
			matched = str;
		      match++;
		    }
		  break;

	      case iuspv_match:
	  		  if (cmd_iuspv_match (str, command))
	  		    {
	  		      if (matched && strcmp (matched, str) != 0)
	  			return 1;
	  		      else
	  			matched = str;
	  		      match++;
	  		    }
	  		  break;
	      case mac_match:
	  		  if (cmd_mac_match (str, command))
	  		    {
	  		      if (matched && strcmp (matched, str) != 0)
	  			return 1;
	  		      else
	  			matched = str;
	  		      match++;
	  		    }
	  		  break;

#ifdef HAVE_IPV6
		case ipv6_match:
		  if (cmd_token->terminal == TERMINAL_IPV6)
		    match++;
		  break;
		case ipv6_prefix_match:
		  if ((ret = cmd_ipv6_prefix_match (command)) != no_match)
		    {
		      if (ret == partly_match)
			return 2;	/* There is incomplete match. */

		      match++;
		    }
		  break;
#endif /* HAVE_IPV6 */
		case ipv4_match:
		  if (cmd_token->terminal == TERMINAL_IPV4)
		    match++;
		  break;
		case ipv4_prefix_match:
		  if ((ret = cmd_ipv4_prefix_match (command)) != no_match)
		    {
		      if (ret == partly_match)
			return 2;	/* There is incomplete match. */

		      match++;
		    }
		  break;
		case extend_match:
		  if (TERMINAL_RECORD (cmd_token->terminal))
		    match++;
		  break;
		case no_match:
		default:
		  break;
		}
	    }
	if (!match)
	  vector_slot (cmd_vector, i) = NULL;
      }
  return 0;
}

/* If src matches dst return dst string, otherwise return NULL */
static const char *
cmd_entry_function (const char *src, struct cmd_token *token)
{
  const char *dst = token->cmd;

  /* Skip variable arguments. */
  if (TERMINAL_RECORD (token->terminal))
    return NULL;

  /* In case of 'command \t', given src is NULL string. */
  if (src == NULL)
    return dst;

  /* Matched with input string. */
  if (strncmp (src, dst, strlen (src)) == 0)
    return dst;

  return NULL;
}

/* If src matches dst return dst string, otherwise return NULL */
/* This version will return the dst string always if it is
   CMD_VARIABLE for '?' key processing */
static const char *
cmd_entry_function_desc (const char *src, struct cmd_token *token)
{
  const char *dst = token->cmd;

  switch (token->terminal)
    {
      case TERMINAL_VARARG:
        return dst;

      case TERMINAL_RANGE:
        if (cmd_range_match (dst, src))
          return dst;
        else
          return NULL;

      case TERMINAL_IUSPV:
          if (cmd_iuspv_match (dst, src))
            return dst;
          else
            return NULL;

      case TERMINAL_MAC:
          if (cmd_mac_match (dst, src))
            return dst;
          else
            return NULL;
#ifdef HAVE_IPV6
      case TERMINAL_IPV6:
        if (cmd_ipv6_match (src))
          return dst;
        else
          return NULL;

      case TERMINAL_IPV6_PREFIX:
        if (cmd_ipv6_prefix_match (src))
          return dst;
        else
          return NULL;
#endif
      case TERMINAL_IPV4:
        if (cmd_ipv4_match (src))
          return dst;
        else
          return NULL;

      case TERMINAL_IPV4_PREFIX:
        if (cmd_ipv4_prefix_match (src))
          return dst;
        else
          return NULL;

      /* Optional or variable commands always match on '?' */
      case TERMINAL_OPTION:
      case TERMINAL_VARIABLE:
        return dst;

      case TERMINAL_LITERAL:
        /* In case of 'command \t', given src is NULL string. */
        if (src == NULL)
          return dst;

        if (strncmp (src, dst, strlen (src)) == 0)
          return dst;
        else
          return NULL;

      default:
        assert(0);
        return NULL;
    }
}

/**
 * Check whether a string is already present in a vector of strings.
 * @param v A vector of char*.
 * @param str A char*.
 * @return 0 if str is already present in the vector, 1 otherwise.
 */
static int
cmd_unique_string (vector v, const char *str)
{
  unsigned int i;
  char *match;

  for (i = 0; i < vector_active (v); i++)
    if ((match = vector_slot (v, i)) != NULL)
      if (strcmp (match, str) == 0)
	return 0;
  return 1;
}

/**
 * Check whether a struct cmd_token matching a given string is already
 * present in a vector of struct cmd_token.
 * @param v A vector of struct cmd_token*.
 * @param str A char* which should be searched for.
 * @return 0 if there is a struct cmd_token* with its cmd matching str,
 *         1 otherwise.
 */
static int
desc_unique_string (vector v, const char *str)
{
  unsigned int i;
  struct cmd_token *token;

  for (i = 0; i < vector_active (v); i++)
    if ((token = vector_slot (v, i)) != NULL)
      if (strcmp (token->cmd, str) == 0)
	return 0;
  return 1;
}

static int 
cmd_try_do_shortcut (enum node_type node, char* first_word) {
  if ( first_word != NULL &&
       node != AUTH_NODE &&
       node != VIEW_NODE &&
       node != AUTH_ENABLE_NODE &&
       node != ENABLE_NODE &&
       /*node != RESTRICTED_NODE &&*/
       0 == strcmp( "do", first_word ) )
    return 1;
  return 0;
}

static void
cmd_matches_free(vector *matches)
{
  unsigned int i;
  vector cmd_matches;

  for (i = 0; i < vector_active(*matches); i++)
    if ((cmd_matches = vector_slot(*matches, i)) != NULL)
      vector_free(cmd_matches);
  vector_free(*matches);
  *matches = NULL;
}

static int
cmd_describe_cmp(const void *a, const void *b)
{
  const struct cmd_token *first = *(struct cmd_token * const *)a;
  const struct cmd_token *second = *(struct cmd_token * const *)b;

  return strcmp(first->cmd, second->cmd);
}

static void
cmd_describe_sort(vector matchvec)
{
  qsort(matchvec->index, vector_active(matchvec),
        sizeof(void*), cmd_describe_cmp);
}

/* '?' describe command support. */
static vector
cmd_describe_command_real (vector vline, struct vty *vty, int *status)
{
  unsigned int i;
  vector cmd_vector;
#define INIT_MATCHVEC_SIZE 10
  vector matchvec;
  struct cmd_element *cmd_element;
  unsigned int index;
  int ret;
  enum match_type match;
  char *command;
  vector matches = NULL;
  vector match_vector;
  uint32_t command_found = 0;
  const char *last_word;

  /* Set index. */
  if (vector_active (vline) == 0)
    {
      *status = CMD_ERR_NO_MATCH;
      return NULL;
    }

  index = vector_active (vline) - 1;

  /* Make copy vector of current node's command vector. */
  cmd_vector = vector_copy (cmd_node_vector (cmdvec, vty->node));

  /* Prepare match vector */
  matchvec = vector_init (INIT_MATCHVEC_SIZE);

  /* Filter commands and build a list how they could possibly continue. */
  for (i = 0; i <= index; i++)
    {
      command = vector_slot (vline, i);

      if (matches)
	cmd_matches_free(&matches);

      ret = cmd_vector_filter(cmd_vector,
	                      FILTER_RELAXED,
	                      vline, i,
	                      &match,
	                      &matches);

      if (ret != CMD_SUCCESS)
	{
	  vector_free (cmd_vector);
	  vector_free (matchvec);
	  cmd_matches_free(&matches);
	  *status = ret;
	  return NULL;
	}

      /* The last match may well be ambigious, so break here */
      if (i == index)
	break;

      if (match == vararg_match)
	{
	  /* We found a vararg match - so we can throw out the current matches here
	   * and don't need to continue checking the command input */
	  unsigned int j, k;

	  for (j = 0; j < vector_active (matches); j++)
	    if ((match_vector = vector_slot (matches, j)) != NULL)
	      for (k = 0; k < vector_active (match_vector); k++)
	        {
	          struct cmd_token *token = vector_slot (match_vector, k);
	          vector_set (matchvec, token);
	        }

	  *status = CMD_SUCCESS;
	  vector_set(matchvec, &token_cr);
	  vector_free (cmd_vector);
	  cmd_matches_free(&matches);
	  cmd_describe_sort(matchvec);
	  return matchvec;
	}

      ret = is_cmd_ambiguous(cmd_vector, command, matches, match);
      if (ret == 1)
	{
	  vector_free (cmd_vector);
	  vector_free (matchvec);
	  cmd_matches_free(&matches);
	  *status = CMD_ERR_AMBIGUOUS;
	  return NULL;
	}
      else if (ret == 2)
	{
	  vector_free (cmd_vector);
	  vector_free (matchvec);
	  cmd_matches_free(&matches);
	  *status = CMD_ERR_NO_MATCH;
	  return NULL;
	}
    }

  /* Make description vector. */
  for (i = 0; i < vector_active (matches); i++)
    {
      if ((cmd_element = vector_slot (cmd_vector, i)) != NULL)
	{
	  unsigned int j;
	  vector vline_trimmed;

#ifdef HAVE_ROUTE_OPTIMIZE
        /*闅愯棌绫诲瀷鐨勫懡浠ゅ湪 ? 鍛戒护鐨勬椂鍊欎笉鏄剧ず*/
	  //fprintf(stdout,"cmd:%s cmd type = %d\r\n",cmd_element->string,cmd_element->attr);
        if(cmd_element->attr == CMD_ATTR_HIDDEN)
      	  continue;
#endif//HAVE_ROUTE_OPTIMIZE
	  command_found++;
	  last_word = vector_slot(vline, vector_active(vline) - 1);
	  if (last_word == NULL || !strlen(last_word))
	    {
	      vline_trimmed = vector_copy(vline);
	      vector_unset(vline_trimmed, vector_active(vline_trimmed) - 1);

	      if (cmd_is_complete(cmd_element, vline_trimmed)
		  && desc_unique_string(matchvec, command_cr))
		{
		  if (match != vararg_match)
		    vector_set(matchvec, &token_cr);
		}

	      vector_free(vline_trimmed);
	    }

	  match_vector = vector_slot (matches, i);
	  if (match_vector)
	    {
	      for (j = 0; j < vector_active(match_vector); j++)
		{
		  struct cmd_token *token = vector_slot(match_vector, j);
		  const char *string;

		  string = cmd_entry_function_desc(command, token);
		  if (string && desc_unique_string(matchvec, string))
		    vector_set(matchvec, token);
		}
	    }
	}
    }

  /*
   * We can get into this situation when the command is complete
   * but the last part of the command is an optional piece of
   * the cli.
   */
  last_word = vector_slot(vline, vector_active(vline) - 1);
  if (command_found == 0 && (last_word == NULL || !strlen(last_word)))
    vector_set(matchvec, &token_cr);

  vector_free (cmd_vector);
  cmd_matches_free(&matches);

  if (vector_slot (matchvec, 0) == NULL)
    {
      vector_free (matchvec);
      *status = CMD_ERR_NO_MATCH;
      return NULL;
    }

  *status = CMD_SUCCESS;
  cmd_describe_sort(matchvec);
  return matchvec;
}

vector
cmd_describe_command (vector vline, struct vty *vty, int *status)
{
  vector ret;

  if ( cmd_try_do_shortcut(vty->node, vector_slot(vline, 0) ) )
    {
      enum node_type onode;
      vector shifted_vline;
      unsigned int index;

      onode = vty->node;
      vty->node = ENABLE_NODE;
      /* We can try it on enable node, cos' the vty is authenticated */

      shifted_vline = vector_init (vector_count(vline));
      /* use memcpy? */
      for (index = 1; index < vector_active (vline); index++) 
	{
	  vector_set_index (shifted_vline, index-1, vector_lookup(vline, index));
	}

      ret = cmd_describe_command_real (shifted_vline, vty, status);

      vector_free(shifted_vline);
      vty->node = onode;
      return ret;
  }


  return cmd_describe_command_real (vline, vty, status);
}


/* Check LCD of matched command. */
static int
cmd_lcd (char **matched)
{
  int i;
  int j;
  int lcd = -1;
  char *s1, *s2;
  char c1, c2;

  if (matched[0] == NULL || matched[1] == NULL)
    return 0;

  for (i = 1; matched[i] != NULL; i++)
    {
      s1 = matched[i - 1];
      s2 = matched[i];

      for (j = 0; (c1 = s1[j]) && (c2 = s2[j]); j++)
	if (c1 != c2)
	  break;

      if (lcd < 0)
	lcd = j;
      else
	{
	  if (lcd > j)
	    lcd = j;
	}
    }
  return lcd;
}

static int
cmd_complete_cmp(const void *a, const void *b)
{
  const char *first = *(char * const *)a;
  const char *second = *(char * const *)b;

  if (!first)
    {
      if (!second)
        return 0;
      return 1;
    }
  if (!second)
    return -1;

  return strcmp(first, second);
}

static void
cmd_complete_sort(vector matchvec)
{
  qsort(matchvec->index, vector_active(matchvec),
        sizeof(void*), cmd_complete_cmp);
}

/* Command line completion support. */
static char **
cmd_complete_command_real (vector vline, struct vty *vty, int *status, int islib)
{
  unsigned int i;
  vector cmd_vector = vector_copy (cmd_node_vector (cmdvec, vty->node));
#define INIT_MATCHVEC_SIZE 10
  vector matchvec;
  unsigned int index;
  char **match_str;
  struct cmd_token *token;
  char *command;
  int lcd;
  vector matches = NULL;
  vector match_vector;

  if (vector_active (vline) == 0)
    {
      vector_free (cmd_vector);
      *status = CMD_ERR_NO_MATCH;
      return NULL;
    }
  else
    index = vector_active (vline) - 1;

  /* First, filter by command string */
  for (i = 0; i <= index; i++)
    {
      command = vector_slot (vline, i);
      enum match_type match;
      int ret;

      if (matches)
        cmd_matches_free(&matches);

      /* First try completion match, if there is exactly match return 1 */
      ret = cmd_vector_filter(cmd_vector,
	                      FILTER_RELAXED,
	                      vline, i,
	                      &match,
	                      &matches);

      if (ret != CMD_SUCCESS)
	{
	  vector_free(cmd_vector);
	  cmd_matches_free(&matches);
	  *status = ret;
	  return NULL;
	}

      /* Break here - the completion mustn't be checked to be non-ambiguous */
      if (i == index)
	break;

      /* If there is exact match then filter ambiguous match else check
	 ambiguousness. */
      ret = is_cmd_ambiguous (cmd_vector, command, matches, match);
      if (ret == 1)
	{
	  vector_free (cmd_vector);
	  cmd_matches_free(&matches);
	  *status = CMD_ERR_AMBIGUOUS;
	  return NULL;
	}
    }
  
  /* Prepare match vector. */
  matchvec = vector_init (INIT_MATCHVEC_SIZE);

  /* Build the possible list of continuations into a list of completions */
  for (i = 0; i < vector_active (matches); i++)
    if ((match_vector = vector_slot (matches, i)))
      {
	const char *string;
	unsigned int j;

	for (j = 0; j < vector_active (match_vector); j++)
	  if ((token = vector_slot (match_vector, j)))
            {
              string = cmd_entry_function (vector_slot (vline, index), token);
              if (string && cmd_unique_string (matchvec, string))
                vector_set (matchvec, (islib != 0 ?
                                      XSTRDUP (MTYPE_TMP, string) :
                                      strdup (string) /* rl freed */));
            }
      }

  /* We don't need cmd_vector any more. */
  vector_free (cmd_vector);
  cmd_matches_free(&matches);

  /* No matched command */
  if (vector_slot (matchvec, 0) == NULL)
    {
      vector_free (matchvec);

      /* In case of 'command \t' pattern.  Do you need '?' command at
         the end of the line. */
      if (vector_slot (vline, index) == '\0')
	*status = CMD_ERR_NOTHING_TODO;
      else
	*status = CMD_ERR_NO_MATCH;
      return NULL;
    }

  /* Only one matched */
  if (vector_slot (matchvec, 1) == NULL)
    {
      match_str = (char **) matchvec->index;
      vector_only_wrapper_free (matchvec);
      *status = CMD_COMPLETE_FULL_MATCH;
      return match_str;
    }
  /* Make it sure last element is NULL. */
  vector_set (matchvec, NULL);

  /* Check LCD of matched strings. */
  if (vector_slot (vline, index) != NULL)
    {
      lcd = cmd_lcd ((char **) matchvec->index);

      if (lcd)
	{
	  int len = strlen (vector_slot (vline, index));

	  if (len < lcd)
	    {
	      char *lcdstr;

	      lcdstr = (islib != 0 ?
                        XMALLOC (MTYPE_TMP, lcd + 1) :
                        malloc(lcd + 1));
	      memcpy (lcdstr, matchvec->index[0], lcd);
	      lcdstr[lcd] = '\0';

	      /* Free matchvec. */
	      for (i = 0; i < vector_active (matchvec); i++)
                {
                  if (vector_slot (matchvec, i))
                    {
                      if (islib != 0)
                        XFREE (MTYPE_TMP, vector_slot (matchvec, i));
                      else
                        free (vector_slot (matchvec, i));
                    }
                }
	      vector_free (matchvec);

	      /* Make new matchvec. */
	      matchvec = vector_init (INIT_MATCHVEC_SIZE);
	      vector_set (matchvec, lcdstr);
	      match_str = (char **) matchvec->index;
	      vector_only_wrapper_free (matchvec);

	      *status = CMD_COMPLETE_MATCH;
	      return match_str;
	    }
	}
    }

  match_str = (char **) matchvec->index;
  cmd_complete_sort(matchvec);
  vector_only_wrapper_free (matchvec);
  *status = CMD_COMPLETE_LIST_MATCH;
  return match_str;
}

char **
cmd_complete_command_lib (vector vline, struct vty *vty, int *status, int islib)
{
  char **ret;

  if ( cmd_try_do_shortcut(vty->node, vector_slot(vline, 0) ) )
    {
      enum node_type onode;
      vector shifted_vline;
      unsigned int index;

      onode = vty->node;
      vty->node = ENABLE_NODE;
      /* We can try it on enable node, cos' the vty is authenticated */

      shifted_vline = vector_init (vector_count(vline));
      /* use memcpy? */
      for (index = 1; index < vector_active (vline); index++) 
	{
	  vector_set_index (shifted_vline, index-1, vector_lookup(vline, index));
	}

      ret = cmd_complete_command_real (shifted_vline, vty, status, islib);

      vector_free(shifted_vline);
      vty->node = onode;
      return ret;
  }

  return cmd_complete_command_real (vline, vty, status, islib);
}

char **
cmd_complete_command (vector vline, struct vty *vty, int *status)
{
  return cmd_complete_command_lib (vline, vty, status, 0);
}

/* return parent node */
/* MUST eventually converge on CONFIG_NODE */
enum node_type
node_parent ( enum node_type node )
{
  enum node_type ret;

  assert (node > CONFIG_NODE);

  switch (node)
    {
    case BGP_VPNV4_NODE:
    case BGP_VPNV6_NODE:
    case BGP_ENCAP_NODE:
    case BGP_ENCAPV6_NODE:
    case BGP_IPV4_NODE:
    case BGP_IPV4M_NODE:
    case BGP_IPV6_NODE:
    case BGP_IPV6M_NODE:
      ret = BGP_NODE;
      break;
    case KEYCHAIN_KEY_NODE:
      ret = KEYCHAIN_NODE;
      break;
    case LINK_PARAMS_NODE:
      ret = INTERFACE_NODE;
      break;
    default:
      ret = CONFIG_NODE;
      break;
    }

  return ret;
}

/* Execute command by argument vline vector. */
static int
cmd_execute_command_real (vector vline,
			  enum filter_type filter,
			  struct vty *vty,
			  struct cmd_element **cmd)
{
  unsigned int i;
  unsigned int index;
  vector cmd_vector;
  struct cmd_element *cmd_element;
  struct cmd_element *matched_element;
  unsigned int matched_count, incomplete_count;
  int argc;
  const char *argv[CMD_ARGC_MAX];
  enum match_type match = 0;
  char *command;
  int ret;
  vector matches;

  /* Make copy of command elements. */
  cmd_vector = vector_copy (cmd_node_vector (cmdvec, vty->node));

  for (index = 0; index < vector_active (vline); index++)
    {
      command = vector_slot (vline, index);
      ret = cmd_vector_filter(cmd_vector,
			      filter,
			      vline, index,
			      &match,
			      &matches);

      if (ret != CMD_SUCCESS)
	{
	  cmd_matches_free(&matches);
	  return ret;
	}

      if (match == vararg_match)
	{
	  cmd_matches_free(&matches);
	  break;
	}

      ret = is_cmd_ambiguous (cmd_vector, command, matches, match);
      cmd_matches_free(&matches);

      if (ret == 1)
	{
	  vector_free(cmd_vector);
	  return CMD_ERR_AMBIGUOUS;
	}
      else if (ret == 2)
	{
	  vector_free(cmd_vector);
	  return CMD_ERR_NO_MATCH;
	}
    }

  /* Check matched count. */
  matched_element = NULL;
  matched_count = 0;
  incomplete_count = 0;

  for (i = 0; i < vector_active (cmd_vector); i++)
    if ((cmd_element = vector_slot (cmd_vector, i)))
      {
	if (cmd_is_complete(cmd_element, vline))
	  {
	    matched_element = cmd_element;
	    matched_count++;
	  }
	else
	  {
	    incomplete_count++;
	  }
      }

  /* Finish of using cmd_vector. */
  vector_free (cmd_vector);

  /* To execute command, matched_count must be 1. */
  if (matched_count == 0)
    {
      if (incomplete_count)
	return CMD_ERR_INCOMPLETE;
      else
	return CMD_ERR_NO_MATCH;
    }

  if (matched_count > 1)
    return CMD_ERR_AMBIGUOUS;

  ret = cmd_parse(matched_element, vline, &argc, argv);
  if (ret != CMD_SUCCESS)
    return ret;

  /* For vtysh execution. */
  if (cmd)
    *cmd = matched_element;

  if (matched_element->daemon)
    return CMD_SUCCESS_DAEMON;

  /* Execute matched command. */
  return (*matched_element->func) (matched_element, vty, argc, argv);
}

/**
 * Execute a given command, handling things like "do ..." and checking
 * whether the given command might apply at a parent node if doesn't
 * apply for the current node.
 *
 * @param vline Command line input, vector of char* where each element is
 *              one input token.
 * @param vty The vty context in which the command should be executed.
 * @param cmd Pointer where the struct cmd_element of the matched command
 *            will be stored, if any. May be set to NULL if this info is
 *            not needed.
 * @param vtysh If set != 0, don't lookup the command at parent nodes.
 * @return The status of the command that has been executed or an error code
 *         as to why no command could be executed.
 */
int
cmd_execute_command (vector vline, struct vty *vty, struct cmd_element **cmd,
		     int vtysh) {
  int ret, saved_ret, tried = 0;
  enum node_type onode, try_node;

  onode = try_node = vty->node;

  if ( cmd_try_do_shortcut(vty->node, vector_slot(vline, 0) ) )
    {
      vector shifted_vline;
      unsigned int index;

      vty->node = ENABLE_NODE;
      /* We can try it on enable node, cos' the vty is authenticated */

      shifted_vline = vector_init (vector_count(vline));
      /* use memcpy? */
      for (index = 1; index < vector_active (vline); index++) 
	{
	  vector_set_index (shifted_vline, index-1, vector_lookup(vline, index));
	}

      ret = cmd_execute_command_real (shifted_vline, FILTER_RELAXED, vty, cmd);

      vector_free(shifted_vline);
      vty->node = onode;
      return ret;
  }


  saved_ret = ret = cmd_execute_command_real (vline, FILTER_RELAXED, vty, cmd);

  if (vtysh)
    return saved_ret;

  /* This assumes all nodes above CONFIG_NODE are childs of CONFIG_NODE */
  while ( ret != CMD_SUCCESS && ret != CMD_WARNING 
	  && vty->node > CONFIG_NODE )
    {
      try_node = node_parent(try_node);
      vty->node = try_node;
      ret = cmd_execute_command_real (vline, FILTER_RELAXED, vty, cmd);
      tried = 1;
      if (ret == CMD_SUCCESS || ret == CMD_WARNING)
	{
	  /* succesfull command, leave the node as is */
	  return ret;
	}
    }
  /* no command succeeded, reset the vty to the original node and
     return the error for this node */
  if ( tried )
    vty->node = onode;
  return saved_ret;
}

/**
 * Execute a given command, matching it strictly against the current node.
 * This mode is used when reading config files.
 *
 * @param vline Command line input, vector of char* where each element is
 *              one input token.
 * @param vty The vty context in which the command should be executed.
 * @param cmd Pointer where the struct cmd_element* of the matched command
 *            will be stored, if any. May be set to NULL if this info is
 *            not needed.
 * @return The status of the command that has been executed or an error code
 *         as to why no command could be executed.
 */
int
cmd_execute_command_strict (vector vline, struct vty *vty,
			    struct cmd_element **cmd)
{
  return cmd_execute_command_real(vline, FILTER_STRICT, vty, cmd);
}

/**
 * Parse one line of config, walking up the parse tree attempting to find a match
 *
 * @param vty The vty context in which the command should be executed.
 * @param cmd Pointer where the struct cmd_element* of the match command
 *            will be stored, if any.  May be set to NULL if this info is
 *            not needed.
 * @param use_daemon Boolean to control whether or not we match on CMD_SUCCESS_DAEMON
 *                   or not.
 * @return The status of the command that has been executed or an error code
 *         as to why no command could be executed.
 */
int
command_config_read_one_line (struct vty *vty, struct cmd_element **cmd, int use_daemon)
{
  vector vline;
  int saved_node;
  int ret;

  vline = cmd_make_strvec (vty->buf);

  /* In case of comment line */
  if (vline == NULL)
    return CMD_SUCCESS;

  /* Execute configuration command : this is strict match */
  ret = cmd_execute_command_strict (vline, vty, cmd);

  saved_node = vty->node;

  while (!(use_daemon && ret == CMD_SUCCESS_DAEMON) &&
	 ret != CMD_SUCCESS && ret != CMD_WARNING &&
	 ret != CMD_ERR_NOTHING_TODO && vty->node != CONFIG_NODE) {
    vty->node = node_parent(vty->node);
    ret = cmd_execute_command_strict (vline, vty, NULL);
  }

  // If climbing the tree did not work then ignore the command and
  // stay at the same node
  if (!(use_daemon && ret == CMD_SUCCESS_DAEMON) &&
      ret != CMD_SUCCESS && ret != CMD_WARNING &&
      ret != CMD_ERR_NOTHING_TODO)
    {
      vty->node = saved_node;
    }

  cmd_free_strvec (vline);

  return ret;
}

/* Configration make from file. */
int
config_from_file (struct vty *vty, FILE *fp, unsigned int *line_num)
{
  int ret;
  *line_num = 0;

  while (fgets (vty->buf, vty->max, fp))
    {
      ++(*line_num);

      ret = command_config_read_one_line (vty, NULL, 0);

      if (ret != CMD_SUCCESS && ret != CMD_WARNING
	  && ret != CMD_ERR_NOTHING_TODO)
	return ret;
    }
  return CMD_SUCCESS;
}


void
install_default_basic (enum node_type node)
{
  install_element (node, &config_exit_cmd);
  install_element (node, &config_quit_cmd);
  install_element (node, &config_help_cmd);
  install_element (node, &config_list_cmd);
}

/* Install common/default commands for a privileged node */
void
install_default (enum node_type node)
{
  /* VIEW_NODE is inited below, via install_default_basic, and
     install_element's of commands to VIEW_NODE automatically are
     also installed to ENABLE_NODE.
    
     For all other nodes, we must ensure install_default_basic is
     also called/
   */
  if (node != VIEW_NODE && node != ENABLE_NODE)
  {
    install_default_basic (node);
    install_element (node, &config_end_cmd);
  }
  install_element (node, &config_write_terminal_cmd);
  install_element (node, &config_write_file_cmd);
  install_element (node, &config_write_memory_cmd);
  install_element (node, &config_write_cmd);
  install_element (node, &show_running_config_cmd);
}

/* Initialize command interface. Install basic nodes and commands. */
void
cmd_init (int terminal)
{
  command_cr = XSTRDUP(MTYPE_CMD_TOKENS, "<cr>");
  token_cr.type = TOKEN_TERMINAL;
  token_cr.terminal = TERMINAL_LITERAL;
  token_cr.cmd = command_cr;
  token_cr.desc = XSTRDUP(MTYPE_CMD_TOKENS, "");

  /* Allocate initial top vector of commands. */
  cmdvec = vector_init (VECTOR_MIN_SIZE);

  host_config_init(default_motd);
  /* Install top nodes. */

  srandom(time(NULL));
}

static void
cmd_terminate_token(struct cmd_token *token)
{
  unsigned int i, j;
  vector keyword_vect;

  if (token->multiple)
    {
      for (i = 0; i < vector_active(token->multiple); i++)
        cmd_terminate_token(vector_slot(token->multiple, i));
      vector_free(token->multiple);
      token->multiple = NULL;
    }

  if (token->keyword)
    {
      for (i = 0; i < vector_active(token->keyword); i++)
        {
          keyword_vect = vector_slot(token->keyword, i);
          for (j = 0; j < vector_active(keyword_vect); j++)
            cmd_terminate_token(vector_slot(keyword_vect, j));
          vector_free(keyword_vect);
        }
      vector_free(token->keyword);
      token->keyword = NULL;
    }

  XFREE(MTYPE_CMD_TOKENS, token->cmd);
  XFREE(MTYPE_CMD_TOKENS, token->desc);

  XFREE(MTYPE_CMD_TOKENS, token);
}

static void
cmd_terminate_element(struct cmd_element *cmd)
{
  unsigned int i;

  if (cmd->tokens == NULL)
    return;

  for (i = 0; i < vector_active(cmd->tokens); i++)
    cmd_terminate_token(vector_slot(cmd->tokens, i));

  vector_free(cmd->tokens);
  cmd->tokens = NULL;
}

void
cmd_terminate ()
{
  unsigned int i, j;
  struct cmd_node *cmd_node;
  struct cmd_element *cmd_element;
  vector cmd_node_v;

  if (cmdvec)
    {
      for (i = 0; i < vector_active (cmdvec); i++) 
        if ((cmd_node = vector_slot (cmdvec, i)) != NULL)
          {
            cmd_node_v = cmd_node->cmd_vector;

            for (j = 0; j < vector_active (cmd_node_v); j++)
              if ((cmd_element = vector_slot (cmd_node_v, j)) != NULL)
                cmd_terminate_element(cmd_element);

            vector_free (cmd_node_v);
            hash_clean (cmd_node->cmd_hash, NULL);
            hash_free (cmd_node->cmd_hash);
            cmd_node->cmd_hash = NULL;
          }

      vector_free (cmdvec);
      cmdvec = NULL;
    }

  if (command_cr)
    XFREE(MTYPE_CMD_TOKENS, command_cr);
  if (token_cr.desc)
    XFREE(MTYPE_CMD_TOKENS, token_cr.desc);
  host_config_exit();
}
