# Music_Synthesizer
A combination of 4 types of waves to make the music that your heart desires. 

The first file I'm uploading is called sine_wave.ini . In there you'll find the code that I used for creating the sine wave output from the ESP32. This is to complete the first week of the "Locked in" sidequest. Next week I'll have all the code with the other three types of waves (square, triangle, sawtooth). I'll add a handmade osciloscope there and a speaker and some more Op-Amp IC's and so on. It's going to be fun ig.

Here's what you'll need for the sine wave output code to work and how to connect the stuff.
- 1 breadboard;
- 1 ESP32;
- 1 10uF capacitor, that is connected to D25 on the ESP32 with it's positive leg and to pin 3 on the Op-Amp IC with it's negative leg;
- 1 NE5532 Op-Amp IC, that has pins 1 and 2 bridged and pin 3 connected to the capacitor's negative leg. Also put a wire on pin 1 to read the sigan using an osciloscope probe connected to it.;
- 2 9V batteries;
- 2 cables for 9V batteries, that are junctioned (positive end of one cable is connected to the negative end of another cable, essentially making a common ground between the batteries which we'll need for the cirquit to oscillate) We connect the positive side of the junctioned cable to pin 4 and the negative side to pin 8. To connect the osciloscope we put ground of the ESP32 and the common ground between the batteries on the same ground rail on the breadboard and connect the ground cable of the probe there.;

That's it for now. See you in a bit with the other stuff for the project.
