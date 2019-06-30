-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"


m2 = Map("voipconfig", translate("Sip Configure"), translate("Sip Configure for Voip"))
-- s = m2:section(TypedSection, "sip", translate("SIP stack configure option"))
s = m2:section(NamedSection, "sip", "voip", translate("SIP stack configure option"))
s.anonymous = false
s.addremove = false

enable = s:option(Flag, "enable", translate("Enable-Sip"))
enable.rmempty = true
enable.default=true

--local Interface
sip_source_interface = s:option(ListValue, "sip_source_interface", translate("Local-Interface"))
web.cbi_add_networks(sip_source_interface)

-- local port
sip_local_port = s:option(Value, "sip_local_port", translate("Local-Port"))
sip_local_port.datatype = "range(64,65536)"
sip_local_port.rmempty = true
sip_local_port.default=5060

-- using
sip_multi_user = s:option(Flag, "sip_multi_user", translate("MultiUser"))
sip_multi_user.rmempty = true
sip_multi_user.default=false

-- local phone number
localphone = s:option(Value, "localphone", translate("PhoneNumber"))
localphone.rmempty = true
localphone.default='8001'
--localphone.datatype="maxlength(16)"
localphone.maxlength=32

-- username
username = s:option(Value, "username", translate("Username"))
-- username.rmempty = true
username.default='8001'
--username.datatype="maxlength(32)"
username.maxlength=32

-- password
password = s:option(Value, "password", translate("Password"))
password.rmempty = true
password.default='123456'
--password.datatype="maxlength(32)"
password.maxlength=32

-- Second local phone number
localphone_sec = s:option(Value, "localphone_sec", translate("PhoneNumber 2"))
localphone_sec.rmempty = true
localphone_sec.default=''
localphone_sec:depends("sip_multi_user", "1")
--localphone_sec.datatype="maxlength(16)"
localphone_sec.maxlength=32

-- Second username
username_sec = s:option(Value, "username_sec", translate("Username 2"))
-- username_sec.rmempty = true
username_sec.default=''
username_sec:depends("sip_multi_user", "1")
--username_sec.datatype="maxlength(32)"
username_sec.maxlength=32

-- Second password
password_sec = s:option(Value, "password_sec", translate("Password 2"))
password_sec.rmempty = true
password_sec.default=''
password_sec:depends("sip_multi_user", "1")
--password_sec.datatype="maxlength(32)"
password_sec.maxlength=32

-- sip server address
sip_server = s:option(Value, "sip_server", translate("SIP-Server-Address"))
sip_server.datatype = "ip4addr"
sip_server.rmempty = true

-- sip server port
sip_port = s:option(Value, "sip_port", translate("SIP-Server-Port"))
sip_port.datatype = "range(64,65536)"
sip_port.rmempty = true
sip_port.default=5060

-- Active/Standby
sip_active_standby = s:option(Flag, "sip_active_standby", translate("Active/Standby"))
sip_active_standby.rmempty = true
sip_active_standby.default=false

-- KeepAlive
sip_keepalive = s:option(Flag, "sip_keepalive", translate("KeepAlive"))
sip_keepalive.rmempty = true
sip_keepalive.default = true
sip_keepalive:depends("sip_active_standby", "1")


-- sip_keepalive_interval
sip_keepalive_interval = s:option(Value, "sip_keepalive_interval", translate("KeepAlive Interval"))
sip_keepalive_interval.datatype = "range(1,60)"
sip_keepalive_interval.rmempty = true
sip_keepalive_interval.default=5
sip_keepalive_interval:depends("sip_keepalive", "1")


-- Second sip server address
sip_server_sec = s:option(Value, "sip_server_sec", translate("Backups SIP-Server-Address"))
sip_server_sec.datatype = "ip4addr"
sip_server_sec.rmempty = true
sip_server_sec:depends("sip_active_standby", "1")

-- Second sip server port
sip_port_sec = s:option(Value, "sip_port_sec", translate("Backups SIP-Server-Port"))
sip_port_sec.datatype = "range(64,65536)"
sip_port_sec.rmempty = true
sip_port_sec.default=5060
sip_port_sec:depends("sip_active_standby", "1")


