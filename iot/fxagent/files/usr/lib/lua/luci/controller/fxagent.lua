
module("luci.controller.fxagent", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/fxagent") then
		return
	end

	local page = entry({"admin", "network", "fxagent"}, cbi("fxagent"), _("fxagent"))
	page.dependent = true

end
