# Amazon KVS Producer demo on AmebaPro2 #

## Download the necessary source code from Github
- Go to `project/realtek_amebapro2_v0_example/src/amazon_kvs/lib_amazon`
    ```
    cd project/realtek_amebapro2_v0_example/src/amazon_kvs/lib_amazon
    ```
- Clone the following repository for KVS producer
	- amazon-kinesis-video-streams-producer-embedded-c
    ```
    git clone -b v1.0.1 --recursive https://github.com/aws-samples/amazon-kinesis-video-streams-producer-embedded-c.git producer
    ```

## Set mbedtls version
- In KVS producer project, we use some function in mbedtls-2.16.6, same as KVS webrtc  
- Set mbedtls version to 2.16.6 in `project/realtek_amebapro2_v0_example/GCC-RELEASE/config.cmake`
    ```
    set(mbedtls "mbedtls-2.16.6")
    ```

## Enlarge SKB buffer number
- go to `component/wifi/driver/src/core/option/rtw_opt_skbuf_rtl8735b.c`  
    ```
    #define MAX_SKB_BUF_NUM      1024
    ```

## No using the wrapper function for snprintf 
- In `project/realtek_amebapro2_v0_example/GCC-RELEASE/toolchain.cmake`, comment the following wrapper function
    ```
    # "-Wl,-wrap,sprintf"
    # "-Wl,-wrap,snprintf"
    # "-Wl,-wrap,vsnprintf"
    ```

## Congiure the example
- configure AWS key, channel name and AWS region in `component/example/kvs_producer_mmf/sample_config.h`
    ```
    /* KVS general configuration */
    #define AWS_ACCESS_KEY                  "xxxxxxxxxx"
    #define AWS_SECRET_KEY                  "xxxxxxxxxx"

    /* KVS stream configuration */
    #define KVS_STREAM_NAME                 "xxxxxxxxxx"
    #define AWS_KVS_REGION                  "us-east-1"
    ```
- configure video parameter in `component/example/kvs_producer_mmf/example_kvs_producer_mmf.c`
    ```
    ...
    #define V1_RESOLUTION VIDEO_HD
    #define V1_FPS 30
    #define V1_GOP 30
    #define V1_BPS 1024*1024
    ```
    
## Select camera sensor

- Check your camera sensor model, and define it in <AmebaPro2_SDK>/project/realtek_amebapro2_v0_example/inc/sensor.h
    ```
    #define USE_SENSOR SENSOR_GC2053
    ```

## Build the project
- run following commands to build the image with option `-DEXAMPLE=kvs_producer_mmf`
    ```
    cd project/realtek_amebapro2_v0_example/GCC-RELEASE
    mkdir build
    cd build
    cmake .. -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -DEXAMPLE=kvs_producer_mmf
    cmake --build . --target flash
    ```

- use image tool to download the image to AmebaPro2 and reboot

- configure WiFi Connection  
    While runnung the example, you may need to configure WiFi connection by using these commands in uart terminal.  
    ```
    ATW0=<WiFi_SSID> : Set the WiFi AP to be connected
    ATW1=<WiFi_Password> : Set the WiFi AP password
    ATWC : Initiate the connection
    ```

- if everything works fine, you should see the following log
    ```
    ...
    Interface 0 IP address : xxx.xxx.xxx.xxx
    WIFI initialized
    ...
    [H264] init encoder
    [ISP] init ISP
    ...
    PUT MEDIA endpoint: s-xxxxxxxx.kinesisvideo.us-east-1.amazonaws.com
    Try to put media
    Info: 100-continue
    Info: Fragment buffering, timecode:1620367399995
    Info: Fragment received, timecode:1620367399995
    Info: Fragment buffering, timecode:1620367401795
    Info: Fragment persisted, timecode:1620367399995
    Info: Fragment received, timecode:1620367401795
    Info: Fragment buffering, timecode:1620367403595
    ...
    ```

## Validate result
- we can use KVS Test Page to test the result  
https://aws-samples.github.io/amazon-kinesis-video-streams-media-viewer/  
