include $(TOPDIR)/rules.mk

PKG_NAME:=ubus_example
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/ubus_example
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+libubus +libblobmsg-json +libubox
	TITLE:=ubus demo
endef

define Package/ubus_example/description
	It's a ubus demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(call Build/Compile/Default)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/server  $(1)/bin
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/client  $(1)/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
