/*
 * ssh_scp.c
 *
 *  Created on: Oct 31, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "getopt.h"
#include <log.h>
#include "command.h"
#include "memory.h"
#include "prefix.h"
#include "network.h"
#include "vty.h"
#include "buffer.h"
#include "host.h"
#include "eloop.h"

#include "ssh_api.h"
#include "ssh_util.h"


struct ssh_scp_connect
{
  BOOL 			is_ssh;
  char 			*user;
  char 			*host;
  int			port;
  char 			*path;
  char 			*password;
  ssh_session 	session;
  ssh_scp 		scp;
  FILE 			*file;
};

/*
 * scp -P 9225 root@183.63.84.114:/root/ipran_u3-w.tat.bz ./
 * scp ipran_u3-w.tat.bz -P 9225 root@183.63.84.114:/root/ipran_u3-w.tat.bz
 */

static int ssh_scp_connect_create(struct vty *vty, struct ssh_scp_connect *loc, BOOL write)
{
	if (loc->is_ssh && write)
	{
		/*
		 * upload
		 */
		loc->session = ssh_connect_api(vty, loc->host, loc->port, loc->user, loc->password);
		if (!loc->session)
		{
			fprintf(ssh_stderr, "Couldn't connect to %s\n", loc->host);
			return -1;
		}
		loc->scp = ssh_scp_new(loc->session, SSH_SCP_WRITE, loc->path);
		if (!loc->scp)
		{
			ssh_disconnect(loc->session);
			ssh_printf(loc->session, "error : %s\n", ssh_get_error(loc->session));
			return -1;
		}
		if (ssh_scp_init(loc->scp) == SSH_ERROR)
		{
			ssh_disconnect(loc->session);
			ssh_printf(loc->session, "error : %s\n", ssh_get_error(loc->session));
			ssh_scp_free(loc->scp);
			loc->scp = NULL;
			return -1;
		}
		return 0;
	}
	else if(loc->is_ssh)
	{
		/*
		 * download
		 */
		loc->session = ssh_connect_api(vty, loc->host, loc->port, loc->user, loc->password);
		if (!loc->session)
		{
			fprintf(ssh_stderr, "Couldn't connect to %s\n", loc->host);
			return -1;
		}
		loc->scp = ssh_scp_new(loc->session, SSH_SCP_READ, loc->path);
		if (!loc->scp)
		{
			ssh_disconnect(loc->session);
			ssh_printf(loc->session, "error : %s\n", ssh_get_error(loc->session));
			return -1;
		}
		if (ssh_scp_init(loc->scp) == SSH_ERROR)
		{
			ssh_disconnect(loc->session);
			ssh_printf(loc->session, "error : %s\n", ssh_get_error(loc->session));
			ssh_scp_free(loc->scp);
			loc->scp = NULL;
			return -1;
		}
		return 0;
	}
	else
	{
		loc->file = fopen(loc->path, write ? "r" : "w");
		if (!loc->file)
		{
	    	if(errno==EISDIR){
	    		if(chdir(loc->path)){
	    			fprintf(ssh_stderr,"Error changing directory to %s: %s\n",loc->path,strerror(errno));
	    			return -1;
	    		}
	    		return 0;
	    	}
	    	fprintf(ssh_stderr,"Error opening %s: %s\n",loc->path,strerror(errno));
	    	return -1;
		}
	    return 0;
	}
	return -1;
}


static int do_ssh_copy_cmd(struct vty *vty, struct ssh_scp_connect *src, struct ssh_scp_connect *dest, BOOL write)
{
	int size = 0;
	socket_t fd = 0;
	struct stat s;
	int w = 0, r = 0;
	char buffer[16384];
	int total = 0;
	int mode = 0;
	char *filename = NULL;
	/* recursive mode doesn't work yet */
	//(void) recursive;
	/* Get the file name and size*/
	if (!src->is_ssh)
	{
		fd = fileno(src->file);
		if (fd < 0)
		{
			fprintf(ssh_stderr, "Invalid file pointer, error: %s\n",
					strerror(errno));
			return -1;
		}
		r = fstat(fd, &s);
		if (r < 0)
		{
			return -1;
		}
		size = s.st_size;
		mode = s.st_mode & ~S_IFMT;
		filename = ssh_basename(src->path);
	}
	else
	{
		size = 0;
		do
		{
			r = ssh_scp_pull_request(src->scp);
			if (r == SSH_SCP_REQUEST_NEWDIR)
			{
				ssh_scp_deny_request(src->scp, "Not in recursive mode");
				continue;
			}
			if (r == SSH_SCP_REQUEST_NEWFILE)
			{
				size = ssh_scp_request_get_size(src->scp);
				filename = strdup(ssh_scp_request_get_filename(src->scp));
				mode = ssh_scp_request_get_permissions(src->scp);
				//ssh_scp_accept_request(src->scp);
				break;
			}
			if (r == SSH_ERROR)
			{
				fprintf(ssh_stderr, "Error: %s\n", ssh_get_error(src->session));
				ssh_string_free_char(filename);
				return -1;
			}
		} while (r != SSH_SCP_REQUEST_NEWFILE);
	}

	if (dest->is_ssh)
	{
		r = ssh_scp_push_file(dest->scp, src->path, size, mode);
		//  snprintf(buffer,sizeof(buffer),"C0644 %d %s\n",size,src->path);
		if (r == SSH_ERROR)
		{
			fprintf(ssh_stderr, "error: %s\n", ssh_get_error(dest->session));
			ssh_string_free_char(filename);
			//ssh_scp_free(dest->scp);
			//dest->scp = NULL;
			return -1;
		}
	}
	else
	{
		if (!dest->file)
		{
			dest->file = fopen(filename, "w");
			if (!dest->file)
			{
				fprintf(ssh_stderr, "Cannot open %s for writing: %s\n", filename,
						strerror(errno));
				if (src->is_ssh)
					ssh_scp_deny_request(src->scp, "Cannot open local file");
				ssh_string_free_char(filename);
				return -1;
			}
		}
		if (src->is_ssh)
		{
			ssh_scp_accept_request(src->scp);
		}
	}
	do
	{
		if (src->is_ssh)
		{
			r = ssh_scp_read(src->scp, buffer, sizeof(buffer));
			if (r == SSH_ERROR)
			{
				fprintf(ssh_stderr, "Error reading scp: %s\n",
						ssh_get_error(src->session));
				ssh_string_free_char(filename);
				return -1;
			}
			if (r == 0)
				break;
		}
		else
		{
			r = fread(buffer, 1, sizeof(buffer), src->file);
			if (r == 0)
				break;
			if (r < 0)
			{
				fprintf(ssh_stderr, "Error reading file: %s\n", strerror(errno));
				ssh_string_free_char(filename);
				return -1;
			}
		}
		if (dest->is_ssh)
		{
			w = ssh_scp_write(dest->scp, buffer, r);
			if (w == SSH_ERROR)
			{
				fprintf(ssh_stderr, "Error writing in scp: %s\n",
						ssh_get_error(dest->session));
				//ssh_scp_free(dest->scp);
				//dest->scp = NULL;
				ssh_string_free_char(filename);
				return -1;
			}
		}
		else
		{
			w = fwrite(buffer, r, 1, dest->file);
			if (w <= 0)
			{
				fprintf(ssh_stderr, "Error writing in local file: %s\n",
						strerror(errno));
				ssh_string_free_char(filename);
				return -1;
			}
		}
		total += r;

	} while (total < size);
	ssh_string_free_char(filename);
	printf("wrote %d bytes\n", total);
	return 0;
}

