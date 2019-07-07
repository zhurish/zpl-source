/*
 * cmd_log.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */


#include "zebra.h"

#include "log.h"
#include "memory.h"
#include "thread.h"
#include "vector.h"
#include "version.h"
#include "workqueue.h"
#include "command.h"
#include "vty.h"
#include "vty_user.h"
#include "host.h"

struct logfilter
{
	struct vty *vty;
	int module;
	int priority;
	int size;
	int total;
};

DEFUN (config_logmsg,
		config_logmsg_cmd,
		"logmsg "LOG_LEVELS" .MESSAGE",
		"Send a message to enabled logging destinations\n"
		LOG_LEVEL_DESC
		"The message to send\n")
{
	int level;
	char *message;

	if ((level = zlog_priority_match(argv[0])) == ZLOG_DISABLED)
		return CMD_ERR_NO_MATCH;

	zlog(NULL, level, "%s",
			((message = argv_concat(argv, argc, 1)) ? message : ""));
	if (message)
		XFREE(MTYPE_TMP, message);
	return CMD_SUCCESS;
}


DEFUN (config_log_stdout,
		config_log_stdout_cmd,
		"log stdout",
		"Logging control\n"
		"Set stdout logging level\n")
{
	int level = zlog_default->default_lvl[ZLOG_DEST_STDOUT];
	if(argc == 1)
	{
		if ((level = zlog_priority_match(argv[0])) == ZLOG_DISABLED)
			return CMD_ERR_NO_MATCH;
	}
	zlog_set_level( ZLOG_DEST_STDOUT, level);
	return CMD_SUCCESS;
	//zlog_set_level( ZLOG_DEST_STDOUT, zlog_default->default_lvl[ZLOG_DEST_STDOUT]);
	//return CMD_SUCCESS;
}

ALIAS (config_log_stdout,
		config_log_stdout_level_cmd,
		"log stdout "LOG_LEVELS,
		"Logging control\n"
		"Set stdout logging level\n"
		LOG_LEVEL_DESC);

DEFUN (no_config_log_stdout,
		no_config_log_stdout_cmd,
		"no log stdout",
		NO_STR
		"Logging control\n"
		"Logging to stdout\n")
{
	if(argc == 1)
	{
		zlog_set_level(ZLOG_DEST_STDOUT, zlog_default->default_lvl[ZLOG_DEST_STDOUT]);
	}
	else
		zlog_set_level( ZLOG_DEST_STDOUT, ZLOG_DISABLED);
	return CMD_SUCCESS;
}

ALIAS (no_config_log_stdout,
		no_config_log_stdout_level_cmd,
		"no log stdout "LOG_LEVELS,
		NO_STR
		"Logging control\n"
		"Set stdout logging level\n"
		LOG_LEVEL_DESC);

DEFUN (config_log_monitor,
		config_log_monitor_cmd,
		"log monitor",
		"Logging control\n"
		"Set terminal line (monitor) logging level\n")
{
	int level = zlog_default->default_lvl[ZLOG_DEST_MONITOR];
	if(argc == 1)
	{
		if ((level = zlog_priority_match(argv[0])) == ZLOG_DISABLED)
			return CMD_ERR_NO_MATCH;
	}
	zlog_set_level( ZLOG_DEST_MONITOR, level);
	return CMD_SUCCESS;
}

ALIAS (config_log_monitor,
		config_log_monitor_level_cmd,
		"log monitor "LOG_LEVELS,
		"Logging control\n"
		"Set terminal line (monitor) logging level\n"
		LOG_LEVEL_DESC);

DEFUN (no_config_log_monitor,
		no_config_log_monitor_cmd,
		"no log monitor [LEVEL]",
		NO_STR
		"Logging control\n"
		"Disable terminal line (monitor) logging\n"
		"Logging level\n")
{
	if(argc == 1)
	{
		zlog_set_level(ZLOG_DEST_MONITOR, zlog_default->default_lvl[ZLOG_DEST_MONITOR]);
	}
	else
		zlog_set_level( ZLOG_DEST_MONITOR, ZLOG_DISABLED);
	return CMD_SUCCESS;
}

ALIAS (no_config_log_monitor,
		no_config_log_monitor_level_cmd,
		"no log monitor "LOG_LEVELS,
		NO_STR
		"Logging control\n"
		"Set terminal line (monitor) logging level\n"
		LOG_LEVEL_DESC);


