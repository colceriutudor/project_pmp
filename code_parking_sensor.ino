#include <SoftwareSerial.h>

SoftwareSerial serialPins(5, 6);

const int BUZZER_PIN = 8;
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

long duration;

int distance, ok;
int reservedParkingSpot_wifi, vehicleParked_wifi;

uint32_t oldtime;

bool vehicleParked;

void distanceSensorSetup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void buzzerSetup() {
  pinMode (BUZZER_PIN, OUTPUT) ;
}

void initVariables() {
  vehicleParked = false;
  vehicleParked_wifi = false;
  reservedParkingSpot_wifi = false;
}

void setup ()
{
  initVariables();

  distanceSensorSetup();

  buzzerSetup();

  serialPins.begin(9600);
  Serial.begin(9600);
}

void alarm() {
  unsigned char i, j ;

  for (i = 0; i < 80; i++) // When a frequency sound
  {
    digitalWrite (BUZZER_PIN, HIGH) ; //send tone
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

void initDistanceSensor() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
}

float getDistance() {
  initDistanceSensor();
  duration = pulseIn(ECHO_PIN, HIGH); //travel time in ms
  return (duration * 0.034 / 2);
}

void readSerialData() {
  if (serialPins.available() > 0) {
    reservedParkingSpot_wifi = serialPins.read();
    vehicleParked_wifi = serialPins.read();
  }
}

void count() {
  if (ok == 0) {
    oldtime = millis();
    ok = 1;
  }
}

void loop ()
{
  float distance = getDistance();

  if (distance < 10) {
    vehicleParked = true;
    count();
  }
  else if (distance >= 10) {
    ok = 0;
    vehicleParked = false;
    digitalWrite (BUZZER_PIN, LOW) ;
  }

  while (vehicleParked == true) {

    readSerialData();

    if (reservedParkingSpot_wifi == true) {
      alarm();
      distance = getDistance();
    }
    else {
      Serial.println((millis() - oldtime) / 1000);

      if ((millis() - oldtime) / 1000 > 60) {
        alarm();
      }

      distance = getDistance();
    }

    if (distance >= 10 || (vehicleParked_wifi == true && reservedParkingSpot_wifi == false)) {
      oldtime = millis();
      break;
    }
  }

  Serial.println("OUT");

  readSerialData();

  delay(1000);
}
