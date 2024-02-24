/* Project:65 EEPROM burner
   Copyright (c) 2012, Christopher Just
   All rights reserved.
   
   This is control software for an Arduino-based programmer for
   a 28c65 EEPROM chip.  For more information, circuit diagrams,
   etc., see:
     http://coronax.wordpress.com/2012/11/25/project65-eeprom-programmer-part-1/
     http://coronax.wordpress.com/2012/12/02/project65-eeprom-programmer-part-2/
         
 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions 
 are met:

     Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above 
     copyright notice, this list of conditions and the following 
     disclaimer in the documentation and/or other materials 
     provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
  Connections:  +5V and Ground, obviously.
  SCL (pin 12 on the MCP23017) goes to A5 (on Arduino UNO)
  SDA (pin 13 on the MCP23017) goes to A4 (on Arduino UNO)
*/

#include <Wire.h>
#include <XModem.h>
#include <stdlib.h>

XModem xmodem;

constexpr int addr1 = 0b00100000;  // 23017 #1
constexpr int addr2 = 0b00100001;  // 23017 #2
bool write_mode = true;

long int max_addr = 16384;

constexpr int EEPROM_CHIP_ENABLE   = 1;
constexpr int EEPROM_OUTPUT_ENABLE = 2;
constexpr int EEPROM_WRITE_ENABLE  = 4;

// CJ Bug - cut this buffer down from 80 bytes after making some additions.
// Apparently I am hard up against the RAM limits for the SD library to 
// work.
char buffer[60];    // buffer used for formatting output messages
int buffer_len = 60;


void set28c65Mode () 
{
  max_addr = 8192;

  // for addr1, port B is the lower 8 bits of the address bus
  // and port A is the upper 5-6 bits (because the wires are
  // less tangled that way).
  Wire.beginTransmission (addr1);
  Wire.write (0x00);  // Address of data direction register for Port A
  Wire.write (0x00);  // Port A: set all as outputs
  Wire.write (0x00);  // Port B: set all as outputs
  Wire.endTransmission ();

  // for addr2, port A is the control bus: chip enable,
  // output enable, write enable, and busy
  // Port B is the data bus.
  Wire.beginTransmission (addr2);
  Wire.write (0x00);  // Address of data direction register for Port A
  Wire.write (0x08);  // Port A: set busy as input, others as output
  Wire.write (0x00);  // Port B: set all as outputs
  Wire.endTransmission ();
  // the busy line needs a pullup resistor.  We can
  // turn on an internal pullup
  //Wire.beginTransmission (addr2);
  //Wire.write (0x0C);  // GPPUA pullup resistor enable port A
  //Wire.write (0x08);  // set busy as input, others as output
  //Wire.endTransmission ();

  setCommandLines (EEPROM_CHIP_ENABLE);
  Serial.println ("\r\nConfigured for 28c65 EEPROM.");
}



void set28c256Mode () 
{
  max_addr = 32768;

  // for addr1, port B is the lower 8 bits of the address bus
  // and port A is the upper 5-6 bits (because the wires are
  // less tangled that way).
  Wire.beginTransmission (addr1);
  Wire.write (0x00);  // Address of data direction register for Port A
  Wire.write (0x00);  // Port A: set all as outputs
  Wire.write (0x00);  // Port B: set all as outputs
  Wire.endTransmission ();

  // for addr2, port A is the control bus: chip enable,
  // output enable, write enable, and busy
  // Port B is the data bus.
  Wire.beginTransmission (addr2);
  Wire.write (0x00);  // Address of data direction register for Port A
  Wire.write (0x00);  // Port A: set all as outputs for 28c256
  Wire.write (0x00);  // Port B: set all as outputs
  Wire.endTransmission ();
  // the busy line needs a pullup resistor.  We can
  // turn on an internal pullup
  //Wire.beginTransmission (addr2);
  //Wire.write (0x0C);  // GPPUA pullup resistor enable port A
  //Wire.write (0x08);  // set busy as input, others as output
  //Wire.endTransmission ();

  setCommandLines (EEPROM_CHIP_ENABLE);

  Serial.println ("\r\nConfigured for 28c256 EEPROM.");
}



void setup () 
{
  Serial.begin (57600);
  Serial.setTimeout(30000);
  Serial.println ("Arduino EEPROM programmer");

  Wire.setClock(400000); // Supposedly fastest setting AT328 supports
  Wire.begin ();

  xmodem.begin(Serial);

  // Setup for SD card reader
  // pinMode (10,OUTPUT);  // required by SPI library
  //pinMode (4, OUTPUT);  // chip select for SD card reader
  //if (!SD.begin (4))
  //  Serial.println ("Couldn't start SD");

  for (;;)
  {
    Serial.println ("Choose EEPROM:");
    Serial.println (" 1. 28c65    8KB");
    Serial.println (" 2. 28c256  32KB");
    Serial.println ("> ");

    while (!Serial.available())
      ;    
    char c = Serial.read();
    Serial.print(c);
    if (c == '1')
    {
      set28c65Mode();
      break;
    }
    else if (c == '2')
    {
      set28c256Mode ();
      break;
    }
  }
}


