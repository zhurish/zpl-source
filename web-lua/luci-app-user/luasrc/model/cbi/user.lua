local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"
local uci = require "luci.model.uci"
local io     = require "io"
local os     = require "os"
local testing = uci.cursor()


local user_table = { }
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


--UserName   Password Level    
if fs.access("/etc/config/sysuser") then
  for line in io.lines("/etc/config/sysuser") do
    if line then
      local u = string.split(line,":")
      if u and u[1] ~= nil then
        user_table[tab_index] = {
            username    = u[1],
            password   = u[2],      
            level = u[3],      
          }
          tab_index=tab_index+1 
      end
    end
  end
end
--[[
function add_user(name, pass, level)
  if name ~= nil then
    sys.call("echo "..name..":"..pass..":"..level.." >> /etc/config/sysuser >/dev/null")
  end
end

function del_user(name)
  if name ~= nil then
    sys.call("echo "..name..":"..pass..":"..level.." >> /etc/config/sysuser >/dev/null")
  end
end
]]
m = SimpleForm("system.user", "System User Configure", 
    translate("System User configure."))
m.rmempty = true
m.reset = false
m.submit = false
m.apply = false
m.save = false


s = m:section(SimpleSection, nil, nil)

name = s:option(Value, "name", translate("User Name"), translate("System UserName"))
name.maxlength=32
name.rmempty = true

pass = s:option(Value, "pass", translate("User Password"), translate("System User Password"))
pass.maxlength=32
pass.inputstyle = "password" 
pass.rmempty = true

level = s:option(Value, "level", translate("User Level"), translate("System User Level"))
level:value("manage", "manage")
level:value("admin", "admin")
level:value("user", "user")
level.default = "user"
level.rmempty = true


add_btn = s:option(Button, "add_btn", translate("Add"))
add_btn.inputstyle = "apply"

add_btn.write = function(self, section) 
  local user=luci.http.formvalue("cbid.system.user.1.name")
  local pass=luci.http.formvalue("cbid.system.user.1.pass")
  local level=luci.http.formvalue("cbid.system.user.1.level") 
  if user then
    user = user:gsub("\r\n", "\n")
  end
  if pass then
    pass = pass:gsub("\r\n", "\n")
  end
  if level then
    level = level:gsub("\r\n", "\n")
  end
  if user and level then
    local pfd = io.popen("adduser -SH "..user)
    if pfd then
      pfd:close()
      pfd = nil
    end 
    --/etc/config/sysuser >/dev/null
    if pass then
      if luci.sys.user.setpasswd(user, pass) == 0 then
        pfd = io.popen("echo "..user..":"..pass..":"..level.." >> /etc/config/sysuser")
        if pfd then
          pfd:close()
          pfd = nil
        end         
        m.message = translate("add user successfully!")
      else 
      pfd = io.popen("echo "..user..":"..pass..":"..level.." >> /tmp/aa2.txt")
      if pfd then
        pfd:close()
        pfd = nil
      end        
        m.message = translate("add user failure!")
      end
    else 
      pfd = io.popen("echo "..user..":"..pass..":"..level.." >> /etc/config/sysuser")
      if pfd then
        pfd:close()
        pfd = nil
      end 
      m.message = translate("add user successfully!") 
      --add_user(user, pass, level) 
    end   
  end
  io.popen("echo "..user..":"..pass..":"..level.." >> /tmp/aa.txt")
  luci.http.redirect(luci.dispatcher.build_url("admin/system/user")) 
end


del_btn = s:option(Button, "del_btn", translate("Delete"))
del_btn.inputstyle = "remove"

del_btn.write = function(self, section) 
  local user=luci.http.formvalue("cbid.system.user.1.name")
  if user then
    user = user:gsub("\r\n", "\n")
  end  
  if user then
    local pfd = io.popen("deluser "..user)
    if pfd then
      pfd:close()
      pfd = nil
    end 
    --del_user(user) sed -i '/^admin:*/d' /etc/config/sysuser
    pfd = io.popen("sed -i '/^"..user..":*/d' >> /etc/config/sysuser")
    if pfd then
      pfd:close()
      pfd = nil
    end     
    m.message = translate("delete user successfully!")
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/system/user"))   
end





m2 = SimpleForm("user-table", "System User Information", 
    translate("System User information."))
m2.rmempty = true
m2.reset = false
m2.submit = false
m2.apply = false
m2.save = false

s = m2:section(Table, user_table)
    
username = s:option(DummyValue, "username", translate("Username"))
password = s:option(DummyValue, "password", translate("Password"))
level = s:option(DummyValue, "level", translate("Level"))

delete = s:option(Button, "delete", translate("Delete"))
delete.inputstyle = "remove"
delete.write = function(self, section)
  local value = user_table[section].username:gsub("\r\n", "\n")
  if value then
    local pfd = io.popen("deluser "..value)
    if pfd then
      pfd:close()
    end 
    pfd = io.popen("sed -i '/^"..value..":*/d' >> /etc/config/sysuser")
    if pfd then
      pfd:close()
      pfd = nil
    end 
    --del_user(value)
    m2.message = translate("delete user successfully!")
  end
  luci.http.redirect(luci.dispatcher.build_url("admin/system/user"))       
end




return m,m2