/*	$OpenBSD: db.c,v 1.16 2016/08/27 01:26:22 guenther Exp $	*/

/*
 * Persistent database management routines for DHCPD.
 */

/*
 * Copyright (c) 1995, 1996 The Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>

#include <netinet/in.h>

#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "dhcp.h"
#include "tree.h"
#include "dhcpd.h"

#ifdef PL_SQLITE_MODULE
#include "sqlite3.h"

static time_t write_time = 0;
static sqlite3 *sql_db = NULL;
/*
lease 1.1.1.1
{
	starts w yyyy/mm/dd hh:mm:ss UTC;
	ends w yyyy/mm/dd hh:mm:ss UTC;
	hardware eth 00:01:02:03:04:05;
	uid 2.3 00:01:02:03:04:05;
	dynamic-bootp;
	abandoned;
	client-hostname:"akcnd"
	hostname:"as"
}
 */
static sqlite3 *db_sql_init()
{
	sqlite3 *db = NULL;
	int result;
	char *errMsg = NULL;
	char *sql_create_table = "create table db_lease( \
		ADDRESS CHAR(48) PRIMARY KEY NOT NULL, \
		START INTEGER, \
		END INTEGER, \
		MAC CHAR(48), \
		UID CHAR(48), \
		BOOTP BOOLEAN, \
		ABANDONED BOOLEAN, \
		CLIENT CHAR(48), \
		HOST CHAR(48) \
		)";

	result = sqlite3_open(SYSCONFDIR"/abcd.db", &db);
	if (result != SQLITE_OK)
	{
		printf("database open fail!\n");
		return NULL;
	}
   // result = sqlite3_exec(db, sql_create_table, NULL, NULL, &errMsg);    //(字段名 字段类型 SQL约束)

	return db;
}

static int db_sql_lookup(sqlite3 *db, struct lease *lease)
{
	int result = 0;
	char *errMsg = NULL;
	char sql_lookup_cmd[512];
	memset(sql_lookup_cmd, 0, sizeof(sql_lookup_cmd));
	snprintf(sql_lookup_cmd, sizeof(sql_lookup_cmd),
			"SELECT from db_lease ADDRESS WHERE ADDRESS = '%s'", piaddr(lease->ip_addr));
	result = sqlite3_exec(db, sql_lookup_cmd, NULL, NULL, &errMsg);    //(字段名 字段类型 SQL约束)
	if(result == SQLITE_OK)
		return 0;
	return -1;
}

static int db_sql_write(sqlite3 *db, struct lease *lease)
{
	int result = 0;
	char *errMsg = NULL;
	char sql_lookup_cmd[512];
	char mac[48], uid[48], boot[48], aban[48], client[48], host[48];

	memset(sql_lookup_cmd, 0, sizeof(sql_lookup_cmd));
	memset(mac, 0, sizeof(mac));
	memset(uid, 0, sizeof(uid));
	memset(client, 0, sizeof(client));
	memset(host, 0, sizeof(host));
	memset(boot, 0, sizeof(boot));
	memset(aban, 0, sizeof(aban));

	if (lease->hardware_addr.hlen)
	{
		sprintf(mac, "%s-%s",
		    hardware_types[lease->hardware_addr.htype],
		    print_hw_addr(lease->hardware_addr.htype,
		    lease->hardware_addr.hlen,
		    lease->hardware_addr.haddr));
	}
	else
		sprintf(mac, "%s","NULL");

	if (lease->uid_len)
	{
		int j;
		char tmp[48];
		memset(tmp, 0, sizeof(tmp));
		sprintf(uid, "%02d-%02x", lease->uid_len,lease->uid[0]);
		for (j = 1; j < lease->uid_len; j++)
		{
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, ":%02x", lease->uid[j]);
			strcat(uid, tmp);
		}
	}
	else
		sprintf(uid, "NULL");

	if (lease->flags & BOOTP_LEASE)
	{
		sprintf(boot, "%d", "1");
	}
	else
		sprintf(boot, "%d", "0");

	if (lease->flags & ABANDONED_LEASE)
	{
		sprintf(aban, "%d", "1");
	}
	else
		sprintf(aban, "%d", "0");

	if (lease->client_hostname)
	{
		sprintf(client, "%s", lease->client_hostname);
	}
	else
		sprintf(client, "NULL");
	if (lease->hostname)
	{
		sprintf(host, "%s",lease->hostname);
	}
	else
		sprintf(host, "NULL");

	snprintf(sql_lookup_cmd, sizeof(sql_lookup_cmd),
			"insert into db_lease (ADDRESS,START,END,MAC,UID,BOOTP,ABANDONED,CLIENT,HOST) \
			VALUES('%s','%d','%d','%s','%s','%s','%s','%s','%s')", piaddr(lease->ip_addr),
			lease->starts, lease->ends, mac, uid, boot, aban, client, host);

	result = sqlite3_exec(db, sql_lookup_cmd, NULL, NULL, &errMsg);    //(字段名 字段类型 SQL约束)
	if(result == SQLITE_OK)
		return 0;
	return -1;
}

