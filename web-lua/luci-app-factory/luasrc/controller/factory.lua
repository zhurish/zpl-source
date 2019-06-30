module("luci.controller.admin.factory", package.seeall)


function index()
  local page
  entry({"admin", "factory"}, alias("admin", "factory", "factory"), _("Factory"), 35).index = true
  entry({"admin", "factory", "FactoryConfig"}, cbi("admin_factory/factoryconfig"), _("Factory Configure"), 1) 
  
  entry({"admin", "factory", "OpenConfig"}, cbi("admin_factory/openconfig"), _("Open Configure"), 2) 
    
  --if fs.access("/etc/config/dbtest") then 
  entry({"admin", "factory", "RoomDBConfig"}, cbi("admin_factory/roomdbconfig"), _("RoomDB Configure"), 9)  
  --end  
  
  entry({"admin", "factory", "AppSystemLog"}, cbi("admin_factory/appsystemlog"), _("AppThronLog"), 10)
end
