local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"

--local uci = require "luci.model.uci"
--local testing = uci.cursor()
local logfile = "/app/etc/thlog.log" 
  
function logfile_rows()
  local y = 1
  if fs.access(logfile) then
    local file = io.open(logfile,"r")
    for line in file:lines() do
       y = y + 1
    end
    file:close()
  end
  return y
end


m = SimpleForm("APP system", "Show App System Log", translate("Show Application System Log"))
m.rmempty = true
m.reset = false
m.submit = false
m.apply = false
m.save = false
s = m:section(SimpleSection, nil, nil)



--s = m:section(SimpleSection, nil, nil)
log_text = s:option(TextValue, "through-log","")
--sel_text.template = "admin_voip/capture_download"
log_text.rmempty = true
log_text.rows = logfile_rows() + 2
log_text.readonly  = true
    
function log_text.cfgvalue()
  if logfile and fs.access(logfile) then
    return fs.readfile(logfile) or ""
  else
    return ""
  end  
end 
    
    
refresh = s:option(Button, "refresh", translate("Refresh"))
refresh.inputstyle = "reload"

refresh.write = function(self, section)
  log_text.rows = logfile_rows() + 2
  log_text.cfgvalue()
end
    
return m