EXTRA_CFLAGS += $(USER_EXTRA_CFLAGS)
EXTRA_CFLAGS += -O1
#EXTRA_CFLAGS += -O3
#EXTRA_CFLAGS += -Wall
#EXTRA_CFLAGS += -Wextra
#EXTRA_CFLAGS += -Werror
#EXTRA_CFLAGS += -pedantic
#EXTRA_CFLAGS += -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes

EXTRA_CFLAGS += -Wno-unused-variable
EXTRA_CFLAGS += -Wno-unused-value
EXTRA_CFLAGS += -Wno-unused-label
EXTRA_CFLAGS += -Wno-unused-parameter
EXTRA_CFLAGS += -Wno-unused-function
EXTRA_CFLAGS += -Wno-unused

EXTRA_CFLAGS += -Wno-uninitialized

EXTRA_CFLAGS += -I$(src)/include

CONFIG_AUTOCFG_CP = y

CONFIG_RTL8192C = n
CONFIG_RTL8192D = n
CONFIG_RTL8723A = n
CONFIG_RTL8188E = y

CONFIG_USB_HCI = y
CONFIG_PCI_HCI = n
CONFIG_SDIO_HCI = n
CONFIG_GSPI_HCI = n

CONFIG_MP_INCLUDED = y
CONFIG_POWER_SAVING = y
CONFIG_USB_AUTOSUSPEND = n
CONFIG_HW_PWRP_DETECTION = n
CONFIG_WIFI_TEST = n
CONFIG_BT_COEXIST = n
CONFIG_RTL8192CU_REDEFINE_1X1 = n
CONFIG_INTEL_WIDI = n
CONFIG_WAPI_SUPPORT = n
CONFIG_EFUSE_CONFIG_FILE = n
CONFIG_EXT_CLK = n
CONFIG_FTP_PROTECT = n
CONFIG_WOWLAN = n
CONFIG_GPIO_WAKEUP = n
CONFIG_ODM_ADAPTIVITY = n

export TopDIR ?= $(shell pwd)

OUTSRC_FILES := hal/OUTSRC/odm_debug.o	\
				hal/OUTSRC/odm_interface.o\
				hal/OUTSRC/odm_HWConfig.o\
				hal/OUTSRC/odm.o\
				hal/OUTSRC/HalPhyRf.o
										
ifeq ($(CONFIG_RTL8192C), y)
RTL871X = rtl8192c

ifeq ($(CONFIG_USB_HCI), y)
MODULE_NAME = 8192cu
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/Hal8192CUFWImg_CE.o	\
								hal/OUTSRC/$(RTL871X)/Hal8192CUPHYImg_CE.o	\
								hal/OUTSRC/$(RTL871X)/Hal8192CUMACImg_CE.o
endif

ifeq ($(CONFIG_PCI_HCI), y)
MODULE_NAME = 8192ce
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/Hal8192CEFWImg_CE.o	\
								hal/OUTSRC/$(RTL871X)/Hal8192CEPHYImg_CE.o	\
								hal/OUTSRC/$(RTL871X)/Hal8192CEMACImg_CE.o
endif

OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/odm_RTL8192C.o\
								hal/OUTSRC/$(RTL871X)/HalDMOutSrc8192C_CE.o

CHIP_FILES := \
	hal/$(RTL871X)/$(RTL871X)_sreset.o \
	hal/$(RTL871X)/$(RTL871X)_xmit.o
CHIP_FILES += $(OUTSRC_FILES)

endif

ifeq ($(CONFIG_RTL8192D), y)
RTL871X = rtl8192d

ifeq ($(CONFIG_USB_HCI), y)
MODULE_NAME = 8192du
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/Hal8192DUFWImg_CE.o \
								hal/OUTSRC/$(RTL871X)/Hal8192DUPHYImg_CE.o \
								hal/OUTSRC/$(RTL871X)/Hal8192DUMACImg_CE.o
endif

ifeq ($(CONFIG_PCI_HCI), y)
MODULE_NAME = 8192de
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/Hal8192DEFWImg_CE.o \
								hal/OUTSRC/$(RTL871X)/Hal8192DEPHYImg_CE.o \
								hal/OUTSRC/$(RTL871X)/Hal8192DEMACImg_CE.o
endif

OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/odm_RTL8192D.o\
								hal/OUTSRC/$(RTL871X)/HalDMOutSrc8192D_CE.o
CHIP_FILES := \
	hal/$(RTL871X)/$(RTL871X)_xmit.o
