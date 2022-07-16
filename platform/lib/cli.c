
#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "vector.h"
#include "vty.h"
#include "command.h"
#include "cli.h"

#ifdef ZPL_SHELL_MODULE

typedef char cmd_multiple_keystr[128];

/*删除多余空格*/
static int cmd_keystr_rm_surplus_isspace(const char *string, char *cmd)
{
  int i = 0, k = 0;
  const char *cp = string;
  while (isspace(*cp))
    cp++;
  while (1)
  {
    switch (*cp)
    {
    case '\0':
      return 0;
    case '(':
    case ')':
    case '|':
    case '{':
    case '}':
    case '[':
    case ']':
    case '-':
      k = 1;
      cmd[i++] = *cp;
      break;
    case ' ':
      switch (*(cp + 1))
      {
      case ')':
      case '|':
      case '}':
      case ']':
      case '-':
        k = 1;
        break;
      case '(':
      case '{':
      case '[':
        break;
      }
      if (k == 0)
      {
        cmd[i++] = *cp;
        k = 1;
      }
      break;
    default:
      k = 0;
      cmd[i++] = *cp;
      break;
    }
    cp++;
  }
  return 0;
}

/* 获取helpstr 个数 */
static int cmd_helpstr_getnum(const char **string)
{
  int i = 0;
  for (i = 0;; i++)
  {
    if (string[i] == NULL)
      break;
  }
  return i;
}

static int cmd_keystr_getnum(const char *string, int *num)
{
  int j = 0, k = 0;
  const char *cp = string;
  while (isspace(*cp))
    cp++;
  while (1)
  {
    switch (*cp)
    {
    case '\0':
      if (num)
        *num = (j + 1 + k);
      return (j + 1 + k);
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case '-':
      break;
    case ' ':
      k++;
      break;
    case '|':
      j++;
      break;
    default:
      break;
    }
    cp++;
  }
  if (num)
    *num = (j + 1 + k);
  return (j + 1 + k);
}

static int cmd_keystr_wrod_get(const char *string, char *dst, int *len, int space)
{
  int i = 0;
  const char *cp = string;
  while (1)
  {
    switch (*cp)
    {
    case '\0':
      if (len)
        *len = i;
      return *cp;
    case '(':
      if (len)
        *len = i;
      return *cp;
    case ')':
      if (len)
        *len = i;
      return *cp;
    case '|':
      if (len)
        *len = i;
      return *cp;
    case ' ':
      if (space)
      {
        if (len)
          *len = i;
        return *cp;
      }
      else
        dst[i++] = *cp;
      break;
    default:
      dst[i++] = *cp;
      break;
    }
    cp++;
  }
  return 0;
}

static int cmd_helpstr_get(const char **helpstrarray, int num, char *helpstr)
{
  if (helpstrarray[num])
  {
    strcat(helpstr, helpstrarray[num]);
    strcat(helpstr, "\n");
    return 1;
  }
  return 0;
}

static int cmd_keystr_word_index(char *str2, cmd_multiple_keystr *keystrarray, int maxcnt)
{
  int i = 0;
  for (i = 0; i < maxcnt; i++)
  {
    if (strlen(keystrarray[i]) && strcmp(str2, keystrarray[i]) == 0)
    {
      return i;
    }
  }
  return -1;
}

static int cmd_keystr_array(const char *string, cmd_multiple_keystr *keystrarray)
{
  int ret = 0, len = 0, h = 0;
  const char *cp = string;
  char tmpbuf[128];
  while (1)
  {
    memset(tmpbuf, 0, sizeof(tmpbuf));
    ret = cmd_keystr_wrod_get(cp, tmpbuf, &len, 1);
    switch (ret)
    {
    case '\0':
      // printf(" cmd_keystr_array Unclosed group/keyword\r\n");
      // cmd_keystr_build(keystrarray, 32, NULL, NULL);
      return h;
    case ' ':
      if (len)
        strcpy(keystrarray[h++], tmpbuf);
      cp += (len + 1);
      switch (*(cp))
      {
      case ')':
      case '|':
      case '}':
      case ']':
      case '(':
      case '{':
      case '[':
        cp++;
        break;
      default:
        break;
      }
      break;

    case '(':
      if (len)
        strcpy(keystrarray[h++], tmpbuf);
      cp += (len + 1);
      break;
    case ')':
      if (len)
        strcpy(keystrarray[h++], tmpbuf);
      cp += (len + 1);
      break;
    case '|':
      if (len)
        strcpy(keystrarray[h++], tmpbuf);
      cp += (len + 1);
      break;
    default:
      break;
    }
  }
  // cmd_keystr_build(keystrarray, 32, NULL, NULL);
  return h;
}

