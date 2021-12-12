/*
 * Zebra configuration command interface routine
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 * 
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _ZEBRA_COMMAND_H
#define _ZEBRA_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_SHELL_MODULE

#include "hash.h"
#include "vector.h"
#include "vty.h"

#ifdef IF_IUSPV_SUPPORT
#define CMD_IUSPV_SUPPORT
#endif

#include "cli_node.h"
#include "cli_helper.h"

/* Node which has some commands and prompt string and configuration
   function pointer . */
struct cmd_node 
{
  /* Node index. */
  enum node_type node;		

  /* Prompt character at vty interface. */
  const char *prompt;			

  /* Is this node's configuration goes to vtysh ? */
  zpl_uint32 vtysh;
  
  /* Node's configuration write function */
  int (*func) (struct vty *);
  //struct list *show_hook_list;
  /* Vector of this node's command list. */
  vector cmd_vector;
  
  /* Hashed index of command node list, for de-dupping primarily */
  struct hash *cmd_hash;
};

enum
{
  CMD_ATTR_DEPRECATED = 1,
  CMD_ATTR_HIDDEN,
};

enum cmd_privilege {
	CMD_VIEW_LEVEL = 1,
	CMD_ENABLE_LEVEL = 2,
	CMD_CONFIG_LEVEL = 3,
	CMD_ADMIN_LEVEL = 4,
  CMD_ROOT_LEVEL = 5,
};

/* Structure of command element. */
struct cmd_element 
{
  const char *string;			/* Command specification by string. */
  int (*func) (struct cmd_element *, struct vty *, zpl_uint32, const char *[]);
  const char *doc;			/* Documentation of this command. */
  zpl_uint32 daemon;                   /* Daemon to which this command belong. */
  vector tokens;		/* Vector of cmd_tokens */
  zpl_uchar attr;			/* Command attributes */
  enum cmd_privilege privilege;
};


enum cmd_token_type
{
  TOKEN_TERMINAL = 0,
  TOKEN_MULTIPLE,
  TOKEN_KEYWORD,
};

enum cmd_terminal_type
{
  _TERMINAL_BUG = 0,
  TERMINAL_LITERAL,
  TERMINAL_OPTION,
  TERMINAL_VARIABLE,
  TERMINAL_VARARG,
  TERMINAL_RANGE,
  TERMINAL_IUSPV,
  TERMINAL_MAC,
  TERMINAL_IPV4,
  TERMINAL_IPV4_PREFIX,
  TERMINAL_IPV6,
  TERMINAL_IPV6_PREFIX,
};

/* argument to be recorded on argv[] if it's not a literal */
#define TERMINAL_RECORD(t) ((t) >= TERMINAL_OPTION)

/* Command description structure. */
struct cmd_token
{
  enum cmd_token_type type;
  enum cmd_terminal_type terminal;

  /* Used for type == MULTIPLE */
  vector multiple; /* vector of cmd_token, type == FINAL */

  /* Used for type == KEYWORD */
  vector keyword; /* vector of vector of cmd_tokens */

  /* Used for type == TERMINAL */
  zpl_char *cmd;                    /* Command string. */
  zpl_char *desc;                    /* Command's description. */
};

/* Return value of the commands. */
#define CMD_SUCCESS              0
#define CMD_WARNING              1
#define CMD_ERR_NO_MATCH         2
#define CMD_ERR_AMBIGUOUS        3
#define CMD_ERR_INCOMPLETE       4
#define CMD_ERR_EXEED_ARGC_MAX   5
#define CMD_ERR_NOTHING_TODO     6
#define CMD_COMPLETE_FULL_MATCH  7
#define CMD_COMPLETE_MATCH       8
#define CMD_COMPLETE_LIST_MATCH  9
#define CMD_SUCCESS_DAEMON      10

/* Argc max counts. */
#define CMD_ARGC_MAX   25

/* Turn off these macros when uisng cpp with extract.pl */
#ifndef VTYSH_EXTRACT_PL  

/* helper defines for end-user DEFUN* macros */
#define DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, attrs, dnum) \
  struct cmd_element cmdname = \
  { \
    .string = cmdstr, \
    .func = funcname, \
    .doc = helpstr, \
    .attr = attrs, \
    .daemon = dnum, \
  };

#define DEFUN_CMD_FUNC_DECL(funcname) \
  static int funcname (struct cmd_element *, struct vty *, int, const char *[]);

#define DEFUN_CMD_FUNC_TEXT(funcname) \
  static int funcname \
    (struct cmd_element *self __attribute__ ((unused)), \
     struct vty *vty __attribute__ ((unused)), \
     int argc __attribute__ ((unused)), \
     const char *argv[] __attribute__ ((unused)) )

