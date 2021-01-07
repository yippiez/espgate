# Espgate

espgate allows you to use your esp8266 or esp32 microcontroller to create a wifi range extender 
(napt) with ad blocking (dns filtering) capabilities.

# How it works
first using NAPT feature of the esp lwip library an acces point gets created which allows any device 
connected to esp to acces internet then everey packet that is going through esp microcontroller gets 
checked if its a dns request and if its on the list the packet simply gets dropped.

# Installation
Download espgate.ino and iplookup.h from this repo install netfilter library from [espnetfilter](https://github.com/yippiez/esp-netfilter)
 and put them in the same directory as your espgate.ino. then you can use the Arduino IDE to compile and flash your code. 
