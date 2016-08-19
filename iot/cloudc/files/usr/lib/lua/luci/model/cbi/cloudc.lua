
require("luci.sys")
require("luci.util")
require("nixio.fs")

local fs = require "nixio.fs"


m = Map("cloudc", translate(""), translate(""))

s=m:section(TypedSection, "global", translate("云客户端配置"))
s.anonymous=true                                                        
s.addremove=false

enable=s:option(Flag, "enable", translate("开启"))
enable.datatype = "bool"
enable.disabled = "0"
enable.enabled = "1"
enable.default = enable.enabled
enable.rmempty  = false

loglevel = s:option(Value, "loglevel", translate("loglevel"), translate("just for develop, will remove it on offical release"))
loglevel.placeholder="debug"

t=m:section(TypedSection, "cloud_server", translate("云服务器配置"))
t.anonymous=true                                                        
t.addremove=false

cloud_server_url = t:option(Value, "cloud_server_url", translate("云服务器URL"))
cloud_server_url.datatype = "host"
cloud_server_url.default = "cloud.server"
cloud_server_port = t:option(Value, "cloud_server_port", translate("云服务器端口"))
cloud_server_port.datatype = "port"
cloud_server_port.default = 80




local apply = luci.http.formvalue("cbi.apply")
if apply then
	io.popen("/etc/init.d/cloudc restart")
end

return m
