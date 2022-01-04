#include <Arduino.h>

/*
MxFxM
LED Clock Firmware

Running on a Teensy 4.1 to control the LED clock.

Max-Felix Mueller - 2021
*/

/*
Connections:

With the pin header on the right hand side, looking from the front, from top to bottom.
Clock    Teensy
+5V      +5V
+3V3     +3V3
SDA      18
SCL      19
SDB      17
GND      GND
*/

#include <Wire.h>

#include <avr/pgmspace.h>

#define Addr_GND_GND 0xC0

void setup()
{
  pinMode(13, OUTPUT); // ARDUINO BOARD LED control

  pinMode(17, OUTPUT);
  digitalWriteFast(17, HIGH);

  Wire.begin();
  Wire.setClock(400000); // I2C 400kHz
}

uint8_t second = 1;
uint8_t minute = 1;
uint8_t hour = 1;

uint8_t minutes[] = {2, 20, 38, 56,
                     3, 21, 39, 57,
                     4, 22, 40, 58,
                     5, 23, 41, 59,
                     6, 24, 42, 60,
                     7, 25, 43, 61,
                     8, 26, 44, 62,
                     9, 27, 45, 63,
                     10, 28, 46, 64,
                     11, 29, 47, 65,
                     12, 30, 48, 66,
                     13, 31, 49, 67,
                     14, 32, 50, 68,
                     15, 33, 51, 69,
                     1, 19, 37, 55};

uint8_t hours[] = {35, 53, 71, 18, 36, 54, 72, 16, 34, 52, 70, 17};

byte PWM_Gamma64[64] =
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0b, 0x0d, 0x0f, 0x11, 0x13, 0x16,
        0x1a, 0x1c, 0x1d, 0x1f, 0x22, 0x25, 0x28, 0x2e,
        0x34, 0x38, 0x3c, 0x40, 0x44, 0x48, 0x4b, 0x4f,
        0x55, 0x5a, 0x5f, 0x64, 0x69, 0x6d, 0x72, 0x77,
        0x7d, 0x80, 0x88, 0x8d, 0x94, 0x9a, 0xa0, 0xa7,
        0xac, 0xb0, 0xb9, 0xbf, 0xc6, 0xcb, 0xcf, 0xd6,
        0xe1, 0xe9, 0xed, 0xf1, 0xf6, 0xfa, 0xfe, 0xff};

void IS_IIC_WriteByte(uint8_t Dev_Add, uint8_t Reg_Add, uint8_t Reg_Dat) // writing an LED register
{
  Wire.beginTransmission(Dev_Add / 2);
  Wire.write(Reg_Add);    // sends regaddress
  Wire.write(Reg_Dat);    // sends regaddress
  Wire.endTransmission(); // stop transmitting
}

void Init3746A(void)
{
  IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);
  IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0
  for (int i = 0; i < 0x48; i++)
  {
    IS_IIC_WriteByte(Addr_GND_GND, i, 0); // PWM
  }

  IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);
  IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x01); // page 1
  for (int i = 1; i < 0x48; i++)
  {
    IS_IIC_WriteByte(Addr_GND_GND, i, 0xff); // scaling
  }

  IS_IIC_WriteByte(Addr_GND_GND, 0x52, 0x70);
  IS_IIC_WriteByte(Addr_GND_GND, 0x51, 0xFF); // GCC
  IS_IIC_WriteByte(Addr_GND_GND, 0x50, 0x01); //
}