static int db_sql_delete(sqlite3 *db, struct lease *lease, int all)
{
	int result = 0;
	char *errMsg = NULL;
	char sql_lookup_cmd[512];
	memset(sql_lookup_cmd, 0, sizeof(sql_lookup_cmd));
	if(all)
		snprintf(sql_lookup_cmd, sizeof(sql_lookup_cmd),
				"DELETE from db_lease");
	else
		snprintf(sql_lookup_cmd, sizeof(sql_lookup_cmd),
			"DELETE from db_lease WHERE ADDRESS = '%s'", piaddr(lease->ip_addr));
	result = sqlite3_exec(db, sql_lookup_cmd, NULL, NULL, &errMsg);    //(字段名 字段类型 SQL约束)
	if(result == SQLITE_OK)
		return 0;
	return -1;
}

static int db_sql_update(sqlite3 *db, struct lease *lease)
{
	int result = 0;
	char *errMsg = NULL;
	char sql_lookup_cmd[512];
	char mac[48], uid[48], boot[48], aban[48], client[48], host[48];

	memset(sql_lookup_cmd, 0, sizeof(sql_lookup_cmd));
	memset(mac, 0, sizeof(mac));
	memset(uid, 0, sizeof(uid));
	memset(client, 0, sizeof(client));
	memset(host, 0, sizeof(host));
	memset(boot, 0, sizeof(boot));
	memset(aban, 0, sizeof(aban));

	if (lease->hardware_addr.hlen)
	{
		sprintf(mac, "%s-%s",
		    hardware_types[lease->hardware_addr.htype],
		    print_hw_addr(lease->hardware_addr.htype,
		    lease->hardware_addr.hlen,
		    lease->hardware_addr.haddr));
	}
	else
		sprintf(mac, "%s","NULL");

	if (lease->uid_len)
	{
		int j;
		char tmp[48];
		memset(tmp, 0, sizeof(tmp));
		sprintf(uid, "%02d-%02x", lease->uid_len,lease->uid[0]);
		for (j = 1; j < lease->uid_len; j++)
		{
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, ":%02x", lease->uid[j]);
			strcat(uid, tmp);
		}
	}
	else
		sprintf(uid, "NULL");

	if (lease->flags & BOOTP_LEASE)
	{
		sprintf(boot, "%d", "1");
	}
	else
		sprintf(boot, "%d", "0");

	if (lease->flags & ABANDONED_LEASE)
	{
		sprintf(aban, "%d", "1");
	}
	else
		sprintf(aban, "%d", "0");

	if (lease->client_hostname)
	{
		sprintf(client, "%s", lease->client_hostname);
	}
	else
		sprintf(client, "NULL");
	if (lease->hostname)
	{
		sprintf(host, "%s",lease->hostname);
	}
	else
		sprintf(host, "NULL");

	snprintf(sql_lookup_cmd, sizeof(sql_lookup_cmd),
			"update db_lease set START='%d',END='%d',MAC='%s',UID='%s',BOOTP='%s', \
				ABANDONED='%s',CLIENT='%s',HOST='%s' where ADDRESS='%s'",
				lease->starts, lease->ends, mac, uid, boot, aban, client, host, piaddr(lease->ip_addr));

	result = sqlite3_exec(db, sql_lookup_cmd, NULL, NULL, &errMsg);    //(字段名 字段类型 SQL约束)
	if(result == SQLITE_OK)
		return 0;
	return -1;
}


