#
# Makefile
#

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT_DIR := $(patsubst %/,%, $(dir $(mkfile_path)))

ifneq (cus_application, $(wildcard cus_application))
SIMCOM_SDK := YES
SIMCOM_SDK_MACRO := $(strip -DSIMCOM_SDK=ON)
TOOL_DIR := $(ROOT_DIR)/tools
APP_DIR := $(ROOT_DIR)
KERNEL_DIR :=$(ROOT_DIR)/kernel
else
SIMCOM_SDK := NO
SIMCOM_SDK_MACRO :=
TOOL_DIR := $(ROOT_DIR)/tavor/tools
APP_DIR := $(ROOT_DIR)/cus_application
KERNEL_DIR :=$(ROOT_DIR)/tavor/Arbel/kernel
endif

EXTRACT_KERNEL_MACRO := python ${TOOL_DIR}/extract_macro_from_buildlog.py
OUT = out
RELEASE = release
RELEASE_SET = SIMCOM_SDK_SET
MAKEFILENAME := Makefile
CRC_SET := crc_set
OBJCPY := arm-none-eabi-objcopy
API_MAP_FILE_NAME := api_map
APP_NAME := customer_app

ifeq ($(OS),Windows_NT)
	PLATFORM := win32
	GCC_ARM_TOOLCHAIN_PACKAGE := cross_tool.zip
	GZIP := $(TOOL_DIR)/$(PLATFORM)/7z/7z.exe
	GZIPARG := x
	COPY := xcopy
	COPYARG := /s /e /q
	COPY_FILE := copy
	COPYARG_FILE := /y
	MKDIR := mkdir
	MKDIRARG :=
	RM := del
	RMARG :=
	RMDIR := rmdir
	RMDIRARG := /s /q
	MOVE := ren
	BUILD_TYPE := "Unix Makefiles"
	BUILD := gnumake
	MAKE := gnumake
	CMAKE := ${TOOL_DIR}/${PLATFORM}/cmake/bin/cmake.exe
	CMAKE_DIR := ${TOOL_DIR}/${PLATFORM}/cmake
	CMAKE_PACKAGE := ${TOOL_DIR}/${PLATFORM}/cmake.zip
	BAD_SLASH := $(strip /)
	GOOD_SLASH := $(strip \)
	GOOD_BREAKER := $(strip ;)
else ifeq ($(shell uname),GNU/Linux)
	PLATFORM := linux
	GCC_ARM_TOOLCHAIN_PACKAGE := cross_tool.tar.bz2
	GZIP := tar
	GZIPARG := -jxf
	COPY := cp
	COPYARG := -rf
	COPY_FILE := cp
	COPYARG_FILE := -rf
	MKDIR := mkdir
	MKDIRARG := -p
	RM := rm
	RMARG :=
	RMDIR := rm
	RMDIRARG := -rf
	MOVE := mv
	BUILD_TYPE := "Unix Makefiles"
	BUILD := make
	MAKE := make
	CMAKE := cmake
	CMAKE_DIR :=
	CMAKE_PACKAGE :=
	BAD_SLASH := $(strip \)
	GOOD_SLASH := $(strip /)
	GOOD_BREAKER := $(strip :)
else
  $(error The os "$(shell uname)" is not supported)
endif
SC_TOOLCHAIN_DIR := ${TOOL_DIR}/${PLATFORM}
GCC_ARM_TOOLCHAIN_DIR := ${TOOL_DIR}/${PLATFORM}/cross_tool/gcc-arm-none-eabi
GCC_ARM_TOOLCHAIN := ${GCC_ARM_TOOLCHAIN_DIR}/bin

export PATH := ${PATH}$(GOOD_BREAKER)$(subst $(BAD_SLASH),$(GOOD_SLASH),${TOOL_DIR}/${PLATFORM})$(GOOD_BREAKER)$(subst $(BAD_SLASH),$(GOOD_SLASH),${GCC_ARM_TOOLCHAIN})


