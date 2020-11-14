#include <RH_ASK.h>


int VRx = A0;
int VRy = A1;
int MS = 5;


byte t[3];

RH_ASK driver(2000,11,12);

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  while (!Serial);

  pinMode(MS,INPUT);
  digitalWrite(MS, HIGH);

  if (!driver.init())
    Serial.println("init failed");

  delay(1000);
}

void loop() {

  t[0] = map(analogRead(VRx),0,1023,0,255);
  t[1] = map(analogRead(VRy),0,1023,0,255);
  t[2] = digitalRead(MS);

  Serial.print("  >>> ");
  Serial.print(t[0]);
  Serial.print(" ");
  Serial.print(t[1]);
  Serial.print(" ");
  Serial.println(t[2]);
  
  driver.send(t, 3);
  driver.waitPacketSent();

  Serial.println("ARDUINO : send");

  delay(25);
}