-- Enable Proxy
sip_proxy_enable = s:option(Flag, "sip_proxy_enable", translate("Enable Proxy"))
sip_proxy_enable.rmempty = true
sip_proxy_enable.default=false

-- sip proxy server address
sip_proxy_server = s:option(Value, "sip_proxy_server", translate("SIP-Proxy-Address"))
sip_proxy_server.datatype = "ip4addr"
sip_proxy_server.rmempty = true
sip_proxy_server:depends("sip_proxy_enable", "1")

-- sip proxy server port
sip_proxy_port = s:option(Value, "sip_proxy_port", translate("SIP-Proxy-Port"))
sip_proxy_port.datatype = "range(64,65536)"
sip_proxy_port.rmempty = true
sip_proxy_port.default=5060
sip_proxy_port:depends("sip_proxy_enable", "1")

-- Second proxy server address
sip_proxy_server_sec = s:option(Value, "sip_proxy_server_sec", translate("Backups SIP-Proxy-Address"))
sip_proxy_server_sec.datatype = "ip4addr"
sip_proxy_server_sec.rmempty = true
sip_proxy_server_sec:depends("sip_proxy_enable", "1")

-- Second proxy server port
sip_proxy_port_sec = s:option(Value, "sip_proxy_port_sec", translate("Backups SIP-Proxy-Port"))
sip_proxy_port_sec.datatype = "range(64,65536)"
sip_proxy_port_sec.rmempty = true
sip_proxy_port_sec.default=5060
sip_proxy_port_sec:depends("sip_proxy_enable", "1")


-- protocol
protocol = s:option(ListValue, "proto", translate("Transport Proto"))
protocol.rmempty = true
protocol.default='UDP'
protocol:value("UDP",translate("UDP"))
protocol:value("TCP",translate("TCP"))
protocol:value("TLS",translate("TLS"))
protocol.rmempty = true


-- payload
payload = s:option(ListValue, "payload", translate("Speech Codec"))
payload.rmempty = true
payload.default='PCMU'
payload:value("PCMU",translate("PCMU"))
payload:value("PCMA",translate("PCMA"))
--payload:value("L016",translate("L016"))
--payload:value("LPC",translate("LPC"))
payload:value("G729",translate("G729"))
payload:value("G7221",translate("G7221"))
--payload:value("G726-40",translate("G726-40"))
--payload:value("G726-32",translate("G726-32"))
--payload:value("G726-24",translate("G726-24"))
--payload:value("G726-16",translate("G726-16"))
--payload:value("AAL2-G726-40",translate("AAL2-G726-40"))
--payload:value("AAL2-G726-32",translate("AAL2-G726-32"))
--payload:value("AAL2-G726-24",translate("AAL2-G726-24"))
--payload:value("AAL2-G726-16",translate("AAL2-G726-16"))
--payload:value("SPEEX-NB",translate("SPEEX-NB"))
--payload:value("SPEEX-WB",translate("SPEEX-WB"))
payload:value("iLBC",translate("iLBC"))
--payload:value("L015",translate("L015"))
--payload:value("AMR",translate("AMR"))
payload:value("G722",translate("G722"))
payload:value("OPUS",translate("OPUS"))
payload.rmempty = true


-- register interval
registerinterval = s:option(Value, "registerinterval", translate("Register Interval"))
registerinterval.rmempty = true
registerinterval.datatype = "range(64,65536)"
registerinterval.default=1200

-- dtmf
dtmf = s:option(ListValue, "dtmf", translate("DTMF"))
dtmf.default='RFC2833'
--dt = s:taboption("dtmf", ListValue, "DTMF", translate("DTMF"))
dtmf:value("RFC2833",translate("RFC2833"))
dtmf:value("SIP-INFO",translate("SIP-INFO"))
dtmf:value("inband",translate("inband"))
--dt.rmempty = true

-- realm
-- realm = s:option(Value, "realm", translate("realm"))



local apply=luci.http.formvalue("cbi.apply")

if apply then
  --sys.call("env -i /usr/bin/lua-sync -m sip -c restart >/dev/null")
  local pfd = io.popen("lua-sync -m sip -c restart")
  if pfd then
    pfd:close()
  end 
  --io.popen("killall -9 VmrMgr")
end

return m2