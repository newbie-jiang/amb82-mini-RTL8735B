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
	"${sdk_root}/component/other/st7789v"
	"${sdk_root}/component/example"
)

list (

	APPEND app_example_sources
    ../../other/st7789v/lcd.c
	../../other/st7789v/lcd_init.c
)



### add source file ###
list(
	APPEND app_example_sources
	app_example.c
	example_st7789_lcd.c
)
list(TRANSFORM app_example_sources PREPEND ${CMAKE_CURRENT_LIST_DIR}/)