CHIP_FILES += $(OUTSRC_FILES)
endif

ifeq ($(CONFIG_RTL8723A), y)

RTL871X = rtl8723a

HAL_COMM_FILES := hal/$(RTL871X)/$(RTL871X)_xmit.o \
								hal/$(RTL871X)/$(RTL871X)_sreset.o

ifeq ($(CONFIG_GSPI_HCI), y)
MODULE_NAME = 8723as
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/Hal8723SHWImg_CE.o
endif

ifeq ($(CONFIG_SDIO_HCI), y)
MODULE_NAME = 8723as
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/Hal8723SHWImg_CE.o
endif

ifeq ($(CONFIG_USB_HCI), y)
MODULE_NAME = 8723au
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/Hal8723UHWImg_CE.o
endif

ifeq ($(CONFIG_PCI_HCI), y)
MODULE_NAME = 8723ae
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/Hal8723EHWImg_CE.o
endif

#hal/OUTSRC/$(RTL871X)/HalHWImg8723A_FW.o
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/HalHWImg8723A_BB.o\
								hal/OUTSRC/$(RTL871X)/HalHWImg8723A_MAC.o\
								hal/OUTSRC/$(RTL871X)/HalHWImg8723A_RF.o\
								hal/OUTSRC/$(RTL871X)/odm_RegConfig8723A.o

OUTSRC_FILES += hal/OUTSRC/rtl8192c/HalDMOutSrc8192C_CE.o
clean_more ?=
clean_more += clean_odm-8192c

PWRSEQ_FILES := hal/HalPwrSeqCmd.o \
				hal/$(RTL871X)/Hal8723PwrSeq.o

CHIP_FILES += $(HAL_COMM_FILES) $(OUTSRC_FILES) $(PWRSEQ_FILES)

ifeq ($(CONFIG_BT_COEXIST), y)
CHIP_FILES += hal/$(RTL871X)/rtl8723a_bt-coexist.o
endif

endif

ifeq ($(CONFIG_RTL8188E), y)

RTL871X = rtl8188e
HAL_COMM_FILES := hal/$(RTL871X)/$(RTL871X)_xmit.o\
					hal/$(RTL871X)/$(RTL871X)_sreset.o

ifeq ($(CONFIG_SDIO_HCI), y)
MODULE_NAME = 8189es
endif

ifeq ($(CONFIG_USB_HCI), y)
MODULE_NAME = 8188eu
endif

ifeq ($(CONFIG_PCI_HCI), y)
MODULE_NAME = 8188ee
endif

#hal/OUTSRC/$(RTL871X)/HalHWImg8188E_FW.o
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/HalHWImg8188E_MAC.o\
		hal/OUTSRC/$(RTL871X)/HalHWImg8188E_BB.o\
		hal/OUTSRC/$(RTL871X)/HalHWImg8188E_RF.o\
		hal/OUTSRC/$(RTL871X)/Hal8188EFWImg_CE.o\
		hal/OUTSRC/$(RTL871X)/HalPhyRf_8188e.o\
		hal/OUTSRC/$(RTL871X)/odm_RegConfig8188E.o\
		hal/OUTSRC/$(RTL871X)/Hal8188ERateAdaptive.o\
		hal/OUTSRC/$(RTL871X)/odm_RTL8188E.o

ifeq ($(CONFIG_RTL8188E), y)
ifeq ($(CONFIG_WOWLAN), y)
OUTSRC_FILES += hal/OUTSRC/$(RTL871X)/HalHWImg8188E_FW.o
endif
endif

PWRSEQ_FILES := hal/HalPwrSeqCmd.o \
				hal/$(RTL871X)/Hal8188EPwrSeq.o

CHIP_FILES += $(HAL_COMM_FILES) $(OUTSRC_FILES) $(PWRSEQ_FILES) 

endif


ifeq ($(CONFIG_GSPI_HCI), y)
HCI_NAME = gspi
endif

ifeq ($(CONFIG_SDIO_HCI), y)
HCI_NAME = sdio
endif

ifeq ($(CONFIG_USB_HCI), y)
HCI_NAME = usb
endif

ifeq ($(CONFIG_PCI_HCI), y)
HCI_NAME = pci
endif


_OS_INTFS_FILES :=	os_dep/osdep_service.o \
			os_dep/linux/os_intfs.o \
			os_dep/linux/$(HCI_NAME)_intf.o \
			os_dep/linux/$(HCI_NAME)_ops_linux.o \
			os_dep/linux/ioctl_linux.o \
			os_dep/linux/xmit_linux.o \
			os_dep/linux/mlme_linux.o \
			os_dep/linux/recv_linux.o \
			os_dep/linux/ioctl_cfg80211.o \
			os_dep/linux/rtw_android.o

