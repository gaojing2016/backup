include $(TOPDIR)/rules.mk

PKG_NAME:=btclient
PKG_VERSION:=1.0
PKG_RELEASE:=0

#openwrt default already exist
#PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
 
define Package/$(PKG_NAME)
    SECTION:=utils
    CATEGORY:=Utilities
    DEFAULT:=y
    DEPENDS:=+libuci 
    TITLE:=btclient test
endef

define Package/$(PKG_NAME)/description
        this is a cloud client to manage the ap
endef

 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/btclient/postinst
#!/bin/sh
/etc/init.d/btclient enable
/etc/init.d/btclient restart
rm -rf /tmp/luci-*
endef

define Package/btclient/postrm
#!/bin/sh
rm -rf /tmp/luci-*
killall -kill btclient
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/btclient  $(1)/usr/bin/
	$(CP)  ./files/*    $(1)/
endef
 
$(eval $(call BuildPackage,$(PKG_NAME)))

