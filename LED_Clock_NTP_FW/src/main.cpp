#include <Arduino.h>

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "gemueNetz";
const char *password = "5285914985753472";

const long utcOffsetInSeconds = 3600;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

#include <Wire.h>

#include <avr/pgmspace.h>

#define Addr_GND_GND 0xC0

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
  for (int i = 1; i < 73; i++)
  {
    IS_IIC_WriteByte(Addr_GND_GND, i, 0); // PWM
  }

  IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);
  IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x01); // page 1
  for (int i = 1; i < 73; i++)
  {
    IS_IIC_WriteByte(Addr_GND_GND, i, 0xff); // scaling
  }

  IS_IIC_WriteByte(Addr_GND_GND, 0x52, 0x70);
  IS_IIC_WriteByte(Addr_GND_GND, 0x51, 0xFF); // GCC
  IS_IIC_WriteByte(Addr_GND_GND, 0x50, 0x01); //
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  pinMode(D3, OUTPUT);
  digitalWrite(D3, HIGH);

  Wire.begin();
  Wire.setClock(400000); // I2C 400kHz

  timeClient.begin();
}

int last_second = 0;
int last_minute = 0;
int last_hour = 0;

void loop()
{
  // init led driver
  Init3746A();

  IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);
  IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0
  for (int i = 0; i < 12; i++)
  {
    IS_IIC_WriteByte(Addr_GND_GND, hours[i], 0x10); // pwm
    delay(100);
    IS_IIC_WriteByte(Addr_GND_GND, hours[i], 0x00); // pwm
  }
  for (int i = 0; i < 60; i++)
  {
    IS_IIC_WriteByte(Addr_GND_GND, minutes[i], 0x10); // pwm
    delay(100);
    IS_IIC_WriteByte(Addr_GND_GND, minutes[i], 0x00); // pwm
  }

  while (1)
  {
    timeClient.update();

    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(", ");
    Serial.print(timeClient.getHours());
    Serial.print(":");
    Serial.print(timeClient.getMinutes());
    Serial.print(":");
    Serial.println(timeClient.getSeconds());
    // Serial.println(timeClient.getFormattedTime());

    int hour = timeClient.getHours();
    if (hour > 12)
    {
      hour = hour - 12;
    }
    if (hour == 0)
    {
      hour = 12;
    }
    int minute = timeClient.getMinutes();
    if (minute == 0)
    {
      minute = 60;
    }
    int second = timeClient.getSeconds();
    if (second == 0)
    {
      second = 60;
    }

    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);
    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0

    // only update changed leds
    if (second != last_second)
    {
      IS_IIC_WriteByte(Addr_GND_GND, minutes[last_second - 1], 0x00); // PWM
    }
    if (minute != last_minute)
    {
      IS_IIC_WriteByte(Addr_GND_GND, minutes[last_minute - 1], 0x00); // PWM
    }
    if (hour != last_hour)
    {
      IS_IIC_WriteByte(Addr_GND_GND, hours[last_hour - 1], 0x00); // PWM
    }

    IS_IIC_WriteByte(Addr_GND_GND, minutes[second - 1], 0x08); // PWM
    IS_IIC_WriteByte(Addr_GND_GND, minutes[minute - 1], 0x10); // PWM
    IS_IIC_WriteByte(Addr_GND_GND, hours[hour - 1], 0x20);     // PWM

    /*
    // reset leds
    for (int i = 1; i < 73; i++)
    {
      IS_IIC_WriteByte(Addr_GND_GND, i, 0x00); // PWM
    }

    // set time
    IS_IIC_WriteByte(Addr_GND_GND, 0xfe, 0xc5);
    IS_IIC_WriteByte(Addr_GND_GND, 0xfd, 0x00); // page 0

    IS_IIC_WriteByte(Addr_GND_GND, minutes[second-1], 0x10); // PWM
    IS_IIC_WriteByte(Addr_GND_GND, minutes[minute-1], 0x20); // PWM
    IS_IIC_WriteByte(Addr_GND_GND, hours[hour-1], 0x25); // PWM
    */

    last_second = second;
    last_minute = minute;
    last_hour = hour;

    delay(100);
  }
}