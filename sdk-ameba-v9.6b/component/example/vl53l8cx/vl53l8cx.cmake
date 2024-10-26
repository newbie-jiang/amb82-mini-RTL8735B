### add lib ###
list(
	APPEND app_example_lib
)

### add flags ###
list(
	APPEND app_example_flags
)

### add header files ###
list (

	APPEND app_example_inc_path
	"${sdk_root}/component/other/vl53l8cx"
	"${sdk_root}/component/example"

	

)

list (

	APPEND app_example_sources
    ../../other/vl53l8cx/vl53l8cx_api.c
    ../../other/vl53l8cx/vl53l8cx_plugin_detection_thresholds.c
    ../../other/vl53l8cx/vl53l8cx_plugin_motion_indicator.c
    ../../other/vl53l8cx/vl53l8cx_plugin_xtalk.c
    ../../other/vl53l8cx/platform.c
    ../../other/vl53l8cx/vl53l8cx.c
    ../../other/vl53l8cx/53l8a1_ranging_sensor.c
    ../../other/vl53l8cx/custom_bus.c
    ../../other/vl53l8cx/app_tof.c
    ../../other/vl53l8cx/app_tof_pin_conf.c

)



### add source file ###
list(
	APPEND app_example_sources
	app_example.c
	example_vl53l8cx.c
)
list(TRANSFORM app_example_sources PREPEND ${CMAKE_CURRENT_LIST_DIR}/)


