##################################################################################
#                                                                                #
#                        EXAMPLE_WIFI_ROAMING_PLUS                               #
#                                                                                #
##################################################################################

Table of Contents
~~~~~~~~~~~~~~~~~
 - Description
 - Setup Guide
 - Parameter Setting and Configuration
 - Result description 
 - Supported list


Description
~~~~~~~~~~~
    This example realizes wifi roaming among local area network with the same SSID. 
    The roaming strategy is prescan before roaming and store the connected channel 
        to flash for prescan.

Setup Guide
~~~~~~~~~~~
	1.Comment out the following line of code in wlan_initialize()
				wifi_fast_connect_enable(1);
	2.GCC:use CMD "make all EXAMPLE=wifi_roaming_plus" to compile wifi_roaming_plus example.


Parameter Setting and Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    1. Set the RSSI_SCAN_THRESHOLD and RSSI_ROAMING_THRESHOLD and FIND_BETTER_RSSI_DELTA according to the WLAN envrionment.



Result description
~~~~~~~~~~~~~~~~~~
    The device can connect to a better RSSI AP and store the connection message to flash.


Supported List
~~~~~~~~~~~~~~
[Supported List]
        Supported IC:
            RTL8735B