void loop() //
{
  // init led driver
  Init3746A();

  while (1)
  {
    // update time
    second = second + 1;
    if (second == 61)
    {
      second = 1;
      minute = minute + 1;
    }

    if (minute == 61)
    {
      minute = 1;
      hour = hour+1;
    }

    if (hour == 13)
    {
      hour = 1;
    }

    // reset leds
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);
    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0

    for (int i = 1; i < 73; i++) {
      IS_IIC_WriteByte(Addr_GND_GND, i, 0x00); // PWM
    }

    // set time
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);
    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0

    IS_IIC_WriteByte(Addr_GND_GND, minutes[second-1], 0x10); // PWM
    IS_IIC_WriteByte(Addr_GND_GND, minutes[minute-1], 0x20); // PWM
    IS_IIC_WriteByte(Addr_GND_GND, hours[hour-1], 0x25); // PWM

    // short pause
    //delay(1);

    /*
    for (i = 0; i < 72; i++)
    {
      if (i == 0)
      {
        IS_IIC_WriteByte(Addr_GND_GND, 71, 0x00); // PWM
      }
      else
      {
        IS_IIC_WriteByte(Addr_GND_GND, i - 1, 0x00); // PWM
      }
      IS_IIC_WriteByte(Addr_GND_GND, i, 0x10); // PWM
      delay(10);
    }
    delay(100);
    */
  }

  /*

  digitalWrite(13, LOW); // turn the ARDUINO BOARD LED on (HIGH is the voltage level)

  while (1)
  {
    // BLUE

    digitalWrite(13, HIGH); // turn the ARDUINO BOARD LED on (HIGH is the voltage level)
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);
    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0

    for (j = 0; j < 64; j++)
    {
      for (i = 1; i < 0x48; i = i + 3)
      {
        IS_IIC_WriteByte(Addr_GND_GND, i, PWM_Gamma64[j]); // PWM}
      }
    }
    delay(500);

    digitalWrite(13, LOW); // turn the ARDUINO BOARD LED OFF (HIGH is the voltage level)
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);
    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0

    for (j = 63; j >= 0; j--)
    {
      for (i = 1; i < 0x48; i = i + 3)

      {

        IS_IIC_WriteByte(Addr_GND_GND, i, PWM_Gamma64[j]); // PWM}
      }
    }

    delay(500);

    // GREEN
    digitalWrite(13, HIGH); // turn the ARDUINO BOARD LED on (HIGH is the voltage level)
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);

    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0
    for (j = 0; j < 64; j++)
    {

      for (i = 2; i < 0x48; i = i + 3)
      {
        IS_IIC_WriteByte(Addr_GND_GND, i, PWM_Gamma64[j]); // PWM}
      }
    }
    delay(500);

    digitalWrite(13, LOW); // turn the ARDUINO BOARD LED OFF (HIGH is the voltage level)
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);

    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0
    for (j = 63; j >= 0; j--)
    {

      for (i = 2; i < 0x48; i = i + 3)

      {
        IS_IIC_WriteByte(Addr_GND_GND, i, PWM_Gamma64[j]); // PWM}
      }
    }
    delay(500);

    // RED
    digitalWrite(13, HIGH); // turn the ARDUINO BOARD LED on (HIGH is the voltage level)
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);

    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0
    for (j = 0; j < 64; j++)
    {

      for (i = 3; i < 0x49; i = i + 3)
      {
        IS_IIC_WriteByte(Addr_GND_GND, i, PWM_Gamma64[j]); // PWM}
      }
    }
    delay(500);

    digitalWrite(13, LOW); // turn the ARDUINO BOARD LED OFF (HIGH is the voltage level)
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);

    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0
    for (j = 63; j >= 0; j--)
    {

      for (i = 3; i < 0x49; i = i + 3)
      {
        IS_IIC_WriteByte(Addr_GND_GND, i, PWM_Gamma64[j]); // PWM}
      }
    }
    delay(500);

    // WHITE
    digitalWrite(13, HIGH); // turn the ARDUINO BOARD LED on (HIGH is the voltage level)
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);

    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0
    for (j = 0; j < 64; j++)
    {
      for (i = 1; i < 0x49; i++)
      {
        IS_IIC_WriteByte(Addr_GND_GND, i, PWM_Gamma64[j]); // PWM}
      }
    }
    delay(500);

    digitalWrite(13, LOW); // turn the ARDUINO BOARD LED OFF (HIGH is the voltage level)
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);

    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0
    for (j = 63; j >= 0; j--)
    {

      for (i = 1; i < 0x49; i++)

      {
        IS_IIC_WriteByte(Addr_GND_GND, i, PWM_Gamma64[j]); // PWM}
      }
    }
    delay(500);
  }
  */
}