/*
 * ssh_keymgt.c
 *
 *  Created on: Nov 3, 2018
 *      Author: zhurish
 */
#include "ssh_def.h"
#include "ssh_api.h"
#include "ssh_util.h"
#include "os_url.h"
#include "vty_include.h"
#include <libssh/libssh.h>
#include <libssh/sftp.h>


static int ssh_do_sftp(ssh_session session, zpl_bool download, ssh_sftp_connect *src, char *localfile)
{
    sftp_session sftp=sftp_new(session);
    sftp_file fichier = NULL;
    zpl_uint32 len = 0;
    char data[2048]={0};

    if(!sftp){
       ssh_printf(session, "sftp error initialising channel: %s\n",
            ssh_get_error(session));
        return -1;
    }
    if(sftp_init(sftp)){
    	sftp_free(sftp);
        ssh_printf(session, "error initialising sftp: %s\n",
            ssh_get_error(session));
        return -1;
    }
    src->file = fopen(localfile, download ? "w+" : "r");
    if(!src->file){
    	sftp_free(sftp);
        ssh_printf(session, "sftp error initialising local file: %s\n", localfile);
        return -1;
    }
    /* this will open a file and copy it into your /home directory */
    /* the small buffer size was intended to stress the library. of course, you can use a buffer till 20kbytes without problem */
    if(download)
    {
		fichier = sftp_open(sftp, src->path, O_RDONLY, 0);
		if(!fichier)
		{
			sftp_free(sftp);
			fclose(src->file);
			remove(localfile);
			ssh_printf(session, "Error opening /usr/bin/ssh: %s\n",
				ssh_get_error(session));
			return -1;
		}
		while((len = sftp_read(fichier, data, sizeof(data))) > 0)
		{
			if(fwrite(data, len, 1, src->file) != len)
			{
				sftp_close(fichier);
				sftp_free(sftp);
				fclose(src->file);
				remove(localfile);
				ssh_printf(session, "Error writing %d bytes: %s\n",
					len, ssh_get_error(session));
				return -1;
			}
		}
    }
    else
    {
		/* open a file for writing... */
		fichier = sftp_open(sftp, src->path, O_WRONLY | O_CREAT, 0644);
		if(!fichier)
		{
			sftp_free(sftp);
			fclose(src->file);
			ssh_printf(session, "Error opening /usr/bin/ssh: %s\n",
				ssh_get_error(session));
			return -1;
		}
		while((len = fread(data, sizeof(data), 1, src->file)) > 0)
		{
			if(sftp_write(fichier, data, len) != len)
			{
				sftp_close(fichier);
				sftp_free(sftp);
				fclose(src->file);
				ssh_printf(session, "Error writing %d bytes: %s\n",
					len, ssh_get_error(session));
				return -1;
			}
		}
    }
    ssh_printf(session, "finished\n");
    /* close the sftp session */
	sftp_close(fichier);
	sftp_free(sftp);
	fclose(src->file);
    return 0;
}


int sftp_action(struct vty *vty, zpl_bool download, char *url, char *localfile)
{
	ssh_sftp_connect src;
	int ret = 0;
	os_url_t spliurl;
	memset(&spliurl, 0, sizeof(os_url_t));
	memset(&src, 0, sizeof(ssh_sftp_connect));

	if (os_url_split(url, &spliurl) != OK)
	{
		os_url_free(&spliurl);
		return -1;
	}
	ssh_url_setup(&src, &spliurl);

	src.session = ssh_connect_api(vty, src.host, src.port, src.user, src.password);
	if (!src.session)
	{
		ssh_printf(src.session, "Couldn't connect to %s\n", src.host);
		os_url_free(&spliurl);
		return -1;
	}
	ret = ssh_do_sftp(src.session, download, &src, localfile);

	ssh_disconnect(src.session);
	ssh_free(src.session);
	os_url_free(&spliurl);
	return ret;
}
