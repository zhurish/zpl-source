/*
 * dhcpcd - DHCP client daemon
 * Copyright (c) 2006-2012 Roy Marples <roy@marples.name>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef CONFIG_H
#define CONFIG_H

#define PACKAGE			"dhcpcd"
#define VERSION			"5.5.6"

#ifndef CONFIG
# define CONFIG			DAEMON_VTY_DIR "/" PACKAGE ".conf"
#endif
#ifndef SCRIPT
# define SCRIPT			DAEMON_VTY_DIR "/" PACKAGE "-run-hooks"
#endif
#ifndef DUID
# define DUID			DAEMON_VTY_DIR "/" PACKAGE ".duid"
#endif
#ifndef LEASEFILE
# define LEASEFILE		DAEMON_VTY_DIR "/" PACKAGE "-%s.lease"
#endif
#ifndef PIDFILE
# define PIDFILE		DAEMON_VTY_DIR "/" PACKAGE "%s%s.pid"
#endif
#ifndef CONTROLSOCKET
# define CONTROLSOCKET		DAEMON_VTY_DIR "/" PACKAGE ".sock"
#endif

#endif
