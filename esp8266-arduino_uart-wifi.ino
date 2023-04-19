#include <ArduinoWiFiServer.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>

#define LED 2
#define CODE1 0
#define CODE2 1
#define CODE3 2

/*ADD YOUR WiFi details BELOW*/
const char *ssid = "SECRET_SSID";
const char *password = "SECRET_PASS";

//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.1.106:1880/update-sensor";

WiFiClient client;
HTTPClient http;


/*
* Connect your controller to WiFi
*/
void connectToWiFi() 
{
//Connect to WiFi Network
  
   WiFi.begin(ssid, password);
   int retries = 0;
while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
   retries++;
    dLED(CODE3);
}
if (retries > 14) {
    dLED(CODE1);
}
if (WiFi.status() == WL_CONNECTED) {
    dLED(CODE2);
}
}

/* Debugging LED */

void dLED(int errCode)
{
    if(errCode == 0)  //Initialization Failed. Need to reset or check code.
    {
      digitalWrite(LED, LOW);
    }
    else if(errCode == 1) //Sucessfully connected to WiFi
    {
      digitalWrite(LED, LOW);
      delay(1000);
      digitalWrite(LED, HIGH);
      errCode = 9;
    }
    else if(errCode == 2) //Attempting wifi connection
    {
      digitalWrite(LED, LOW);
      delay(100);
      digitalWrite(LED, HIGH);
      errCode = 9;      
    }
}

void sendToServer(String gaugeData)
{

  if(WiFi.status()== WL_CONNECTED)
{
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
  
      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
      // Specify content-type header
      //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      //String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensor=BME280&value1=24.25&value2=49.54&value3=1005.14";           
      // Send HTTP POST request
      //int httpResponseCode = http.POST(httpRequestData);
      
      http.addHeader("Content-Type", "text/plain");
      int httpResponseCode = http.POST(gaugeData);
      
      
      // If you need an HTTP request with a content type: application/json, use the following:
      //http.addHeader("Content-Type", "application/json");
      //int httpResponseCode = http.POST("{\"api_key\":\"tPmAT5Ab3j7F9\",\"sensor\":\"BME280\",\"value1\":\"24.25\",\"value2\":\"49.54\",\"value3\":\"1005.14\"}");

      // If you need an HTTP request with a content type: text/plain
      //http.addHeader("Content-Type", "text/plain");
      //int httpResponseCode = http.POST("Hello, World!");
     
      //Serial.print("HTTP Response code: ");
      //Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      dLED(CODE1);
    }
  
}

void setup() {
  connectToWiFi();
  Serial.begin(115200);
  digitalWrite(LED, OUTPUT);
  digitalWrite(LED, HIGH);

}

void loop() {
  if(Serial.available() > 0)
  {
    char data = Serial.read();
    data +=  data;
    String gData = String(data);
    sendToServer(gData);
  }
    
}
