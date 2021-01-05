/*
 * Code for the master board - Arduino UNO
*/

#include <SoftwareSerial.h>

/*
 * serial pins for the communication between the boards
*/
SoftwareSerial serialPins(5, 6);

/*
 * variables used for the board pins
*/
const int BUZZER_PIN = 8;
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

/*
 * control variables
*/
long duration;
int distance, timer_started;
int reservedParkingSpot_wifi, vehicleParked_wifi, correctClient_wifi;
uint32_t oldtime;
bool vehicleParked;

/*
 * set the pins for the distance sensor module
 */
void distanceSensorSetup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

/*
 * set the pins for the buzzer module
 */
void buzzerSetup() {
  pinMode (BUZZER_PIN, OUTPUT) ;
}

/*
 * initial state of the control variables
 */
void initVariables() {
  vehicleParked = false;
  vehicleParked_wifi = false;
  reservedParkingSpot_wifi = false;
}

/*
 * setup function for all the components
 * including the serial pins and serial monnitor
 */
void setup ()
{
  initVariables();

  distanceSensorSetup();

  buzzerSetup();

  serialPins.begin(9600);
  Serial.begin(9600);
}

/*
 * the function for the buzzer module, making an alarm noise
 */
void alarm() {
  unsigned char i;

  for (i = 0; i < 80; i++) //freq sound
  {
    digitalWrite (BUZZER_PIN, HIGH) ; //send a tone
    delay (1) ;
    digitalWrite (BUZZER_PIN, LOW) ; //no tone
    delay (1) ;
  }
  for (i = 0; i < 100; i++)
  {
    digitalWrite (BUZZER_PIN, HIGH) ;
    delay (2) ;
    digitalWrite (BUZZER_PIN, LOW) ;
    delay (2) ;
  }
}

/*
 * initial state of the distance sensor pins
 * as the doccumentation states, the trig pin must be
 * assigned a LOW / HIGH / LOW sequence in order to 
 * initialize the distance sensor and have it functional
 */
void initDistanceSensor() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
}

/*
 * function that calculates the distance given from the
 * sensor and returns it in
 * UNIT : cm
 */
float getDistance() {
  
  //initialize the sensor
  initDistanceSensor();

  //get the travel time in ms
  duration = pulseIn(ECHO_PIN, HIGH);

  //convert the travel time from ms to cm
  return (duration * 0.034 / 2);
}

/*
 * function that sends the parking status (if vehicle is parked or not)
 * to the wifi esp module using the serial communication
 */
void writeParkingStatus() {
  serialPins.write((int)vehicleParked);
}

/*
 * function that reads from the serial pins all the sent information
 * from the wifi module
 */
void readSerialData() {

  //check if any information is being recieved via serial pins
  if (serialPins.available() > 0) {
    reservedParkingSpot_wifi = serialPins.read();
    vehicleParked_wifi = serialPins.read();
    correctClient_wifi = serialPins.read();
  }

  Serial.println("reserved : " + reservedParkingSpot_wifi);
  Serial.println("confirmed parking : " + reservedParkingSpot_wifi);
  Serial.println("correct client confirmation : " + reservedParkingSpot_wifi);

}

/*
 * function that begins the timer countdown
 */
void count() {

  //if the timer hasn't started yet, start it
  if (timer_started == 0) {
    oldtime = millis();
    timer_started = 1;
  }
}

/*
 * main loop function
 */
void loop ()
{
  //get the distance from the sensor
  float distance = getDistance();

  //a vehicle is considered as parked only when the sensor
  //detects that the distance between the two is less than 10cm
  if (distance < 10) {

    //change the status to parked (will also be sent via serial)
    vehicleParked = true;

    //start the timer
    count();
  }
  //the vehicle left the parking spot
  else if (distance >= 10) {

    //stop the count
    timer_started = 0;

    //change the status to empty space
    vehicleParked = false;

    //stop the alarm if any
    digitalWrite (BUZZER_PIN, LOW) ;
  }

  //vehicle hasn't left the parking spot yet
  while (vehicleParked) {

    //get the information from the wifi module
    readSerialData();

    //and notify that a vehicle is here now
    writeParkingStatus();

    //if the wrong vehicle parked (doesn't match the reservation)
    if (!correctClient_wifi) {

      //alarm the driver (cannot park in a reserved space)
      alarm();

      //monnitor to see if the vehicle wants to leave
      distance = getDistance();
    }
    else {

      //if the parking spot is indeed reserved
      //and a certain driver stops there
      if (reservedParkingSpot_wifi) {

        //alarm the driver (cannot park in a reserved space)
        alarm();
  
        //monnitor to see if the vehicle wants to leave
        distance = getDistance();
      }
      //if the parking spot is not taken / reserved and someone parks
      else {

        //give them a 1 minute cooldown to hit the "park" button
        if ((millis() - oldtime) / 1000 > 60)
          alarm();

        //monnitor to see if the vehicle wants to leave
        distance = getDistance();
      }

      //if, at any time, the vehicle leaves the parking spot, or they hit the "park" button
      //and the spot is not reserved, stop the alarm
      if (distance >= 10 || (vehicleParked_wifi && !reservedParkingSpot_wifi && correctClient_wifi)) {

        //reset the timer
        oldtime = millis();

        //send to the module that someone has parked (correctly)
        writeParkingStatus();
        
        break;
      }
    }
  }

  //update the control varialbes provided by the wifi module
  readSerialData();

  delay(1000);
}