/* DEFUN for vty command interafce. Little bit hacky ;-).
 *
 * DEFUN(funcname, cmdname, cmdstr, helpstr)
 *
 * funcname
 * ========
 *
 * Name of the function that will be defined.
 *
 * cmdname
 * =======
 *
 * Name of the struct that will be defined for the command.
 *
 * cmdstr
 * ======
 *
 * The cmdstr defines the command syntax. It is used by the vty subsystem
 * and vtysh to perform matching and completion in the cli. So you have to take
 * care to construct it adhering to the following grammar. The names used
 * for the production rules losely represent the names used in lib/command.c
 *
 * cmdstr = cmd_token , { " " , cmd_token } ;
 *
 * cmd_token = cmd_terminal
 *           | cmd_multiple
 *           | cmd_keyword ;
 *
 * cmd_terminal_fixed = fixed_string
 *                    | variable
 *                    | range
 *                    | ipv4
 *                    | ipv4_prefix
 *                    | ipv6
 *                    | ipv6_prefix ;
 *
 * cmd_terminal = cmd_terminal_fixed
 *              | option
 *              | vararg ;
 *
 * multiple_part = cmd_terminal_fixed ;
 * cmd_multiple = "(" , multiple_part , ( "|" | { "|" , multiple_part } ) , ")" ;
 *
 * keyword_part = fixed_string , { " " , ( cmd_terminal_fixed | cmd_multiple ) } ;
 * cmd_keyword = "{" , keyword_part , { "|" , keyword_part } , "}" ;
 *
 * lowercase = "a" | ... | "z" ;
 * uppercase = "A" | ... | "Z" ;
 * digit = "0" | ... | "9" ;
 * number = digit , { digit } ;
 *
 * fixed_string = (lowercase | digit) , { lowercase | digit | uppercase | "-" | "_" } ;
 * variable = uppercase , { uppercase | "_" } ;
 * range = "<" , number , "-" , number , ">" ;
 * ipv4 = "A.B.C.D" ;
 * ipv4_prefix = "A.B.C.D/M" ;
 * ipv6 = "X:X::X:X" ;
 * ipv6_prefix = "X:X::X:X/M" ;
 * option = "[" , variable , "]" ;
 * vararg = "." , variable ;
 *
 * To put that all in a textual description: A cmdstr is a sequence of tokens,
 * separated by spaces.
 *
 * Terminal Tokens:
 *
 * A very simple cmdstring would be something like: "show ip bgp". It consists
 * of three Terminal Tokens, each containing a fixed string. When this command
 * is called, no arguments will be passed down to the function implementing it,
 * as it only consists of fixed strings.
 *
 * Apart from fixed strings, Terminal Tokens can also contain variables:
 * An example would be "show ip bgp A.B.C.D". This command expects an IPv4
 * as argument. As this is a variable, the IP address entered by the user will
 * be passed down as an argument. Apart from two exceptions, the other options
 * for Terminal Tokens behave exactly as we just discussed and only make a
 * difference for the CLI. The two exceptions will be discussed in the next
 * paragraphs.
 *
 * A Terminal Token can contain a so called option match. This is a simple
 * string variable that the user may omit. An example would be:
 * "show interface [IFNAME]". If the user calls this without an interface as
 * argument, no arguments will be passed down to the function implementing
 * this command. Otherwise, the interface name will be provided to the function
 * as a regular argument.

 * Also, a Terminal Token can contain a so called vararg. This is used e.g. in
 * "show ip bgp regexp .LINE". The last token is a vararg match and will
 * consume all the arguments the user inputs on the command line and append
 * those to the list of arguments passed down to the function implementing this
 * command. (Therefore, it doesn't make much sense to have any tokens after a
 * vararg because the vararg will already consume all the words the user entered
 * in the CLI)
 *
 * Multiple Tokens:
 *
 * The Multiple Token type can be used if there are multiple possibilities what
 * arguments may be used for a command, but it should map to the same function
 * nonetheless. An example would be "ip route A.B.C.D/M (reject|blackhole)"
 * In that case both "reject" and "blackhole" would be acceptable as last
 * arguments. The words matched by Multiple Tokens are always added to the
 * argument list, even if they are matched by fixed strings. Such a Multiple
 * Token can contain almost any type of token that would also be acceptable
 * for a Terminal Token, the exception are optional variables and varag.
 *
 * There is one special case that is used in some places of Quagga that should be
 * pointed out here zpl_int16ly. An example would be "password (8|) WORD". This
 * construct is used to have fixed strings communicated as arguments. (The "8"
 * will be passed down as an argument in this case) It does not mean that
 * the "8" is optional. Another historic and possibly surprising property of
 * this construct is that it consumes two parts of helpstr. (Help
 * strings will be explained later)
 *
 * Keyword Tokens:
 *
 * There are commands that take a lot of different and possibly optional arguments.
 * An example from ospf would be the "default-information originate" command. This
 * command takes a lot of optional arguments that may be provided in any order.
 * To accomodate such commands, the Keyword Token has been implemented.
 * Using the keyword token, the "default-information originate" command and all
 * its possible options can be represented using this single cmdstr:
 * "default-information originate \
 *  {always|metric <0-16777214>|metric-type (1|2)|route-map WORD}"
 *
 * Keywords always start with a fixed string and may be followed by arguments.
 * Except optional variables and vararg, everything is permitted here.
 *
 * For the special case of a keyword without arguments, either NULL or the
 * keyword itself will be pushed as an argument, depending on whether the
 * keyword is present.
 * For the other keywords, arguments will be only pushed for
 * variables/Multiple Tokens. If the keyword is not present, the arguments that
 * would have been pushed will be substituted by NULL.
 *
 * A few examples:
 *   "default information originate metric-type 1 metric 1000"
 * would yield the following arguments:
 *   { NULL, "1000", "1", NULL }
 *
 *   "default information originate always route-map RMAP-DEFAULT"
 * would yield the following arguments:
 *   { "always", NULL, NULL, "RMAP-DEFAULT" }
 *
 * helpstr
 * =======
 *
 * The helpstr is used to show a zpl_int16 explantion for the commands that
 * are available when the user presses '?' on the CLI. It is the concatenation
 * of the helpstrings for all the tokens that make up the command.
 *
 * There should be one helpstring for each token in the cmdstr except those
 * containing other tokens, like Multiple or Keyword Tokens. For those, there
 * will only be the helpstrings of the contained tokens.
 *
 * The individual helpstrings are expected to be in the same order as their
 * respective Tokens appear in the cmdstr. They should each be terminated with
 * a linefeed. The last helpstring should be terminated with a linefeed as well.
 *
 * Care should also be taken to avoid having similar tokens with different
 * helpstrings. Imagine e.g. the commands "show ip ospf" and "show ip bgp".
 * they both contain a helpstring for "show", but only one will be displayed
 * when the user enters "sh?". If those two helpstrings differ, it is not
 * defined which one will be shown and the behavior is therefore unpredictable.
 */
