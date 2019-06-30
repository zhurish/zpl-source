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

function sys_app_product_type_X5CM() 
  local l_tmp =  testing:get("product","global","type")
  if l_tmp and string.find(l_tmp,"X5CM") then
      return true
  end
  return false 
end

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

if not fs.access("/etc/config/userauth") then
  sys.call("echo \"config userauth 'db'\" > /etc/config/userauth")  
end
  
sys_app_sync_cmd("lua-sync -m userauth -c refresh")

function make_card_enabled()                                              
  local en_tmp = testing:get("userauth","db","enablemake")
  if en_tmp and en_tmp == "1" then
    return true;
  else
    return false
  end                                    
end 
local enablemake = make_card_enabled()

--Factory MAC Address
m2 = SimpleForm("userauth", "User Configure", translate("User Configure Option"))
m2.rmempty = true
m2.reset = false
m2.submit = false
m2.apply = false

s = m2:section(SimpleSection, nil, nil)

--[[
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
    testing:set("userauth","db","enablemake", 0) 
    testing:commit("userauth") 
    enablemake = false
    sys_app_sync_cmd("lua-sync -m userauth -c stop")
  else
    testing:set("userauth","db","enablemake", 1) 
    testing:commit("userauth")
    enablemake = true   
    sys_app_sync_cmd("lua-sync -m userauth -c start")  
  end
end
]]

username = s:option(Value, "username", translate("User Name"))
username.rmempty = false
--username.default=u_d
username.maxlength=32
--username.datatype="maxlength(32)"
function username.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("userauth","db","username", value) 
    testing:commit("userauth") 
  end                                                
end

function username.cfgvalue()
  local l_tmp =  testing:get("userauth","db","username")
  return l_tmp or "" 
end

userid = s:option(Value, "userid", translate("User ID"), 
  translate("User ID must be unique"))
userid.rmempty = false
userid.maxlength=32
function userid.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("userauth","db","userid", value) 
    testing:commit("userauth") 
  end                                                
end

function userid.cfgvalue()
  local l_tmp =  testing:get("userauth","db","userid")
  return l_tmp or "" 
end

--phone
phone = s:option(Value, "phone", translate("PhoneNumber"), translate("phone number must be unique"))
phone.rmempty = false
phone.maxlength=16
function phone.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("userauth","db","phone", value) 
    testing:commit("userauth") 
  end                                                
end

function phone.cfgvalue()
  local l_tmp =  testing:get("userauth","db","phone")
  return l_tmp or "" 
end

--room
room = s:option(Value, "room", translate("RoomNumber"), translate("This parameter must be unique"))
room.rmempty = false
room.maxlength=4
function room.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("userauth","db","room", value) 
    testing:commit("userauth") 
  end                                                
end

function room.cfgvalue()
  local l_tmp =  testing:get("userauth","db","room")
  return l_tmp or "" 
end

-- Card 
cardid = s:option(Value, "cardid", translate("Card ID"), translate("Card ID must be unique"))
cardid.rmempty = false
cardid.maxlength=16
function cardid.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("userauth","db","cardid", value) 
    testing:commit("userauth") 
  end                                                
end

function cardid.cfgvalue()
  local l_tmp =  testing:get("userauth","db","cardid")
  return l_tmp or "" 
end

start_date = s:option(DummyValue, "start_date", translate("Start DateTime"))
start_date.rmempty = false
start_date.default=os.date("%Y-%m-%dT%H:%M", os.time())
--start_date:depends("make_card", "1")
start_date.template = "admin_card/datetime-start"


stop_date = s:option(DummyValue, "stop_date", translate("Stop DateTime"))
stop_date.rmempty = false
--2019/04/16T04:10
stop_date.default=os.date("%Y-%m-%dT%H:%M", os.time())
--stop_date:depends("make_card", "1")
stop_date.template = "admin_card/datetime-stop"


cardtype = s:option(ListValue, "cardtype", translate("Card Type"))
cardtype.rmempty = false
cardtype.default="Whitelist"
--cardtype:depends("make_card", "1")
cardtype:value("Whitelist",translate("Whitelist"))
cardtype:value("Blacklist",translate("Blacklist"))
cardtype:value("Unknow",translate("Unknow"))
function cardtype.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("userauth","db","cardtype", value) 
    testing:commit("userauth") 
  end                                                
end

