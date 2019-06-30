local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"

local uci = require "luci.model.uci"
local testing = uci.cursor()


local room_table = { }
local tab_index = 0
local searchfile = "/tmp/app/tmp/search.txt"

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

sys_app_sync_cmd("lua-sync -m dbase -c select")
 
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
--building unit     room     phone            UserName         User ID    
if fs.access(searchfile) then
  for line in io.lines(searchfile) do
    if line then
      local u = string.split(line," ")
      if u and u[3] ~= nil then
        room_table[tab_index] = {
            username    = u[5],
            userid   = u[6],
            --building = u[1],
            --unit = u[2],
            room = u[3],       
            phone = u[4],      
          }
          tab_index=tab_index+1 
      end
    end
  end
end

if fs.access("/etc/config/dbase") then

m1 = SimpleForm("Location Building", "Building Room Information Configure", 
    translate("Telephone information of local  Building Room Configure Option"))
m1.rmempty = true
m1.reset = false
m1.submit = false
m1.apply = false
m1.save = false


s = m1:section(SimpleSection, nil, nil)

username = s:option(Value, "username", translate("User Name"))
username.rmempty = true
--username.datatype="maxlength(32)"
username.maxlength=32
function username.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("dbase","roomdb","username", value) 
    testing:commit("dbase") 
  end                                                
end

function username.cfgvalue()
  local l_tmp =  testing:get("dbase","roomdb","username")
  return l_tmp or "" 
end

userid = s:option(Value, "userid", translate("User ID"), translate("User ID must be unique"))
userid.rmempty = true
--userid.datatype="maxlength(32)"
userid.maxlength=32
function userid.write(self, section, value)  
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("dbase","roomdb","userid", value) 
    testing:commit("dbase") 
  end                                                
end

function userid.cfgvalue()
  local l_tmp =  testing:get("dbase","roomdb","userid")
  return l_tmp or "" 
end


room = s:option(Value, "room", translate("Room Number"))
room.rmempty = true
room.datatype="range(100,999999999)"
function room.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("dbase","roomdb","room", value) 
    testing:commit("dbase") 
  end                                                
end

function room.cfgvalue()
  local l_tmp =  testing:get("dbase","roomdb","room")
  return l_tmp or "" 
end

phone = s:option(Value, "phone", translate("Phone Number"), translate("Phone Number must be unique"))
phone.rmempty = true
--phone.datatype="maxlength(32)"
phone.maxlength=32
function phone.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("dbase","roomdb","phone", value) 
    testing:commit("dbase") 
  end                                                
end

function phone.cfgvalue()
  local l_tmp =  testing:get("dbase","roomdb","phone")
  return l_tmp or "" 
end



add_btn = s:option(Button, "add_btn", translate("Add"))
add_btn.inputstyle = "apply"

add_btn.write = function(self, section) 
  sys_app_sync_cmd("lua-sync -m dbase -c add")
  luci.http.redirect(luci.dispatcher.build_url("admin/factory/RoomDBConfig")) 
end
--[[
del_btn = s:option(Button, "del_btn", translate("Delete"))
del_btn.inputstyle = "remove"

del_btn.write = function(self, section)    
  sys_app_sync_cmd("lua-sync -m dbase -c delete")
  luci.http.redirect(luci.dispatcher.build_url("admin/factory/RoomDBConfig"))   
end
]]

clr_btn = s:option(Button, "clear_btn", translate("Clear"))
clr_btn.inputstyle = "remove"

clr_btn.write = function(self, section)    
  testing:delete("dbase","roomdb","username")
  testing:delete("dbase","roomdb","userid")
  testing:delete("dbase","roomdb","room")
  testing:delete("dbase","roomdb","phone")         
  testing:commit("dbase") 
end


select_btn = s:option(Button, "select", translate("Search"))
select_btn.inputstyle = "apply"

select_btn.write = function(self, section)
  sys_app_sync_cmd("lua-sync -m dbase -c select")
  luci.http.redirect(luci.dispatcher.build_url("admin/factory/RoomDBConfig")) 
end

end


    
m2 = SimpleForm("Room Building", "Building Room Information", 
    translate("Telephone information of local  Building Room"))
m2.rmempty = true
m2.reset = false
m2.submit = false
m2.apply = false
m2.save = false

s = m2:section(Table, room_table)
    
tun = s:option(DummyValue, "username", translate("Username"))
tuid = s:option(DummyValue, "userid", translate("User ID"))
troom = s:option(DummyValue, "room", translate("Room Number"))
tphone = s:option(DummyValue, "phone", translate("Phone Number"))


edit = s:option(Button, "edit", translate("Edit"))
edit.inputstyle = "apply"

edit.write = function(self, section)
  local value = room_table[section].username:gsub("\r\n", "\n")
  if value then
    testing:set("dbase","roomdb","username", value)
  end
  value = room_table[section].userid:gsub("\r\n", "\n")
  if value then
    testing:set("dbase","roomdb","userid", value)
  end
  
  value = room_table[section].room:gsub("\r\n", "\n")
  if value then
    testing:set("dbase","roomdb","room", value)
  end
  
  value = room_table[section].phone:gsub("\r\n", "\n")
  if value then
    testing:set("dbase","roomdb","phone", value)
  end
  testing:commit("dbase") 
  luci.http.redirect(luci.dispatcher.build_url("admin/factory/RoomDBConfig"))   
end

delete = s:option(Button, "delete", translate("Delete"))
delete.inputstyle = "remove"
delete.write = function(self, section)
  local value = room_table[section].username:gsub("\r\n", "\n")
  if value then
    testing:set("dbase","roomdb","username", value)
  end
  value = room_table[section].userid:gsub("\r\n", "\n")
  if value then
    testing:set("dbase","roomdb","userid", value)
  end
  
  value = room_table[section].room:gsub("\r\n", "\n")
  if value then
    testing:set("dbase","roomdb","room", value)
  end
  
  value = room_table[section].phone:gsub("\r\n", "\n")
  if value then
    testing:set("dbase","roomdb","phone", value)
  end
  testing:commit("dbase") 
  sys_app_sync_cmd("lua-sync -m dbase -c delete") 
  luci.http.redirect(luci.dispatcher.build_url("admin/factory/RoomDBConfig"))       
end

    
if fs.access("/etc/config/dbase") then
  return m1,m2
else
  return m2
end