ifeq ($(CONFIG_SDIO_HCI), y)
_OS_INTFS_FILES += os_dep/linux/custom_gpio_linux.o
_OS_INTFS_FILES += os_dep/linux/$(HCI_NAME)_ops_linux.o
endif

ifeq ($(CONFIG_GSPI_HCI), y)
_OS_INTFS_FILES += os_dep/linux/custom_gpio_linux.o
_OS_INTFS_FILES += os_dep/linux/$(HCI_NAME)_ops_linux.o
endif

_HAL_INTFS_FILES :=	hal/hal_intf.o \
			hal/hal_com.o \
			hal/$(RTL871X)/$(RTL871X)_hal_init.o \
			hal/$(RTL871X)/$(RTL871X)_phycfg.o \
			hal/$(RTL871X)/$(RTL871X)_rf6052.o \
			hal/$(RTL871X)/$(RTL871X)_dm.o \
			hal/$(RTL871X)/$(RTL871X)_rxdesc.o \
			hal/$(RTL871X)/$(RTL871X)_cmd.o \
			hal/$(RTL871X)/$(HCI_NAME)/$(HCI_NAME)_halinit.o \
			hal/$(RTL871X)/$(HCI_NAME)/rtl$(MODULE_NAME)_led.o \
			hal/$(RTL871X)/$(HCI_NAME)/rtl$(MODULE_NAME)_xmit.o \
			hal/$(RTL871X)/$(HCI_NAME)/rtl$(MODULE_NAME)_recv.o

ifeq ($(CONFIG_SDIO_HCI), y)
_HAL_INTFS_FILES += hal/$(RTL871X)/$(HCI_NAME)/$(HCI_NAME)_ops.o
else
ifeq ($(CONFIG_GSPI_HCI), y)
_HAL_INTFS_FILES += hal/$(RTL871X)/$(HCI_NAME)/$(HCI_NAME)_ops.o
else
_HAL_INTFS_FILES += hal/$(RTL871X)/$(HCI_NAME)/$(HCI_NAME)_ops_linux.o
endif
endif

ifeq ($(CONFIG_MP_INCLUDED), y)
_HAL_INTFS_FILES += hal/$(RTL871X)/$(RTL871X)_mp.o
endif

_HAL_INTFS_FILES += $(CHIP_FILES)


#ifeq ($(CONFIG_AUTOCFG_CP), y)

#ifeq ($(CONFIG_RTL8188E)$(CONFIG_SDIO_HCI),yy) 
#$(shell cp $(TopDIR)/autoconf_rtl8189e_$(HCI_NAME)_linux.h $(TopDIR)/include/autoconf.h)
#else
#$(shell cp $(TopDIR)/autoconf_$(RTL871X)_$(HCI_NAME)_linux.h $(TopDIR)/include/autoconf.h)
#endif
#endif


ifeq ($(CONFIG_USB_HCI), y)
ifeq ($(CONFIG_USB_AUTOSUSPEND), y)
EXTRA_CFLAGS += -DCONFIG_USB_AUTOSUSPEND
endif
endif

ifeq ($(CONFIG_MP_INCLUDED), y)
#MODULE_NAME := $(MODULE_NAME)_mp
EXTRA_CFLAGS += -DCONFIG_MP_INCLUDED
endif

ifeq ($(CONFIG_POWER_SAVING), y)
EXTRA_CFLAGS += -DCONFIG_POWER_SAVING
endif

ifeq ($(CONFIG_HW_PWRP_DETECTION), y)
EXTRA_CFLAGS += -DCONFIG_HW_PWRP_DETECTION
endif

ifeq ($(CONFIG_WIFI_TEST), y)
EXTRA_CFLAGS += -DCONFIG_WIFI_TEST
endif

ifeq ($(CONFIG_BT_COEXIST), y)
EXTRA_CFLAGS += -DCONFIG_BT_COEXIST
endif

ifeq ($(CONFIG_RTL8192CU_REDEFINE_1X1), y)
EXTRA_CFLAGS += -DRTL8192C_RECONFIG_TO_1T1R
endif

ifeq ($(CONFIG_INTEL_WIDI), y)
EXTRA_CFLAGS += -DCONFIG_INTEL_WIDI
endif

