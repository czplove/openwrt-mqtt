module("luci.controller.mqttclient",package.seeall)
function index()
	entry({"admin","network","mqttclient"},cbi("mqttclient"),_("MQTT Client"),100)
	end
