#include <SFE_BMP180.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Ethernet.h>
#include <stdlib.h>

//Ethernet
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
// fill in an available IP address on your network here,
// for manual configuration:
IPAddress ip(192, 168, 1, 150);

// fill in your Domain Name Server address here:
IPAddress myDns(8,8,8,8);

// initialize the library instance:
EthernetClient client;

//Name or IP of the remote server
char serverName[] = "192.168.1.2";
//Port at remote server
int serverPort = 3000;



// last time you connected to the server, in milliseconds
unsigned long lastConnectionTime = 0;   
// delay between updates, in milliseconds
// the "L" is needed to use long type numbers
const unsigned long postingInterval = 300L * 1000L; 

//Remote page
char pageName[] = "/api/sensor";
int totalCount = 0;
// insure params is big enough to hold your variables
char params[64];
//API key
//Check server.js
char apiKey[] = "32gss64XZab";

//Value form HSM20G
int H;
//Values from BMP180
double T,P;

//Buffer form HTTP POST request
char outBuf[64];


//BMP180 - Pressure
SFE_BMP180 pressure;
// baseline pressure
double baseline; 
int timeOfLastPressureRead = 0;

//HSM-20G - Humidity
int HSM20GPIN = A0;

//Dallas DS18B20
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//Setup
void setup() {
  
  //Serial
  // Serial monitor baud rate
  Serial.begin(9600); 
  Serial.println("Setup");
  
  //Ethernet
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, myDns);
  Serial.print("IP address: ");
  Serial.println(Ethernet.localIP());
  
  //Setup BMP180
  // Initialize the sensor (it is important to get calibration values stored on the device).
  if (pressure.begin())
    Serial.println("BMP180 init success");

  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.
    Serial.println("BMP180 init fail (disconnected?)\n");
    while(1); // Pause forever.
  }
  // Get the baseline pressure:
  baseline = getPressure();
  Serial.print("baseline pressure: ");
  Serial.print(baseline);
  Serial.println(" mb"); 
  

  //Setup Dallas
  sensors.begin();

}

void loop() {
  
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }

}


// this method makes a HTTP connection to the server:
void httpRequest() {
  

  H = getHumidity();
  T = getTemperature();
  P = getPressure();
  sensors.requestTemperatures();
  
  Serial.println(" ");
  Serial.println("T1 \t H \t P \t T2");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.print("\t");
  Serial.print(H);
  Serial.print("\t");
  Serial.print((int)P);
  Serial.print("\t");
  Serial.println((int)T);
  
    
  
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();
  // if there's a successful connection:
  
  if (client.connect(serverName, serverPort)) {
    //buffer for temp string
    char strTempBuf[8];
    Serial.println("Request:");
    //From double to string
    dtostrf(sensors.getTempCByIndex(0),0,1,strTempBuf);
    //Send sensor values H, P and T as INT encoded in the POST paramenter data 
    //At server.js we decode the values properlly
    sprintf(params, "key=%s&data=1:%s;2:%i;3:%i;4:%i", apiKey, strTempBuf, (int)H, (int)P, (int)T);
    sprintf(outBuf,"POST %s HTTP/1.1",pageName);
    client.println(outBuf);
    sprintf(outBuf,"Host: %s",serverName);
    client.println(outBuf);
    client.println(F("User-Agent: Arduino"));
    client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
    sprintf(outBuf,"Content-Length: %u\r\n",strlen(params));
    client.println(outBuf);
    client.print(params);
    Serial.println(params);
    Serial.println("Response:");

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("Connection failed");
  }
  

  
  
}

/*********************************
 *int getHumidity()
 *HSM20G
 *Get Humidity
 *******************************/

int getHumidity(){
  // read humidity value from A0 pin
  int humValue = analogRead(HSM20GPIN); 
  // eqn
  // convert analog value to voltage
  int voltage = ((humValue*5.0)/1023.0); 
  //equation for humidity
  return (3.71*pow(voltage,3))-(20.65*pow(voltage,2))+(64.81*voltage)-27.44;
}


/*********************************
 *double getTemperature()
 *BMP180
 *Get Temperature
 *******************************/
double getTemperature()
{
  char status;
  double T,P,p0,a;
  // You must first get a temperature measurement to perform a pressure reading.
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      return(T);

    }
    else Serial.println("error retrieving temperature\n");
  }
  else Serial.println("error starting temperature\n");
}

/*********************************
 *double getPressure()
 *BMP180
 *Get Pressure
 *******************************/
double getPressure()
{
  char status;
  double T,P,p0,a;
  // You must first get a temperature measurement to perform a pressure reading.
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          return(P);
        }
        else Serial.println("error retrieving pressure\n");
      }
      else Serial.println("error starting pressure\n");
    }
    else Serial.println("error retrieving temperature\n");
  }
  else Serial.println("error starting temperature\n");
}