DEFUN (config_log_file,
		config_log_file_cmd,
		"log file FILENAME",
		"Logging control\n"
		"Logging to file\n"
		"Logging filename\n")
{
	int level = zlog_default->default_lvl[ZLOG_DEST_FILE];
	if(argc == 2)
	{
		if ((level = zlog_priority_match(argv[1])) == ZLOG_DISABLED)
			return CMD_ERR_NO_MATCH;
	}
	zlog_set_file(argv[0], level);
	zlog_set_level( ZLOG_DEST_FILE, level);
	return CMD_SUCCESS;
}

ALIAS (config_log_file,
		config_log_file_level_cmd,
		"log file FILENAME "LOG_LEVELS,
		"Logging control\n"
		"Logging to file\n"
		"Logging filename\n"
		LOG_LEVEL_DESC);

DEFUN (config_log_file_size,
		config_log_file_size_cmd,
		"log file size (1|2|4|6|8|16)",
		"Logging control\n"
		"Logging to file\n"
		"Logging filename size\n"
		"1M\n"
		"2M\n"
		"4M\n"
		"6M\n"
		"8M\n"
		"16M\n")
{
	int size;
	zlog_get_file_size(&size);
	if(size != atoi(argv[0]))
		return zlog_set_file_size(atoi(argv[0]));
	return CMD_SUCCESS;
}

DEFUN (no_config_log_file,
		no_config_log_file_cmd,
		"no log file [FILENAME]",
		NO_STR
		"Logging control\n"
		"Cancel logging to file\n"
		"Logging file name\n")
{
	if(argc == 2)
		zlog_set_level(ZLOG_DEST_FILE, zlog_default->default_lvl[ZLOG_DEST_FILE]);
	else
	{
		zlog_close_file();
		if (host_config_set_api(API_SET_LOGFILE_CMD, NULL) != OK) {
			return CMD_WARNING;
		}
	}
	zlog_set_level(ZLOG_DEST_FILE, ZLOG_DISABLED);
	return CMD_SUCCESS;
}

ALIAS(no_config_log_file,
		no_config_log_file_level_cmd,
		"no log file FILENAME " LOG_LEVELS,
		NO_STR
		"Logging control\n"
		"Cancel logging to file\n"
		"Logging file name\n"
		LOG_LEVEL_DESC)

DEFUN (no_config_log_file_size,
		no_config_log_file_size_cmd,
		"no log (file|buffer) size",
		NO_STR
		"Logging control\n"
		"Logging to file\n"
		"Logging filename size\n")
{
	int size;
	if(os_memcmp(argv[0], "file", 3) == 0)
	{
		zlog_get_file_size(&size);
		if(size != ZLOG_FILE_SIZE)
			return zlog_set_file_size(ZLOG_FILE_SIZE);
	}
	else if(os_memcmp(argv[0], "buffer", 3) == 0)
	{
		zlog_get_buffer_size(&size);
		if(size != ZLOG_BUFF_SIZE)
			return zlog_set_buffer_size(ZLOG_BUFF_SIZE);
	}

	return CMD_SUCCESS;
}

DEFUN (save_log_file,
		save_log_file_cmd,
		"save log (file|buffer)",
		"save\n"
		"Logging control\n"
		"Logging file\n"
		"Logging buffer\n")
{
	if(os_memcmp(argv[0], "file", 3) == 0)
		zlog_file_save();
	else if(os_memcmp(argv[0], "buffer", 3) == 0)
		zlog_buffer_save();
	return CMD_SUCCESS;
}

DEFUN (config_log_buffer,
		config_log_buffer_cmd,
		"log buffer",
		"Logging control\n"
		"Logging to buffer\n")
{
	int level = zlog_default->default_lvl[ZLOG_DEST_BUFFER];

	if(argc == 1)
	{
		if ((level = zlog_priority_match(argv[0])) == ZLOG_DISABLED)
			return CMD_ERR_NO_MATCH;
	}
	zlog_set_level(ZLOG_DEST_BUFFER, level);
	return CMD_SUCCESS;
}

ALIAS (config_log_buffer,
		config_log_buffer_level_cmd,
		"log buffer "LOG_LEVELS,
		"Logging control\n"
		"Logging to buffer\n"
		LOG_LEVEL_DESC)

DEFUN (no_config_log_buffer,
		no_config_log_buffer_cmd,
		"no log buffer",
		NO_STR
		"Logging control\n"
		"Logging to buffer\n")
{
	if(argc == 1)
	{
		zlog_set_level(ZLOG_DEST_BUFFER, zlog_default->default_lvl[ZLOG_DEST_BUFFER]);
	}
	else
		zlog_set_level(ZLOG_DEST_BUFFER, ZLOG_DISABLED);
	return CMD_SUCCESS;
}

