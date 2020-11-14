#include <RH_ASK.h>
RH_ASK driver(2000,8,9);

#define DEBUG

// Button
#define bLeft 2
#define bCenter 7
#define bRight 4

// Moteur traction

#define PRO_A 5
#define PRO_R 6

// Moteur direction

#define DIR_G 3
#define DIR_D 11

// Moteur stop
#define STOP 12
#define NO_STOP 13
#define stop_delay 300
#define start_delay 30
#define return_delay 25

// Buzzer

#define Buzzer 7

bool buzz = false;
int intBuzzer = 500;
bool poolBuzzer = true;

// Receive data
byte receive[3];

unsigned long elapsedTime;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  // Debugging only
  while (!Serial);
  if (!driver.init())
    Serial.println("init failed");

  
  // Initialisation
  pinMode(PRO_A, OUTPUT);
  pinMode(PRO_R, OUTPUT);
  analogWrite(PRO_A, LOW);
  analogWrite(PRO_R, LOW);

  pinMode(DIR_G, OUTPUT);
  pinMode(DIR_D, OUTPUT);
  analogWrite(DIR_G, LOW);
  analogWrite(DIR_D, LOW);
  

  // Button
  pinMode(bLeft, INPUT_PULLUP);
  pinMode(bCenter, INPUT_PULLUP);
  pinMode(bRight, INPUT_PULLUP);
  
  

  pinMode(stop_delay, OUTPUT);
  pinMode(return_delay, OUTPUT);
  digitalWrite(stop_delay, LOW);
  digitalWrite(return_delay, LOW);
}



/*
 * Drive the car
 */


boolean driveStopBool = false;
boolean centerBool = false;
int lastButton = 0;


void stopDirection() {
  if (centerBool) {
    if (digitalRead(bLeft) == LOW || digitalRead(bRight) == LOW ) {
      Serial.println("Stop direction");
      digitalWrite(DIR_G, LOW);
      digitalWrite(DIR_D, LOW);
      centerBool = false;
    }
  } else {
     if (lastButton == 1 || lastButton == 2) {
         if (digitalRead(bCenter) == LOW ||  1 == lastButton && digitalRead(bRight) == LOW || 2 == lastButton && digitalRead(bLeft) == LOW ) {
          Serial.println("Stop direction");
          digitalWrite(DIR_G, LOW);
          digitalWrite(DIR_D, LOW);
          lastButton = 0;
        }
     }
     else {
      if (digitalRead(bLeft) == LOW || digitalRead(bCenter) == LOW || digitalRead(bRight) == LOW) {
        Serial.println("Stop direction");
        digitalWrite(DIR_G, LOW);
        digitalWrite(DIR_D, LOW);
      }
    }
  }
}


void drive (byte x,byte y, byte z) {
  if (z == 0) {
    buzz = true;
  }
  int pro_pwm;
  if (x >= 120 and x <= 140) {
    analogWrite(PRO_A, LOW);
    analogWrite(PRO_R, LOW);
    if (driveStopBool) {
      digitalWrite(STOP, HIGH);
      digitalWrite(NO_STOP, LOW);
      delay(start_delay);
      digitalWrite(STOP, LOW);
      digitalWrite(NO_STOP, LOW);
      delay(stop_delay);
      digitalWrite(STOP, LOW);
      digitalWrite(NO_STOP, HIGH);
      delay(return_delay);
      digitalWrite(STOP, LOW);
      digitalWrite(NO_STOP, LOW);
    }
    driveStopBool = false;
    
  } else if (x > 140) {
    if (PRO_R == HIGH) {
      analogWrite(PRO_A, LOW);
      analogWrite(PRO_R, LOW);
      digitalWrite(STOP, HIGH);
      digitalWrite(NO_STOP, LOW);
      delay(start_delay);
      digitalWrite(STOP, LOW);
      digitalWrite(NO_STOP, LOW);
      delay(stop_delay);
      digitalWrite(STOP, LOW);
      digitalWrite(NO_STOP, HIGH);
      delay(return_delay);
      digitalWrite(STOP, LOW);
      digitalWrite(NO_STOP, LOW);
    }
    pro_pwm = (x - 133) * 2;
    analogWrite(PRO_A, pro_pwm);
    analogWrite(PRO_R, LOW);  
    driveStopBool = true;
  } else if (x < 120) {
    if (PRO_A == HIGH) {
      analogWrite(PRO_A, LOW);
      analogWrite(PRO_R, LOW);
      digitalWrite(STOP, HIGH);
      digitalWrite(NO_STOP, LOW);
      delay(start_delay);
      digitalWrite(STOP, LOW);
      digitalWrite(NO_STOP, LOW);
      delay(stop_delay);
      digitalWrite(STOP, LOW);
      digitalWrite(NO_STOP, HIGH);
      delay(return_delay);
      digitalWrite(STOP, LOW);
      digitalWrite(NO_STOP, LOW);
    }
    pro_pwm = (125 - x) * 2;  
    analogWrite(PRO_A, LOW);
    analogWrite(PRO_R, pro_pwm);
    //analogWrite(PRO_P, pro_pmw);
    driveStopBool = true;
  }


  

  // Direction
  if (y >= 64 and y <= 192) {
    centerBool = false;
    if (digitalRead(bCenter) != LOW) {
      if (digitalRead(bLeft) == LOW) {
        analogWrite(DIR_D, 50);
        analogWrite(DIR_G, LOW);
        lastButton = 1;
      } else if (digitalRead(bRight) == LOW) {
        analogWrite(DIR_D, LOW);
        analogWrite(DIR_G, 50); 
        lastButton = 2;
      }
    }
  }
  else {
    if (digitalRead(bCenter) == LOW) {
      centerBool = true;
    }
    if (y > 64) {
      if (digitalRead(bRight) != LOW) {
        //Serial.println("right");
        analogWrite(DIR_D, 50);
        analogWrite(DIR_G, LOW);
      }
    } else if (y < 192) {
      if (digitalRead(bLeft) != LOW) {
        //Serial.println("left");
        analogWrite(DIR_G, 50);
        analogWrite(DIR_D, LOW);
      }
    }
  }
}

void receiveData() {
  if (driver.recv(receive, 3)) // Non-blocking
    {
      // Message with a good checksum received, dump it.
      //driver.printBuffer("Got:", receive, 4);
      #ifdef DEBUG
        Serial.print(receive[0]);
        Serial.print(" : ");
        Serial.print(receive[1]);
        Serial.print(" : ");
        Serial.println(receive[2]);
        Serial.println(millis()- elapsedTime);
        elapsedTime = millis();
       #endif

       drive(receive[0], receive[1], receive[2]);

       //Serial.println("Receive");

       for (int i = 0; i < 100; i++) {
        stopDirection();
        delay(1);
       }
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  receiveData();
  stopDirection();
  yield();
}
