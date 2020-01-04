module("luci.controller.admin.voip", package.seeall)


function index()
	local page
	entry({"admin", "voip"}, alias("admin", "voip", "voip"), _("Voip"), 35).index = true
	entry({"admin", "voip", "VoipConfig"}, cbi("admin_voip/voipconfig"), _("Session"), 1)		
	entry({"admin", "voip", "SipClient"}, cbi("admin_voip/sipconfig"), _("SipStack"), 2)
	--entry({"admin", "voip", "VoipTesting"}, cbi("admin_voip/voip_testing"), "Testing", 3)		
	entry({"admin", "voip", "VoipStatus"}, template("admin_voip/voip_status"), "Status", 4)
end