ALIAS (no_config_log_buffer,
		no_config_log_buffer_level_cmd,
		"no log buffer "LOG_LEVELS,
		NO_STR
		"Logging control\n"
		"Logging to buffer\n"
		LOG_LEVEL_DESC)

DEFUN (config_log_buffer_size,
		config_log_buffer_size_cmd,
		"log buffer size <1-512>",
		"Logging control\n"
		"Logging to buffer\n"
		"Logging buffer size\n"
		"size value\n")
{
	int size;

	zlog_get_buffer_size(&size);
	if(size != atoi(argv[0]))
		return zlog_set_buffer_size(atoi(argv[0]));
	return CMD_SUCCESS;
}



DEFUN (config_log_syslog,
		config_log_syslog_cmd,
		"log syslog",
		"Logging control\n"
		"Logging to syslog\n")
{
	int level = zlog_default->default_lvl[ZLOG_DEST_SYSLOG];

	if(argc == 1)
	{
		if ((level = zlog_priority_match(argv[0])) == ZLOG_DISABLED)
			return CMD_ERR_NO_MATCH;
	}
	zlog_set_level(ZLOG_DEST_SYSLOG, level);
	return CMD_SUCCESS;
}

ALIAS (config_log_syslog,
		config_log_syslog_level_cmd,
		"log syslog "LOG_LEVELS,
		"Logging control\n"
		"Logging to syslog\n"
		LOG_LEVEL_DESC);

DEFUN (no_config_log_syslog,
		no_config_log_syslog_cmd,
		"no log syslog",
		NO_STR
		"Logging control\n"
		"Logging to syslog\n")
{
	if(argc == 1)
		zlog_set_level(ZLOG_DEST_SYSLOG, zlog_default->default_lvl[ZLOG_DEST_SYSLOG]);
	else
		zlog_set_level(ZLOG_DEST_SYSLOG, ZLOG_DISABLED);
	return CMD_SUCCESS;
}

ALIAS(no_config_log_syslog,
		no_config_log_syslog_level_cmd,
		"no log syslog "LOG_LEVELS,
		NO_STR
		"Logging control\n"
		"Logging to syslog\n"
		LOG_LEVEL_DESC)

#ifdef PL_SYSLOG_MODULE

DEFUN (syslog_host,
		syslog_host_cmd,
		"syslog host (A.B.C.D|dynamics)",
		"Syslog logging control\n"
		"syslog host configure\n"
		"Specify by IPv4 address(e.g. 0.0.0.0)\n"
		"sntp server dynamics\n")
{
	int port = SYSLOGC_DEFAULT_PORT;
	if (argc == 2)
		port = atoi(argv[1]);
	if (syslogc_is_enable())
	{
		if(strstr(argv[0], "."))
		{
			if(syslogc_is_dynamics())
			{
				syslogc_dynamics_disable();
			}
			syslogc_host_config_set(argv[0], port, zlog_default->facility);
		}
		else
		{
			if(!syslogc_is_dynamics())
			{
				syslogc_dynamics_enable();
			}
			syslogc_host_config_set(NULL, port, zlog_default->facility);
		}
	} else {
		syslogc_enable(host.name);
		if(strstr(argv[0], "."))
		{
			if(syslogc_is_dynamics())
			{
				syslogc_dynamics_disable();
			}
			syslogc_host_config_set(argv[0], port, zlog_default->facility);
		}
		else
		{
			if(!syslogc_is_dynamics())
			{
				syslogc_dynamics_enable();
			}
			syslogc_host_config_set(NULL, port, zlog_default->facility);
		}
		//syslogc_host_config_set(argv[0], port, zlog_default->facility);
	}
	return CMD_SUCCESS;
}

ALIAS(syslog_host,
		syslog_host_port_cmd,
		"syslog host (A.B.C.D|dynamics) port <100-65536>",
		"Syslog logging control\n"
		"syslog host configure\n"
		"Specify by IPv4 address(e.g. 0.0.0.0)\n"
		"sntp server dynamics\n"
		"sntp server dynamics\n"
		"syslog server UDP port\n"
		"Specify by UDP port(e.g. 514)\n")