static int do_ssh_url_setup(struct ssh_scp_connect *dest,  os_url_t *spliurl)
{
	dest->is_ssh = TRUE;
	dest->user = spliurl->user;
	dest->host = spliurl->host;
	dest->port = spliurl->port;
	dest->path = spliurl->filename;
	dest->password = spliurl->pass;
	dest->session = NULL;
	dest->scp = NULL;
	dest->file = NULL;
/*
	spliurl->port;
	spliurl->path;
*/
	return OK;
}

/*
 * copy url-string scp://zhurish:centos@127.0.0.1//tmp/modem-usb.info /tmp/ab
 * copy url-string scp://zhurish:centos@1.1.1.2//tmp/modem-usb.info /tmp/ab
 */
int do_ssh_copy(struct vty *vty, BOOL download, char *url, char *localfile)
{
  struct ssh_scp_connect dest, src;
  int r = 0;

  os_url_t spliurl;
  memset(&spliurl, 0, sizeof(os_url_t));
  memset(&dest, 0, sizeof(struct ssh_scp_connect));
  memset(&src, 0, sizeof(struct ssh_scp_connect));

  if(os_url_split(url, &spliurl) != OK)
  {
	  os_url_free(&spliurl);
	  return -1;
  }

  ssh_init();

  if(download)
  {
	  dest.is_ssh = FALSE;
	  dest.path = strdup(localfile);
	  r = ssh_scp_connect_create(vty, &dest, FALSE);
	  if(r != 0)
	  {
		  os_url_free(&spliurl);
		  return -1;
	  }
	  src.is_ssh = TRUE;
	  do_ssh_url_setup(&src, &spliurl);
	  r = ssh_scp_connect_create(vty, &src, FALSE);
	  if(r != 0)
	  {
		  if(dest.path)
			  free(dest.path);
		  os_url_free(&spliurl);
		  return -1;
	  }
  }
  else
  {
	  dest.is_ssh = TRUE;
	  do_ssh_url_setup(&dest, &spliurl);
	  r = ssh_scp_connect_create(vty, &dest, TRUE);
	  if(r != 0)
	  {
		  os_url_free(&spliurl);
		  return -1;
	  }
	  src.path = strdup(localfile);
	  src.is_ssh = FALSE;
	  r = ssh_scp_connect_create(vty, &src, TRUE);
	  if(r != 0)
	  {
		  os_url_free(&spliurl);
		  if(src.path)
			  free(src.path);
		  return -1;
	  }
  }

  r = do_ssh_copy_cmd(vty, &src, &dest, download ? TRUE:FALSE);

  if (dest.is_ssh && dest.scp != NULL)
  {
	  if(ssh_scp_close(dest.scp) == SSH_ERROR)
	  {
		  fprintf(ssh_stderr,"Error closing scp: %s\n",ssh_get_error(dest.session));
		  ssh_scp_free(dest.scp);
		  dest.scp=NULL;
	  }
  }
  else
  {
	  fclose(dest.file);
	  dest.file=NULL;
  }

  if (src.is_ssh && src.scp != NULL)
  {
	  if(ssh_scp_close(src.scp) == SSH_ERROR)
	  {
		  fprintf(ssh_stderr,"Error closing scp: %s\n",ssh_get_error(src.session));
		  ssh_scp_free(src.scp);
		  src.scp=NULL;
	  }
  }
  else
  {
	  fclose(src.file);
	  src.file=NULL;
  }
  os_url_free(&spliurl);
  if(dest.path)
	  free(dest.path);
  if(src.path)
	  free(src.path);
  ssh_disconnect(dest.session);
  //ssh_finalize();
  return r;
}