#define DEFUN(funcname, cmdname, cmdstr, helpstr) \
  DEFUN_CMD_FUNC_DECL(funcname) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, 0, 0) \
  DEFUN_CMD_FUNC_TEXT(funcname)

#define DEFUN_ATTR(funcname, cmdname, cmdstr, helpstr, attr) \
  DEFUN_CMD_FUNC_DECL(funcname) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, attr, 0) \
  DEFUN_CMD_FUNC_TEXT(funcname)

#define DEFUN_HIDDEN(funcname, cmdname, cmdstr, helpstr) \
  DEFUN_ATTR (funcname, cmdname, cmdstr, helpstr, CMD_ATTR_HIDDEN)

#define DEFUN_DEPRECATED(funcname, cmdname, cmdstr, helpstr) \
  DEFUN_ATTR (funcname, cmdname, cmdstr, helpstr, CMD_ATTR_DEPRECATED) \

/* DEFUN_NOSH for commands that vtysh should ignore */
#define DEFUN_NOSH(funcname, cmdname, cmdstr, helpstr) \
  DEFUN(funcname, cmdname, cmdstr, helpstr)

/* DEFSH for vtysh. */
#define DEFSH(daemon, cmdname, cmdstr, helpstr) \
  DEFUN_CMD_ELEMENT(NULL, cmdname, cmdstr, helpstr, 0, daemon) \

/* DEFUN + DEFSH */
#define DEFUNSH(daemon, funcname, cmdname, cmdstr, helpstr) \
  DEFUN_CMD_FUNC_DECL(funcname) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, 0, daemon) \
  DEFUN_CMD_FUNC_TEXT(funcname)

/* DEFUN + DEFSH with attributes */
#define DEFUNSH_ATTR(daemon, funcname, cmdname, cmdstr, helpstr, attr) \
  DEFUN_CMD_FUNC_DECL(funcname) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, attr, daemon) \
  DEFUN_CMD_FUNC_TEXT(funcname)

