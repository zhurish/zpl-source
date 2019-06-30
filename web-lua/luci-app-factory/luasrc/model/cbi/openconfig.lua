-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"


m2 = Map("openconfig", translate("Open Configure"), translate("Open Configure"))
-- s = m2:section(TypedSection, "sip", translate("SIP stack configure option"))
s = m2:section(NamedSection, "open", "hardware", translate("Open Configure option"))
s.anonymous = false
s.addremove = false


-- opentime
opentime = s:option(Value, "opentime", translate("Open Time"))
opentime.datatype = "range(0,255)"
opentime.rmempty = true
opentime.default=2

-- openholdtime
openwaittime = s:option(Value, "openwaittime", translate("Open Wait Time"))
openwaittime.datatype = "range(0,255)"
openwaittime.rmempty = true
openwaittime.default=2

-- relay_active_level
relay_active_level = s:option(ListValue, "relay_active_level", translate("Sensor Output Level"))
relay_active_level.rmempty = true
relay_active_level.default='Low'
relay_active_level:value("High",translate("High"))
relay_active_level:value("Low",translate("Low"))
relay_active_level.rmempty = true


tamperalarm = s:option(Flag, "tamperalarm", translate("Tamper Alarm"))
tamperalarm.rmempty = true
tamperalarm.default=false

keep_open_alarm = s:option(Flag, "keep_open_alarm", translate("Keep Open Alarm"))
keep_open_alarm.rmempty = true
keep_open_alarm.default=true

-- opendelaytime
openholdtime = s:option(Value, "openholdtime", translate("Open Hold Time"))
openholdtime.datatype = "range(0,255)"
openholdtime.rmempty = true
openholdtime.default=15
openholdtime:depends("keep_open_alarm", "1")

-- opentype
opentype = s:option(ListValue, "opentype", translate("Open Type"))
opentype.default='FaceAndCard'
opentype:value("Card",translate("Card"))
opentype:value("Face",translate("Face"))
opentype:value("FaceAndCard",translate("FaceAndCard"))
opentype:value("FaceOrCard",translate("FaceOrCard"))
opentype.rmempty = true

-- advanced
advanced = s:option(Flag, "advanced", translate("Advanced"))
advanced.default=false
advanced.rmempty = true

-- wiggins
wiggins = s:option(ListValue, "wiggins", translate("Wiggins Bit"))
wiggins.rmempty = true
wiggins.default='26-Bit'
wiggins:value("26-Bit",translate("26-Bit"))
wiggins:value("34-Bit",translate("34-Bit"))
wiggins:value("66-Bit",translate("66-Bit"))
wiggins:depends("advanced", "1")


local apply=luci.http.formvalue("cbi.apply")

if apply then
  local pfd = io.popen("lua-sync -m app -c open-option")
  if pfd then
    pfd:close()
  end 
end

return m2