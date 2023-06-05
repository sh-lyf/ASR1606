#
# Makefile
#
ifeq ($(OS),Windows_NT)
RM := rm
BAD_SLASH := $(strip /)
GOOD_SLASH := $(strip \)
GOOD_BREAKER := $(strip :)
else
RM := rm
BAD_SLASH := $(strip \)
GOOD_SLASH := $(strip /)
GOOD_BREAKER := $(strip :)
endif

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR := $(patsubst %/,%,$(dir $(mkfile_path)))
APP_TOP_DIR := $(patsubst %/,%,$(dir $(CUR_DIR)))

ABOOT_DIR := $(TOOL_DIR)/$(PLATFORM)/aboot
APP_BIN_DIR := $(APP_TOP_DIR)/out/$(MODULE)
SC_IMAGES_DIR := $(ABOOT_DIR)/images/sc_images/$(MODULE)

ARELEASE := $(ABOOT_DIR)/arelease.exe
ARELEASE := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ARELEASE))
TARGET_PATHNAME := $(APP_BIN_DIR)/$(MODULE).zip
TARGET_PATHNAME := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(TARGET_PATHNAME))

cp_image := $(KERNEL_TARGET_DIR)/$(MODULE)/cp.bin
rd_image := $(SC_IMAGES_DIR)/ReliableData.bin
rf_image := $(SC_IMAGES_DIR)/rf.bin
app_image := $(APP_BIN_DIR)/$(APP_NAME).bin

CUR_DIR := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(CUR_DIR))
APP_TOP_DIR := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_TOP_DIR))
ABOOT_DIR := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ABOOT_DIR))
APP_BIN_DIR := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_BIN_DIR))
SC_IMAGES_DIR := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(SC_IMAGES_DIR))
cp_image := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(cp_image))
rd_image := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(rd_image))
rf_image := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(rf_image))
app_image := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(app_image))
boot33_image := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ABOOT_DIR)/images/boot33.bin)

FLASH_TARGET_PRE := CRANEL_A0

ifeq (TRUE,$(patsubst %S,TRUE,$(word 2,$(subst _, ,$(MODULE)))))
	FLASH_TARGET_SIZE := _8M
else ifeq (TRUE,$(patsubst %V,TRUE,$(word 2,$(subst _, ,$(MODULE)))))
	FLASH_TARGET_SIZE := _4M
endif

ifneq (,$(findstring A7680C_M,$(MODULE)))
	FLASH_TARGET_CUSTOM := _ASR5311
else ifneq (,$(findstring WHXA,$(MODULE)))
	FLASH_TARGET_CUSTOM := _WHXA
else ifneq (,$(findstring GWSD,$(MODULE)))
	FLASH_TARGET_CUSTOM := _GWSD
else ifneq (,$(findstring JWZD,$(MODULE)))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring WHXBY,$(MODULE)))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring A7680C_LANS_XC,$(MODULE)))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring A7680C_LANV_XC,$(MODULE)))
	FLASH_TARGET_CUSTOM := _APP_320K
else ifneq (,$(findstring BL_POC_MMI,$(MODULE)))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring _ST,$(MODULE)))
	FLASH_TARGET_CUSTOM := _ST
else ifneq (,$(findstring _XYJ,$(MODULE)))
	FLASH_TARGET_CUSTOM := _XYJ
else ifneq (,$(findstring ZS,$(MODULE)))
	FLASH_TARGET_CUSTOM := _ZS
else ifneq (,$(findstring GZYM,$(MODULE)))
	FLASH_TARGET_CUSTOM := _GZYM
else ifneq (,$(findstring QZZT_POC_MMI,$(MODULE)))
	FLASH_TARGET_CUSTOM := _QZZT
else ifneq (,$(findstring _HX,$(MODULE)))
	FLASH_TARGET_CUSTOM := _HX
else ifneq ($(word 2,$(subst _, ,$(MODULE)))$(word 2,$(subst _, ,$(MODULE))),$(patsubst F%,,$(word 2,$(subst _, ,$(MODULE))))$(patsubst B%,,$(word 2,$(subst _, ,$(MODULE)))))
	FLASH_TARGET_CUSTOM := _BT
else
	FLASH_TARGET_CUSTOM :=
endif

ifneq (,$(findstring HTTX,$(MODULE)))
ifneq (,$(findstring MANS_HTTX,$(MODULE)))
	FLASH_TARGET_CUSTOM := _ASR5311_HTTX
else
	FLASH_TARGET_CUSTOM := _HTTX
endif
endif

ifneq (,$(findstring A7680C_MANS_XC,$(MODULE)))
	FLASH_TARGET_CUSTOM := _ASR5311_APP_512K
endif

ifneq (,$(findstring A7680C_MANV,$(MODULE)))
	FLASH_TARGET_CUSTOM := _ASR5311_APP_140K
endif

FLASH_TARGET := $(FLASH_TARGET_PRE)$(FLASH_TARGET_SIZE)$(FLASH_TARGET_CUSTOM)

Images := "customer_app=$(app_image),cp=$(cp_image),rd=$(rd_image),rfbin=$(rf_image)"

.PHONE: all

all: boot33
	$(ARELEASE) -c $(ABOOT_DIR) -g -p ASR_CRANEL_EVB -v $(FLASH_TARGET) -i $(Images) $(TARGET_PATHNAME)

boot33:
ifneq (,$(findstring _ST,$(MODULE)))
	copy $(dir $(boot33_image))\boot33\cus_st\boot33.bin $(boot33_image)
else
	copy $(dir $(boot33_image))\boot33\ori\boot33.bin $(boot33_image)
endif
