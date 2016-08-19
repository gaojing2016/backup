include $(TOPDIR)/rules.mk

PKG_NAME:=fxagent
PKG_VERSION:=1.0
PKG_RELEASE:=0

#openwrt default already exist
#PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
#DEPENDS:=+libuci +libubus +libblobmsg-json +libubox +opkg-customer +libpthread +libcurl +libstdcpp +alljoyn +alljoyn-about +alljoyn-config +alljoyn-services_common +alljoyn-notification

include $(INCLUDE_DIR)/package.mk
 
define Package/$(PKG_NAME)
    SECTION:=utils
    CATEGORY:=Utilities
    DEFAULT:=y
    DEPENDS:=+libuci +libubus +libblobmsg-json +libubox +opkg-customer +libpthread +libcurl \
		+libstdcpp +mxml \
		+alljoyn +alljoyn-about +alljoyn-config +alljoyn-controlpanel +alljoyn-services_common +alljoyn-notification \
		+libsqlite3
    TITLE:=cloud client to manage the ap
endef

define Package/$(PKG_NAME)/description
        this is a cloud client to manage the ap
endef

 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/loudc/postinst
#!/bin/sh
/etc/init.d/fxagent enable
/etc/init.d/fxagent restart
rm -rf /tmp/luci-*
endef

define Package/fxagent/postrm
#!/bin/sh
rm -rf /tmp/luci-*
killall -kill fxagent
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/fxagent  $(1)/usr/bin/
	$(CP)  ./files/*    $(1)/
endef
 
$(eval $(call BuildPackage,$(PKG_NAME)))

