
module("luci.controller.cloudc", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/cloudc") then
		return
	end

	local page = entry({"admin", "network", "cloudc"}, cbi("cloudc"), _("cloudc"))
	page.dependent = true

end
