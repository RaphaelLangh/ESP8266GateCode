#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

// Parameters for gate credentials
int  address     = 0;
String pass_gate = "1a2b3c";
String id_gate   = "Gate_1234";
String readID    = "";
String readPass  = "";

// Parameters for ESP8266 wifi setup
const char *ssid      = "gate_project";
const char *password  = "12345";
String     returnOk[] = {"true"};  
const int  ledPin     = D7; // GPIO2
ESP8266WebServer server(80);

void setup() {
  EEPROM.begin(512);
  Serial.begin(9600); 
  delay(1000);

  pinMode(ledPin, OUTPUT);
  
  Serial.println();
  Serial.print("Configuring access point...");

  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
// Writing data to memory
  EEPROM.put(0, id_gate);
  EEPROM.put(50, pass_gate);
  getGatePass();
  Serial.println("gate id we just wrote is: "+getGateID());
  Serial.println("gate password we just wrote is: "+readPass);
  
//only open and close are used
  server.on( "/open", HTTP_GET, openGate); // Listen for command from andriod
  server.on( "/id", HTTP_POST, sendGateId);
  server.on( "/close", HTTP_GET, completeCloseGate);   
  server.begin();
  Serial.println("HTTP server started, send gate id ");
}

void sendGateId()
{
  String answer = getGateID();
  Serial.println(answer);   
  server.send( 200, "application/gson; charset=utf-8", answer);
}

String getGateID()
{
  //Read data from memory
  EEPROM.get(0, readID);
  return readID;
}

void getGatePass()
{
  //Read data from memory
  EEPROM.get(50, readPass);  
}

void openGate()
{
  if (server.hasArg("idGate") && server.hasArg("password"))
  {
    if(server.arg("idGate") == getGateID())
    {
    getGatePass();
    Serial.println("Get Gate Pass: " + readPass);    
    Serial.println("server.arg : " + server.arg("password"));  
    if (server.arg("password") == readPass) // Verify that the correct password is received 
    { 
      digitalWrite(ledPin, HIGH); // open gate
      server.send (200, "application/gson; charset=utf-8","true");
      Serial.println("Correct Password. Gate is now opening.");
    }
    else 
    {
      Serial.println("Incorrect password.");
      server.send(200, "application/gson; charset=utf-8", "false");
    }
    }
    else{
       Serial.println("wrong gate");
       server.send(200, "application/gson; charset=utf-8", "false");
      }
  }
  else
  {
   server.send(203, "application/gson; charset=utf-8", "false");
   Serial.println("problem transmission");
  }
}

void simpleCloseGate()
{
  digitalWrite(ledPin, LOW); // open gate  
  Serial.println("Gate is now closing.");
}

void completeCloseGate()
{
  getGatePass();
  if (server.hasArg("password") && server.hasArg("nPassword") && (readPass != server.arg("nPassword")))
  {   
    if (server.arg("password") == readPass) // Verify that the correct password is received 
    {
      simpleCloseGate(); 
      String newPassword = server.arg("nPassword");
      EEPROM.put(50, newPassword);
      getGatePass();
      Serial.println("gate password we just wrote is: "+readPass);
      server.send(200, "application/json; charset=utf-8", "true");
      Serial.println("You have successfully changed the password");
    }
    else 
    {
      Serial.println("You are not authorised to change the password.");
      server.send(200, "application/json; charset=utf-8", "false");
    }
  }
}

void loop()
{
  server.handleClient();
}
