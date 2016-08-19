include $(TOPDIR)/rules.mk

PKG_NAME:=udpclient2
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
    TITLE:=udpclient2 test
endef

define Package/$(PKG_NAME)/description
        this is a cloud client to manage the ap
endef

 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/udpclient2/postinst
#!/bin/sh
/etc/init.d/udpclient2 enable
/etc/init.d/udpclient2 restart
rm -rf /tmp/luci-*
endef

define Package/udpclient2/postrm
#!/bin/sh
rm -rf /tmp/luci-*
killall -kill udpclient2 
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/udpclient2  $(1)/usr/bin/
	$(CP)  ./files/*    $(1)/
endef
 
$(eval $(call BuildPackage,$(PKG_NAME)))

