
#include "auto_include.h"
#include "zplos_include.h"
#include "libgl.h"

/*
static struct lib_global * lib_global_new (void)
{
  return XCALLOC (MTYPE_GLOBAL, sizeof (struct lib_global));
}

static void lib_global_free (struct lib_global *lib_global)
{
  XFREE (MTYPE_GLOBAL, lib_global);
}


struct lib_global * lib_global_lookup (const char *name)
{
  struct listnode *node;
  struct lib_global *lib_global;

  if (name == NULL)
    return NULL;

  for (ALL_LIST_ELEMENTS_RO (lib_global_list, node, lib_global))
    {
      if (strcmp (lib_global->name, name) == 0)
	return lib_global;
    }
  return NULL;
}


static struct lib_global *
lib_global_get (const char *name)
{
  struct lib_global *lib_global;

  lib_global = lib_global_lookup (name);

  if (lib_global)
    return lib_global;

  lib_global = lib_global_new ();
  lib_global->name = strdup (name);
  lib_global->key = list_new ();
  lib_global->key->cmp = (int (*)(void *, void *)) key_cmp_func;
  lib_global->key->del = (void (*)(void *)) key_delete_func;
  listnode_add (lib_global_list, lib_global);

  return lib_global;
}

static void
lib_global_delete (struct lib_global *lib_global)
{
  if (lib_global->name)
    free (lib_global->name);

  list_delete (lib_global->key);
  listnode_delete (lib_global_list, lib_global);
  lib_global_free (lib_global);
}
*/