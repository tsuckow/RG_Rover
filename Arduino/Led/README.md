# LED Display and Driver

## The LED Display

I found the LED display from the NASA JPL rover interesting. It's an Adafruit 16x32 displays
(https://www.adafruit.com/product/420 or https://www.dfrobot.com/product-1643.html). So, I created
a little frame for my Sawppy;-)

## Controller

I'm using an Adafruit Metro M4 Express as dedicated controller. That allows the RasPi to send different
commands/strings to the display, but it does not need to do the low-level control. My initial
Arduino UNO was not powerful enough for this job.

This sketch uses the Adafruit libraries and the RasPi can send commands via the USB/Serial link.

## Protocol

All commands are ended with a CR. The Metro sends an 'OK' back, when the command is executed (some take some time)

- 'c' - Connection, second char 't' or 'f'
- 'e' - Eyes, second char 'l' - left, 'r' - right, 'c' - center
- 'f' - Face, second char 'h' - happy, or 'f' - frown

- 't' - Scroll Text, second char - number of repeats, rest of the line, the text to display