DEFUN (no_syslog_host,
		no_syslog_host_cmd,
		"no syslog host",
		NO_STR
		"Syslog logging control\n"
		"syslog host configure\n")
{
	if (syslogc_is_enable())
	{
		syslogc_disable();
	}
	return CMD_SUCCESS;
}

DEFUN (syslog_mode,
		syslog_mode_cmd,
		"syslog mode (tcp|udp)",
		"Syslog logging control\n"
		"syslog mode configure\n"
		"Specify by TCP protocol\n"
		"Specify by UDP protocol\n")
{
	int mode = SYSLOG_UDP_MODE;
	if (argc == 1) {
		if (os_memcmp(argv[0], "tcp", 1) == 0)
			mode = SYSLOG_TCP_MODE;
	}
	syslogc_mode_set(mode);
	return CMD_SUCCESS;
}

DEFUN (no_syslog_mode,
		no_syslog_mode_cmd,
		"no syslog mode",
		NO_STR
		"Syslog logging control\n"
		"syslog mode configure\n")
{
	syslogc_mode_set(SYSLOG_UDP_MODE);
	return CMD_SUCCESS;
}

#endif



DEFUN (config_log_facility,
		config_log_facility_cmd,
		"log facility "LOG_FACILITIES,
		"Logging control\n"
		"Facility parameter for syslog messages\n"
		LOG_FACILITY_DESC)
{
	int facility;

	if ((facility = zlog_facility_match(argv[0])) < 0)
		return CMD_ERR_NO_MATCH;
	zlog_set_facility(facility);
	return CMD_SUCCESS;
}

DEFUN (no_config_log_facility,
		no_config_log_facility_cmd,
		"no log facility [FACILITY]",
		NO_STR
		"Logging control\n"
		"Reset syslog facility to default (daemon)\n"
		"Syslog facility\n")
{
	zlog_set_facility(LOG_LOCAL7);
	//zlog_default->facility = LOG_DAEMON;
	return CMD_SUCCESS;
}

DEFUN_DEPRECATED (config_log_trap,
		config_log_trap_cmd,
		"log trapping",
		"Logging control\n"
		"Set terminal line (trapping) logging\n")
{
/*	int new_level;
	if ((new_level = zlog_priority_match(argv[0])) == ZLOG_DISABLED)
		return CMD_ERR_NO_MATCH;
	zlog_set_trap(new_level);*/
	vty->trapping = 1;
	return CMD_SUCCESS;
}

DEFUN_DEPRECATED (no_config_log_trap,
		no_config_log_trap_cmd,
		"no log trapping",
		NO_STR
		"Logging control\n"
		"Set terminal line (trapping) logging\n")
{
/*	zlog_set_trap(LOG_DEBUG);*/
	vty->trapping = 0;
	return CMD_SUCCESS;
}

DEFUN (config_log_record_priority,
		config_log_record_priority_cmd,
		"log record-priority",
		"Logging control\n"
		"Log the priority of the message within the message\n")
{
	zlog_set_record_priority(1);
	return CMD_SUCCESS;
}

DEFUN (no_config_log_record_priority,
		no_config_log_record_priority_cmd,
		"no log record-priority",
		NO_STR
		"Logging control\n"
		"Do not log the priority of the message within the message\n")
{
	zlog_set_record_priority(0);
	return CMD_SUCCESS;
}

