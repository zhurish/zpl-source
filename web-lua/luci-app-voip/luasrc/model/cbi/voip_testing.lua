-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"
local uci = require "luci.model.uci"
local http = require "luci.http"
local NX   = require "nixio"
-- 
-- Call Testing
--
local testing = uci.cursor()
local callstate = testing:get("voipconfig","testing","callstate") 
local call_num = testing:get("voipconfig","testing","callnum")


function call_enabled()                                              
        local flag = false                                                                                   
        if callstate and callstate == "1" then                       
        flag = true                                                                                                                  
        else                                                                
        flag = false                                                                                           
        end                                                                                             
        return flag                                    
end 
    
function call_sys_app_sync_cmd(cmd)                                              
  local fp = io.popen(cmd, "r") 
  if fp then  
    local fvalue = fp:read("*a") 
    if fvalue then                       
      fp:close()                                                    
      if fvalue:find("OK") then
        return true
      else
        return false  
      end                                                                                                          
    end 
    fp:close()      
  end
  return false                                      
end  
       
--[[        
m2 = Map("voipconfig", translate("Voip Testing"), 
  translate("Voip Testing,eg call phome nummber 2003 or 2004->@2003:2004; \
  call room 203->203; call unit 2, room 309->2:309; call building 1, unit 2, room 309->1:2:309"))
]]
m2 = SimpleForm("voip-testing", translate("Voip Testing"), 
  translate("Voip Testing,eg call phome nummber 2003 or 2004->@2003:2004; \
  call room 203->203; call unit 2, room 309->2:309; call building 1, unit 2, room 309->1:2:309"))
  
m2.reset = false
m2.submit = false
m2.apply = true
m2.save = false

local callTest = { }
callTest[" "] = {callnum=call_num, enabled=call_enabled()}

--s = m2:section(SimpleSection, nil, nil)
s = m2:section(Table, callTest)
--s = m2:section(NamedSection, "testing", "voip", translate("Call Testing"))
s.anonymous = false
s.addremove = false

-- callnum
call_number = s:option(Value, "callnum", translate("RoomNumber"))
call_number.rmempty = true
call_number.maxlength=32

function call_number.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then  
    testing:set("voipconfig","testing","enable", 1)
    testing:set("voipconfig","testing","callnum", value)  
    testing:commit("voipconfig")
  end                                                
end

function call_number.cfgvalue()
	local call_tmp = testing:get("voipconfig","testing","callnum")
	return call_tmp or ""
end

--function callnum.validate(self, value, section)   
--     return value and value:match("^[0-9@]")  
--end   
  
call_cmd = s:option(Button, "callendisable", translate("StartCall/StopCall"))

call_cmd.render = function(self, section, scope)
  if callTest[section].enabled then
    self.title = translate("StopCall")
    self.inputstyle = "remove"
  else
    self.title = translate("StartCall")
    self.inputstyle = "apply"
  end
  Button.render(self, section, scope)
end

call_cmd.write = function(self, section)
  if callTest[section].enabled then
    if call_sys_app_sync_cmd("lua-sync -m voip -c stop-call") then
      testing:set("voipconfig","testing","callnum","")
      testing:commit("voipconfig")
      callTest[section].enabled = false
    end
    return true
  else        
    testing:commit("voipconfig")
    if call_sys_app_sync_cmd("lua-sync -m voip -c start-call") then
      callTest[section].enabled = true
    else
      testing:set("voipconfig","testing","callnum","")
      testing:commit("voipconfig")
    end  
  end
end


-- 
-- Capture Testing
--

--m3 = Map("voipconfig", translate("Capture Testing"), translate("Capture Testing"))
m3 = SimpleForm("Capture", "Capture Testing", translate("Capture Testing"))
m3.rmempty = true
m3.reset = false
m3.submit = false
m3.apply = true
m3.save = false

function capture_enabled()
  local en_tmp = testing:get("voipconfig","testing","capture_enable")
  if en_tmp and en_tmp == "1" then
    return true;
  else
    return false
  end 
end

local loglevel = testing:get("voipconfig","testing","log_level") 
local logmodsip = testing:get("voipconfig","testing","log_mod_sip")
local logmodmedia = testing:get("voipconfig","testing","log_mod_media") 
local logmodapp = testing:get("voipconfig","testing","log_mod_app")