void loop () 
{
  Serial.println("Select:");
  Serial.println("  1. Write Image");
  Serial.println("  2. Verify Image");
  Serial.println("  3. Test Write");
  Serial.println("  4. Test Read");
  Serial.print ("> ");

  while (!Serial.available())
    ;    
  char c = Serial.read();
  Serial.println(c);

  if (c == '1')
  {
    writeFile();
  }
  else if (c == '2')
  {
    verifyFile();
  }
  else if (c == '3')
  {
    testWrite();
  }
  else if (c == '4')
  {
    testRead();
  }
  else
  {
    Serial.print("Error");
    Serial.print("> ");
  }
}

#if 0
void loop () 
{
  char command_buffer[25];

  Serial.print ("> ");
  byte len;
  while ((len = Serial.readBytesUntil('\r', command_buffer,24)) == 0)
    ;
  command_buffer[len] = 0;
  Serial.print("'");
  Serial.print(command_buffer);
  Serial.print("'");
  if (command_buffer[len-1] == 13) // just in case we have a cr/lf
    command_buffer[len-1] = 0;

  //Serial.println (command_buffer);
  if (strcmp (command_buffer, "testread") == 0)
  {
    testRead();
  }
  else if (strcmp (command_buffer, "testwrite") == 0)
  {
    testWrite();    
  }
  else if (strcmp (command_buffer, "write") == 0)
  {
    writeFile ();
  }
  else if (strcmp (command_buffer, "verify") == 0)
  {
    verifyFile ();
  }
  else 
  {
    Serial.println ("Syntax Error.");
  }
}
#endif

/* Writes contents of the array value[] to the first few bytes of the EEPROM */
void testWrite() 
{
  Serial.println ("Writing 16 bytes to EEPROM:");
  byte value[] = { 0x01, 0x12, 0x23, 0x30, 0x40, 0x50, 0x61, 0x79, 0x88, 0x9b, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0x13 };

  //setWriteMode();
  for (int i = 0; i < 16; ++i) 
  {
    writeByte (i, value[i], true);
#if 0
    setAddressBus (i);
    setDataBus (value[i]);
    delay (2);
    // toggle WRITE_ENABLE to begin the write process
    setCommandLines (EEPROM_CHIP_ENABLE | EEPROM_WRITE_ENABLE);
    setCommandLines (EEPROM_CHIP_ENABLE);
    delay (10); // wait for write to finish - max 10 ms
#endif 
  }
  Serial.println ("Done.\n");
}


/* Reads and prints out hte first 16 bytes of the EEPROM */
void testRead()
{
  Serial.println ("Reading first 16 bytes of EEPROM:");
  // try to read the first 16 bytes of the eeprom
  setReadMode();
  setCommandLines (EEPROM_CHIP_ENABLE | EEPROM_OUTPUT_ENABLE);
  for (int i = 0; i < 16; ++i)
  {
    setAddressBus (i);
    byte b = getDataBus();
    Serial.println (b, HEX);
  }
  setCommandLines (EEPROM_CHIP_ENABLE);
  Serial.println ("Done.\n");
}


/* Returns the byte read from address addr */
byte readByte (int addr)
{
  // try to read the first 16 bytes of the eeprom
  setReadMode();
  setAddressBus (addr);
  setCommandLines (EEPROM_CHIP_ENABLE | EEPROM_OUTPUT_ENABLE);
  byte b = getDataBus();
  setCommandLines (EEPROM_CHIP_ENABLE);
  return b;
}



/* Writes val to the address addr.
 * The wait_for_write bool here was an attempt to support page mode.
 * However, it appears event with the i2c clock set at the highest
 * rate the 23017 datasheet supports (1.7MHz), we still end up with
 * 1.14 ms between WE pulses - far longer than the 150 us that is 
 * the maximum for page mode writes.
 */
void writeByte (int addr, byte val, bool wait_for_write)
{
  setWriteMode();
  setAddressBus (addr);
  setDataBus (val);
  setCommandLines (EEPROM_CHIP_ENABLE | EEPROM_WRITE_ENABLE);
  setCommandLines (EEPROM_CHIP_ENABLE);

  // wait for completion by reading from the bus and waiting
  // until the highest bit of the read value equals
  // the highest bit of val.
  if (wait_for_write)
  {
    byte b;
    do
    {
      b = readByte (addr);
    } while ((val & 0x80) != (b & 0x80));
  }
}



// verify or write data:
long int num_differences = 0;
long int address = 0;