DEFUN (config_log_timestamp,
		config_log_timestamp_cmd,
		"log timestamp (date|short|bsd|iso|rfc3164|rfc3339)",
		"Logging control\n"
		"Timestamp configuration\n"
		"Timestamp format date\n"
		"Timestamp format short\n"
		"Timestamp format bsd\n"
		"Timestamp format iso\n"
		"Timestamp format rfc3164\n"
		"Timestamp format rfc3339\n")
{
	zlog_timestamp_t value = 0;
	if (argc != 1) {
		vty_out(vty, "Insufficient arguments%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(os_memcmp(argv[0],"date", 3) == 0)
		value = ZLOG_TIMESTAMP_DATE;
	else if(os_memcmp(argv[0],"short", 3) == 0)
		value = ZLOG_TIMESTAMP_SHORT;
	else if(os_memcmp(argv[0],"bsd", 3) == 0)
		value = ZLOG_TIMESTAMP_BSD;
	else if(os_memcmp(argv[0],"iso", 3) == 0)
		value = ZLOG_TIMESTAMP_ISO;
	else if(os_memcmp(argv[0],"rfc3164", 3) == 0)
	{
		if(os_memcmp(argv[0],"rfc3164", 5) == 0)
			value = ZLOG_TIMESTAMP_RFC3164;
		if(os_memcmp(argv[0],"rfc3339", 5) == 0)
			value = ZLOG_TIMESTAMP_RFC3339;
	}
	if(value)
		zlog_set_timestamp(value);
	return CMD_SUCCESS;
}

DEFUN (no_config_log_timestamp,
		no_config_log_timestamp_cmd,
		"no log timestamp",
		NO_STR
		"Logging control\n"
		"Timestamp configuration\n")
{
	zlog_set_timestamp(ZLOG_TIMESTAMP_NONE);
	return CMD_SUCCESS;
}




#ifdef ZLOG_TESTING_ENABLE

DEFUN (config_log_testing,
		config_log_testing_cmd,
		"log testing",
		"Logging control\n"
		"Logging to Testing\n")
{
	int level = LOG_DEBUG;
	if(argc == 1)
	{
		if ((level = zlog_priority_match(argv[0])) == ZLOG_DISABLED)
			return CMD_ERR_NO_MATCH;
	}
	if(!zlog_testing_enabled())
		zlog_testing_enable(TRUE);
	zlog_testing_priority(level);
	return CMD_SUCCESS;
}

ALIAS (config_log_testing,
		config_log_testing_level_cmd,
		"log testing "LOG_LEVELS,
		"Logging control\n"
		"Logging to Testing\n"
		LOG_LEVEL_DESC);


DEFUN (no_config_log_testing,
		no_config_log_testing_cmd,
		"no log testing",
		NO_STR
		"Logging control\n"
		"Logging to Testing\n")
{
	if(argc == 1)
		zlog_testing_priority(LOG_DEBUG);
	else
	{
		zlog_testing_priority(LOG_ERR);
		zlog_testing_enable(FALSE);
	}
	return CMD_SUCCESS;
}

ALIAS(no_config_log_testing,
		no_config_log_testing_level_cmd,
		"no log testing "LOG_LEVELS,
		NO_STR
		"Logging control\n"
		"Logging to Testing\n"
		LOG_LEVEL_DESC)

DEFUN (config_log_testing_file,
		config_log_testing_file_cmd,
		"log testing file FILENAME",
		"Logging control\n"
		"Logging to Testing\n"
		"Logging to file\n"
		"Logging filename\n")
{
	int level = LOG_DEBUG;
	if(argc == 2)
	{
		if ((level = zlog_priority_match(argv[1])) == ZLOG_DISABLED)
			return CMD_ERR_NO_MATCH;
	}
	if(!zlog_testing_enabled())
		zlog_testing_enable(TRUE);
	zlog_testing_priority(level);
	zlog_testing_file(argv[0]);
	return CMD_SUCCESS;
}

ALIAS (config_log_testing_file,
		config_log_testing_file_level_cmd,
		"log testing file FILENAME "LOG_LEVELS,
		"Logging control\n"
		"Logging to Testing\n"
		"Logging to file\n"
		"Logging filename\n"
		LOG_LEVEL_DESC);

DEFUN (no_config_log_testing_file,
		no_config_log_testing_file_cmd,
		"no log testing file [FILENAME]",
		NO_STR
		"Logging control\n"
		"Logging to Testing\n"
		"Cancel logging to file\n"
		"Logging file name\n")
{
	if(argc == 2)
	{
		zlog_testing_file(NULL);
		zlog_testing_priority(LOG_DEBUG);
		zlog_testing_enable(FALSE);
	}
	else
	{
		zlog_testing_file(NULL);
		zlog_testing_priority(LOG_DEBUG);
		zlog_testing_enable(FALSE);
	}
	return CMD_SUCCESS;
}

ALIAS(no_config_log_testing_file,
		no_config_log_testing_file_level_cmd,
		"no log testing file FILENAME " LOG_LEVELS,
		NO_STR
		"Logging control\n"
		"Logging to Testing\n"
		"Cancel logging to file\n"
		"Logging file name\n"
		LOG_LEVEL_DESC)


DEFUN (show_config_log_testing_file,
		show_config_log_file_testing_cmd,
		"show logging testing file",
		SHOW_STR
		"Logging control\n"
		"Logging to Testing\n"
		"log file information\n")
{
	FILE *fp = NULL;
	char filetmp[256];
	char buf[4096];
	if (zlog_default == NULL || !zlog_default->testlog.filename)
		return CMD_WARNING;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);

	memset(filetmp, 0, sizeof(filetmp));
	sprintf(filetmp, "%s%s", ZLOG_VIRTUAL_PATH, zlog_default->testlog.filename);
	fp = fopen (filetmp, "r");

	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
			char *s;
			/* work backwards to ignore trailling isspace() */
			for (s = buf + strlen(buf); (s > buf) && isspace((int) *(s - 1)); s--)
				;
			*s = '\0';
			vty_out(vty, "%s%s", buf, VTY_NEWLINE);
		}
		fclose(fp);
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return CMD_SUCCESS;
}
#endif






