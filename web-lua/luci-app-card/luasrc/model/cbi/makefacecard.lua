-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys  = require "luci.sys"
local web  = require "luci.tools.webadmin"
local fs   = require "nixio.fs"
local NX   = require "nixio"
local http = require "luci.http"
local util = require "nixio.util"

local uci = require "luci.model.uci"
local testing = uci.cursor()

--[[
function get_current_cardid()   
  --     
  -- WLAN, WIFI 4; LAN 0x28; WAN 0x2e;    
  --                                                                                         
  local mval = io.popen("hexdump -v -n 6 -s 0x04 -e '5/1 \"%02x:\" 1/1 \"%02x\"' /dev/mtd2", "r")
  --local mval = nil                         
  if mval then                                                                                                      
    local mvalue = mval:read("*l")                                                                         
    if mvalue then                       
      mval:close()                                                       
      return mvalue                                                                                                           
    end
    mval:close()      
  end                                                                                                                                                                 
  return nil                                    
end
]]
function sys_app_sync_cmd(cmd)                                              
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

if not fs.access("/etc/config/facecard") then
  sys.call("echo \"config facecard 'db'\" > /etc/config/facecard") 
  sys.call("echo \"        option make_card '1'\" >> /etc/config/facecard")  
end
  
--sys_app_sync_cmd("lua-sync -m card -c ready")
--sys_app_sync_cmd("echo asdad > /tmp/card")
--[[
local pfd = io.popen("lua-sync -m card -c edit")
if pfd then
  pfd:close()
end 
  
local u_d = testing:get("facecard","db","username")
local ui_d = testing:get("facecard","db","userid")
local mc_d = testing:get("facecard","db","make_card")
local ci_d = testing:get("facecard","db","cardid")
local start_d = testing:get("facecard","db","start_date")
local stop_d = testing:get("facecard","db","stop_date")
local ti_d = testing:get("facecard","db","typeid")
local fi_d = testing:get("facecard","db","make_face")
--local fimg_d = testing:get("facecard","db","face_img")
--local enablemake = testing:get("facecard","db","enablemake")
]]

function make_card_enabled()                                              
  local en_tmp = testing:get("facecard","db","enablemake")
  if en_tmp and en_tmp == "1" then
    return true;
  else
    return false
  end                                    
end 
local enablemake = make_card_enabled()
--Factory MAC Address
m2 = SimpleForm("face_card", "Face ID and Card ID Configure", translate("Face ID and Card ID Configure Option"))
m2.rmempty = true
m2.reset = false
m2.submit = false
m2.apply = false

s = m2:section(SimpleSection, nil, nil)

en_make = s:option(Button, "endisable", translate("Enable/Disable"))
en_make.default=enablemake

en_make.render = function(self, section, scope)
  enablemake = make_card_enabled()
  if enablemake then
    self.title = translate("Disabled")
    self.inputstyle = "remove"
  else
    self.title = translate("Enabled")
    self.inputstyle = "apply"
  end
  Button.render(self, section, scope)
end

en_make.write = function(self, section)
  if enablemake then
    testing:set("facecard","db","enablemake", 0) 
    testing:commit("facecard") 
    enablemake = false
    sys_app_sync_cmd("lua-sync -m card -c stop")
  else
    testing:set("facecard","db","enablemake", 1) 
    testing:commit("facecard")
    enablemake = true   
    sys_app_sync_cmd("lua-sync -m card -c start")  
  end
end


username = s:option(Value, "username", translate("User Name"))
username.rmempty = true
--username.default=u_d
username.maxlength=32
--username.datatype="maxlength(32)"
function username.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("facecard","db","username", value) 
    testing:commit("facecard") 
  end                                                
end

function username.cfgvalue()
  local l_tmp =  testing:get("facecard","db","username")
  return l_tmp or "" 
end

userid = s:option(Value, "userid", translate("User ID"), translate("User ID must be unique"))
userid.rmempty = true
--userid.default=ui_d
userid.maxlength=32
--userid.datatype="maxlength(32)"
function userid.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("facecard","db","userid", value) 
    testing:commit("facecard") 
  end                                                