static int cmd_keystr_build(struct cli_element *cel, cmd_multiple_keystr *string, int maxcnt, cmd_multiple_keystr *keystrarray, const char **helpstr)
{
  int i = 0, j = 0, knum = 0, index = -1;
  char clikey[1024];
  char clihelpstr[1024];
  struct cmd_element *cmd = NULL; 
  cmd_multiple_keystr localkeystr[64];
  memset(clikey, 0, sizeof(clikey));
  memset(clihelpstr, 0, sizeof(clihelpstr));
  memset(localkeystr, 0, sizeof(localkeystr));
  for (i = 0; i < maxcnt; i++)
  {
    if (strlen(string[i]))
    {
      strcat(clikey, string[i]);
      strcat(clikey, " ");
      if (keystrarray)
      {
        knum = cmd_keystr_array(string[i], localkeystr);
        if (knum == 0)
        {
          index = cmd_keystr_word_index(string[i], keystrarray, maxcnt);
          if (index >= 0 && helpstr)
          {
            cmd_helpstr_get(helpstr, index, clihelpstr);
          }
        }
        else
        {
          for (j = 0; j < knum; j++)
          {
            index = cmd_keystr_word_index(localkeystr[j], keystrarray, maxcnt);
            if (index >= 0 && helpstr)
            {
              cmd_helpstr_get(helpstr, index, clihelpstr);
            }
          }
        }
      }
    }
    else
      break;
  }
  printf("=======%d=%s\r\n", i, clikey);
  printf("====help==%s", clihelpstr);
  cmd = new_element(clikey, clihelpstr, 0, cel->flags, NULL);
  if(cmd)
  {
    cmd->cli_func = cel->func;
    cmd->func = NULL;
    install_element (cel->node, cel->privilege, cmd);
  }
  return 0;
}

static int cmd_keystr_cli_element_change(struct cli_element *cel, const char *string, const char **helpstr)
{
  int ret = 0, len = 0, k = 0;
  const char *cp = string;
  char tmpbuf[128];
  cmd_multiple_keystr keystr[CLI_KEYSTR_MAX];
  cmd_multiple_keystr keystrarray[CLI_KEYSTR_MAX];
  memset(keystr, 0, sizeof(keystr));
  memset(keystrarray, 0, sizeof(keystrarray));
  cmd_keystr_array(string, keystrarray);
  while (1)
  {
    memset(tmpbuf, 0, sizeof(tmpbuf));
    ret = cmd_keystr_wrod_get(cp, tmpbuf, &len, 0);
    switch (ret)
    {
    case '\0':
      // printf("Unclosed group/keyword\r\n");
      return 0;
    case ' ':
      memset(keystr[k + 1], 0, sizeof(cmd_multiple_keystr));
      strcpy(keystr[k], tmpbuf);
      cp += (len + 1);
      k++;
      switch (*(cp))
      {
      case ')':
      case '|':
      case '}':
      case ']':
      case '(':
      case '{':
      case '[':
        cp++;
        break;
      default:
        break;
      }
      break;

    case '(':
      memset(keystr[k + 1], 0, sizeof(cmd_multiple_keystr));
      strcpy(keystr[k], tmpbuf);
      cp += (len + 1);
      k++;
      break;
    case ')':
      memset(keystr[k + 1], 0, sizeof(cmd_multiple_keystr));
      strcpy(keystr[k], tmpbuf);
      if (len)
        cmd_keystr_build(cel, keystr, CLI_KEYSTR_MAX, keystrarray, helpstr);
      cp += (len + 1);
      k--;
      break;
    case '|':
      memset(keystr[k + 1], 0, sizeof(cmd_multiple_keystr));
      strcpy(keystr[k], tmpbuf);
      if (len)
        cmd_keystr_build(cel, keystr, CLI_KEYSTR_MAX, keystrarray, helpstr);
      cp += (len + 1);
      if (*cp == ')')
        cmd_keystr_build(cel, keystr, CLI_KEYSTR_MAX, keystrarray, helpstr);
      break;
    default:
      break;
    }
  }
  return 0;
}

