#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
SoftwareSerial serialPins(D6, D5);

const char* ssid = "eSkyNet";
const char* password = "CccTcD-Link-eNet1";

WiFiServer server(80);
WiFiClient client;

int recievedData;

bool reserved, isVehicleParked;

void setup() {
  reserved = false;
  isVehicleParked = false;

  Serial.begin(115200);
  delay(10);

  serialPins.begin(9600);
  delay(10);

  WiFi.begin(ssid, password);
  WiFi.softAP("Homikes","sotapssidsecure");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected");

  server.begin();
  Serial.println("server on");
  Serial.println(WiFi.localIP());
}

void parseServerRequest(String request){
  int value = LOW;
  if (request.indexOf("/RESERVED") != -1)  {
    reserved = true;
  }
  if (request.indexOf("/PARK") != -1)  {
    isVehicleParked = true;
    reserved = false;
  }
}

void respond(){
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("Parking space is ");

  if (reserved == false) {
    client.print("NOT reserved");
  } else {
    client.print("reserved!!");
  }

  client.println("<br><br>");
  client.println("<a href=\"/PARK\"\"><button>Park </button></a>");
  client.println("<a href=\"/RESERVED\"\"><button>Reserve </button></a><br/>");
  client.println("</html>");
}

void notifyParkingStatus(){
  serialPins.write((int)reserved);
  serialPins.write((int)isVehicleParked);
}

void loop() {
  client = server.available();
  if (!client) {
    return;
  }
  
  while (!client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  parseServerRequest(request);

  respond();

  notifyParkingStatus();
  
  delay(1);
}