bool WriteReceiveHandler(void *blk_id, size_t idSize, byte *data, size_t dataSize)
{
  for (int i = 0; i < dataSize; ++i)
  {
    //if (address %100 == 0)
    //  Serial.print (".");

    byte b1 = data[i];
    writeByte (address, b1, true);//(address % 64 == 63) || (i == dataSize - 1));
    ++address;
  }
  return true;
}


/* Write the contents of the specified file to the EEPROM, 
 * starting at address 0 and going until the end of the
 * file or max_addr, whichever comes first.
 */
void writeFile ()
{
  num_differences = 0;
  address = 0;
  xmodem.setRecieveBlockHandler(WriteReceiveHandler);
  Serial.println ("Writing: Begin XModem transfer.");
  Serial.flush();
  if (!xmodem.receive())
    Serial.println ("Error: XModem transfer failed.");

  setAddressBus(0);

  if (address != max_addr)
  {
    snprintf (buffer, buffer_len, "Error: Expected %ld bytes, received %ld.\n", max_addr, address);
    Serial.println(buffer);
  }
  else
  {
    snprintf (buffer, buffer_len, "Wrote %ld bytes; %ld max.\n", address, max_addr);
    Serial.println (buffer);
  }
}



bool VerifyReceiveHandler(void *blk_id, size_t idSize, byte *data, size_t dataSize)
{
  for (int i = 0; i < dataSize; ++i)
  {
    //if (address %100 == 0)
    //  Serial.print (".");

    byte b1 = data[i];
    byte b2 = readByte(address++);
    if (b1 != b2)
      ++ num_differences;
  }
  return true;
}

/* Verifies that the contents matches the downloaded file.
 */
void verifyFile ()
{
  num_differences = 0;
  address = 0;
  xmodem.setRecieveBlockHandler(VerifyReceiveHandler);
  Serial.println ("Verifying: Begin XModem transfer.");
  Serial.flush();
  if (!xmodem.receive())
    Serial.println ("Error: XModem transfer failed.");

  if (address != max_addr)
  {
    snprintf (buffer, buffer_len, "Error: Expected %d bytes, received %d.", max_addr, address);
    Serial.println(buffer);
  }

  setAddressBus(0);

  if (num_differences == 0)
  {
    Serial.println ("\nFile matches ROM.\n");
  } 
  else
  {
    snprintf (buffer, buffer_len, "\nFile differs in %ld places.\n", num_differences);
    Serial.println (buffer);
  }
}



/* Sets the value of the three command lines.
 * The argument is a bitwise or of any of the three EEPROM_*_ENABLE
 * defines.
 */
void setCommandLines (byte lines)
{
  Wire.beginTransmission (addr2);
  Wire.write (0x12);
  // Note that since the command lines are active low, we need to invert 
  // these three bits to get the right values.
  Wire.write (lines^0b111);
  Wire.endTransmission ();
}


void setAddressBus (int address)
{
  byte low_byte = address & 0x00FF;
  // in my v2.1 board, I accidentally have the low byte of the address connected to the
  // EEPROM the wrong way around, so we need to reverse order of bits here...
  // FIX THIS in v2.2!
  low_byte = ((low_byte & 0x000f) << 4) | ((low_byte & 0x00f0) >> 4);
  low_byte = ((low_byte & 0x0033) << 2) | ((low_byte & 0x00cc) >> 2);
  low_byte = ((low_byte & 0x0055) << 1) | ((low_byte & 0x00aa) >> 1);

  byte high_byte = address >> 8;
  Wire.beginTransmission (addr1);
  Wire.write (0x12);        // port A data address
  Wire.write (high_byte);
  Wire.write (low_byte);    // sequential write to port B
  Wire.endTransmission ();
}


/* Set data pins to outputs */
void setWriteMode ()
{
  if (write_mode == false)
  {
    Wire.beginTransmission (addr2);
    Wire.write (0x01); // IO direction register port A
    Wire.write (0x00); // set all as outputs
    Wire.endTransmission ();
    write_mode = true;
  }
}


/* Set data pins to inputs */
void setReadMode ()
{
  if (write_mode == true)
  {
    Wire.beginTransmission (addr2);
    Wire.write (0x01); // IO direction register port A
    Wire.write (0xFF); // set all as inputs
    Wire.endTransmission ();
    write_mode = false;
  }
}


void setDataBus (byte data)
{
  //setWriteMode ();  // set data pins to outputs
  Wire.beginTransmission (addr2);
  Wire.write (0x13);
  Wire.write (data);
  Wire.endTransmission ();
}


byte getDataBus ()
{
  //setReadMode();  // set data pins to inputs
  Wire.beginTransmission (addr2);
  Wire.write (0x13);
  Wire.endTransmission (); 
  Wire.requestFrom (addr2, 1);
  byte data = Wire.read();
  
  return data;
}



