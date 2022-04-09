/* Virtual terminal interface shell.
 * Copyright (C) 2000 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <zplos_include.h>
#include <lib_include.h>

#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>

#include <readline/readline.h>
#include <readline/history.h>



#include "vtysh.h"
#include "vtysh_user.h"

/* VTY shell program name. */
//char *progname;

/* A static variable for holding the line. */
static char *line_read;

/* SIGTSTP handler.  This function care user's ^Z input. */
static void
sigtstp(int sig)
{
  /* Execute "end" command. */
  vtysh_execute("end");

  /* Initialize readline. */
  rl_initialize();
  printf("\n");
}

/* SIGINT handler.  This function care user's ^Z input.  */
static void
sigint(int sig)
{
    rl_initialize();
    printf("\n");
    rl_forced_update_display();
}

/* Signale wrapper for vtysh. We don't use sigevent because
 * vtysh doesn't use threads. TODO */
static void
vtysh_signal_set(int signo, void (*func)(int))
{
  struct sigaction sig;
  struct sigaction osig;

  sig.sa_handler = func;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
#ifdef SA_RESTART
  sig.sa_flags |= SA_RESTART;
#endif /* SA_RESTART */

  sigaction(signo, &sig, &osig);
}

/* Initialization of signal handles. */
static void
vtysh_signal_init()
{
  vtysh_signal_set(SIGINT, sigint);
  vtysh_signal_set(SIGTSTP, sigtstp);
  vtysh_signal_set(SIGPIPE, SIG_IGN);
}


/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
static char *
vtysh_rl_gets()
{
  HIST_ENTRY *last;
  /* If the buffer has already been allocated, return the memory
   * to the free pool. */
  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  /* Get a line from the user.  Change prompt according to node.  XXX. */
  line_read = readline(vty_prompt(vtysh_client.vty));

  /* If the line has any text in it, save it on the history. But only if
   * last command in history isn't the same one. */
  if (line_read && *line_read)
  {
    using_history();
    last = previous_history();
    if (!last || strcmp(last->line, line_read) != 0)
    {
      add_history(line_read);
      //append_history(1, history_file);
    }
  }

  return (line_read);
}


/* VTY shell main routine. */
int main(int argc, char **argv, char **env)
{

  /* Initialize user input buffer. */
  line_read = NULL;

  /* Signal and others. */
  vtysh_signal_init();


  /* Make vty structure and register commands. */
  vtysh_init();
  vtysh_cmd_init();
  vtysh_user_init();


  /* Read vtysh configuration file before connecting to daemons. */
  //vtysh_read_config(config_default);


  /* Make sure we pass authentication before proceeding. */
  vtysh_auth();

  /* Do not connect until we have passed authentication. */
  if (vtysh_connect(ZEBRA_VTYSH_PATH) <= 0)
  {
    fprintf(stderr, "Exiting: failed to connect to any daemons.\n");
    //exit(1);
  }
  

  vty_hello(vtysh_client.vty);

  /* Enter into enable node. */
  vtysh_execute("enable");

  /* Main command loop. */
  while (vtysh_rl_gets())
    vtysh_execute(line_read);

  //history_truncate_file(history_file, 1000);
  printf("\n");

  /* Rest in peace. */
  exit(0);
}