end

function userid.cfgvalue()
  local l_tmp =  testing:get("facecard","db","userid")
  return l_tmp or "" 
end

-- Edit
--[[
make_edit = s:option(Flag, "make_edit", translate("Edit Info"))
make_edit.rmempty = true
make_edit.default=mc_d
function make_edit.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("facecard","db","make_edit", value) 
    testing:commit("facecard") 
  end                                                
end

function make_edit.cfgvalue()
  local l_tmp =  testing:get("facecard","db","make_edit")
  return l_tmp or "" 
end
]]
-- Card 
make_card = s:option(Flag, "make_card", translate("Make Card"))
make_card.rmempty = true
make_card.default=true
function make_card.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("facecard","db","make_card", value) 
    testing:commit("facecard") 
  end                                                
end

function make_card.cfgvalue()
  local l_tmp =  testing:get("facecard","db","make_card")
  return l_tmp or "" 
end

cardid = s:option(Value, "cardid", translate("Card ID"), translate("Card ID must be unique"))
cardid.rmempty = true
--cardid.default=ci_d
cardid.maxlength=16
--cardid.datatype="maxlength(16)"
--cardid:depends("make_card", "1")
function cardid.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("facecard","db","cardid", value) 
    testing:commit("facecard") 
  end                                                
end

function cardid.cfgvalue()
  local l_tmp =  testing:get("facecard","db","cardid")
  return l_tmp or "" 
end

--[[
start_date = s:option(Value, "start_date", translate("Start DateTime"))
start_date.rmempty = true
start_date.default=start_d
start_date:depends("make_card", "1")
function start_date.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("facecard","db","start_date", value) 
    testing:commit("facecard") 
  end                                                
end

function start_date.cfgvalue()
  local l_tmp =  testing:get("facecard","db","start_date")
  return l_tmp or "" 
end

stop_date = s:option(Value, "stop_date", translate("Stop DateTime"))
stop_date.rmempty = true
stop_date.default=start_d
stop_date:depends("make_card", "1")
function stop_date.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("facecard","db","stop_date", value) 
    testing:commit("facecard") 
  end                                                
end

function stop_date.cfgvalue()
  local l_tmp =  testing:get("facecard","db","stop_date")
  return l_tmp or "" 
end
]]


start_date = s:option(DummyValue, "start_date", translate("Start DateTime"))
start_date.rmempty = true
start_date.default=os.date("%Y-%m-%dT%H:%M", os.time())
--start_date:depends("make_card", "1")
start_date.template = "admin_card/datetime-start"


stop_date = s:option(DummyValue, "stop_date", translate("Stop DateTime"))
stop_date.rmempty = true
--2019/04/16T04:10
stop_date.default=os.date("%Y-%m-%dT%H:%M", os.time())
--stop_date:depends("make_card", "1")
stop_date.template = "admin_card/datetime-stop"

--[[
if luci.http.formvalue("download") then
  sys.call("echo adasdasdas > /tmp/down.txt")
  capture_download()
end
]]

typeid = s:option(ListValue, "typeid", translate("Card Type"))
typeid.rmempty = true
--typeid.default=ti_d
--typeid:depends("make_card", "1")
typeid:value("Unknow",translate("Unknow"))
typeid:value("Blacklist",translate("Blacklist"))
typeid:value("Whitelist",translate("Whitelist"))
function typeid.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("facecard","db","typeid", value) 
    testing:commit("facecard") 
  end                                                
end

function typeid.cfgvalue()
  local l_tmp =  testing:get("facecard","db","typeid")
  return l_tmp or "" 
end


-- Face 
make_face = s:option(Flag, "make_face", translate("Make Face ID"))
make_face.rmempty = true
--make_face.default=fi_d
function make_face.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("facecard","db","make_face", value) 
    testing:commit("facecard") 
  end                                                
end

