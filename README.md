# Project:65 EEPROM Programmer

This is a simple Arduino-based EEPROM Programmer designed to handle the 8KB 28c65 and the 32KB 28c256 Electronically-Erasable Programmable ROMs. Originally built as a breadboard project, it uses a pair of MCP23017 port expander chips to provide the required number of GPIO pins.

The original version of the programmer used an SD card (and an Arduino Ethernet shield) to store the binary images. The current version instead downloads them over the USB/serial connection using ZModem.

For a run-down of the original (28c65-only) version, see [here].
 

[//]: # 

	[here]: <https://coronax.wordpress.com/projects/
   
