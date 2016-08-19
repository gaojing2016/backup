include $(TOPDIR)/rules.mk

PKG_NAME:=wechat
PKG_VERSION:=1.0
PKG_RELEASE:=0

#openwrt default already exist
#PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
#    DEPENDS:=+libuci +libubus +libblobmsg-json +libubox +libpthread +librt +libstdc++

include $(INCLUDE_DIR)/package.mk
 
define Package/$(PKG_NAME)
    SECTION:=utils
    CATEGORY:=Utilities
    DEFAULT:=y
    DEPENDS:=+libuci +libubus +libblobmsg-json +libubox +libpthread +librt +libstdcpp
    TITLE:=wechat test
endef

define Package/$(PKG_NAME)/description
        this is a wechat test for the way to IOT_Home_Gateway
endef

 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/wechat/postinst
#!/bin/sh
/etc/init.d/wechat enable
/etc/init.d/wechat restart
rm -rf /tmp/luci-*
endef

define Package/wechat/postrm
#!/bin/sh
rm -rf /tmp/luci-*
killall -kill wechat
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/wechat  $(1)/usr/bin/
	$(CP)  ./files/*    $(1)/
endef
 
$(eval $(call BuildPackage,$(PKG_NAME)))