int cli_install_gen(void *p, enum node_type node,
                    enum cmd_privilege privilege, zpl_uint16 flags, struct cli_element *cel)
{
  char cmdstr[1024];
  int num = 0;
  memset(cmdstr, 0, sizeof(cmdstr));
  cmd_keystr_rm_surplus_isspace(cel->str, cmdstr);
  cmd_keystr_getnum(cmdstr, &num);
  if(num != cmd_helpstr_getnum(cel->help) || strstr(cel->str, "[") || strstr(cel->str, "{"))
  {
    fprintf(stderr, "\nError parsing command: \"%s\"\n", cel->str);
    fprintf(stderr, "This is a programming error. Check your CLI helpstr.\n");
    exit(1);
  }
  cel->flags = flags;
  cel->privilege = privilege;
  cel->node = node;
  return cmd_keystr_cli_element_change(cel, cmdstr, cel->help);
}

int cli_callback_init(struct cli *cli, struct vty *vty)
{
  int i = 0;
  memset(cli, 0, sizeof(struct cli));
  cli->out_func = vty_out;
  cli->out_val = vty;
  cli->index = vty->index;
  cli->node = vty->node;  
  cli->index_range = vty->index_range;
  for(i = 0; i < VTY_RANGE_MAX; i++)
    cli->cli_range_index[i] = vty->vty_range_index[i];
  cli->index_value = vty->index_value;
  cli->index_sub = vty->index_sub;
  return OK;
}

#if 0
/* Generic CLI Installation. */
int
cli_install_gen (struct cli_tree *ctree, int mode,
                 u_char privilege, u_int16_t flags, struct cli_element *cel)
{
  struct cli_builder cb;
  struct cli_node *node;
  vector parent;
  int index, max;

  /* Set flags. */
  if (flags)
    SET_FLAG (cel->flags, flags);

  /* Check help string is there.  */
  cli_check_help (cel, &index, &max);

  if (mode > MAX_MODE)
    return -1;

  /* Lookup root node.  */
  node = vector_lookup_index (ctree->modes, mode);

  /* Install a new root node.  */
  if (! node)
    {
      node = cli_node_new ();
      vector_set_index (ctree->modes, mode, node);
    }

  if (pal_strstr (cel->str, "IFNAME"))
    cli_ifname_reflect (cel, index, max);

  /* Set initial value before calling cli_build().  */
  parent = vector_init (VECTOR_MIN_SIZE);
  vector_set (parent, node);
  cb.str = cel->str;
  cb.index = 0;

  cli_build (parent, NULL, NULL, &cb, cel, privilege, 0);

  vector_free (parent);

  return 0;
}

/* Common functions for each mode.  */
void
cli_install_default (struct cli_tree *ctree, int mode)
{
  cli_install_gen (ctree, mode, PRIVILEGE_NORMAL, 0, &cli_help_cli);
  cli_install_gen (ctree, mode, PRIVILEGE_NORMAL, 0, &cli_show_list_cli);
  cli_install_gen (ctree, mode, PRIVILEGE_NORMAL, 0, &cli_show_tree_cli);

  /* IMI shell send exit to IMI.  For protocol module, this is same as
     cli_install().  */
  cli_install_imi (ctree, mode, PM_EMPTY, PRIVILEGE_NORMAL, 0,
                   &cli_config_exit_cli);

  /* "quit" is only for EXEC MODE.  "end" should not be installed in
     EXEC MODE.  */
  if (mode == EXEC_MODE)
    cli_install_gen (ctree, mode, PRIVILEGE_NORMAL, 0, &cli_config_quit_cli);
  else
    {
      cli_install_imi (ctree, mode, PM_EMPTY, PRIVILEGE_NORMAL,
                       CLI_FLAG_HIDDEN, &cli_config_quit_cli);
      cli_install_imi (ctree, mode, PM_EMPTY, PRIVILEGE_NORMAL,
                       CLI_FLAG_HIDDEN, &cli_config_end_cli);
    }
}

#endif

#endif