This example shows how to use dfu to update OTA FW.

The instructions are as follows

1.It need to install the libusb.
2.After installing the libusb, the device manager will show the usb_dfu_class
3.dfu-util.exe -d 1d6b:0202 -a 0 -D ota.bin

Note:The detail can reference the dfu_tool.zip
[Supported List]
	Supported :
	    AmebaPro2
