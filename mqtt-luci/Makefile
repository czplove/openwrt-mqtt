include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-mqttclient
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-mqttclient
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=NJIT 802.1X Client for LuCI
	PKGARCH:=all
endef

define Package/luci-app-mqttclient/description
    This package contains LuCI configuration pages for njit8021xclient.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-mqttclient/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
    
	$(INSTALL_CONF) ./files/root/etc/config/mqttclient $(1)/etc/config/mqttclient
	$(INSTALL_BIN) ./files/root/etc/init.d/mqttclient $(1)/etc/init.d/mqttclient
	$(INSTALL_DATA) ./files/root/usr/lib/lua/luci/model/cbi/mqttclient.lua $(1)/usr/lib/lua/luci/model/cbi/mqttclient.lua
	$(INSTALL_DATA) ./files/root/usr/lib/lua/luci/controller/mqttclient.lua $(1)/usr/lib/lua/luci/controller/mqttclient.lua
endef

$(eval $(call BuildPackage,luci-app-mqttclient))
