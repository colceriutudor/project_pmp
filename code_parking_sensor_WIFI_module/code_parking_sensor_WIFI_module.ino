#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

/*
 * serial pins for the communication between the boards
 */
SoftwareSerial serialPins(D6, D5);

const char* ssid = "";
const char* password = "";

/*
 * server created by the wifi module
 */
WiFiServer server(80);

/*
 * client connected to the server
 */
WiFiClient client;

/*
 * information recieved from the uno board
 * via serial communication
 */
int recievedData;

/*
 * control variables
*/
bool isCorrectParkedClient;
bool reserved, isVehicleParked;
String reservedClientID;


/*
 *  assume there can only be one
 *  client connected to the network
 *  performing operations
*/
String connectedClientID;

/*
 * set the initial state for the control variables (false)
 */
void initializeVariables(){
  reserved = false;
  isVehicleParked = false;
  isCorrectParkedClient = true;
  reservedClientID = "";
}

/*
 * function that initializes the wifi server
 * with the appropriate credentials
 * and also prints the IP address for the users to connect to
 */
void startServer(){
  //begin a wifi connection using the provided credentials
  WiFi.begin(ssid, password);

  //set up the soft access point in order to connect
  //to the page (via provided IP)
  WiFi.softAP("Homikes", "sotapssidsecure");

  //wait for a connection to be established
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  //after the connection is made start the server
  server.begin();
  Serial.println("server on");

  //and print the IP for the user to connect to
  Serial.println(WiFi.localIP());
}

/*
 * main setup function for the components
 */
void setup() {
  initializeVariables();

  //init and set the baudrate for the serial monnitor
  Serial.begin(115200);
  delay(10);

  //init and set the baudrate for the sw serial pins
  serialPins.begin(9600);
  delay(10);

  //start the server
  startServer();
}

/*
 * function that gets the requests via user input from the server
 * and parses them accordingly
 */
void parseServerRequest(String request) {

  //found "RESERVED" in the request String
  if (request.indexOf("/RESERVED") != -1)  {

    //if the reserve button is pushed
    //then reserve the parking space
    reserved = true;

    //and mark the client that reserved it
    reservedClientID = getClientStatus();

    Serial.println(reservedClientID + " reserved the spot");
  }

  //found "PARK" in the request String
  if (request.indexOf("/PARK") != -1)  {

    //see what client wants to park
    connectedClientID = getClientStatus();

    Serial.println(connectedClientID + " wants to park");
    Serial.println(reservedClientID + " booked");

    //check if the parked client is the correct
    //one (the one that reserved the parking space)
    if (reservedClientID.equals(connectedClientID)) {

      //if a vehicle is parked, the parking space
      //is no longer reserved
      reserved = false;

      //if a person just parked set the
      //variable to true
      isVehicleParked = true;
    }
    //else notify the system
    else
      notifyParkingStatus();
  }
}

/*
 * function that changes the server accordingly (via HTML)
 * after client feedback
 */
void respond() {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.print("Parking space is ");

  //display the state of the parking space
  //for the user
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

/*
 * function that sends the parking space status (vehicle parked or not)
 * to the main board in order to be parsed accordingly
 * using the serial communication between the two boards
 */
void notifyParkingStatus() {
  serialPins.write((int)reserved);
  serialPins.write((int)isVehicleParked);
  serialPins.write((int)isCorrectParkedClient);
}

/*
 * functions that revieves the parking space status
 * from the main board via serial pins
 */
void readParkingStatus() {

  //if a vehicle is not parked anymore, update the infromation
  //stored in the control variables
  if (serialPins.available() > 0 && serialPins.read() == 0)
    isVehicleParked = false;
}

/*
 * function that returns the client address
 * computes the client MAC address and converts it to String
 * in order to be further used
 */
String getClientStatus() {
  String clientAddress = "";

  struct station_info *stat_info;
  struct ip4_addr *IPaddress;

  stat_info = wifi_softap_get_station_info();

  if (stat_info != NULL) {    
    clientAddress =
      String(stat_info->bssid[0]) +
      String(stat_info->bssid[1]) +
      String(stat_info->bssid[2]) +
      String(stat_info->bssid[3]) +
      String(stat_info->bssid[4]) +
      String(stat_info->bssid[5]);
  }

  return clientAddress;
}

/*
 * main loop for this module (ESP8266WiFi)
 */
void loop() {
  
  //get the client status (client found or not)
  client = server.available();
  if (!client) {
    return;
  }

  //and wait for someone to connect to the serever
  //after startup
  while (!client.available()) {
    delay(1);
  }

  //via the user input, generate server requests
  String request = client.readStringUntil('\r');
  Serial.println(request);

  //discard all the information written by the
  //client and not yet read
  client.flush();

  readParkingStatus();

  //after the request is made by the client
  //send it to be parsed
  parseServerRequest(request);

  //and respond to the request
  //displaying the updated information
  respond();

  //also signal to the main board using
  //the serial communication
  notifyParkingStatus();

  delay(1);
}
