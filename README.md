# Project:65 EEPROM Programmer

This is a simple Arduino-based EEPROM Programmer designed to handle the 8KB 28c65 and the 32KB 28c256 Electronically-Erasable Programmable ROMs. Originally built as a breadboard project, it uses a pair of MCP23017 port expander chips to provide the required number of GPIO pins.

The original version of the programmer used an SD card (and an Arduino Ethernet shield) to store the binary images. The current version instead downloads them over the USB/serial connection using ZModem.

For a run-down of the original (28c65-only) version, see:

* [Project:65 EEPROM Programmer, Part 1](https://coronax.wordpress.com/2012/11/25/project65-eeprom-programmer-part-1/)
* [Project:65 EEPROM Programmer, Part 2](https://coronax.wordpress.com/2012/12/02/project65-eeprom-programmer-part-2/)


[//]: # 

   [Project Blog]: <https://coronax.wordpress.com/2012/11/25/project65-eeprom-programmer-part-1/>
