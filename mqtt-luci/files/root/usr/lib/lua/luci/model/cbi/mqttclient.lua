require("luci.sys")

m = Map("mqttclient", translate("MQTT Client"), translate("Configure MQTT Client."))

s = m:section(TypedSection, "login", "")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enable", translate("Enable"))
name = s:option(Value, "username", translate("Username"))
pass = s:option(Value, "password", translate("Password"))
pass.password = true
server_ip = s:option(Value, "server_ip", translate("ServerIp"))

server_port = s:option(Value, "server_port", translate("ServerPort"))

local apply = luci.http.formvalue("cbi.apply")
-- if apply then
io.popen("/etc/init.d/mqttclient start >/dev/null")
-- end

return m
