/*
 * ssh_scp.c
 *
 *  Created on: Oct 31, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"

#include "libssh_autoconfig.h"
#include "libssh/priv.h"
#include "libssh/scp.h"
#include "ssh_api.h"
#include "ssh_util.h"
#include "sshd_main.h"

/*
 * scp -P 9225 root@183.63.84.114:/root/ipran_u3-w.tat.bz ./
 * scp ipran_u3-w.tat.bz -P 9225 root@183.63.84.114:/root/ipran_u3-w.tat.bz
 */

int ssh_url_setup(struct ssh_scp_connect *dest,  os_url_t *spliurl)
{
	dest->is_ssh = zpl_true;
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


static int ssh_read_local_file(ssh_session session, ssh_scp scp, struct ssh_scp_connect *src)
{
	zpl_uint32 filesize = 0;
	char buffer[2048];
	zpl_uint32 r = 0, total = 0, w = 0;
	int fd = 0;
	struct stat s;
	/* Get the file name and size*/
	if (src->file)
	{
		fd = fileno(src->file);
		if (fd < 0)
		{
			ssh_printf(NULL, "Invalid file pointer, error: %s\n",
					strerror(errno));
			return -1;
		}
		r = fstat(fd, &s);
		if (r < 0)
		{
			return -1;
		}
		filesize = s.st_size;
		r = ssh_scp_push_file(scp, src->path, filesize, s.st_mode & ~S_IFMT);
		if (r == SSH_ERROR)
		{
			ssh_printf(NULL, "error: %s\n", ssh_get_error(session));
			return -1;
		}
		ssh_printf(NULL,"upload size %d\n", filesize);
	}
	do
	{
		r = fread(buffer, sizeof(buffer), 1, src->file);
		if (r == 0)
			break;
		if (r < 0)
		{
			ssh_printf(NULL, "Error reading file: %s\n", strerror(errno));
			return -1;
		}

		w = ssh_scp_write(scp, buffer, r);
		if (w == SSH_ERROR)
		{
			ssh_printf(NULL, "Error writing in scp: %s\n",
					ssh_get_error(session));
			return -1;
		}
		total += r;

	} while (total < filesize);
	fclose(src->file);
	return OK;
}


static int ssh_write_local_file(ssh_session session, ssh_scp scp, struct ssh_scp_connect *src, zpl_uint32 filesize)
{
	char buffer[2048];
	zpl_uint32 r= 0, total = 0, w = 0;
	do
	{
		r = ssh_scp_read(scp, buffer, sizeof(buffer));
		if (r == SSH_ERROR)
		{
			ssh_printf(NULL, "Error reading scp: %s\n", ssh_get_error(session));
			return -1;
		}
		if (r == 0)
			return -1;
		else
		{
			w = fwrite(buffer, r, 1, src->file);
			if (w <= 0)
			{
				ssh_printf(NULL, "Error writing in local file: %s\n", strerror(errno));
				return -1;
			}
		}
		total += r;

	} while (total < filesize);

	fflush(src->file);
	fclose(src->file);
	return OK;
}


static int ssh_upload_files(ssh_session session, struct ssh_scp_connect *src, char *localfile)
{
	//int size = 0;
	//char buffer[2048];
	//int mode = 0;
	//char *filename;
	//int r= 0, total = 0, w = 0;
	ssh_scp scp = ssh_scp_new(session, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, src->path);
	if (ssh_scp_init(scp) != SSH_OK)
	{
		ssh_printf(NULL, "error initializing scp: %s\n", ssh_get_error(session));
		ssh_scp_free(scp);
		return -1;
	}
	ssh_printf(NULL, "Trying to upload files '%s'\n", src->path);

	if(!src->file)
	{
		src->file = fopen(localfile, "r");
	}
	if(!src->file)
	{
		ssh_scp_close(scp);
		ssh_scp_free(scp);
		return -1;
	}
	if(ssh_read_local_file(session, scp, src) != OK)
	{
		ssh_scp_close(scp);
		ssh_scp_free(scp);
		return -1;
	}
	ssh_printf(NULL, "done\n");

	ssh_scp_close(scp);
	ssh_scp_free(scp);
	return 0;
}

static int ssh_download_files(ssh_session session, struct ssh_scp_connect *src, char *localfile)
{
	zpl_uint32 size = 0;
	//char buffer[2048];
	zpl_uint32 mode = 0;
	char *filename;
	int r= 0;//, total = 0, w = 0;
	ssh_scp scp = ssh_scp_new(session, SSH_SCP_READ | SSH_SCP_RECURSIVE, src->path);
	if (ssh_scp_init(scp) != SSH_OK)
	{
		ssh_printf(NULL, "error initializing scp: %s\n", ssh_get_error(session));
		ssh_scp_free(scp);
		return -1;
	}
	ssh_printf(NULL, "Trying to download files '%s'\n", src->path);
	do
	{
		r = ssh_scp_pull_request(scp);
		switch (r)
		{
		case SSH_SCP_REQUEST_NEWFILE:
			size = ssh_scp_request_get_size(scp);
			filename = strdup(ssh_scp_request_get_filename(scp));
			mode = ssh_scp_request_get_permissions(scp);
			ssh_printf(NULL,"downloading file %s, size %d\n", filename, size);
			ssh_scp_accept_request(scp);
			if(!src->file)
			{
				src->file = fopen(localfile, "w+");
			}
			free(filename);
			if(!src->file)
			{
				ssh_scp_close(scp);
				ssh_scp_free(scp);
				return -1;
			}
			if(ssh_write_local_file(session, scp, src, size) != OK)
			{
				ssh_scp_close(scp);
				ssh_scp_free(scp);
				return -1;
			}
			ssh_printf(NULL, "done\n");
			break;
		case SSH_ERROR:
			ssh_printf(NULL, "Error: %s\n", ssh_get_error(session));
			ssh_scp_close(scp);
			ssh_scp_free(scp);
			return -1;
		case SSH_SCP_REQUEST_WARNING:
			ssh_printf(NULL, "Warning: %s\n", ssh_scp_request_get_warning(scp));
			break;
		case SSH_SCP_REQUEST_NEWDIR:
			filename = strdup(ssh_scp_request_get_filename(scp));
			mode = ssh_scp_request_get_permissions(scp);
			mkdir(filename, mode);
			ssh_printf(NULL,"downloading directory %s\n", filename);
			free(filename);
			ssh_scp_accept_request(scp);
			break;
		case SSH_SCP_REQUEST_ENDDIR:
			//ssh_printf(NULL,"End of directory\n");
			break;
		case SSH_SCP_REQUEST_EOF:
			//ssh_printf(NULL,"End of requests\n");
			goto end;
		}
	} while (1);
end:
	ssh_scp_close(scp);
	ssh_scp_free(scp);
	return 0;
}

int ssh_scp_upload(struct vty *vty, zpl_bool download, char *url, char *localfile)
{
	struct ssh_scp_connect src;
	int ret = 0;
	os_url_t spliurl;
	memset(&spliurl, 0, sizeof(os_url_t));
	memset(&src, 0, sizeof(struct ssh_scp_connect));

	if (os_url_split(url, &spliurl) != OK)
	{
		os_url_free(&spliurl);
		return -1;
	}
	ssh_url_setup(&src, &spliurl);
	ssh_stdout_set(vty);

	src.session = ssh_connect_api(vty, src.host, src.port, src.user, src.password);
	if (!src.session)
	{
		ssh_printf(NULL, "Couldn't connect to %s\n", src.host);
		ssh_stdout_set(NULL);
		os_url_free(&spliurl);
		return -1;
	}
	ret = ssh_upload_files(src.session, &src, localfile);

	ssh_disconnect(src.session);
	ssh_free(src.session);
	ssh_stdout_set(NULL);
	os_url_free(&spliurl);
	return ret;
}

int ssh_scp_download(struct vty *vty, zpl_bool download, char *url, char *localfile)
{
	struct ssh_scp_connect src;
	int ret = 0;
	os_url_t spliurl;
	memset(&spliurl, 0, sizeof(os_url_t));
	memset(&src, 0, sizeof(struct ssh_scp_connect));

	if (os_url_split(url, &spliurl) != OK)
	{
		os_url_free(&spliurl);
		return -1;
	}
	ssh_url_setup(&src, &spliurl);
	ssh_stdout_set(vty);

	src.session = ssh_connect_api(vty, src.host, src.port, src.user, src.password);
	if (!src.session)
	{
		ssh_printf(NULL, "Couldn't connect to %s\n", src.host);
		ssh_stdout_set(NULL);
		os_url_free(&spliurl);
		return -1;
	}
	ret = ssh_download_files(src.session, &src, localfile);

	ssh_disconnect(src.session);
	ssh_free(src.session);
	ssh_stdout_set(NULL);
	os_url_free(&spliurl);
	return ret;
}




#ifdef SSH_SCPD_ENABLE
/*
 * scp server
 */
static int ssh_scpd_thread(sshd_client_t *sshclient);

static int ssh_scpd_output(socket_t fd, zpl_uint32 revents, void *userdata)
{
    char buf[4096];
    int n = -1, ret = 0;
    os_bzero(buf, sizeof(buf));
    sshd_client_t * client = (ssh_channel) userdata;
    //ssh_channel channel = (ssh_channel) userdata;
    if ((revents & POLLIN) != 0) {
        n = read(fd, buf, sizeof(buf));
        if (n > 0) {
        	if(client != NULL && client->channel)
        		ret = ssh_channel_write(client->channel, buf, n);
        	if(ret < 0)
        		return SSH_ERROR;
        	else if (ret == 0)
        		return SSH_ERROR;
        	return ret;
        }
        if(n <= 0)
        {
        	//ssh_event_remove_fd(fd);
        	if(client->config->event)
        		ssh_event_remove_fd(client->config->event, fd);
        	close(fd);
        	client->sock = 0;
        	return -1;
        }
    }
    return n;
}

int ssh_scpd_init(sshd_client_t *sshclient, char *cmd)
{
	int socket[2] = { 0, 0 };

	if (!sshclient)
		return SSH_ERROR;

	ssh_scp scp = malloc(sizeof(struct ssh_scp_struct));
	if (scp == NULL)
	{
		ssh_printf(NULL,
				"Error allocating memory for ssh_scp\n");
		return NULL;
	}
	ZERO_STRUCTP(scp);

	scp->channel = NULL;
	scp->state = SSH_SCP_NEW;

	if (socketpair(AF_UNIX, IPSTACK_SOCK_STREAM, 0, socket) == 0)
	{
		sshclient->sock = socket[1];
	}
	else
	{
		free(scp);
		return SSH_ERROR;
	}
	sshclient->scp_data.scp = scp;
	sshclient->scp_data.input = socket[0];
	sshclient->scp_data.output = socket[0];

	if(strstr(cmd, "-f"))
		scp->mode = SSH_SCP_WRITE;
	else
		scp->mode = SSH_SCP_READ ;

	if (scp->mode == SSH_SCP_WRITE)
		scp->state = SSH_SCP_WRITE_INITED;
	else
		scp->state = SSH_SCP_READ_INITED;

	ssh_event_add_fd(sshclient->config->event, sshclient->sock, POLLIN, ssh_scpd_output,
			sshclient);

	sshclient->scp_data.filename = strdup(strstr(cmd, "/"));
	ssh_printf(NULL,"%s :%s\n", __func__, sshclient->scp_data.filename);
	os_job_add(ssh_scpd_thread, sshclient);
	return SSH_OK;
}

int ssh_scpd_exit(sshd_client_t *sshclient)
{
	os_job_del(ssh_scpd_thread, sshclient);
	return OK;
}


static int _ssh_scpd_waiting_respone(sshd_client_t *sshclient)
{
	zpl_uint8 code = -1;
	int ret = 0;
	ret = read(sshclient->scp_data.input, &code, 1);
	if(ret == 1 && code == 0)
	{
		return OK;
	}
	return ERROR;
}

static int _ssh_scpd_read_buffer(sshd_client_t *sshclient, char *buf, zpl_uint32 len)
{
	int ret = 0;
	ret = read(sshclient->scp_data.input, buf, len);
	if(ret >= 0)
	{
		return ret;
	}
	return ERROR;
}

static int _ssh_scpd_write_buffer(sshd_client_t *sshclient, char *buf, zpl_uint32 len)
{
	zpl_uint8 code = -1;
	int ret = 0;
	ret = write(sshclient->scp_data.output, buf, len);
	if(ret >= 0)
	{
		return ret;
	}
	return ERROR;
}

static int ssh_scpd_write(sshd_client_t *sshclient, const void *buffer, zpl_uint32 len)
{
  int w;
  int r;
  zpl_uint8 code;
  ssh_scp scp = sshclient->scp_data.scp;
  if(scp==NULL)
      return SSH_ERROR;
  if(scp->state != SSH_SCP_WRITE_WRITING){
    ssh_printf(NULL,"ssh_scp_write called under invalid state\n");
    return SSH_ERROR;
  }
  if(scp->processed + len > scp->filelen)
    len = (size_t) (scp->filelen - scp->processed);

  w=_ssh_scpd_write_buffer(sshclient, buffer, len);
  if(w != SSH_ERROR)
    scp->processed += w;
  else {
    scp->state=SSH_SCP_ERROR;

    return SSH_ERROR;
  }
    r = _ssh_scpd_read_buffer(sshclient, &code, 1);
    if(r == SSH_ERROR){
      return SSH_ERROR;
    }
    if(code == 1 || code == 2){
      ssh_printf(NULL, "SCP: Error: status code %i received\n", code);
      return SSH_ERROR;
    }
  /* Check if we arrived at end of file */
  if(scp->processed == scp->filelen) {
    code = 0;
    w = _ssh_scpd_write_buffer(sshclient, &code, 1);
    if(w == SSH_ERROR){
      scp->state = SSH_SCP_ERROR;
      return SSH_ERROR;
    }
    scp->processed=scp->filelen=0;
    scp->state=SSH_SCP_WRITE_INITED;
  }
  return SSH_OK;
}

static int ssh_scpd_start_file(sshd_client_t *sshclient, const char *filename, zpl_uint32 size, zpl_uint32 mode)
{
  char buffer[1024];
  int r;
  zpl_uint8 code;
  char *file;
  char *perms;
  ssh_scp scp = sshclient->scp_data.scp;
  if(scp==NULL)
      return SSH_ERROR;
  if(scp->state != SSH_SCP_WRITE_INITED){
	ssh_printf(NULL,"ssh_scp_push_file called under invalid state\n");
    return SSH_ERROR;
  }
  file=ssh_basename(filename);
  perms=ssh_scp_string_mode(mode);
  //SSH_LOG(SSH_LOG_PROTOCOL,"SCP pushing file %s, size %" PRIu64 " with permissions '%s'",file,size,perms);
  snprintf(buffer, sizeof(buffer), "C%s %" PRIu64 " %s\n", perms, size, file);
  SAFE_FREE(file);
  SAFE_FREE(perms);
  r=_ssh_scpd_write_buffer(sshclient,buffer,strlen(buffer));
  if(r==SSH_ERROR){
    scp->state=SSH_SCP_ERROR;
    return SSH_ERROR;
  }
  r=_ssh_scpd_read_buffer(sshclient, &code, 1);
  if(r<=0){
	ssh_printf(NULL, "Error reading status code: %s\n",ssh_get_error(scp->session));
    scp->state=SSH_SCP_ERROR;
    return SSH_ERROR;
  }
  if(code != 0){
	ssh_printf(NULL, "scp status code %ud not valid\n", code);
    scp->state=SSH_SCP_ERROR;
    return SSH_ERROR;
  }
  scp->filelen = size;
  scp->processed = 0;
  scp->state = SSH_SCP_WRITE_WRITING;
  return SSH_OK;
}


static int ssh_scpd_close(sshd_client_t *sshclient)
{
	int n = 0;
    zlog_debug(MODULE_UTILS, "%s :", __func__);

    ssh_event_remove_session(sshclient->config->event, sshclient->session);

	//ssh_channel_send_eof(sshclient->sdata.channel);
	ssh_channel_close(sshclient->channel);

    ssh_disconnect(sshclient->session);
    ssh_free(sshclient->session);

	close(sshclient->scp_data.input);
	//close(sshclient->scp_data.output);
	free(sshclient->scp_data.scp);
	if(sshclient->scp_data.filename)
		free(sshclient->scp_data.filename);
    XFREE(MTYPE_SSH_CLIENT, sshclient);
    sshclient = NULL;
    return OK;
}

static int ssh_scpd_thread(sshd_client_t *sshclient)
{
	int ret = 0;
	int fd = 0;
	ssh_printf(NULL,"%s :%s\n", __func__, sshclient->scp_data.filename);

	if(_ssh_scpd_waiting_respone(sshclient) == OK)
	{
		struct stat s;
		fd = open(sshclient->scp_data.filename, O_RDONLY);
		if (fd < 0)
		{
			close(fd);
			ssh_scpd_close(sshclient);
			return SSH_ERROR;
		}
		ret = fstat(fd, &s);
		if (ret < 0)
		{
			close(fd);
			ssh_scpd_close(sshclient);
			return SSH_ERROR;
		}
		sshclient->scp_data.scp->filelen = s.st_size;
		sshclient->scp_data.scp->processed = 0;
		ret = ssh_scpd_start_file(sshclient, sshclient->scp_data.filename, s.st_size, s.st_mode & ~S_IFMT);
		if(ret == SSH_OK)
		{
			char buffer[2048];
			int r = 0, total = 0, w = 0;
			do
			{
				r = read(fd, buffer, sizeof(buffer));
				if (r == 0)
					break;
				if (r < 0)
				{
					close(fd);
					ssh_scpd_close(sshclient);
					ssh_printf(NULL, "Error reading file: %s\n", strerror(errno));
					return SSH_ERROR;
				}
				w = ssh_scpd_write(sshclient, buffer, r);
				if (w == SSH_ERROR)
				{
					close(fd);
					ssh_scpd_close(sshclient);
					ssh_printf(NULL, "Error writing in scp\n");
					return SSH_ERROR;
				}
				total += r;

			} while (total < s.st_size);
			close(fd);
			close(sshclient->scp_data.input);
			free(sshclient->scp_data.scp);
			if(sshclient->scp_data.filename)
				free(sshclient->scp_data.filename);
			//ssh_scpd_close(sshclient);
			ssh_printf(NULL, "finsh writing in scp\n");
			return SSH_OK;
		}
		close(fd);
		ssh_scpd_close(sshclient);
		return SSH_ERROR;
	}
	ssh_scpd_close(sshclient);
	return SSH_ERROR;
}
#endif
/*
2018/08/12 16:29:04 UTILS: SSH ssh_message_handle_channel_request: Received a exec channel_request for channel (43:0) ()
2018/08/12 16:29:04 UTILS: sshd_exec_request : scp -f /tmp/resolv.conf
2018/08/12 16:29:04 UTILS: data_function :(0->1)
2018/08/12 16:29:04 UTILS: process_stdout :C0644 32 resolv.conf
� r�� r�萳�w(21)
2018/08/12 16:29:04 UTILS: data_function :(0->1)
2018/08/12 16:29:04 UTILS: process_stdout :search lan
nameserver 127.0.0.1
(33)
2018/08/12 16:29:04 UTILS: data_function :(0->1)
2018/08/12 16:29:04 UTILS: process_stdout : (-1)
2018/08/12 16:29:04 UTILS: process_stderr : (-1)
*/
