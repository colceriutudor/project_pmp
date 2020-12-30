const int BUZZER_PIN = 8;
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

long duration;

bool vehicleParked;

int ok;

void setup ()
{
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode (BUZZER_PIN, OUTPUT) ;

  serialPins.begin(9600);
  Serial.begin(9600);
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

void alarm() {
  unsigned char i, j ;

  for (i = 0; i < 80; i++)
  {
    digitalWrite (BUZZER_PIN, HIGH);
    delay (1) ;
    digitalWrite (BUZZER_PIN, LOW);
    delay (1) ;
  }
  for (i = 0; i < 100; i++)
  {
    digitalWrite (BUZZER_PIN, HIGH);
    delay (2);
    digitalWrite (BUZZER_PIN, LOW);
    delay (2);
  }
  
  ok = 1;
}

void alarm() {
  unsigned char i, j ;

  for (i = 0; i < 80; i++)
  {
    digitalWrite (BUZZER_PIN, HIGH);
    delay (1) ;
    digitalWrite (BUZZER_PIN, LOW);
    delay (1) ;
  }
  for (i = 0; i < 100; i++)
  {
    digitalWrite (BUZZER_PIN, HIGH);
    delay (2);
    digitalWrite (BUZZER_PIN, LOW);
    delay (2);
  }
}

void loop ()
{
  float distance = getDistance();

  while (distance < 10) {
    alarm();
	getDistance();
  }
  else if (distance >= 10) {
    ok = 0;
    digitalWrite (BUZZER_PIN, LOW) ;
  }

  delay(1000);
}
