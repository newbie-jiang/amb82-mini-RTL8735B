# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# compile C with /usr/bin/arm-none-eabi-gcc
C_DEFINES = -DARM_MATH_ARMV8MML -DCONFIG_BUILD_BOOT=1 -DCONFIG_BUILD_LIB=1 -DCONFIG_BUILD_RAM=1 -DCONFIG_PLATFORM_8735B -DCONFIG_RTL8735B_PLATFORM=1 -D__ARMVFP__ -D__ARM_ARCH_7EM__=0 -D__ARM_ARCH_7M__=0 -D__ARM_ARCH_8M_BASE__=0 -D__ARM_ARCH_8M_MAIN__=1 -D__ARM_FEATURE_FP16_SCALAR_ARITHMETIC=1 -D__DSP_PRESENT=1 -D__FPU_PRESENT -D__thumb2__

C_INCLUDES = -I//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/inc -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/mbed/hal -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/mbed/hal_ext -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/mbed/targets/hal/rtl8735b -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/mbed/api -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/stdlib -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/at_cmd -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/network -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/network/cJSON -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/cmsis/cmsis-core/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/cmsis/rtl8735b/lib/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/cmsis/rtl8735b/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/source/ram_ns/halmac/halmac_88xx -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/host/storage/inc/quirks -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/device/class/ethernet/inc -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/source/ram_ns/halmac/halmac_88xx/halmac_8822b -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/device/class/ethernet/src -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/device/core/inc -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/source/ram_ns/halmac/halmac_88xx/halmac_8735b -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/source/ram_ns/halmac/halmac_88xx_v1 -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/device/class/vendor/inc -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/host/storage/inc -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/host/storage/inc/scatterlist -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/source/ram_ns/halmac/halmac_88xx/halmac_8821c -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/host/vendor_spec -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/source/ram_ns/halmac -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/source/ram_ns/halmac/halmac_88xx_v1/halmac_8814b -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/host/storage/inc/scsi -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/inc -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/source/ram_ns/halmac/halmac_88xx/halmac_8195b -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/usb_otg/device -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/misc/utilities/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/app/stdio_port -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/app/xmodem/rom -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/app/shell -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/app/shell/rom_ns -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/app/rtl_printf/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/os/os_dep/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/os/freertos -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/os/freertos/freertos_v202012.00/Source/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/driver/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/driver/src/osdep -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/driver/src/phl -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/driver/src/hal -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/driver/src/hal/halmac -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/driver/src/hci -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/driver/src/hal/phydm/rtl8735b -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/driver/src/hal/phydm -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/wpa_supplicant/wpa_supplicant -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/os/freertos/freertos_posix/lib/include/FreeRTOS_POSIX -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/os/freertos/freertos_posix/lib/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/os/freertos/freertos_posix/lib/FreeRTOS-Plus-POSIX/include/portable/realtek/rtl8735b -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/os/freertos/freertos_posix/lib/FreeRTOS-Plus-POSIX/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/os/freertos/freertos_posix/lib/include/private -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/lwip/api -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/lwip/lwip_v2.1.2/src/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/lwip/lwip_v2.1.2/src/include/lwip -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/lwip/lwip_v2.1.2/src/include/compat/posix -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/lwip/lwip_v2.1.2/port/realtek -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/lwip/lwip_v2.1.2/port/realtek/freertos -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/ssl/mbedtls-2.28.1/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/ssl/ssl_ram_map/rom -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/usb/usb_class/device/class -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/usb/usb_class/device -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/usb/usb_class/host/uvc/inc -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/video/driver/common -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/video/driver/RTL8735B -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/media/rtp_codec -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/media/samples -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/media/mmfv2 -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/api -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/wifi_config -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/wifi/wifi_fast_connect -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/sdio/sd_host/inc -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/file_system/fatfs -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/file_system/fatfs/r0.14 -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/file_system/ftl_common -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/file_system/vfs -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/file_system/littlefs -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/file_system/littlefs/r2.41 -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/audio/3rdparty/faac/libfaac -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/audio/3rdparty/faac/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/audio/3rdparty/haac -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/media/muxer -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/media/3rdparty/fmp4/libmov/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/media/3rdparty/fmp4/libflv/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/cmsis/cmsis-dsp/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/application/qr_code_scanner/inc -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/audio/3rdparty/speex/speex -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/audio/3rdparty/AEC/AEC -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/audio/3rdparty/opus-1.3.1/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/audio/3rdparty/libopusenc-0.2.1/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/audio/3rdparty/fdk-aac-2.0.2/libAACenc/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/audio/3rdparty/fdk-aac-2.0.2/libAACdec/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/audio/3rdparty/fdk-aac-2.0.2/libSYS/include -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/video -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/fwlib/rtl8735b/lib/source/ram/video/semihost -I/home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/component/soc/8735b/cmsis/voe/rom -I//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/bootloader/../../../../component/os/freertos/freertos_v202012.00/Source/portable/GCC/ARM_CM33_NTZ/non_secure -I//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/bootloader/../../../../component/soc/8735b/fwlib/rtl8735b/lib/source/ram/video -I//home/hdj/ameba-mini/sdk-ameba-v9.6b_20231123/project/realtek_amebapro2_v0_example/GCC-RELEASE/bootloader/../../../../component/soc/8735b/fwlib/rtl8735b/lib/source/ram/video/enc/inc

C_FLAGS = -march=armv8-m.main+dsp -mthumb -mcmse -mfpu=fpv5-sp-d16 -mfp16-format=ieee -mfloat-abi=softfp -fno-common -fsigned-char -Os -fstack-usage -fdata-sections -ffunction-sections  -fno-optimize-sibling-calls -g -gdwarf-3 -MMD -nostartfiles -nodefaultlibs -nostdlib  -std=gnu99 -fdiagnostics-color=always -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Wno-write-strings -Wno-maybe-uninitialized