function make_face.cfgvalue()
  local l_tmp =  testing:get("facecard","db","make_face")
  return l_tmp or "" 
end

faceid = s:option(Value, "faceid", translate("Face ID"))
faceid.rmempty = true
faceid.datatype = "range(0,65536)"
--cardid.datatype="maxlength(16)"
--faceid:depends("make_face", "1")
function faceid.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("facecard","db","faceid", value) 
    testing:commit("facecard") 
  end                                                
end

function faceid.cfgvalue()
  local l_tmp =  testing:get("facecard","db","faceid")
  return l_tmp or "" 
end
--[[
img = s:option(FileUpload, "Face_Img", translate("Face ID Img"))
img:depends("make_face", "1")
]]
img = s:option(FileUpload, "")
--img:depends("make_face", "1")
img.template = "admin_card/img_upload"

local imgupdir = "/tmp/app/img"

if not fs.access(imgupdir) then
  fs.mkdir(imgupdir)
end
local fd

local upload_tmp   = "/.ultmp.tmp"
  
http.setfilehandler(
  function(meta, chunk, eof)
    if not fd then
      if not meta then return end
      fd = NX.open(imgupdir .. upload_tmp, "w")
      if not fd then
        --um.value = translate("Create upload file error.")
        return
      end
    end
    if chunk and fd then
      fd:write(chunk)
    end
    if eof and fd then
      fd:close()
      fd = nil
      if meta.file then
        fs.rename(imgupdir .. upload_tmp, imgupdir .."/".. meta.file)
        testing:set("facecard","db","face_img", meta.file) 
        testing:commit("facecard") 
        --sys.call("echo "..meta.file" > /tmp/img.name")
      end
      --um.value = translate("File saved to") .. ' "/tmp/upload/' .. meta.file .. '"'
    end
  end
)



apply = s:option(Button, "apply", translate("Apply"))
apply.inputstyle = "apply"

apply.write = function(self, section)
  local f = http.formvalue("img_upload_file")
  --[[if f then
    f = f:gsub("\r\n", "\n")
    if f then  
      testing:set("facecard","db","face_img", f)
    end  
  end ]]

  local value = luci.http.formvalue("start_date")
  if value then
    value = value:gsub("\r\n", "\n")
    if value then  
      testing:set("facecard","db","start_date", value)
    end 
  end
  
  value = luci.http.formvalue("stop_date")
  if value then
    value = value:gsub("\r\n", "\n")
    if value then  
      testing:set("facecard","db","stop_date", value)
    end 
  end
  testing:commit("facecard") 
  
  sys_app_sync_cmd("lua-sync -m card -c add")
  sys.call("echo sfsadadadad > /tmp/img.atmp")
  --if sys_app_sync_cmd("lua-sync -m voip -c stop-call") then
  --end
end

clear = s:option(Button, "clear", translate("Clear"))
clear.inputstyle = "remove"

clear.write = function(self, section)
  testing:delete("facecard","db","username")
  testing:delete("facecard","db","userid")
  testing:delete("facecard","db","cardid")
  testing:delete("facecard","db","start_date")  
  testing:delete("facecard","db","stop_date")
  testing:delete("facecard","db","typeid")
  testing:delete("facecard","db","make_face")
  testing:delete("facecard","db","make_card")  
  testing:delete("facecard","db","face_img")
  testing:delete("facecard","db","faceid")  
  testing:commit("facecard") 
  sys.call("echo sfsadadadad > /tmp/img.ntmp")
end
--[[
refresh = s:option(Button, "refresh", translate("Refresh"))
refresh.inputstyle = "reload"
refresh.write = function(self, section)
  local ci_d = testing:get("facecard","db","cardid")
  cardid.default=ci_d
  luci.http.redirect(luci.dispatcher.build_url("admin/card/makefacecard")) 
end
]]
refresh = s:option(Button, "refresh", translate("Refresh"))
--refresh.inputstyle = "reload"
refresh.template = "admin_card/refresh_button"

return m2