GW_MODULE:=adapt
GW_MODULE_NAME:=adapt

include $(TOPDIR)/rules.mk

PKG_NAME:=btadapter
PKG_BASE_VERSION:=0.1
PKG_PATCHLEVEL:=.00
PKG_RELEASE:=1
#PKG_MD5SUM:=389c2623619281e6849da25cee7e1bb7
PKG_VERSION:=$(PKG_BASE_VERSION)$(PKG_PATCHLEVEL)
PKG_TARBALL_VERSION:=$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_OBJ_DIR = $(PKG_BUILD_DIR)/obj

# SCons supports parallel builds but does not support make's jobserver
#PKG_BUILD_PARALLEL:=$(if $(CONFIG_PKG_BUILD_USE_JOBSERVER),0,1)

#PKG_CONFIG_DEPENDS:= CONFIG_PACKAGE_$(PKG_NAME)-samples

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  TITLE:= bluetooth led adapt
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=alljoyn \
	+alljoyn-about \
	+alljoyn-config \
	+alljoyn-controlpanel \
	+alljoyn-notification \
	+alljoyn-services_common \
	+libsqlite3
endef

define Package/$(PKG_NAME)/description
bluetooth led adapt for non-alljoyn device
endef



ifeq ($(CONFIG_DEBUG),y)
  GW_ADAPT_BUILD_VARIANT:=debug
else
  GW_ADAPT_BUILD_VARIANT:=release
endif


PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/$(GW_ADAPT_BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR) $(PKG_BUILD_OBJ_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/ -r
endef


#define Build/Configure/Default
# Override default to do nothing
#endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)
endef

#define Build/InstallDev
#	$(INSTALL_DIR) $(1)/usr/bin
#	$(INSTALL_BIN) $()/gw-adapt $(1)/usr/bin
#	$(INSTALL_DIR) $(1)/usr/lib
#endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/btadapter $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/FactoryConfigService.conf $(1)/usr/bin/FactoryBt.conf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ConfigService.conf $(1)/usr/bin/Bt.conf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ConfigService.conf $(1)/usr/bin/TYled334455.conf
	$(INSTALL_DIR) $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