DEFUN (show_logging,
		show_logging_cmd,
		"show logging",
		SHOW_STR
		"Show current logging configuration\n")
{
	struct zlog *zl = zlog_default;

	vty_out(vty, "Syslog logging      : ");
	if (zl->maxlvl[ZLOG_DEST_SYSLOG] == ZLOG_DISABLED)
		vty_out(vty, "disabled");
	else
		vty_out(vty, "level %s, facility %s, ident %s",
				zlog_priority_name(zl->maxlvl[ZLOG_DEST_SYSLOG]),
				zlog_facility_name(zl->facility), zl->ident);
	vty_out(vty, "%s", VTY_NEWLINE);

	vty_out(vty, "Stdout logging      : ");
	if (zl->maxlvl[ZLOG_DEST_STDOUT] == ZLOG_DISABLED)
		vty_out(vty, "disabled");
	else
		vty_out(vty, "level %s", zlog_priority_name(zl->maxlvl[ZLOG_DEST_STDOUT]));
	vty_out(vty, "%s", VTY_NEWLINE);

	vty_out(vty, "Monitor logging     : ");
	if (zl->maxlvl[ZLOG_DEST_MONITOR] == ZLOG_DISABLED)
		vty_out(vty, "disabled");
	else
		vty_out(vty, "level %s", zlog_priority_name(zl->maxlvl[ZLOG_DEST_MONITOR]));
	vty_out(vty, "%s", VTY_NEWLINE);

	vty_out(vty, "Trapping logging     :%s%s", vty->trapping ? "enable":"disabled",VTY_NEWLINE);


	vty_out(vty, "File logging        : ");
	if ((zl->maxlvl[ZLOG_DEST_FILE] == ZLOG_DISABLED) || !zl->fp)
		vty_out(vty, "disabled");
	else
	{
		vty_out(vty, "level %s, filename %s",
				zlog_priority_name(zl->maxlvl[ZLOG_DEST_FILE]), zl->filename);
		vty_out(vty, ", size %d M",zl->filesize);
	}
	vty_out(vty, "%s", VTY_NEWLINE);

	vty_out(vty, "Buffer logging      : ");
	if ((zl->maxlvl[ZLOG_DEST_BUFFER] == ZLOG_DISABLED) || !zl->fp)
		vty_out(vty, "disabled");
	else
	{
		vty_out(vty, "level %s, size %d",
				zlog_priority_name(zl->maxlvl[ZLOG_DEST_BUFFER]), zl->log_buffer.max_size);
	}
	vty_out(vty, "%s", VTY_NEWLINE);


	vty_out(vty, "Protocol name       : %s%s", zlog_proto_names(zl->protocol),
			VTY_NEWLINE);
	vty_out(vty, "Record priority     : %s%s",
			(zl->record_priority ? "enabled" : "disabled"), VTY_NEWLINE);

	if(zl->timestamp == ZLOG_TIMESTAMP_NONE)
		vty_out(vty, "Timestamp           : none%s",VTY_NEWLINE);
	else if(zl->timestamp == ZLOG_TIMESTAMP_DATE)
		vty_out(vty, "Timestamp           : data%s",VTY_NEWLINE);
	else if(zl->timestamp == ZLOG_TIMESTAMP_SHORT)
		vty_out(vty, "Timestamp           : short%s",VTY_NEWLINE);
	else if(zl->timestamp == ZLOG_TIMESTAMP_BSD)
		vty_out(vty, "Timestamp           : bsd%s",VTY_NEWLINE);
	else if(zl->timestamp == ZLOG_TIMESTAMP_ISO)
		vty_out(vty, "Timestamp           : iso%s",VTY_NEWLINE);
	else if(zl->timestamp == ZLOG_TIMESTAMP_RFC3164)
		vty_out(vty, "Timestamp           : rfc3164%s",VTY_NEWLINE);
	else if(zl->timestamp == ZLOG_TIMESTAMP_RFC3339)
		vty_out(vty, "Timestamp           : rfc3339%s",VTY_NEWLINE);

#ifdef ZLOG_TESTING_ENABLE
	if(zl->testing)
	{
		vty_out(vty, "Testing logging     : ");
		vty_out(vty, "level %s, filename %s",
					zlog_priority_name(zl->testlog.priority), zl->testlog.filename);
		vty_out(vty, ", size %d M",zl->testlog.filesize);
		vty_out(vty, "%s", VTY_NEWLINE);

	}
#endif
	return CMD_SUCCESS;
}