-include ${ROOT_DIR}/simcom/config/custom_list.mak
include ${APP_DIR}/makeDepend.mak

MAKEFILE_DEPEND = $(patsubst %,${APP_DIR}/%,${MAKEFILE_DEPEND_LIST})

ifeq (YES,$(SIMCOM_SDK))
SCMODULE := $(sort $(notdir $(wildcard $(KERNEL_DIR)/*)))
else
SCMODULE := $(sort $(notdir $(wildcard ${ROOT_DIR}/simcom/config/config_*.mak)))
SCMODULE := ${patsubst config_%.mak,%,${SCMODULE}}
endif

.PHONY:info info_pre cleanAll clean clean_all_app clean_all_kernel

info: info_pre info_modules info_end

all:info-all
info-all: info_pre info_modules_custom info_end

info_end:
ifeq (NO,$(SIMCOM_SDK))
	@echo -
	@echo -  install list:
	@echo -      install
	@echo -      install_set
	@echo -      install_[module]
	@echo -      uninstall
	@echo -      uninstall_set
	@echo -      uninstall_[module]
endif
	@echo -
	@echo -  clean list:
	@echo -      clean                 [clean all modules]
ifeq (YES,$(SIMCOM_SDK))
	@echo -      clean_[module]        [clean a module]
else
	@echo -      clean_app             [clean all app, target and object files]
	@echo -      clean_kernel          [clean all kernel, just target files]
	@echo -      clean_[module]        [clean app and kernel, just kernel target files]
	@echo -      clean_[module]_app    [target and object files]
	@echo -      clean_[module]_kernel [just target files]
endif
	@echo -
	@echo ---------------------------------------------

info_pre:
	@echo ---------------------------------------------
	@echo -
	@echo -  build method: gnumake [target]
	@echo -
ifeq (NO,$(SIMCOM_SDK))
	@echo -  target:[module](_[custom])(_app/_kernel/_force/_remake),[clean list],[install list]
	@echo -
	@echo -  show customs of the modules: gnumake [module]-list
	@echo -  show all modules and customs at onece: gnumake all/info-all
else
	@echo -  target:[module list],[clean list]
endif
	@echo -
	@echo -  module list:

info_modules:

info_modules_custom:


${TOOL_DIR}/${PLATFORM}/cmake:${TOOL_DIR}/${PLATFORM}/cmake.zip
	cd $(dir $@) && $(GZIP) $(GZIPARG) $<

${TOOL_DIR}/${PLATFORM}/cross_tool:${TOOL_DIR}/${PLATFORM}/cross_tool.zip
	cd $(dir $@) && $(GZIP) $(GZIPARG) $<

clean: clean_app

ifeq (NO,$(SIMCOM_SDK))
clean: clean_kernel cleanAll clean_file

cleanAll:
	cd ./tavor/Arbel/build && cleanAll.bat

clean_kernel:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(KERNEL_DIR))

uninstall:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE))

endif

clean_app:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(OUT))

define user_option_process
.PHONY:clean_$(1)$(2)$(3)$(4) clean_$(1)$(2)$(3)$(4)_app clean_$(1)$(2)$(3)$(4)_kernel $(1)$(2)$(3)$(4) $(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).elf

## info create
ifeq (,$(4))
info_modules:$(1)$(2)$(3)$(4)_info
endif
info_modules_custom:$(1)$(2)$(3)$(4)_info

$(1)$(2)$(3)$(4)_info:
	@echo -      $(1)$(2)$(3)$(4)

ifneq (,$(4))
$(1)$(2)$(3)-list:$(1)$(2)$(3)-list_s $(patsubst _%,%,$(4))_info
else
$(1)$(2)$(3)-list:$(1)$(2)$(3)-list_s
	@echo -
	@echo ---------------------------------------------

$(1)$(2)$(3)-list_s:
	@echo ---------------------------------------------
	@echo -
	@echo -  $(1)$(2)$(3) custom list:
endif
## info create end



ifeq (NO,$(SIMCOM_SDK))

$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/$(API_MAP_FILE_NAME).c:$(TOOL_DIR)/apiMapInit.mak
	-$(RMDIR) $(RMDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	$(MKDIR) $(MKDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	$(MAKE) -C $(TOOL_DIR) -f apiMapInit.mak API_MAP_FILE_NAME=$(API_MAP_FILE_NAME) SC_MODULE=$(1) SC_HD_CNF=$(2) SC_HD_OPT=$(3) SC_USR_OPT=$(4) SC_MODULE_FULL=$(1)$(2)$(3)$(4)
ifeq (,$(findstring QL, $(4)))
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ROOT_DIR)/simcom/sc_interface/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
else
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ROOT_DIR)/quectel/ql_interface/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
endif

./tavor/Arbel/bin/$(1)$(2)$(3)$(4)_cp.bin:$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/$(API_MAP_FILE_NAME).c
$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(MAKEFILENAME):$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/$(API_MAP_FILE_NAME).c

tavor/Arbel/bin/$(1)$(2)$(3)$(4)_cp.bin:
ifeq (,$(wildcard $(ROOT_DIR)/tavor/Arbel/build/sc_kernel_build_$(1)$(2)$(3)$(4)))
	$(MAKE) cleanAll
endif
	-del $(ROOT_DIR)/tavor/Arbel/build/sc_kernel_build_*
	cd. > $(ROOT_DIR)/tavor/Arbel/build/sc_kernel_build_$(1)$(2)$(3)$(4)
ifeq (TRUE,$(patsubst %S,TRUE,$(2)))
	cd tavor/Arbel/build && module_lteonly_buildcust_8mram_craneL.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4)
else ifeq (TRUE,$(patsubst %V,TRUE,$(2)))
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_craneL.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4)
endif

$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin:tavor/Arbel/bin/$(1)$(2)$(3)$(4)_cp.bin
	-rm $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@)/*.bin)
	-rm $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@)/*.axf)
	-rm $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@)/*.txt)
	-rm $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@)/*.map)
	-$(MKDIR) $(MKDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	$(COPY_FILE) $(COPYARG_FILE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$<))/*.bin) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	$(COPY_FILE) $(COPYARG_FILE) $$(patsubst %.bin,%.axf,$$(subst $(BAD_SLASH),$(GOOD_SLASH),$$<)) $$(patsubst %.bin,%.axf,$$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@))
	$(COPY_FILE) $(COPYARG_FILE) $$(patsubst %.bin,%.map,$$(subst $(BAD_SLASH),$(GOOD_SLASH),$$<)) $$(patsubst %.bin,%.map,$$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@))
	$(COPY_FILE) $(COPYARG_FILE) $$(patsubst %.bin,%_MDB.txt,$$(subst $(BAD_SLASH),$(GOOD_SLASH),$$<)) $$(patsubst %.bin,%_MDB.txt,$$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@))
ifeq (win32,$(PLATFORM))
	$(RM) $(RMARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@)
	$(MOVE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$@))/$(1)$(2)$(3)$(4)_cp.bin) $$(notdir $$@)
else
	$(RM) $(RMARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@)
	$(MOVE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$@))/$(1)$(2)$(3)$(4)_cp.bin) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@)
endif

$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/KernelDefinitions.cmake:$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin
	-rm $$@
	$(EXTRACT_KERNEL_MACRO) $(ROOT_DIR)/tavor/Arbel/build/buildlog.txt yes $$@
endif

$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(MAKEFILENAME):$(CMAKE_DIR) ${TOOL_DIR}/${PLATFORM}/cross_tool $(APP_DIR)/config/ToolChain.cmake $(APP_DIR)/config/Config_APP.cmake $(KERNEL_DIR)/$(1)$(2)$(3)$(4)/KernelDefinitions.cmake ${MAKEFILE_DEPEND}
	-$(RMDIR) $(RMDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	-$(MKDIR) $(MKDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
ifeq (NO,$(SIMCOM_SDK))
	cd $$(dir $$@) && $$(CMAKE) \
		-DSCMODULE="$(1)$(2)$(3)$(4)" $(SIMCOM_SDK_MACRO) \
		-DKERNEL_DIR="$(KERNEL_DIR)" \
		$(patsubst %,-D%=ON,$(subst _, ,$(patsubst _%,%,$(4)))) \
		-D$(1)$(2)$(3)$(4)=ON \
		-DGCC_ARM_TOOLCHAIN_DIR="$(GCC_ARM_TOOLCHAIN_DIR)" \
		-DCMAKE_TOOLCHAIN_FILE="$(APP_DIR)/config/ToolChain.cmake" \
		-DAPP_NAME="$(APP_NAME)" \
		-G $(BUILD_TYPE) \
		$(APP_DIR)
else
	$$(CMAKE) -B$$(dir $$@) -DSCMODULE="$(1)$(2)$(3)$(4)" $(SIMCOM_SDK_MACRO) \
		-DKERNEL_DIR="$(KERNEL_DIR)" \
		$(patsubst %,-D%=ON,$(word 3,$(subst _, ,$(1)$(2)$(3)$(4)))) \
		-D$(1)$(2)$(3)$(4)=ON \
		-DGCC_ARM_TOOLCHAIN_DIR="$(GCC_ARM_TOOLCHAIN_DIR)" \
		-DCMAKE_TOOLCHAIN_FILE="$(APP_DIR)/config/ToolChain.cmake" \
		-DAPP_NAME="$(APP_NAME)" \
		-G $(BUILD_TYPE) \
		$(APP_DIR)
endif

$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).elf: $(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(MAKEFILENAME)
	$(BUILD) -C $$(dir $$<)

$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).bin: $(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).elf
	${OBJCPY} -O binary $$< $$@
	$(CRC_SET) $$@ $$(patsubst %/,%,$$(dir $$@))/$(APP_NAME)_crc.bin
ifeq (win32,$(PLATFORM))
	$(RM) $(RMARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@)
	$(MOVE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$@))/$(APP_NAME)_crc.bin) $$(notdir $$@)
else
	$(RM) $(RMARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@)
	$(MOVE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$@))/$(APP_NAME)_crc.bin) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@)
endif

$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(1)$(2)$(3)$(4).zip:$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).bin $(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin $(APP_DIR)/script/make_image.mak
	$(MAKE) -C $(APP_DIR)/script -f make_image.mak PLATFORM=$(PLATFORM) TOOL_DIR=$(TOOL_DIR) MODULE=$(1)$(2)$(3)$(4)  KERNEL_TARGET_DIR=$(KERNEL_DIR) APP_NAME=$(APP_NAME)

$(1)$(2)$(3)$(4):$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(1)$(2)$(3)$(4).zip
	@echo ---------------------------
	@echo ----- build success!! -----
	@echo ---------------------------

$(1)$(2)$(3)$(4)_app:
	$(MAKE) clean_$(1)$(2)$(3)$(4)_app
	$(MAKE) $(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).bin
	@echo ---------------------------
	@echo ----- build success!! -----
	@echo ---------------------------

$(1)$(2)$(3)$(4)_kernel:
	$(MAKE) clean_$(1)$(2)$(3)$(4)_kernel
	$(MAKE) $(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin
	@echo ---------------------------
	@echo ----- build success!! -----
	@echo ---------------------------

$(1)$(2)$(3)$(4)_force:
	$(MAKE) clean_$(1)$(2)$(3)$(4)_app
	$(MAKE) clean_$(1)$(2)$(3)$(4)_kernel
	$(MAKE) $(1)$(2)$(3)$(4)

$(1)$(2)$(3)$(4)_remake:
	$(MAKE) clean_$(1)$(2)$(3)$(4)
	$(MAKE) cleanAll
	$(MAKE) $(1)$(2)$(3)$(4)



clean_$(1)$(2)$(3)$(4):clean_$(1)$(2)$(3)$(4)_app
clean_$(1)$(2)$(3)$(4)_app:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4))

ifeq (NO,$(SIMCOM_SDK))
clean_$(1)$(2)$(3)$(4):clean_$(1)$(2)$(3)$(4)_kernel clean_$(1)$(2)$(3)$(4)_file
clean_$(1)$(2)$(3)$(4)_kernel:clean_$(1)$(2)$(3)$(4)_file
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(KERNEL_DIR)/$(1)$(2)$(3)$(4))

clean_file:clean_$(1)$(2)$(3)$(4)_file
clean_$(1)$(2)$(3)$(4)_file:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(KERNEL_DIR)/$(1)$(2)$(3)$(4))
	$(MAKE) -C $(TOOL_DIR) -f apiMapInit.mak clean API_MAP_FILE_NAME=$(API_MAP_FILE_NAME) SC_MODULE=$(1) SC_HD_CNF=$(2) SC_HD_OPT=$(3) SC_USR_OPT=$(4)

install_$(1)$(2)$(3)$(4):$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).bin $(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin
	$(BUILD) -C $(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/ install
	@echo ------------------------------------------
	@echo     install [$(1)$(2)$(3)$(4)] success!!
	@echo ------------------------------------------

uninstall_$(1)$(2)$(3)$(4):
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(1)$(2)$(3)$(4))

endif
endef

define user_option_pre_process
$(eval $(call user_option_process,$(1),$(2),$(3),))

$(foreach custom,$($(1)$(2)$(3)_USR_OPT_LIST),$(eval $(call user_option_process,$(1),$(2),$(3),_$(custom))))

$(eval $(call get_usr_opt_list,$(1)$(2)$(3)))
endef

define hard_option_process
ifeq (,$($(1)$(2)_HD_OPT_LIST))
$$(eval $$(call user_option_pre_process,$(1),$(2),,))
else
$$(foreach hard_option,$($(1)$(2)_HD_OPT_LIST),$$(eval $$(call user_option_pre_process,$(1),$(2),$$(subst _STANDARD,,_$$(hard_option)),)))
endif
endef

define hard_config_process
$(foreach hard_config,$($(1)_HD_CFG_LIST),$(eval $(call hard_option_process,$(1),_$(hard_config))))
endef

ifeq (NO,$(SIMCOM_SDK))
# info create
define get_usr_opt_list
ALL_USR_OPT_LIST += $($(1)_USR_OPT_LIST)
endef

define make_custom_info
$(1)_info:
	@echo -      $(1)
endef
# info create end

$(foreach modules,$(SCMODULE),$(eval $(call hard_config_process,$(modules))))

$(foreach custom,$(sort $(ALL_USR_OPT_LIST)),$(eval $(call make_custom_info,$(custom))))
else
$(foreach modules,$(SCMODULE),$(eval $(call user_option_process,$(modules),,,)))
endif




################################################
##   release simcom SDK for multiple modules
################################################

ifeq (NO,$(SIMCOM_SDK))
KERNEL_TARGETLIST := $(sort $(notdir $(wildcard $(KERNEL_DIR)/*)))
APP_TARGETLIST := $(sort $(notdir $(wildcard $(APP_DIR)/$(OUT)/*)))
VALID_TARGETLIST := $(filter $(APP_TARGETLIST),$(KERNEL_TARGETLIST))

define copy_app_object
ifneq (,$(findstring _QL,$(1)))
install_set:$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/ql_lib/lib/$(1)/libql_lib.a
$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/ql_lib/lib/$(1)/libql_lib.a: preinstall_set install_set_ql $(APP_DIR)/$(OUT)/$(1)/lib/libql_lib.a
	-$(MKDIR) $(MKDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/ql_lib/$(1)/inc/)
	-$(MKDIR) $(MKDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/ql_lib/$(1)/lib/)
	$(COPY) $(COPYARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/ql_lib/$(1)/inc) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/ql_lib/$(1)/inc/)
	$(COPY_FILE) $(COPYARG_FILE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(OUT)/$(1)/lib/libql_lib.a) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/ql_lib/$(1)/lib/libql_lib.a)
else
install_set:$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/sc_lib/$(1)/lib/libsc_lib.a
$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/sc_lib/$(1)/lib/libsc_lib.a: preinstall_set install_set_sc $(APP_DIR)/$(OUT)/$(1)/lib/libsc_lib.a
	-$(MKDIR) $(MKDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/sc_lib/${1}/inc)
	-$(MKDIR) $(MKDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/sc_lib/${1}/lib)
	$(COPY) $(COPYARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/sc_lib/$(1)/inc) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/sc_lib/$(1)/inc/)
	$(COPY_FILE) $(COPYARG_FILE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(OUT)/$(1)/lib/libsc_lib.a) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/sc_lib/$(1)/lib/libsc_lib.a)
endif
endef

.PHONE: install_set preinstall_set uninstall_set

uninstall_set:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET))

preinstall_set: uninstall_set
	@echo ------------------------------------------
	@echo KERNEL_TARGETLIST: $(KERNEL_TARGETLIST)
	@echo APP_TARGETLIST: $(APP_TARGETLIST)
	@echo target set list: $(VALID_TARGETLIST)
	@echo ------------------------------------------

install_set_sc:
	@echo ----copy sc source----
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/sc_demo/)
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/sc_lib/inc/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/sc_demo) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/sc_demo/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/sc_lib/inc) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/sc_lib/inc/)
	$(COPY_FILE) $(COPYARG_FILE) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/sc_application.c) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/)

install_set_ql:
	@echo ----copy ql source----
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/ql_demo/)
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/ql_lib/inc/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/ql_demo) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/ql_demo/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/ql_lib/inc) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/ql_lib/inc/)
	$(COPY_FILE) $(COPYARG_FILE) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/ql_application.c) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/)

install_set:
	@echo ----copy general source----
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/config/)
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/script/)
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/examples/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/config) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/config/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/script) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/script/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/examples) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/examples/)
	$(COPY_FILE) $(COPYARG_FILE) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/CMakeLists.txt) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/)
	$(COPY_FILE) $(COPYARG_FILE) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/app_build_doc.md) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/)
	$(COPY_FILE) $(COPYARG_FILE) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/makeDepend.mak) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/)
	$(COPY_FILE) $(COPYARG_FILE) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ROOT_DIR)/Makefile) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/)
	@echo ----copy kernel object----
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/kernel/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(KERNEL_DIR)) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/kernel/)
	@echo ----copy tools----
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/tools/linux/aboot/)
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/tools/win32/7z/)
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/tools/win32/aboot/)
	$(COPY_FILE) $(COPYARG_FILE) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(TOOL_DIR)/linux/*) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/tools/linux/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(TOOL_DIR)/linux/aboot) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/tools/linux/aboot/)
	$(COPY_FILE) $(COPYARG_FILE) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(TOOL_DIR)/win32/*) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/tools/win32/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(TOOL_DIR)/win32/7z) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/tools/win32/7z/)
	$(COPY) $(COPYARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(TOOL_DIR)/win32/aboot) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(RELEASE)/$(RELEASE_SET)/tools/win32/aboot/)
	@echo ------------------------------------------
	@echo          install set success!!
	@echo ------------------------------------------

$(foreach module,$(VALID_TARGETLIST),$(eval $(call copy_app_object,$(module))))
endif
