# Bad apple on esp32!!

The video is streamed frame by frame, using go. To esp32 (in my case esp32c6-devkitc-1), it connects to a wifi network and listents on port 10(TCP) and displays the video on a st7789 lcd screen <br>
If you want to change the default configuration options refear to bad-apple-esp32c6/main/definitions.h (yes i know i could use the configuration feature of the idf but i was lazy) <br>
The image is intentinally sideways bc my display is 320x240 <br>
<br>
### Requirements to replicate my exact setup:
- esp32c6-devkitc-1
- 2.8inch Touch LCD Shield Rev 2.1 (by WaveShare)
- esp idf version 5.4.2
- cables
