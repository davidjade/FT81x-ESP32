This is a re-working of the FT800-FT813 source code to increase SPI throughput on the ESP32 by using native SPI with DMA. It does this by both batching SPI transactions as efficiently as possible as well as also using a more generalized DMA strategy, which should enable this to be ported to other platforms more easily. 

These changes also add some additional features such as support for very large SPI memory transfers directly to the FT81x memory. This can be used to directly stream uncompressed bitmaps for faster dynamic updates than by going through the CMD buffer, such as treating the whole display (or just a region, depending on the DL) as a memory mapped bitmap display. To do this, you would set up a bitmap in the FT81x's memory and then use SPI to directly update the bitmap and the FT81x will show those changes in real-time with no need to refresh the DL. You can set up either the entire screen or multiple rectangular regions of the display for direct memory mapped access along with the appropriate display list that assigns those bitmaps to screen regions. 

While this is not demoed here, these changes are also the basis for another version of this code (a display driver) that can enable the FT81x to be used with the LittlevGL GUI library, which does update the display using these methods. However, due to using a larger 800x480 16-bit display with a serial data protocol, full screen refreshes do have to push over 6 mbits over SPI for a full refresh. While it runs close to the theoretical max speeds of the FT81x for direct memory transfers (stable at 30Mhz and successfully pushed to 32Mhz), that translates to only 4-5 FPS using 2-wire SPI mode. By using Quad SPI (only supported in the ESP32 LVGL driver) it should be 4x faster for ~20 frames per second. The ESP32 LVGL driver project, which has a newer and more advanced version of the DMA code for the FT813 is here: https://github.com/lvgl/lv_port_esp32 

Just keep in mind that physical SPI wiring is the largest determiner on stability so if it is unstable, try a slower SPI clock speed or shorten/improve the SPI wiring.

Also note that I left my platformio.ini and sdkconfig files in place for my particular ESP32 board configurations so others can see how I got it to run. Also note the the pins to use are defined in the source code.
  
Here are some stats on the performance improvements to this demo program vs. using the Arduino framework with an ESP32:

Original FT81x demo			- only stable for me at 16Mhz
- time1: 1586us
- time2: 94us
	
New FT81x-ESP32				- stable for me at 30Mhz
- time1: 152us
- time2: 27us