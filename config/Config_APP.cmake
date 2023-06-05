#设置私有编译宏
SET(SDK_ADD_PRIV_PREMACRO
    -D__TM_ZONE=tm_zone
    -D__TM_GMTOFF=tm_gmtoff
    -D_REENT_SMALL
)

# 设置连接脚本
if(DEFINED GWSD)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_GWSD.ld")
elseif(DEFINED WHXA)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_WHXA.ld")
elseif(DEFINED HTTX)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_HTTX.ld")
elseif(DEFINED JWZD)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_APP_512K.ld")
elseif(DEFINED WHXBY)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_APP_512K.ld")
elseif(DEFINED BL)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_APP_512K.ld")
elseif(DEFINED A7680C_LANS_XC)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_APP_512K.ld")
elseif(DEFINED A7680C_MANS_XC)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_APP_512K.ld")
elseif(DEFINED A7680C_LANV_XC)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_APP_384K.ld")
elseif(DEFINED ST)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_ST.ld")
elseif(DEFINED XYJ)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_XYJ.ld")
elseif(DEFINED A7680C_MANV)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_ASR5311_APP_140K.ld")
elseif(DEFINED ZS)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_ZS.ld")
elseif(DEFINED GZYM)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_GZYM.ld")
elseif(DEFINED QZZT)
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript_APP_512K.ld")
else()
    SET(LINK_SCRIP "${CMAKE_CURRENT_SOURCE_DIR}/config/app_linkscript.ld")
endif()

# 设置三方库
# SET(THIRD_LIB
#     "${CMAKE_CURRENT_SOURCE_DIR}/third_lib/*.a"
# )
