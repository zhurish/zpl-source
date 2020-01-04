-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys = require "luci.sys"
local web = require "luci.tools.webadmin"
local fs = require "nixio.fs"

local uci = require "luci.model.uci"
local testing = uci.cursor()

if not fs.access("/etc/config/userauth") then
  sys.call("echo \"config userauth 'db'\" > /etc/config/userauth")  
end

--Factory MAC Address
m = SimpleForm("card", "Card Information", translate("Card Information"))
m.rmempty = true
m.reset = false
m.submit = false
m.apply = false
m.save = false

s = m:section(SimpleSection, nil, nil)

--product_location = s:option(Value, "location", translate("Installation Location"))
cardid = s:option(Value, "cardid", translate("Card ID"))
cardid.rmempty = false
cardid.maxlength=16


function cardid.cfgvalue()
  local l_tmp =  testing:get("userauth","db","cardid")
  return l_tmp or "" 
end


  
refresh = s:option(Button, "refresh", translate("Refresh"))
refresh.inputstyle = "reload"

refresh.write = function(self, section)
  cardid.cfgvalue()
end
    
return m