#define DEFUNSH_HIDDEN(daemon, funcname, cmdname, cmdstr, helpstr) \
  DEFUNSH_ATTR (daemon, funcname, cmdname, cmdstr, helpstr, CMD_ATTR_HIDDEN)

#define DEFUNSH_DEPRECATED(daemon, funcname, cmdname, cmdstr, helpstr) \
  DEFUNSH_ATTR (daemon, funcname, cmdname, cmdstr, helpstr, CMD_ATTR_DEPRECATED)

/* ALIAS macro which define existing command's alias. */
#define ALIAS(funcname, cmdname, cmdstr, helpstr) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, 0, 0)

#define ALIAS_ATTR(funcname, cmdname, cmdstr, helpstr, attr) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, attr, 0)

#define ALIAS_HIDDEN(funcname, cmdname, cmdstr, helpstr) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, CMD_ATTR_HIDDEN, 0)

#define ALIAS_DEPRECATED(funcname, cmdname, cmdstr, helpstr) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, CMD_ATTR_DEPRECATED, 0)

#define ALIAS_SH(daemon, funcname, cmdname, cmdstr, helpstr) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, 0, daemon)

#define ALIAS_SH_HIDDEN(daemon, funcname, cmdname, cmdstr, helpstr) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, CMD_ATTR_HIDDEN, daemon)

#define ALIAS_SH_DEPRECATED(daemon, funcname, cmdname, cmdstr, helpstr) \
  DEFUN_CMD_ELEMENT(funcname, cmdname, cmdstr, helpstr, CMD_ATTR_DEPRECATED, daemon)

#endif /* VTYSH_EXTRACT_PL */

/*
 * Sometimes #defines create maximum values that
 * need to have strings created from them that
 * allow the parser to match against them.
 * These macros allow that.
 */
#define CMD_CREATE_STR(s)  CMD_CREATE_STR_HELPER(s)
#define CMD_CREATE_STR_HELPER(s) #s
#define CMD_RANGE_STR(a,s) "<" CMD_CREATE_STR(a) "-" CMD_CREATE_STR(s) ">"


/* Common descriptions. */


/* Prototypes. */
extern void install_node (struct cmd_node *, int (*) (struct vty *));
extern void reinstall_node (enum node_type node, int (*func) (struct vty *));
extern void install_default (enum node_type);
extern void install_element (enum node_type, enum cmd_privilege privilege, struct cmd_element *);

/* Concatenates argv[shift] through argv[argc-1] into a single NUL-terminated
   string with a space between each element (allocated using
   XMALLOC(MTYPE_TMP)).  Returns NULL if shift >= argc. */
extern zpl_char *argv_concat (const char **argv, zpl_uint32 argc, zpl_uint32 shift);

extern vector cmd_make_strvec (const char *);
extern void cmd_free_strvec (vector);
extern vector cmd_describe_command (vector, struct vty *, zpl_uint32 *status);
extern zpl_char **cmd_complete_command (vector, struct vty *, zpl_uint32 *status);
extern zpl_char **cmd_complete_command_lib (vector, struct vty *, zpl_uint32 *status, zpl_uint32 islib);
extern const char *cmd_prompt (enum node_type);
extern int command_config_read_one_line (struct vty *vty, struct cmd_element **, zpl_uint32 use_config_node);
extern int config_from_file (struct vty *, FILE *, zpl_uint32  *line_num);
//extern enum node_type node_parent (enum node_type);
extern int cmd_execute_command (vector, struct vty *, struct cmd_element **, zpl_uint32);
extern int cmd_execute_command_strict (vector, struct vty *, struct cmd_element **);
extern void cmd_init (zpl_bool);
extern void cmd_terminate (void);
extern vector cmd_node_vector (vector v, enum node_type ntype);
extern void install_default_basic (enum node_type node);
extern zpl_char * zencrypt (const char *passwd);
/* Export typical functions. */
extern struct cmd_element config_end_cmd;
extern struct cmd_element config_exit_cmd;
extern struct cmd_element config_quit_cmd;
extern struct cmd_element config_help_cmd;
extern struct cmd_element config_list_cmd;
extern struct cmd_element config_write_terminal_cmd;
extern struct cmd_element config_write_file_cmd;
extern struct cmd_element config_write_memory_cmd;
extern struct cmd_element config_write_cmd;
extern struct cmd_element show_running_config_cmd;

extern void print_version (const char *);



/* "<cr>" global */
extern zpl_char *command_cr;

#endif /* ZPL_SHELL_MODULE */ 
#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_COMMAND_H */