static int show_logbuffer_module_level(zbuffer_t *pstBuf, void *pVoid)
{
	struct logfilter * pUser = (struct logfilter *) pVoid;
	if ((pstBuf->module == pUser->module) && pUser->module)
	{
		if ((pstBuf->level == pUser->priority) && pUser->priority)
		{
			vty_out (pUser->vty, "%s", pstBuf->log);
			return CMD_SUCCESS;
		}
	}
	if ((pstBuf->level == pUser->priority) && pUser->priority)
	{
		vty_out (pUser->vty, "%s", pstBuf->log);
		return CMD_SUCCESS;
	}
	if ((pstBuf->module == pUser->module) && pUser->module)
	{
		vty_out (pUser->vty, "%s", pstBuf->log);
		return CMD_SUCCESS;
	}
	if(pUser->size)
	{
		vty_out (pUser->vty, "%s", pstBuf->log);
		pUser->total++;
		if(pUser->size == pUser->total)
			return CMD_WARNING;
		return CMD_SUCCESS;
	}
	vty_out (pUser->vty, "%s", pstBuf->log);
	return CMD_SUCCESS;
}

DEFUN (show_config_log_buffer,
		show_config_log_buffer_cmd,
		"show logging buffer",
		SHOW_STR
		"Logging control\n"
		"buffer information\n")
{
	struct logfilter pUser;
	os_memset(&pUser, 0, sizeof(pUser));
	pUser.vty = vty;
	if(argc == 1)
	{
		int level = zlog_priority_match(argv[0]);
		if(level == 0)
		{
			if(all_digit(argv[0]) == 0)
			{
				//TODO module
			}
			else
			{
				pUser.size = atoi(argv[0]);
			}
		}
		else
			pUser.priority = level;
	}
	else if(argc == 2)
	{
		//TODO module argv[0]
		pUser.priority = zlog_priority_match(argv[1]);
	}
	zlog_buffer_callback_api(show_logbuffer_module_level, &pUser);
	return CMD_SUCCESS;
}

ALIAS(show_config_log_buffer,
		show_config_log_buffer_level_cmd,
		"show logging buffer " LOG_LEVELS,
		SHOW_STR
		"Logging control\n"
		"buffer information\n"
		LOG_LEVEL_DESC)

ALIAS(show_config_log_buffer,
		show_config_log_buffer_module_cmd,
		"show logging buffer (ospf|bgp|vlan|mac)",
		SHOW_STR
		"Logging control\n"
		"buffer information\n"
		"OSPF module\n"
		"BGP module\n"
		"VLAN module\n"
		"MAC module\n")


ALIAS(show_config_log_buffer,
		show_config_log_buffer_module_level_cmd,
		"show logging buffer (ospf|bgp|vlan|mac) " LOG_LEVELS,
		SHOW_STR
		"Logging control\n"
		"buffer information\n"
		"OSPF module\n"
		"BGP module\n"
		"VLAN module\n"
		"MAC module\n"
		LOG_LEVEL_DESC)


ALIAS(show_config_log_buffer,
		show_config_log_buffer_size_cmd,
		"show logging buffer <1-512>",
		SHOW_STR
		"Logging control\n"
		"buffer information\n"
		"size to show\n")



DEFUN (show_config_log_file,
		show_config_log_file_cmd,
		"show logging file",
		SHOW_STR
		"Logging control\n"
		"log file information\n")
{
	FILE *fp = NULL;
	char filetmp[256];
	char buf[4096];
	if (zlog_default == NULL || !zlog_default->filename)
		return CMD_WARNING;
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);

	memset(filetmp, 0, sizeof(filetmp));
	sprintf(filetmp, "%s%s", ZLOG_VIRTUAL_PATH, zlog_default->filename);
	fp = fopen (filetmp, "r");

	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
			char *s;
			/* work backwards to ignore trailling isspace() */
			for (s = buf + strlen(buf); (s > buf) && isspace((int) *(s - 1)); s--)
				;
			*s = '\0';
			vty_out(vty, "%s%s", buf, VTY_NEWLINE);
		}
		fclose(fp);
	}
	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);
	return CMD_SUCCESS;
}