function cardtype.cfgvalue()
  local l_tmp =  testing:get("userauth","db","cardtype")
  return l_tmp or "" 
end


if sys_app_product_type_X5CM() then
-- Face 
faceid = s:option(Value, "faceid", translate("Face ID"))
faceid.rmempty = false
faceid.datatype = "range(1,65536)"
function faceid.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("userauth","db","faceid", value) 
    testing:commit("userauth") 
  end                                                
end

function faceid.cfgvalue()
  local l_tmp =  testing:get("userauth","db","faceid")
  return l_tmp or "" 
end
--[[
img = s:option(FileUpload, "Face_Img", translate("Face ID Img"))
img:depends("make_face", "1")
]]
img = s:option(FileUpload, "")
--img:depends("make_face", "1")
img.template = "admin_card/img_upload"
img.rmempty = false

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
        testing:set("userauth","db","faceimg", meta.file) 
        testing:commit("userauth") 
        --sys.call("echo "..meta.file" > /tmp/img.name")
      end
      --um.value = translate("File saved to") .. ' "/tmp/upload/' .. meta.file .. '"'
    end
  end
)

end

apply = s:option(Button, "apply", translate("Apply"))
apply.inputstyle = "apply"

apply.write = function(self, section)
  if sys_app_product_type_X5CM() then
    local f = http.formvalue("img_upload_file")
  end
  --[[if f then
    f = f:gsub("\r\n", "\n")
    if f then  
      testing:set("userauth","db","face_img", f)
    end  
  end ]]

  local value = luci.http.formvalue("start_date")
  if value then
    value = value:gsub("\r\n", "\n")
    if value then  
      testing:set("userauth","db","start_date", value)
    end 
  end
  
  value = luci.http.formvalue("stop_date")
  if value then
    value = value:gsub("\r\n", "\n")
    if value then  
      testing:set("userauth","db","stop_date", value)
    end 
  end
  testing:commit("userauth") 
  
  sys_app_sync_cmd("lua-sync -m userauth -c add")
  sys.call("echo sfsadadadad > /tmp/img.atmp")
  --if sys_app_sync_cmd("lua-sync -m voip -c stop-call") then
  luci.http.redirect(luci.dispatcher.build_url("admin/card/userauth"))
  --end
end

clear = s:option(Button, "clear", translate("Clear"))
clear.inputstyle = "remove"

clear.write = function(self, section)
  testing:delete("userauth","db","username")
  testing:delete("userauth","db","userid")
  testing:delete("userauth","db","cardid")
  testing:delete("userauth","db","phone")
  testing:delete("userauth","db","room")    
  testing:delete("userauth","db","start_date")  
  testing:delete("userauth","db","stop_date")
  testing:delete("userauth","db","cardtype")
  if sys_app_product_type_X5CM() then
    testing:delete("userauth","db","faceimg")
    testing:delete("userauth","db","faceid")  
  end
  testing:commit("userauth") 
  sys.call("echo sfsadadadad > /tmp/img.ntmp")
end
--[[
refresh = s:option(Button, "refresh", translate("Refresh"))
refresh.inputstyle = "reload"
refresh.write = function(self, section)
  local ci_d = testing:get("userauth","db","cardid")
  cardid.default=ci_d
  luci.http.redirect(luci.dispatcher.build_url("admin/card/makeuserauth")) 
end
]]
refresh = s:option(Button, "refresh", translate("Refresh"))
refresh.inputstyle = "reload"
refresh.template = "admin_card/refresh_button"

--refresh.inputstyle = "button"

--refresh.write = function(self, section)
--  luci.http.redirect(luci.dispatcher.build_url("admin/card/userauth"))
--end





local card_table = { }
local tab_index = 0
      
function string.split(input, delimiter)
    input = tostring(input)
    delimiter = tostring(delimiter)
    if (delimiter=='') then return false end
    local pos,arr = 0, {}
    -- for each divider found
    for st,sp in function() return string.find(input, delimiter, pos, true) end do
        table.insert(arr, string.sub(input, pos, st - 1))
        pos = sp + 1
    end
    table.insert(arr, string.sub(input, pos))
    return arr
end


--sys_app_sync_cmd("lua-sync -m userauth -c refresh") 

