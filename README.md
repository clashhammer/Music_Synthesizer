# Music_Synthesizer
A combination of 4 types of waves to make the music that your heart desires. 

The first file I'm uploading is called sine_wave.ini . In there you'll find the code that I used for creating the sine wave output from the ESP32. This is to complete the first week of the "Locked in" sidequest. Next week I'll have all the code with the other three types of waves (square, triangle, sawtooth). I'll add a handmade osciloscope there and a speaker and some more Op-Amp IC's and so on. It's going to be fun ig.

Here's what you'll need for the sine wave output code to work and how to connect the stuff.
- 1 breadboard;
- 1 ESP32;
- 1 10uF capacitor, that is connected to D25 on the ESP32 with it's positive leg and to pin 3 on the Op-Amp IC with it's negative leg;
- 1 NE5532 Op-Amp IC, that has pins 1 and 2 bridged and pin 3 connected to the capacitor's negative leg. Also put a wire on pin 1 to read the signal using an osciloscope probe connected to it.;
- 2 9V batteries;
- 2 cables for 9V batteries, that are junctioned (positive end of one cable is connected to the negative end of another cable, essentially making a common ground between the batteries which we'll need for the cirquit to oscillate) We connect the positive side of the junctioned cable to pin 8 and the negative side to pin 4. To connect the osciloscope we put ground of the ESP32 and the common ground between the batteries on the same ground rail on the breadboard and connect the ground cable of the probe there.;

Btw if you haven't used ESP32 before (like me), to connect it to your computer to upload the code you need to install a driver, corelating to what your ESP32 type is. There is a little chip under the big one on the board. To install the correct drivers you need to see what the letters on this chip say. If they say "SILABS CP210x" you need to install the CP210x driver. If it says "CH340" you need to install the CH340 driver on your computer. if it says nothing you need to install the CH340 driver.

Here are the links to both drivers' sites
- CP210x - https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=overview

- CH340 - https://www.wch-ic.com/downloads/CH341SER_EXE.html

Week 2: I made some code that outputs different kinds of waves. To test it yourself you need everything that you needed before plus a button connected on D33 on the ESP32.

See you soon!
