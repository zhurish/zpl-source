module("luci.controller.admin.card", package.seeall)
local fs = require "nixio.fs"

function index()
  local page
  entry({"admin", "card"}, alias("admin", "card", "card"), _("Card-FaceID"), 36).index = true 
  entry({"admin", "card", "faceconfig"}, cbi("admin_card/face"), _("Face Recognition"), 1).sysauth="root"   
  entry({"admin", "card", "userauth"}, cbi("admin_card/userauth"), _("UserAuth Configure"), 3)
  --entry({"admin", "card", "makefacecard"}, cbi("admin_card/makefacecard"), _("Face-Card Configure"), 3)
  --entry({"admin", "card", "cardconfig"}, cbi("admin_card/card"), _("Face-Card Information"), 3)  
    --page = node("admin", "card", "cardconfig")
    --page.target = cbi("admin_card/card")
    --page.title  = _("Face-Card Information")
    --page.order  = 4

    page = entry({"admin", "card", "img_show"}, post("img_show"), nil)
    page.leaf = true     
end



function card_sys_app_sync_cmd(cmd)                                              
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


--[[
function diag_command(cmd, addr)
  if addr and addr:match("^[a-zA-Z0-9%-%.:_]+$") then
    luci.http.prepare_content("text/plain")

    local util = io.popen(cmd % addr)
    if util then
      while true do
        local ln = util:read("*l")
        if not ln then break end
        luci.http.write(ln)
        luci.http.write("\n")
      end

      util:close()
    end

    return
  end

  luci.http.status(500, "Bad address")
end
]]
function img_show(userid)
  if userid then
    local res = true
    --card_sys_app_sync_cmd("lua-sync -m img-show -c "..userid)
    --local imgurl = "http://127.0.0.1:/www/app/aa.jpg"
--[[    luci.http.prepare_content("text/plain")
    if fs.access("/tmp/app/fimg") then
      imgurl = fs.readfile("/tmp/app/fimg")
    end
    ]]
    --wget http://10.10.10.100:22331/getPic?userid=11&faceid=212
    luci.sys.call("wget \"http://10.10.10.100:22331/getPic?faceId=1&userId="..userid.."\" -O /tmp/app/img/"..userid..".jpg")
    luci.sys.call("chmod 7777 /tmp/app/img/"..userid..".jpg")
    if res then
        luci.http.write("/app/"..userid..".jpg")
        luci.http.write("\n")
    else   
        luci.http.write("ERROR")
        luci.http.write("\n") 
    end
    return
  end
  luci.http.status(500, "Bad address")
  --diag_command("ping -c 5 -W 1 %q 2>&1", addr)
end