function capture_download()

  local capturefile = testing:get("voipconfig","testing","capturefile")
  if capturefile == nil then
    return
  end
  --luci.sys.call("env -i /sbin/ifdown %q >/dev/null 2>/dev/null" % iface)
  luci.sys.call("cd /tmp/app && tar -zcf %s tmp >/dev/null 2>/dev/null" % capturefile)
  
  if fs.access("/tmp/app/%s" % capturefile) then
    local fd = NX.open("/tmp/app/%s" % capturefile)
    if not fd then
      luci.sys.call("rm /tmp/app/%s >/dev/null 2>/dev/null" % capturefile)
      return
    end
    
    http.header('Content-Disposition', 'attachment; filename="%s"' % capturefile)
    http.prepare_content("application/octet-stream")
    while true do
      local block = fd:read(NX.const.buffersize)
      if (not block) or (#block ==0) then
        break
      else
        http.write(block)
      end
    end
    fd:close()
    http.close()
    luci.sys.call("rm /tmp/app/%s >/dev/null 2>/dev/null" % capturefile)
  end
end

--s3 = m3:section(SimpleSection, nil, nil)
local capture_table = { }
capture_table[" "] = {loglevel=loglevel, logmodsip=logmodsip, logmodmedia=logmodmedia, logmodapp=logmodapp, enabled=capture_enabled()}
s3 = m3:section(Table,capture_table)
--local capturefile = testing:get("voipconfig","testing","capturefile")


log_level = s3:option(ListValue, "log_level", translate("Debug Level"))
log_level.default = 'Debug'
log_level:value("Debug",translate("Debug"))
log_level:value("Information",translate("Information"))
log_level:value("Notifications",translate("Notifications"))
log_level:value("Warnings",translate("Warnings"))

function log_level.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then  
  testing:set("voipconfig","testing","log_level", value) 
  testing:commit("voipconfig")    
  end                                                
end
function log_level.cfgvalue()
  local l_tmp = testing:get("voipconfig","testing","log_level")
  return l_tmp  or "Debug" 
end

-- voip_module
sip_module = s3:option(Flag, "log_mod_sip", translate("SIP"))
sip_module.rmempty = true
sip_module.default = true

function sip_module.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then  
  testing:set("voipconfig","testing","log_mod_sip", value) 
  testing:commit("voipconfig")    
  end                                                
end
function sip_module.cfgvalue()
  local l_tmp = testing:get("voipconfig","testing","log_mod_sip")
  return l_tmp or true 
end

-- app_module
app_module = s3:option(Flag, "log_mod_app", translate("app"))
app_module.rmempty = true
app_module.default = true

function app_module.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then  
  testing:set("voipconfig","testing","log_mod_app", value) 
  testing:commit("voipconfig")    
  end                                                
end
function app_module.cfgvalue()
  local l_tmp =  testing:get("voipconfig","testing","log_mod_app")
  return l_tmp  or true 
end

-- media_module
media_module = s3:option(Flag, "log_mod_media", translate("Media"))
media_module.rmempty = true
media_module.default = false

function media_module.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then  
  testing:set("voipconfig","testing","log_mod_media", value) 
  testing:commit("voipconfig")    
  end                                                
end
function media_module.cfgvalue()
  local l_tmp =  testing:get("voipconfig","testing","log_mod_media")
  return l_tmp or true 
end


--local capture_table = { }
--capture_table["%s" % "Capture"] = {name="Capture", enabled=capture_enable}
--s = m3:section(Table,capture_table)
capcmd = s3:option(Button, "captureendisable", translate("Start-Capture/Stop-Capture"))

capcmd.render = function(self, section, scope)
        if capture_table[section].enabled then
                self.title = translate("Stop-Capture")
                self.inputstyle = "remove"
        else
                self.title = translate("Start-Capture")
                self.inputstyle = "apply"
        end

        Button.render(self, section, scope)
end

capcmd.write = function(self, section)
  if capture_table[section].enabled then
    if call_sys_app_sync_cmd("lua-sync -m voip -c stop-capture") then
      capture_table[section].enabled = false
    end
  else        
    testing:set("voipconfig","testing","capture_enable",1)
    testing:delete("voipconfig","testing","capturefile")
    testing:commit("voipconfig")
    if call_sys_app_sync_cmd("lua-sync -m voip -c start-capture") then
      capture_table[section].enabled = true
    else
      testing:set("voipconfig","testing","capture_enable",0)
      testing:commit("voipconfig")
    end  
  end
    --[[    if capture_table[section].enabled then
                capture_table[section].enabled = false
                io.popen("lua-sync -m voip -c stop-capture")
                return true
        else
                capture_table[section].enabled = true
                testing:set("voipconfig","testing","capture_enable",1)
                testing:delete("voipconfig","testing","capturefile")
                --uci delete voipconfig.testing.capturefile
                testing:commit("voipconfig")
                io.popen("lua-sync -m voip -c start-capture")
                return true
        end
        ]]
end
--[[
down_btn = s3:option(Button, "download", translate("Download"))
down_btn.inputstyle = "download"

down_btn.write = function(self, section)
  sys.call("echo adasdasdas > /tmp/down.txt")
end
]]


cap_download = s3:option(FileUpload, "")
cap_download.template = "admin_voip/capture_download"

if luci.http.formvalue("download") then
  sys.call("echo adasdasdas > /tmp/down.txt")
  capture_download()
  --if capture_enabled() then
  --  io.popen("lua-sync -m voip -c stop-capture")
  --end
end
--local apply=luci.http.formvalue("cbid.table. .download")

--if apply then
  --sys.call("env -i /usr/bin/lua-sync -m sip -c restart >/dev/null")
--  io.popen("echo adasdasdas > /tmp/down.txt")
  --io.popen("killall -9 VmrMgr")
--end
return m2,m3