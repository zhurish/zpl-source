local fs = require "nixio.fs"
local http = require "luci.http"
local util = require "nixio.util"
local sys = require "luci.sys"
local NX   = require "nixio"

local filelist = {}
local attr

local filelist_tb = {}


if not fs.access("/etc/config/dbase") then
  io.popen("mkdir /tmp/app/tftpboot -p")
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


function IsIpkFile(name)
  if name then
    local ext = string.sub(string.lower(name), -4)--截取后4位
    if ext then
      if ext == ".ipk" or ext == ".apk" then
        return true
      end
    end 
    --[[ext = string.sub(name, 1, 5)--截取前5位
    if ext then
      if ext == "esp32" or ext == "stm32" then
        return true
      end
    end 
    ]]
    ext = string.find(string.lower(name),"esp32")
    if ext then
      return true
    end
    ext = string.find(string.lower(name),"stm32")
    if ext then
      return true
    end      
  end
  return false
end

function file_install_finsh_get(file, name)
  if fs.access(file) then
    for line in io.lines(file) do
      if line and line == name then
        --return IsIpkFile(line)
        return true
      end
    end
  end
  return false
end

function file_install_finsh_set(file, name)
  sys.call("echo "..name.." >> "..file) 
end

function file_install_finsh_del(file, name)
  sys.call("sed -i '/^"..name..":*/d' >> "..file)
  --pfd = io.popen("sed -i '/^"..user..":*/d' >> /etc/config/sysuser") 
end

util.consume((fs.glob("/tmp/app/tftpboot/*")), filelist_tb)

for i, f in ipairs(filelist_tb) do
  attr = fs.stat(f)
  if attr then
    filelist[i] = {}
    filelist[i].name = fs.basename(f)
    if attr.size > 1048576 then
      filelist[i].size = string.format("%.2f",attr.size/1048576).." M"
    elseif attr.size > 1024 then
      filelist[i].size = string.format("%.2f",attr.size/1024).." K"
    elseif attr.size then
      filelist[i].size = tostring(attr.size).." B"
    end
    filelist[i].remove = 0
    filelist[i].install = false
    filelist[i].finsh = file_install_finsh_get("/tmp/app/tmp/.install-list", filelist[i].name)
  end
end


uplist = SimpleForm("filelist", translate("Upload File List"), nil)
uplist.reset = false
uplist.submit = false

tb = uplist:section(Table, filelist)
nm = tb:option(DummyValue, "name", translate("File Name"))
sz = tb:option(DummyValue, "size", translate("Size"))
btnrm = tb:option(Button, "remove", translate("Remove"))
btnrm.inputstyle = "remove"

btnrm.render = function(self, section, scope)
	self.inputstyle = "remove"
	Button.render(self, section, scope)
end

btnrm.write = function(self, section)
	local v = fs.unlink("/tmp/app/tftpboot/" .. fs.basename(filelist[section].name))
	if v then 
	file_install_finsh_del("/tmp/app/tmp/.install-list", filelist[section].name)
	table.remove(filelist, section) 
	end
	return v
end

btnis = tb:option(Button, "install", translate("Install"))

btnis.render = function(self, section, scope)
  if not filelist[section] then return false end             
  if IsIpkFile(filelist[section].name) then                  
    scope.display = "block"                            
    self.inputstyle = "apply"       
    if filelist[section].finsh then
      self.title = translate("Finsh")
    else                
      self.title = translate("Install")   
    end                                    
  else                
    btnis.disabled = true                                                            
    scope.display = "none"                    
    self.title = ""
    --translate(" * ")               
  end                                               
  Button.render(self, section, scope)               
end

btnis.write = function(self, section)
  if filelist[section].name then
    local ext = string.sub(filelist[section].name, -4)--截取后4位
    if ext and ext == ".ipk" then
      local r = sys.exec(string.format('opkg --force-depends install "/tmp/app/tftpboot/%s"', filelist[section].name))
      return
    end   
    ext = string.find(string.lower(filelist[section].name),"esp32")
    if ext then
      filelist[section].finsh = sys_app_sync_cmd("lua-sync -m file -c install-"..filelist[section].name)
      if filelist[section].finsh then
        file_install_finsh_set("/tmp/app/tmp/.install-list", filelist[section].name)
      end
      return
    end 
    ext = string.find(string.lower(filelist[section].name),"stm32")
    if ext then
      filelist[section].finsh = sys_app_sync_cmd("lua-sync -m file -c install-"..filelist[section].name)
      if filelist[section].finsh then
        file_install_finsh_set("/tmp/app/tmp/.install-list", filelist[section].name)
      end
      return
    end  
    --[[
    ext = string.sub(filelist[section].name, 1, 5)--截取前5位
    if ext and ext == "esp32" then
      filelist[section].finsh = sys_app_sync_cmd("lua-sync -m file -c install-"..filelist[section].name)
      if filelist[section].finsh then
        file_install_finsh_set("/tmp/app/tmp/.install-list", filelist[section].name)
      end
      return
    end  
    if ext and ext == "stm32" then
      filelist[section].finsh = sys_app_sync_cmd("lua-sync -m file -c install-"..filelist[section].name)
      if filelist[section].finsh then
        file_install_finsh_set("/tmp/app/tmp/.install-list", filelist[section].name)
      end
      return
    end
    ]]
  end  
	--form.description = string.format('<span style="color: red">%s</span>', r)
end

--[[
]]
local updir = "/tmp/app/tftpboot"

ful = SimpleForm("upload", translate("Upload"), nil)
ful.reset = false
ful.submit = false

sul = ful:section(SimpleSection, "", translate("Upload file to '/tmp/app/tftpboot/'"))
fu = sul:option(FileUpload, "")
fu.rmempty = false
fu.template = "cbi/other_upload"

if not fs.access(updir) then
  fs.mkdir(updir)
end
local fd

local upload_tmp   = "/.ultmp.tmp"
  
http.setfilehandler(
  function(meta, chunk, eof)
    if not fd then
      if not meta then return end
      fd = NX.open(updir .. upload_tmp, "w")
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
        local ext = nil
        fs.rename(updir .. upload_tmp, updir .."/".. meta.file)
        ext = string.sub(string.lower(meta.file), -4)--截取后4位
        if ext and ext == ".bin" then
          local r = sys.exec("chmod +x "..updir .."/".. meta.file)
        end  
      end
      --um.value = translate("File saved to") .. ' "/tmp/upload/' .. meta.file .. '"'
    end
  end
)

if http.formvalue("upload") then
  local f = http.formvalue("upload_file")
  --if #f <= 0 then
  --  um.value = translate("No specify upload file.")
  --end
  luci.http.redirect(luci.dispatcher.build_url("admin/system/filetransfer"))  
end



return uplist, ful
