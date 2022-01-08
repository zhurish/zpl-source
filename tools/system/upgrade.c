/*
 * upgrade.c
 *
 *  Created on: Aug 11, 2018
 *      Author: zhurish
 */
#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "getopt.h"
#include "upgrade.h"


/*
 * upgrade -p 1.1.1.1 -f xxx.tar.gz -r
 * upgrade -f xxx.tar.gz -r reboot
 */
static const struct option longopts[] = {
	{"server",     required_argument,  NULL, 's'},
	{"file",       required_argument,  NULL, 'f'},
	{"reboot",     no_argument,        NULL, 'r'},
	{"help",       no_argument,        NULL, 'h'},
	{NULL,         0,                  NULL, 0}
};


static void _usage (char *progname, int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
  else
    {
      printf ("Usage : %s [OPTION...]\n\n" \
	      "-s, --server     tftp server address\n"\
	      "-f, --file  		file name\n"\
	      "-r, --reboot    	Set reboot flag\n"\
	      "-h, --help       Display this help and exit\n"\
	      "\n", progname);
    }
  exit (status);
}

int main(int argc, char *argv[])
{
	char *p = NULL;
	char *progname = NULL;
	int reboot_flag = 0;
	char *server = NULL;
	char *file = NULL;
	char cmd[1024];
	char buf[1024];
	progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);
	while (1)
	{
		int opt;
		opt = getopt_long(argc, argv, "s:f:rh", longopts, 0);
		if (opt == EOF)
			break;

		switch (opt)
		{
		case 0:
			break;
		case 's':
			server = optarg;
			break;

		case 'f':
			file = optarg;
			break;

		case 'r':
			reboot_flag = 1;
			break;
		case 'h':
			_usage(progname, 0);
			break;
		default:
			_usage(progname, 1);
			break;
		}
	}

	/*
	 * upgrade -p 1.1.1.1 -f xxx.tar.gz -r
	 * upgrade -f xxx.tar.gz -r reboot
	 *
	 * tftp -r xxx.tar.gz - g 1.1.1.1
	 *
	 * tar -jxvf xxx.tar.gz
	 *
	 */

	chdir(BASE_DIR);

	if(server && strlen(server) && file)
	{
		memset(cmd, 0, sizeof(cmd));
		memset(buf, 0, sizeof(buf));
		snprintf(cmd, sizeof(cmd), "tftp -r %s -g %s", file, server);

		if(super_output_system(cmd, buf, sizeof(buf)) == OK)
		{
			if(strlen(buf))
				fprintf(stdout, "%s\n", buf);
		}
		else
			return -1;
	}
	if(file)
	{
		if(access(file, 0) != 0)
		{
			fprintf(stdout, " file %s is not exist.\n", file);
			return -1;
		}
		memset(cmd, 0, sizeof(cmd));
		memset(buf, 0, sizeof(buf));
		snprintf(cmd, sizeof(cmd), "tar -jxvf %s", file);
		if(super_output_system(cmd, buf, sizeof(buf)) != OK)
		{
			remove(file);
			return -1;
		}
		if(strlen(buf))
			fprintf(stdout, "%s\n", buf);
		remove(file);
		fprintf(stdout, " upgrade OK.\n");
		return OK;
	}
	_usage(progname, 0);
	return OK;
}

/*int main(int argc, char *argv[])
{
	printf("hello");
}*/

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/netlink.h>


#define UEVENT_BUFFER_SIZE 2048

int main(void)
{
    struct ipstack_sockaddr_nl client;
    struct timeval tv;
    int CppLive, rcvlen, ret;
    fd_set fds;
    int buffersize = 1024;
    CppLive = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    memset(&client, 0, sizeof(client));
    client.nl_family = AF_NETLINK;
    client.nl_pid = getpid();
    client.nl_groups = 1; /* receive broadcast message*/
    setsockopt(CppLive, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));
    bind(CppLive, (struct ipstack_sockaddr*)&client, sizeof(client));
    while (1) {
        char buf[UEVENT_BUFFER_SIZE] = { 0 };
        FD_ZERO(&fds);
        FD_SET(CppLive, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;
        ret = select(CppLive + 1, &fds, NULL, NULL, &tv);
        if(ret < 0)
            continue;
        if(!(ret > 0 && FD_ISSET(CppLive, &fds)))
            continue;
        /* receive data */
        rcvlen = recv(CppLive, &buf, sizeof(buf), 0);
        if (rcvlen > 0) {
            printf("%s\n", buf);
            /*You can do something here to make the program more perfect!!!*/
        }
    }
    close(CppLive);
    return 0;
}
#endif
