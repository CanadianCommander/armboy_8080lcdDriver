# ssd1289 interface lcd driver
This is a dynamic kernel module for the [ArmBoy kernel](https://github.com/CanadianCommander/armboy-kernel).
This driver provides an interface between the lcd and the system, exposing drawing functions to user applications.
## function vector map 
0. init            
1. initDefault
2. clearDisplayCall
3. drawBitmapCall
4. drawRectangleCall
5. setClipRegionCall
6. clearClipRegionCall

See ssd1289SysCall.c for function call argument information.
Note. One should use the interface provided by the [system api](https://github.com/CanadianCommander/armboy-api) instead of directly using this one.
