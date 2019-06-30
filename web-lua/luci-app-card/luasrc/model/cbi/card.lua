local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"

local uci = require "luci.model.uci"
local testing = uci.cursor()

local card_table = { }
local tab_index = 0
--[[
card_table[tab_index] = {
  username    = nil,
  userid   = nil,
  cardid = nil,
  start_date = nil,
  stop_date = nil,        
  type = nil,   
  img = nil,      
}
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


sys_app_sync_cmd("lua-sync -m card -c refresh") 

if fs.access("/tmp/app/tmp/face-card.txt") then
  for line in io.lines("/tmp/app/tmp/face-card.txt") do
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
        card_table[tab_index] = {
            username    = u[1],
            userid   = u[2],
            cardid = u[3],
            start_date = u[4],
            stop_date = u[5],       
            type = u[6],   
            img = u[7]      
          }
          tab_index=tab_index+1 
      end
    end
  end
end

if not fs.access("/www/app") then
  sys.call("ln -sf /tmp/app/img /www/app")
end

m = SimpleForm("CardId", translate("Card Information"), translate(" User Card ID Information"))
m.rmempty = true
m.reset = false
m.submit = false
m.apply = false


s = m:section(Table, card_table)

un = s:option(DummyValue, "username", translate("Username"))
uid = s:option(DummyValue, "userid", translate("User ID"))
cid = s:option(DummyValue, "cardid", translate("Card ID"))

start_date = s:option(DummyValue, "start_date", translate("Start Date"))
stop_date = s:option(DummyValue, "stop_date", translate("Stop Date"))

type = s:option(DummyValue, "type", translate("Type"))

imgid = s:option(DummyValue, "img", translate("Face ID"))
imgid.inputstyle = "hidden"
--imgid:depends("use_curl","1")
--imgid = s:option(DummyValue, "imgid")

load_img = s:option(Button, "load_img", translate("IMG"))
--load_img = s:option(Button, "load_img", translate("IMG"))
load_img.inputstyle = "apply"
load_img.template = "admin_card/img_show"

--load_img.write = function(self, section)
--  luci.http.redirect(luci.dispatcher.build_url("admin/card/makefacecard")) 
--end

edit = s:option(Button, "edit", translate("Edit"))
edit.inputstyle = "apply"

edit.write = function(self, section)
  local value
  if card_table[section].username then
    value = card_table[section].username:gsub("\r\n", "\n")
    if value then
      testing:set("facecard","db","username", value)
    end
  end
  
  if card_table[section].userid then
    value = card_table[section].userid:gsub("\r\n", "\n")
    if value then
      testing:set("facecard","db","userid", value)
    end
  end
  
  if card_table[section].cardid then
    value = card_table[section].cardid:gsub("\r\n", "\n")
    if value then
      testing:set("facecard","db","cardid", value)
    end
    value = 1
    --testing:set("facecard","db","make_card", value)
  end
  
  if card_table[section].start_date then
    value = card_table[section].start_date:gsub("\r\n", "\n")
    if value then
      testing:set("facecard","db","start_date", value)
    end
  end
  
  if card_table[section].stop_date then
    value = card_table[section].stop_date:gsub("\r\n", "\n")
    if value then
      testing:set("facecard","db","stop_date", value)
    end
  end
  if card_table[section].type then
    value = card_table[section].type:gsub("\r\n", "\n")
    if value then
      testing:set("facecard","db","typeid", value)
    end
  end
  if card_table[section].img then
    value = card_table[section].img:gsub("\r\n", "\n")
    if value then
      testing:set("facecard","db","face_img", value)
    end
    value = 1
    --testing:set("facecard","db","make_face", value)
  end

  testing:commit("facecard") 
  luci.http.redirect(luci.dispatcher.build_url("admin/card/makefacecard"))    
  --sys.call("echo adasdadas >/dev/null")
end

delete = s:option(Button, "delete", translate("Delete"))
delete.inputstyle = "remove"
delete.write = function(self, section)
  local value
  if card_table[section].userid then
    value = card_table[section].userid:gsub("\r\n", "\n")
    if value then
      testing:set("facecard","db","userid", value)
    end
  end
  if card_table[section].cardid then
    value = card_table[section].cardid:gsub("\r\n", "\n")
    if value then
      testing:set("facecard","db","cardid", value)
    end
  end
  testing:commit("facecard")  
  sys_app_sync_cmd("lua-sync -m card -c del") 
  luci.http.redirect(luci.dispatcher.build_url("admin/card/cardconfig"))       
  --sys.call("echo adasdadas >/dev/null")
end

return m