if fs.access("/tmp/app/tmp/userauth.txt") then
  for line in io.lines("/tmp/app/tmp/userauth.txt") do
    if line then
      local u = string.split(line," ")
      if u and u[3] ~= nil then
        if u[1] == "," then u[1] = nil end
        if u[2] == "," then u[2] = nil end
        if u[3] == "," then u[3] = nil end
        if u[4] == "," then u[4] = nil end
        if u[5] == "," then u[5] = nil end
        if u[6] == "," then u[6] = nil end
        if u[7] == "," then u[7] = nil end
        if u[8] == "," then u[8] = nil end
        if u[9] == "," then u[9] = nil end
        card_table[tab_index] = {
            username    = u[1],
            userid   = u[2],
            phone    = u[3],
            room   = u[4],
            cardid = u[5],
            start_date = u[6],
            stop_date = u[7],       
            cardtype = u[8],   
            img = u[9]      
          }
          tab_index=tab_index+1 
      end
    end
  end
end

if not fs.access("/www/app") then
  sys.call("ln -sf /tmp/app/img /www/app")
end

m = SimpleForm("userauthinfo", translate("User Information"), translate(" User Information"))
m.rmempty = true
m.reset = false
m.submit = false
m.apply = false


s = m:section(Table, card_table)

un = s:option(DummyValue, "username", translate("Username"))
uid = s:option(DummyValue, "userid", translate("User ID"))
cph = s:option(DummyValue, "phone", translate("Phone"))

urom = s:option(DummyValue, "room", translate("Room"))
cid = s:option(DummyValue, "cardid", translate("Card ID"))

start_date = s:option(DummyValue, "start_date", translate("Start Date"))
stop_date = s:option(DummyValue, "stop_date", translate("Stop Date"))

type = s:option(DummyValue, "cardtype", translate("Type"))

--imgid = s:option(DummyValue, "img", translate("Face ID"))
--imgid.inputstyle = "hidden"
--imgid:depends("use_curl","1")
--imgid = s:option(DummyValue, "imgid")
if sys_app_product_type_X5CM() then
load_img = s:option(Button, "load_img", translate("IMG"))
--load_img = s:option(Button, "load_img", translate("IMG"))
load_img.inputstyle = "button"
load_img.template = "admin_card/img_show"

--load_img.write = function(self, section)
--  luci.http.redirect(luci.dispatcher.build_url("admin/card/makefacecard")) 
--end
end

--[[
edit = s:option(Button, "edit", translate("Edit"))
edit.inputstyle = "apply"

edit.write = function(self, section)
  local value
  if card_table[section].username then
    value = card_table[section].username:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","username", value)
    end
  end
  
  if card_table[section].userid then
    value = card_table[section].userid:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","userid", value)
    end
  end
  
  if card_table[section].phone then
    value = card_table[section].phone:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","phone", value)
    end
  end
  
  if card_table[section].room then
    value = card_table[section].room:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","room", value)
    end
  end
      
  if card_table[section].cardid then
    value = card_table[section].cardid:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","cardid", value)
    end
    value = 1
    --testing:set("facecard","db","make_card", value)
  end
  
  if card_table[section].start_date then
    value = card_table[section].start_date:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","start_date", value)
    end
  end
  
  if card_table[section].stop_date then
    value = card_table[section].stop_date:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","stop_date", value)
    end
  end
  if card_table[section].cardtype then
    value = card_table[section].cardtype:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","cardtype", value)
    end
  end
  if card_table[section].img then
    value = card_table[section].img:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","faceimg", value)
    end
    value = 1
    --testing:set("facecard","db","make_face", value)
  end

  testing:commit("userauth") 
  luci.http.redirect(luci.dispatcher.build_url("admin/card/userauth"))    
  --sys.call("echo adasdadas >/dev/null")
end
]]

delete = s:option(Button, "delete", translate("Delete"))
delete.inputstyle = "remove"
delete.write = function(self, section)
  local value
  if card_table[section].userid then
    value = card_table[section].userid:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","userid", value)
    end
  end
  if card_table[section].cardid then
    value = card_table[section].cardid:gsub("\r\n", "\n")
    if value then
      testing:set("userauth","db","cardid", value)
    end
  end
  testing:commit("userauth")
  sys_app_sync_cmd("lua-sync -m userauth -c del") 
  luci.http.redirect(luci.dispatcher.build_url("admin/card/userauth"))       
  --sys.call("echo adasdadas >/dev/null")
end


return m2, m