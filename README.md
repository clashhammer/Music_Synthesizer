# Music_Synthesizer
A project that studies the effect of sound and how waves produce it. Works like a piano and has a screen that outputs like a digital oscilloscope showing the wave that you hear and telling you it's frequency.

I got the idea from a girl I know that wants to be a singer and she said she wanted to buy a synthesizer to get a step closer to composing her own song. That's when I thought "how cool it would be to try and make one, maybe even give it to her if it turns out good.". And yeah, that's basically how I got an idea to build a musical synthesizer.

Project parts are in the file called bom.csv

To build a prototype follow the schematic and see where each wire is connected then upload the code to the ESP32 to use it. My own prototype uses just one custom PAM 8403 amplifier board and just one PCM5102 DAC board. I have color coded and connected all parts adjusting to the pinout and code in the schematic for easy understanding.

Also for the code you'll need to download some libraries from the ARDUINO IDE library picker for the code to propperly compile. These are:
- ESP_I2S (should be the second one in the list)
- Adafruit_GFX (again should be second one)
- Adafruit_ST7789 (first one this time)

Btw if you haven't used ESP32 before (like me), to connect it to your computer and upload the code you need to install a driver, corelating to what your ESP32 type is. There is a little chip under the big one on the board. To install the correct drivers you need to see what the letters on this chip say. If they say "SILABS CP210x" you need to install the CP210x driver. If it says "CH340" you need to install the CH340 driver on your computer. if it says nothing you need to install the CH340 driver.

Here are the links to both drivers' sites
- CP210x - https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=overview

- CH340 - https://www.wch-ic.com/downloads/CH341SER_EXE.html

Here are some photos of the finished part of the project like schematic, pcb and 3D display of it and youtube video of how it works.

<img width="1393" height="817" alt="image" src="https://github.com/user-attachments/assets/bfae3068-817c-429a-9da3-773daee73581" />
<img width="1513" height="448" alt="image" src="https://github.com/user-attachments/assets/94bef8fe-e3f2-41a2-88c5-fb4fb4076a7a" />
<img width="1323" height="397" alt="image" src="https://github.com/user-attachments/assets/36bfdf83-a32e-4d47-86ea-8766c584eca7" />

Youtube video (unlisted): https://youtube.com/shorts/R9iYHdPvPAw

I added an MIT Licence for free use of my project files.