ifeq ($(CONFIG_WAPI_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_WAPI_SUPPORT
endif

ifeq ($(CONFIG_EFUSE_CONFIG_FILE), y)
EXTRA_CFLAGS += -DCONFIG_EFUSE_CONFIG_FILE
endif

ifeq ($(CONFIG_EXT_CLK), y)
EXTRA_CFLAGS += -DCONFIG_EXT_CLK
endif

ifeq ($(CONFIG_FTP_PROTECT), y)
EXTRA_CFLAGS += -DCONFIG_FTP_PROTECT
endif

ifeq ($(CONFIG_ODM_ADAPTIVITY), y)
EXTRA_CFLAGS += -DCONFIG_ODM_ADAPTIVITY
endif

ifeq ($(CONFIG_RTL8188E), y)
ifeq ($(CONFIG_WOWLAN), y)
EXTRA_CFLAGS += -DCONFIG_WOWLAN
endif
ifeq ($(CONFIG_GPIO_WAKEUP), y)
EXTRA_CFLAGS += -DCONFIG_GPIO_WAKEUP
endif
endif

ifeq ($(CONFIG_RTL8188E), y)
ifeq ($(CONFIG_EFUSE_CONFIG_FILE), y)
EXTRA_CFLAGS += -DCONFIG_RF_GAIN_OFFSET
endif
endif

EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN

ifneq ($(USER_MODULE_NAME),)
MODULE_NAME := $(USER_MODULE_NAME)
endif

ifneq ($(KERNELRELEASE),)

rtk_core :=	core/rtw_cmd.o \
		core/rtw_security.o \
		core/rtw_debug.o \
		core/rtw_io.o \
		core/rtw_ioctl_query.o \
		core/rtw_ioctl_set.o \
		core/rtw_ieee80211.o \
		core/rtw_mlme.o \
		core/rtw_mlme_ext.o \
		core/rtw_wlan_util.o \
		core/rtw_pwrctrl.o \
		core/rtw_rf.o \
		core/rtw_recv.o \
		core/rtw_sta_mgt.o \
		core/rtw_ap.o \
		core/rtw_xmit.o	\
		core/rtw_p2p.o \
		core/rtw_tdls.o \
		core/rtw_br_ext.o \
		core/rtw_iol.o \
		core/rtw_led.o \
		core/rtw_sreset.o \
		core/rtw_odm.o

$(MODULE_NAME)-y += $(rtk_core)

$(MODULE_NAME)-$(CONFIG_INTEL_WIDI) += core/rtw_intel_widi.o

$(MODULE_NAME)-$(CONFIG_WAPI_SUPPORT) += core/rtw_wapi.o	\
					core/rtw_wapi_sms4.o

$(MODULE_NAME)-y += core/efuse/rtw_efuse.o

$(MODULE_NAME)-y += $(_HAL_INTFS_FILES)

$(MODULE_NAME)-y += $(_OS_INTFS_FILES)

$(MODULE_NAME)-$(CONFIG_MP_INCLUDED) += core/rtw_mp.o \
					core/rtw_mp_ioctl.o
ifeq ($(CONFIG_RTL8723A), y)

$(MODULE_NAME)-$(CONFIG_MP_INCLUDED)+= core/rtw_bt_mp.o
endif

obj-$(CONFIG_RTL8188EU) := $(MODULE_NAME).o

else

export CONFIG_RTL8188EU = m

all: modules

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd)  modules

strip:
	$(CROSS_COMPILE)strip $(MODULE_NAME).ko --strip-unneeded

install:
	install -p -m 644 $(MODULE_NAME).ko  $(MODDESTDIR)
	/sbin/depmod -a ${KVER}

uninstall:
	rm -f $(MODDESTDIR)/$(MODULE_NAME).ko
	/sbin/depmod -a ${KVER}

config_r:
	@echo "make config"
	/bin/bash script/Configure script/config.in

.PHONY: modules clean clean_odm-8192c

clean_odm-8192c:
	cd hal/OUTSRC/rtl8192c ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko

clean: $(clean_more)
	rm -fr *.mod.c *.mod *.o .*.cmd *.ko *~
	rm -fr .tmp_versions
	rm -fr Module.symvers ; rm -fr Module.markers ; rm -fr modules.order
	cd core/efuse ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd core ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd hal/$(RTL871X)/$(HCI_NAME) ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd hal/$(RTL871X) ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd hal/OUTSRC/$(RTL871X) ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko 
	cd hal/OUTSRC/ ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko 
	cd hal ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd os_dep/linux ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd os_dep ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
endif

