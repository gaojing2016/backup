
require("luci.sys")
require("luci.util")
require("nixio.fs")

local fs = require "nixio.fs"


m = Map("fxagent", translate(""), translate(""))

s=m:section(TypedSection, "global", translate("FXAGENT配置"))
s.anonymous=true                                                        
s.addremove=false

enable=s:option(Flag, "enable", translate("开启"))
enable.datatype = "bool"
enable.disabled = "0"
enable.enabled = "1"
enable.default = enable.enabled
enable.rmempty  = false


local apply = luci.http.formvalue("cbi.apply")
if apply then
	io.popen("/etc/init.d/fxagent restart")
end

return m
