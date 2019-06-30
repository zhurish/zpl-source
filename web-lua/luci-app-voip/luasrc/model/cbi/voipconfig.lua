-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"


m2 = Map("voipconfig", translate("Voip Configure"), translate("Voip Configure"))
-- s = m2:section(TypedSection, "voip", translate("Voip configure option"))
s = m2:section(NamedSection, "voip", "voip", translate("Voip configure option"))

s.anonymous = false
s.addremove = false

enable = s:option(Flag, "enable", translate("Enable-Voip"))
enable.rmempty = true
enable.default=1

-- local port
localport = s:option(Value, "localport", translate("Local-Port"))
localport.datatype = "range(64,65536)"
localport.rmempty = true
localport.default=5060

-- volume
volume = s:option(Value, "volume", translate("Volume"))
volume.datatype = "range(0,100)"
volume.rmempty = true
volume.default=80

-- advanced
advanced = s:option(Flag, "advanced", translate("Advanced"))
advanced.rmempty = true
advanced.default = false



-- dtmf_echo
dtmf_echo = s:option(Flag, "enable", translate("DTMF Echo"))
dtmf_echo.rmempty = true
dtmf_echo.default = false
dtmf_echo:depends("advanced", "1")


-- disbale_rtcp
disbale_rtcp = s:option(Flag, "disbale_rtcp", translate("Disable RTCP"))
disbale_rtcp.rmempty = true
disbale_rtcp.default = false
disbale_rtcp:depends("advanced", "1")

-- disbale_avpf
disbale_avpf = s:option(Flag, "disbale_avpf", translate("Disable AVPF"))
disbale_avpf.rmempty = true
disbale_avpf.default = false
disbale_avpf:depends("advanced", "1")

-- bitrate
bitrate = s:option(Value, "bitrate", translate("Bitrate"))
bitrate.datatype = "range(0,10000000)" --
bitrate.rmempty = true
bitrate.default = 0
bitrate:depends("advanced", "1")

-- jitter
jitter = s:option(Value, "jitter", translate("Jitter"))
jitter.datatype = "range(0,10000000)" --miliseconds
jitter.rmempty = true
jitter.default = 0
jitter:depends("advanced", "1")

-- ec_canceller
ec_canceller = s:option(Flag, "ec_canceller", translate("Echo Canceller"))
ec_canceller.rmempty = true
ec_canceller.default = true
ec_canceller:depends("advanced", "1")

-- ec_tail
ec_tail = s:option(Value, "ec_tail", translate("Echo Tail"))
ec_tail.datatype = "range(0,65536)"
ec_tail.rmempty = true
ec_tail.default = 0
ec_tail:depends("ec_canceller", "1")

-- ec_delay
ec_delay = s:option(Value, "ec_delay", translate("Echo Delay"))
ec_delay.datatype = "range(0,65536)"
ec_delay.rmempty = true
ec_delay.default = 0
ec_delay:depends("ec_canceller", "1")


-- ec_framesize
ec_framesize = s:option(Value, "ec_framesize", translate("Echo Framesize"))
ec_framesize.datatype = "range(0,65536)"
ec_framesize.rmempty = true
ec_framesize.default = 0
ec_framesize:depends("ec_canceller", "1")


-- ec_limiter
ec_limiter = s:option(Flag, "ec_limiter", translate("Echo Limiter"))
ec_limiter.rmempty = true
ec_limiter.default = true
ec_limiter:depends("advanced", "1")


-- el_force
el_force = s:option(Value, "el_force", translate("Echo Limiter Force"))
el_force.datatype = "range(0.0, 1.0)"
el_force.rmempty = true
el_force.default = 0.0
el_force:depends("ec_limiter", "1")

-- el_speed
el_speed = s:option(Value, "el_speed", translate("Echo Limiter Speed"))
el_speed.datatype = "range(0.0, 1.0)"
el_speed.rmempty = true
el_speed.default = 0.0
el_speed:depends("ec_limiter", "1")


-- el_thres
el_thres = s:option(Value, "el_thres", translate("Echo Limiter Threshold"))
el_thres.datatype = "range(0.0, 1.0)"
el_thres.rmempty = true
el_thres.default = 0.0
el_thres:depends("ec_limiter", "1")

-- el_transmit
el_transmit = s:option(Value, "el_transmit", translate("Echo Limiter Transmit"))
el_transmit.datatype = "range(0.0, 1.0)"
el_transmit.rmempty = true
el_transmit.default = 0.0
el_transmit:depends("ec_limiter", "1")

-- el_sustain
el_sustain = s:option(Value, "el_sustain", translate("Echo Limiter Sustain"))
el_sustain.datatype = "range(0,65536)"
el_sustain.rmempty = true
el_sustain.default = 0
el_sustain:depends("ec_limiter", "1")


-- noise_gate
noise_gate = s:option(Flag, "noise_gate", translate("Noise Gate"))
noise_gate.rmempty = true
noise_gate.default = true
noise_gate:depends("advanced", "1")

-- ng_floorgain
ng_floorgain = s:option(Value, "ng_floorgain", translate("Noise Floorgain"))
ng_floorgain.datatype = "range(0.0, 1.0)"
ng_floorgain.rmempty = true
ng_floorgain.default = 0.0
ng_floorgain:depends("noise_gate", "1")

-- ng_threshold
ng_threshold = s:option(Value, "ng_threshold", translate("Noise Threshold"))
ng_threshold.datatype = "range(0.0, 1.0)"
ng_threshold.rmempty = true
ng_threshold.default = 0.0
ng_threshold:depends("noise_gate", "1")


local apply=luci.http.formvalue("cbi.apply")

if apply then
  local pfd = io.popen("lua-sync -m voip -c restart")
  if pfd then
    pfd:close()
  end 
end

return m2