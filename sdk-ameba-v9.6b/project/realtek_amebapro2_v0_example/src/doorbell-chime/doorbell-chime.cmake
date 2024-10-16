### add .cmkae need if neeeded ###
if(BUILD_LIB)
	message(STATUS "build MMF libraries")
endif()
ADD_LIBRARY (skynet_lib STATIC IMPORTED )
SET_PROPERTY ( TARGET skynet_lib PROPERTY IMPORTED_LOCATION ${prj_root}/src/doorbell-chime/skynet_iot/GCC/libSkynetAPI_iot.a )

list(
    APPEND app_example_lib
    skynet_lib
)

### add flags ###
list(
	APPEND app_example_flags
)

### add header files ###
list (
    APPEND app_example_inc_path
    "${CMAKE_CURRENT_LIST_DIR}"
)

### add source file ###
## add the file under the folder
#[[
set(EXAMPLE_SOURCE_PATH)
file(GLOB EXAMPLE_SOURCE_PATH ${CMAKE_CURRENT_LIST_DIR}/*.c)
#DOORBELL-CHIME
list(
	APPEND app_example_sources
#${EXAMPLE_SOURCE_PATH}
)
]]

## add source file individulay
#DOORBELL-CHIME
list(
	APPEND app_example_sources
    
    app_example.c
    skynet_device.c
	module_p2p.c
	module_p2p_aac.c
	example_doorbell_chime.c
	#skynet_wakeup.c
    #doorbell_demo.c
)
list(TRANSFORM app_example_sources PREPEND ${CMAKE_CURRENT_LIST_DIR}/)