static int sql_read_callback(void *data, int argc, char **argv, char **azColName)
{
	int i;
	uint32_t  address = 0;
	struct lease lease;
	memset(&lease, 0, sizeof lease);
	for(i = 0; i < argc; i++)
	{
		if(strcmp(azColName[i], "ADDRESS") == 0 && argv[i])
		{
			address = inet_addr(argv[i]);
			memcpy(lease.ip_addr.iabuf, &address, 4);
			lease.ip_addr.len = 4;
		}
		if(strcmp(azColName[i], "START") == 0 && argv[i])
		{
			lease.starts = atoi(argv[i]);
		}
		if(strcmp(azColName[i], "END") == 0 && argv[i])
		{
			lease.ends = atoi(argv[i]);
		}
		if(strcmp(azColName[i], "MAC") == 0 && argv[i])
		{
			char tmp[48];
			char *stk = NULL;
			int i = 0;
			memset(tmp, 0, sizeof(tmp));
			if(!strstr(argv[i], "NULL"))
			{
				sscanf(argv[i], "%d[^-]", tmp);
				if(strstr(tmp, "ethernet") == 0)
					lease.hardware_addr.htype = HTYPE_ETHER,
				lease.hardware_addr.hlen = strchr_count(argv[i], ':');
				stk = argv[i] + strlen(tmp) + 1;
				while(i < lease.hardware_addr.hlen && stk)
				{
					sscanf(stk, "%02x",&lease.hardware_addr.haddr[i++]);
					stk += 3;
				}

				parse_warn("mac len %d : %02x:%02x:%02x:%02x:%02x:%02x\n", lease.hardware_addr.hlen,
						lease.hardware_addr.haddr[0], lease.hardware_addr.haddr[1],
						lease.hardware_addr.haddr[2], lease.hardware_addr.haddr[3],
						lease.hardware_addr.haddr[4], lease.hardware_addr.haddr[5]);
			}
		}
		if(strcmp(azColName[i], "UID") == 0 && argv[i])
		{
			if(!strstr(argv[i], "NULL"))
			{
				char *stk = NULL;
				int i = 0;
				sscanf(argv[i], "%02d[^-]", &lease.uid_len);
				stk = argv[i] + 3;
				while(i < lease.uid_len && stk)
				{
					sscanf(stk, "%02x", &lease.uid[i++]);
					stk += 3;
				}
				parse_warn("uid len %d : %02x:%02x:%02x:%02x:%02x:%02x\n", lease.uid_len,
						lease.uid[0], lease.uid[1],
						lease.uid[2], lease.uid[3],
						lease.uid[4], lease.uid[5]);
			}
		}
		if(strcmp(azColName[i], "BOOTP") == 0 && argv[i])
		{
			if(strstr(argv[i], "1"))
				lease.flags |= BOOTP_LEASE;
		}
		if(strcmp(azColName[i], "ABANDONED") == 0 && argv[i])
		{
			if(strstr(argv[i], "1"))
				lease.flags |= ABANDONED_LEASE;
		}
		if(strcmp(azColName[i], "CLIENT") == 0 && argv[i])
		{
			if(!strstr(argv[i], "NULL"))
				lease.client_hostname = strdup(argv[i]);
		}
		if(strcmp(azColName[i], "HOST") == 0 && argv[i])
		{
			if(!strstr(argv[i], "NULL"))
				lease.hostname = strdup(argv[i]);
		}
		parse_warn("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	if (address)
		enter_lease(&lease);
	return 0;
}

static int db_sql_read(sqlite3 *db)
{
	int result = 0;
	char *errMsg = NULL;
	//char *sql_select_table = "SELECT from db_lease ADDRESS WHERE ADDRESS = '%s'";
	char sql_lookup_cmd[512];
	memset(sql_lookup_cmd, 0, sizeof(sql_lookup_cmd));
	snprintf(sql_lookup_cmd, sizeof(sql_lookup_cmd), "SELECT * from db_lease");
	result = sqlite3_exec(db, sql_lookup_cmd, sql_read_callback, NULL, &errMsg);    //(字段名 字段类型 SQL约束)
	if(result == SQLITE_OK)
		return 0;
	return -1;
}
/*
insert into db_lease (ADDRESS,START,END,MAC,UID,BOOTP,ABANDONED,CLIENT,HOST) VALUES('1.1.1.1','2018-01-02 23:23:23','2018-01-02 23:34:34','ETH 01:02:03:04:05:06','UID-010203040506','sadsadadsa','dasdadas','clientsaa','localhost');
sqlite>
sqlite> select * from db_lease;
1.1.1.1|2018-01-02 23:23:23|2018-01-02 23:34:34|ETH 01:02:03:04:05:06|UID-010203040506|sadsadadsa|dasdadas|clientsaa|localhost
sqlite> select * from db_lease where ADDRESS='1.1.1.1';
1.1.1.1|2018-01-02 23:23:23|2018-01-02 23:34:34|ETH 01:02:03:04:05:06|UID-010203040506|sadsadadsa|dasdadas|clientsaa|localhost
*/
void
sql_read_leases(void)
{
	if(!sql_db)
		sql_db = db_sql_init();
	if(sql_db)
	{
		db_sql_read(sql_db);
		sqlite3_close(sql_db);
		sql_db = NULL;
	}
}
/*
 * Write the specified lease to the current lease database file.
 */
int
write_lease(struct lease *lease)
{
	if(!sql_db)
		sql_db = db_sql_init();
	if(sql_db)
	{
		if(db_sql_lookup(sql_db, lease))
			db_sql_update(sql_db, lease);
		else
			db_sql_write(sql_db, lease);
	}
	return 0;
}

/*
 * Commit any leases that have been written out...
 */
int
commit_leases(void)
{
	if ((root_group.cur_time - write_time > 3600)) {
		write_time = root_group.cur_time;
		new_lease_file();
	}
	return (1);
}

void
db_startup(void)
{
	/* Read in the existing lease file... */
	sql_read_leases();
	time(&write_time);
	new_lease_file();
}

void
new_lease_file(void)
{
	if(!sql_db)
		sql_db = db_sql_init();
	if(sql_db)
	{
		db_sql_delete(sql_db, NULL, 1);
		/* Write out all the leases that we know of... */
		write_leases();
		sqlite3_close(sql_db);
		sql_db = NULL;
	}
/*	db_sql_delete(sql_db, NULL, 1);
	 Write out all the leases that we know of...
	write_leases();*/
}
#else

FILE *db_file;

static int counting = 0;
static int count = 0;
time_t write_time;

/*
"lease 1.1.1.1
{
	starts w yyyy/mm/dd hh:mm:ss UTC;
	ends w yyyy/mm/dd hh:mm:ss UTC;
	hardware eth 00:01:02:03:04:05;
	uid 2.3 00:01:02:03:04:05;
	dynamic-bootp;
	abandoned;
	client-hostname:"akcnd"
	hostname:"as"
}
*/
/*
 * Write the specified lease to the current lease database file.
 */
int
write_lease(struct lease *lease)
{
	char tbuf[26];	/* "w yyyy/mm/dd hh:mm:ss UTC" */
	size_t rsltsz;
	int errors = 0;
	int i;

	if (counting)
		++count;
	if (fprintf(db_file, "lease %s {\n", piaddr(lease->ip_addr)) == -1)
		++errors;

	rsltsz = strftime(tbuf, sizeof(tbuf), DB_TIMEFMT,
	    gmtime(&lease->starts));
	if (rsltsz == 0 || fprintf(db_file, "\tstarts %s;\n", tbuf) == -1)
		errors++;

	rsltsz = strftime(tbuf, sizeof(tbuf), DB_TIMEFMT,
	    gmtime(&lease->ends));
	if (rsltsz == 0 || fprintf(db_file, "\tends %s;\n", tbuf) == -1)
		errors++;

	if (lease->hardware_addr.hlen) {
		if (fprintf(db_file, "\thardware %s %s;",
		    hardware_types[lease->hardware_addr.htype],
		    print_hw_addr(lease->hardware_addr.htype,
		    lease->hardware_addr.hlen,
		    lease->hardware_addr.haddr)) == -1)
			++errors;
	}

	if (lease->uid_len) {
		int j;

		if (fprintf(db_file, "\n\tuid %02x", lease->uid[0]) == -1)
			++errors;

		for (j = 1; j < lease->uid_len; j++) {
			if (fprintf(db_file, ":%02x", lease->uid[j]) == -1)
				++errors;
		}
		if (fputc(';', db_file) == EOF)
			++errors;
	}

	if (lease->flags & BOOTP_LEASE) {
		if (fprintf(db_file, "\n\tdynamic-bootp;") == -1)
			++errors;
	}

	if (lease->flags & ABANDONED_LEASE) {
		if (fprintf(db_file, "\n\tabandoned;") == -1)
			++errors;
	}

	if (lease->client_hostname) {
		for (i = 0; lease->client_hostname[i]; i++)
			if (lease->client_hostname[i] < 33 ||
			    lease->client_hostname[i] > 126)
				goto bad_client_hostname;
		if (fprintf(db_file, "\n\tclient-hostname \"%s\";",
		    lease->client_hostname) == -1)
			++errors;
	}

bad_client_hostname:
	if (lease->hostname) {
		for (i = 0; lease->hostname[i]; i++)
			if (lease->hostname[i] < 33 ||
			    lease->hostname[i] > 126)
				goto bad_hostname;
		if (fprintf(db_file, "\n\thostname \"%s\";",
		    lease->hostname) == -1)
			++errors;
	}

bad_hostname:
	if (fputs("\n}\n", db_file) == EOF)
		++errors;

	if (errors)
		dhcpd_note("write_lease: unable to write lease %s",
		    piaddr(lease->ip_addr));

	return (!errors);
}

/*
 * Commit any leases that have been written out...
 */
int
commit_leases(void)
{
	/*
	 * Commit any outstanding writes to the lease database file. We need to
	 * do this even if we're rewriting the file below, just in case the
	 * rewrite fails.
	 */
	if (fflush(db_file) == EOF) {
		dhcpd_note("commit_leases: unable to commit: %m");
		return (0);
	}

	if (fsync(fileno(db_file)) == -1) {
		dhcpd_note("commit_leases: unable to commit: %m");
		return (0);
	}

	/*
	 * If we've written more than a thousand leases or if we haven't
	 * rewritten the lease database in over an hour, rewrite it now.
	 */
	if (count > 1000 || (count && root_group.cur_time - write_time > 3600)) {
		count = 0;
		write_time = root_group.cur_time;
		new_lease_file();
	}

	return (1);
}

void
db_startup(void)
{
	int db_fd;

	/* open lease file. once we dropped privs it has to stay open */
	db_fd = open(path_dhcpd_db, O_WRONLY|O_CREAT, 0640);
	if (db_fd == -1)
	{
		dhcpd_error("Can't create new lease file: %m");
		return;
	}
	if ((db_file = fdopen(db_fd, "w")) == NULL)
	{
		dhcpd_error("Can't fdopen new lease file!");
		return;
	}
	/* Read in the existing lease file... */
	//read_leases();
	time(&write_time);

	new_lease_file();
}

void
new_lease_file(void)
{
	fflush(db_file);
	rewind(db_file);

	/* Write out all the leases that we know of... */
	counting = 0;
	write_leases();

	fflush(db_file);
	ftruncate(fileno(db_file), ftello(db_file));
	fsync(fileno(db_file));

	counting = 1;
}
#endif