int cmd_log_init()
{
	install_element(VIEW_NODE, &show_logging_cmd);
	install_element(VIEW_NODE, &show_config_log_file_cmd);
	install_element(VIEW_NODE, &show_config_log_buffer_cmd);
	install_element(VIEW_NODE, &show_config_log_buffer_level_cmd);
	install_element(VIEW_NODE, &show_config_log_buffer_module_cmd);
	install_element(VIEW_NODE, &show_config_log_buffer_module_level_cmd);
	install_element(VIEW_NODE, &show_config_log_buffer_size_cmd);

	install_element(ENABLE_NODE, &config_logmsg_cmd);
	install_element(CONFIG_NODE, &config_log_stdout_cmd);
	install_element(CONFIG_NODE, &config_log_stdout_level_cmd);
	install_element(CONFIG_NODE, &no_config_log_stdout_cmd);
	install_element(CONFIG_NODE, &config_log_monitor_cmd);
	install_element(CONFIG_NODE, &config_log_monitor_level_cmd);
	install_element(CONFIG_NODE, &no_config_log_monitor_cmd);
	install_element(CONFIG_NODE, &config_log_file_cmd);
	install_element(CONFIG_NODE, &config_log_file_level_cmd);
	install_element(CONFIG_NODE, &config_log_file_size_cmd);
	install_element(CONFIG_NODE, &no_config_log_file_cmd);
	install_element(CONFIG_NODE, &no_config_log_file_level_cmd);
	install_element(CONFIG_NODE, &no_config_log_file_size_cmd);

	install_element(CONFIG_NODE, &config_log_buffer_cmd);
	install_element(CONFIG_NODE, &config_log_buffer_level_cmd);
	install_element(CONFIG_NODE, &config_log_buffer_size_cmd);
	install_element(CONFIG_NODE, &no_config_log_buffer_cmd);
	install_element(CONFIG_NODE, &no_config_log_buffer_level_cmd);


	install_element(CONFIG_NODE, &save_log_file_cmd);


	install_element(CONFIG_NODE, &config_log_syslog_cmd);
	install_element(CONFIG_NODE, &config_log_syslog_level_cmd);

	//install_element(CONFIG_NODE, &config_log_syslog_facility_cmd);
	install_element(CONFIG_NODE, &no_config_log_syslog_cmd);
	install_element(CONFIG_NODE, &no_config_log_syslog_level_cmd);

	install_element(CONFIG_NODE, &config_log_facility_cmd);
	install_element(CONFIG_NODE, &no_config_log_facility_cmd);

	install_element(CONFIG_NODE, &config_log_trap_cmd);
	install_element(CONFIG_NODE, &no_config_log_trap_cmd);

	install_element(CONFIG_NODE, &config_log_record_priority_cmd);
	install_element(CONFIG_NODE, &no_config_log_record_priority_cmd);
	install_element(CONFIG_NODE, &config_log_timestamp_cmd);
	install_element(CONFIG_NODE, &no_config_log_timestamp_cmd);

#ifdef PL_SYSLOG_MODULE
	install_element(CONFIG_NODE, &syslog_host_cmd);
	install_element(CONFIG_NODE, &syslog_host_port_cmd);
	install_element(CONFIG_NODE, &no_syslog_host_cmd);
	install_element(CONFIG_NODE, &syslog_mode_cmd);
	install_element(CONFIG_NODE, &no_syslog_mode_cmd);
#endif

#ifdef ZLOG_TESTING_ENABLE
	install_element(CONFIG_NODE, &config_log_testing_cmd);
	install_element(CONFIG_NODE, &config_log_testing_level_cmd);
	install_element(CONFIG_NODE, &no_config_log_testing_cmd);
	install_element(CONFIG_NODE, &no_config_log_testing_level_cmd);
	install_element(CONFIG_NODE, &config_log_testing_file_cmd);
	install_element(CONFIG_NODE, &config_log_testing_file_level_cmd);
	install_element(CONFIG_NODE, &no_config_log_testing_file_cmd);
	install_element(CONFIG_NODE, &no_config_log_testing_file_level_cmd);
	install_element(VIEW_NODE, &show_config_log_file_testing_cmd);
#endif
	return OK;
}
