# ArduinoSensorsWebClient
Client for https://github.com/jricardosimoes/SensorsServerAndFront

Usage
Libraries

BMP180
Install the library for the breakout board from
https://github.com/sparkfun/BMP180_Breakout_Arduino_Library
Follow the hookup instructions form 
https://learn.sparkfun.com/tutorials/bmp180-barometric-pressure-sensor-hookup-
Pay attention to the Vin that must be connected to the Arduino's 3.3V pin only. 

DS18B20
Install the following library from
https://github.com/milesburton/Arduino-Temperature-Control-Library/blob/master/DallasTemperature.h
Change the value of the digital pin where the sensor is hooked do Arduino
#define ONE_WIRE_BUS 2

HSM20G
This is a analog sensor. Change the pin configuration if desired.
int HSM20GPIN = A0;

Ethernet
The sketch was written for the official Ethernet board, with default library.
Enter a MAC address and IP address for your controller or left untouched.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
Next, fill in an available IP address on your network for manual configuration (no DHCP):
IPAddress ip(192, 168, 1, 150);

Enter in your Domain Name Server address or left the Google's one.
IPAddress myDns(8,8,8,8);

The next change must reflect the Name or IP of the remote server and the respective port. Change the following variables according your scenario.
char serverName[] = "192.168.1.2";
int serverPort = 3000;

The variable postingInterval holds the delay between updates, in milliseconds.
const unsigned long postingInterval = 300L * 1000L; 

Remote page is the page where the sketch post the data. 
char pageName[] = "/api/sensor";


The API key acts as a sort of password authenticating the client. Changes must be according the value define in the file server.js
char apiKey[] = "32gss64XZab";

Make this changes, compile and upload the software to the board. Open a serial console an see the debug values, Arduino's requests and server's responses.
