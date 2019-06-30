-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"

local uci = require "luci.model.uci"
local testing = uci.cursor()

if fs.access("/app/etc/x5b-app.sh") then

function voip_enabled()                                              
  local en_tmp = testing:get("product","global","voip")
  if en_tmp and en_tmp == "1" then
    return true;
  else
    return false
  end                                    
end 


function voip_app_start() 
	sys.call("/app/etc/x5b-app.sh start >/dev/null")                                                                               
end

function voip_app_stop()                                              
	sys.call("/app/etc/x5b-app.sh stop >/dev/null")                      
end
end -- fs.access("/app/etc/x5b-app.sh") then




function factory_get_system_mac()   
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

--[[
if fs.access("/app/etc/x5b-app.sh") then

local voip = { }
voip["%s" % "Voip"] = {name="Voip", enabled=voip_enabled()}

m1 = SimpleForm("Global VOIP", "Factory Voip", translate("Global Voip Function Configure"))
m1.reset = false
m1.submit = false
m1.apply = true
m1.save = true

--s = m:section(SimpleSection, nil, translate("Factory configure option"))
s = m1:section(Table,voip)
n = s:option(DummyValue,"name",translate("Voip"))
e = s:option(Button, "endisable", translate("Enable/Disable"))

--local voip = voip_enabled()
e.render = function(self, section, scope)
  if voip[section].enabled then
    self.title = translate("Disabled")
    self.inputstyle = "remove"
  else
    self.title = translate("Enabled")
    self.inputstyle = "apply"
  end
  Button.render(self, section, scope)
end

e.write = function(self, section)
  if voip[section].enabled then
    voip[section].enabled = false
    testing:set("product","global","voip", 0) 
    testing:commit("product") 
    voip_app_stop()
  else
    voip[section].enabled = true
    testing:set("product","global","voip", 1) 
    testing:commit("product")     
    voip_app_start()
  end
end
end -- if fs.access("/app/etc/x5b-app.sh") then
]]

--[[
--Factory Serial Number
SerialN = SimpleForm("Serial Number", "Factory Serial Number", translate("Factory Serial Number Configure"))
SerialN.rmempty = true
s = SerialN:section(SimpleSection, nil, nil)

serial_number = s:option(Value, "serial-number", translate("<abbr title=\"System Serial Number\">Serial Number</abbr>"))
serial_number.rmempty = true
serial_number.default=factory_get_system_mac()

function serial_number.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then  
  if fs.access("/usr/bin/mac-tools") then                                                                
    sys.call("mac-tools -t sys -m %s >/dev/null" % value)
  end  
    --sys.call("echo \"%s\" >/tmp/mac.txt"% value)     
  end                                                
end
]]

--Factory MAC Address
m2 = Map("product", translate("Product Global Configure"), translate("Product Global Configure Option"))
-- s = m2:section(TypedSection, "sip", translate("SIP stack configure option"))
s = m2:section(NamedSection, "global", "product", nil)
s.anonymous = false
s.addremove = false
--[[
m2 = SimpleForm("Product Configure", "Product Global Configure", translate("Product Global Configure Option"))
m2.rmempty = true
s = m2:section(SimpleSection, nil, nil)
]]

mac = s:option(Value, "macaddress", translate("<abbr title=\"System Mac Address\">SYS Mac-Address</abbr>"))
mac.datatype = "macaddr"
mac.rmempty = true
mac.default=factory_get_system_mac()

function mac.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then  
  if fs.access("/usr/bin/mac-tools") then                                                                
    sys.call("mac-tools -t sys -m %s >/dev/null" % value)
  end  
    --sys.call("echo \"%s\" >/tmp/mac.txt"% value)     
  end                                                
end

function mac.cfgvalue()
  local l_tmp =  factory_get_system_mac()
  return l_tmp or "" 
end
 
-- product
product = s:option(ListValue, "product", translate("Product Type"))
product.rmempty = false
product.default='X5BM'
product:value("X5BM",translate("X5BM"))
product:value("X5CM",translate("X5CM"))

function product.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("product","global","type", value) 
    testing:commit("product") 
  end                                                
end

function product.cfgvalue()
  local l_tmp =  testing:get("product","global","type")
  return l_tmp or "" 
end

-- product location
product_location = s:option(ListValue, "location", translate("Location Type"))
product_location.rmempty = false
product_location.default='Unit'
product_location:value("Unit",translate("Unit"))
product_location:value("Wall",translate("Wall"))
product_location:value("Building",translate("Building"))
product_location:value("Company",translate("Company"))
--product_location:value("Building",translate("Building"))


function product_location.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("product","global","location", value) 
    testing:commit("product") 
  end                                                
end

function product_location.cfgvalue()
  local l_tmp =  testing:get("product","global","location")
  return l_tmp or "" 
end


customizer = s:option(ListValue, "customizer", translate("Customizer Type"))--manufacturer
customizer.rmempty = true
customizer.default='Secom'
customizer:value("Secom",translate("Secom"))
customizer:value("Huifu",translate("Huifu"))
customizer:value("Local",translate("Local"))


function customizer.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("product","global","customizer", value) 
    testing:commit("product") 
  end                                                
end

function customizer.cfgvalue()
  local l_tmp =  testing:get("product","global","customizer")
  return l_tmp or "" 
end


thlog_max = s:option(Value, "thlog_max", translate("Max LOG Number"))
thlog_max.rmempty = false
thlog_max.default='2000'
thlog_max.datatype = "range(64,9000)"

function thlog_max.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("product","global","thlog_max", value) 
    testing:commit("product") 
  end                                                
end

function thlog_max.cfgvalue()
  local l_tmp =  testing:get("product","global","thlog_max")
  return l_tmp or "" 
end

device_name = s:option(Value, "device_name", translate("Device Name"))
device_name.rmempty = true
device_name.maxlength=32
device_name:depends("customizer", "Huifu")

function device_name.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("product","global","device_name", value) 
    testing:commit("product") 
  end                                                
end

function device_name.cfgvalue()
  local l_tmp =  testing:get("product","global","device_name")
  return l_tmp or "" 
end

server_address = s:option(Value, "server_address", translate("Server Address"))
server_address.rmempty = true
server_address.datatype = "ip4addr"
server_address:depends("customizer", "Huifu")

function server_address.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("product","global","server_address", value) 
    testing:commit("product") 
  end                                                
end

function server_address.cfgvalue()
  local l_tmp =  testing:get("product","global","server_address")
  return l_tmp or "" 
end

direction = s:option(ListValue, "direction", translate("Direction Type"))
direction.rmempty = true
direction.default='out'
direction:value("out",translate("out"))
direction:value("in",translate("in"))
direction:depends("customizer", "Huifu")

function direction.write(self, section, value)           
  value = value:gsub("\r\n", "\n")                   
  if value then 
    testing:set("product","global","direction", value) 
    testing:commit("product") 
  end                                                
end

function direction.cfgvalue()
  local l_tmp =  testing:get("product","global","direction")
  return l_tmp or "" 
end


local apply=luci.http.formvalue("cbi.apply")

if apply then
  local pfd = io.popen("lua-sync -m app -c factory")
  if pfd then
    pfd:close()
  end 
end

return m2