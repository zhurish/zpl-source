/*
 * Process id output.
 * Copyright (C) 1998, 1999 Kunihiro Ishiguro
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


#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"

#include "daemon.h"
#include "log.h"

#define PIDFILE_MASK 0644
#if 1//ndef HAVE_FCNTL

pid_t
pid_output (const char *path)
{
  FILE *fp;
  pid_t pid;
  mode_t oldumask;

  pid = getpid();

  oldumask = umask(0777 & ~PIDFILE_MASK);
  fp = fopen (path, "w");
  if (fp != NULL) 
    {
      fprintf (fp, "%d\n", (int) pid);
      fclose (fp);
      umask(oldumask);
      return pid;
    }
  /* XXX Why do we continue instead of exiting?  This seems incompatible
     with the behavior of the fcntl version below. */
  zlog_warn(MODULE_DEFAULT, "Can't fopen pid lock file %s (%s), continuing",
	    path, ipstack_strerror(ipstack_errno));
  umask(oldumask);
  return -1;
}


pid_t
pid_input (const char *path)
{
  FILE *fp;
  pid_t pid;
  mode_t oldumask;

  pid = getpid();

  oldumask = umask(0777 & ~PIDFILE_MASK);
  fp = fopen (path, "r");
  if (fp != NULL)
    {
      fscanf (fp, "%d", &pid);
      fclose (fp);
      umask(oldumask);
      return pid;
    }
  /* XXX Why do we continue instead of exiting?  This seems incompatible
     with the behavior of the fcntl version below. */
  zlog_warn(MODULE_DEFAULT, "Can't fopen pid lock file %s (%s), continuing",
	    path, ipstack_strerror(ipstack_errno));
  umask(oldumask);
  return -1;
}
#else /* HAVE_FCNTL */

pid_t
pid_output (const char *path)
{
  int tmp;
  int fd;
  pid_t pid;
  zpl_char buf[16];
  struct flock lock;  
  mode_t oldumask;

  pid = getpid ();

  oldumask = umask(0777 & ~PIDFILE_MASK);
  fd = open (path, O_RDWR | O_CREAT, PIDFILE_MASK);
  if (fd < 0)
    {
      zlog_err(MODULE_DEFAULT, "Can't create pid lock file %s (%s), exiting",
	       path, ipstack_strerror(ipstack_errno));
      umask(oldumask);
      exit(1);
    }
  else
    {
      zpl_size_t pidsize;

      umask(oldumask);
      memset (&lock, 0, sizeof(lock));

      lock.l_type = F_WRLCK;
      lock.l_whence = SEEK_SET;

      if (fcntl(fd, F_SETLK, &lock) < 0)
        {
          fprintf(stderr,"Could not write pid %d to pid_file (%s)\r\n",pid,strerror(ipstack_errno));
          zlog_err(MODULE_DEFAULT, "Could not lock pid_file %s, exiting", path);
          exit(1);
        }

      sprintf (buf, "%d\n", (int) pid);
      pidsize = strlen(buf);
      if ((tmp = write (fd, buf, pidsize)) != (int)pidsize)
        zlog_err(MODULE_DEFAULT, "Could not write pid %d to pid_file %s, rc was %d: %s",
	         (int)pid,path,tmp,ipstack_strerror(ipstack_errno));
      else if (ftruncate(fd, pidsize) < 0)
        zlog_err(MODULE_DEFAULT, "Could not truncate pid_file %s to %u bytes: %s",
	         path,(zpl_uint32)pidsize,ipstack_strerror(ipstack_errno));
    }
  return pid;
}

#endif /* HAVE_FCNTL */
