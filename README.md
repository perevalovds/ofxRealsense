# ofxRealsense - openFrameworks addon for working with Intel Realsense 415,435,435i cameras

Author: Denis Perevalov



## Credits

During development, I used ideas and links from analogous addons: 
* https://github.com/hiroMTB/ofxRealsense2 
* https://github.com/tecartlab/ofxRSSDK 
* http://www.ofxaddons.com/contributors/tyhenry 
but designed addon by my own taste.


## Supported OS, oF

Windows 10, oF 0.10.1, Visual Studio C++ 2017, 64/32 bit

By default, addon is configured to 64 bit.

For switching to 32 bit, comment/uncomment "vs" part of addon_config.mk for enabling 32 bit, and copy to libs/Win32 folder lib and dlls from libs-32-copy-if-need.
