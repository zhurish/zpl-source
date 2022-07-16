#ifndef __CLI_H__
#define __CLI_H__

#ifdef ZPL_SHELL_MODULE

#include "command.h"
#include "vty.h"

#ifndef VTYSH_EXTRACT_PL


#define CLI(func_name, cli_name, cmd_str, ...)                          \
                                                                        \
/* Function prototype.  */                                              \
int func_name (struct cli *, int, char**);                              \
                                                                        \
/* Help string array.  */                                               \
char *cli_name ## _help[] = {__VA_ARGS__, NULL};                        \
                                                                        \
/* Define CLI structure.  */                                            \
struct cli_element cli_name =                                           \
{                                                                       \
  /* Command line string.  */                                           \
  cmd_str,                                                              \
                                                                        \
   /* Function pointer.  */                                             \
  func_name,                                                            \
                                                                        \
  /* Help string is defined as an array.  Last must be NULL.  */        \
  cli_name ## _help                                                     \
};                                                                      \
                                                                        \
/* Start function body at here.  */                                     \
int func_name (struct cli *cli, int argc, char** argv)

/* ALIAS to CLI macro.  Define CLI structure only.  There is no
   function body.  */
#define ALI(func_name, cli_name, cmd_str, ...)                          \
                                                                        \
extern int func_name (struct cli *cli, int argc, char** argv);          \
                                                                        \
/* Help string array.  */                                               \
char *cli_name ## _help[] = {__VA_ARGS__, NULL};                        \
                                                                        \
struct cli_element cli_name =                                           \
{                                                                       \
  cmd_str,                                                              \
  func_name,                                                            \
  cli_name ## _help                                                     \
}

/* IMI/IMISH ALI generated by cli.pl for IMI and IMIsh.  */
#define IMI_ALI(func_name, cli_name, cmd_str, ...)                      \
                                                                        \
/* Help string array.  */                                               \
char *cli_name ## _help[] = {__VA_ARGS__, NULL};                        \
                                                                        \
struct cli_element cli_name =                                           \
{                                                                       \
  cmd_str,                                                              \
  func_name,                                                            \
  cli_name ## _help                                                     \
}

#endif /* VTYSH_EXTRACT_PL */

#define CLI_NEWLINE   "\r\n"
/* CLI output function macro.  Instead of defining a function, user
   can specify CLI output function dynamically.  */
#define cli_out(cli, ...)                                                     \
        (*(cli)->out_func)((cli)->out_val, __VA_ARGS__)



/* Argument to cli functions.  */
struct cli
{
  /* Output function to be used by cli_out().  */
  int (*out_func)(void *, const char *, ...);

  /* Output function's first argument.  */
  void *out_val;
  zpl_int32 node;
  void *index;

  zpl_uint32 index_range;
  void *cli_range_index[VTY_RANGE_MAX];
  /* */
  zpl_uint32 index_value;

  void *index_sub;
};

/* Configuration output function.  */

/* CLI element.  */
struct cli_element
{
  /* Command line string.  */
  const char *str;

  /* Function to execute this command.  */
  int (*func) (struct cli *, int, const char **);

  /* Help strings array. */
  const char **help;
  enum node_type node;
  enum cmd_privilege privilege;
  zpl_uint16 flags;
};

#define CLI_KEYSTR_MAX  64

extern int cli_install_gen(void *, enum node_type,
                    enum cmd_privilege , zpl_uint16 , struct cli_element *);
extern int cli_callback_init(struct cli *cli, struct vty *vty);

#endif

#endif /* __CLI_H__ */
