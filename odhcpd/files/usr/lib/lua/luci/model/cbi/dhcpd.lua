
require("luci.sys")
require("luci.util")
require("nixio.fs")

local fs = require "nixio.fs"
--local peerdns

if fs.access("/etc/config/network") then
    m1 = Map("network",translate("局域网IP配置:"))

    s = m1:section(NamedSection, "lan")
    s.anonymous=true
    s.addremove=false

    ipaddr=s:option(Value, "ipaddr", translate("IP地址:"))
    --ipaddr.placeholder='192.168.1.1'
    ipaddr.default = '192.168.1.1'

    netmask=s:option(Value, "netmask", translate("子网掩码:"))
    --netmask.placeholder = '255.255.255.0'
    netmask.default = '255.255.255.0'

    dns=s:option(DynamicList, "dns", translate("Use custom DNS Server:"))
    dns:depends("peerdns", "") 
    dns.datatype = "ipaddr"
    dns.cast     = "string"
    dns.default = '192.168.1.1'

    if fs.access("/etc/config/dhcp") then
        m = Map("dhcp", translate("DHCP服务"), translate("当前页面可以配置DHCP服务的相关参数。"))

        s=m:section(NamedSection, "lan", translate("DHCP服务器："))
        s.anonymous=true
        s.addremove=false

        dd = s:option( Flag, "dynamicdhcp",
        translate("Dynamic <abbr title=\"Dynamic Host Configuration Protocol\">DHCP</abbr>"),
        translate("Dynamically allocate DHCP addresses for clients. If disabled, only " ..
        "clients having static leases will be served."))
        dd.default = dd.enabled

        start=s:option(Value,"start", translate("地址池起始地址："), translate("Lowest leased address as offset from the network address."))
        start.optional = true
        start.datatype = "or(uinteger,ip4addr)"
        --start.placeholder='100'
        start.default='100'

        limit=s:option(Value,"limit",translate("地址池limit:"), translate("Maximum number of leased addresses."))
        limit.optional = true
        limit.datatype = "uinteger"
        --limit.placeholder='150'
        limit.default='150'

        leasetime=s:option(ListValue,"leasetime", translate("租用时间："), translate("Expiry time of leased addresses, minimum is 2 minutes (<code>2m</code>)."))
        leasetime.rmempty = true
        leasetime.override_values = true
        leasetime:value("12h", translate("12小时"))
        leasetime:value("2m", translate("2分钟"))
        leasetime:value("1h", translate("1小时"))
        leasetime:value("24h", translate("一天"))
        leasetime:value("168h", translate("一周"))
        leasetime.default = '12h'


        --    if fs.access("/etc/config/network") then   
        --      m2 = Map("network")
        --    s = m2:section(NamedSection, "", "")


        -- enable=s:option(Flag, "enabled", translate("手动设置DNS服务器"))
        -- enable.rmempty=false


    end
end


return m, m1



