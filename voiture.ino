#include <Arduino_FreeRTOS.h>


// define four Tasks
TaskHandle_t TaskStopDirectionHandle;
TaskHandle_t TaskDriveHandle;
TaskHandle_t TaskReceiveHandle;
TaskHandle_t TaskBuzzerHandle;

void TaskStopDirection(void* pvParameters);
void TaskDrive(void* pvParameters);
void TaskReceive(void* pvParameters);
void TaskBuzzer(void* pvParameters);


#include <VirtualWire.h>

#define RX_PIN 8

#define DEBUG
#define noDEBUG_STACK

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
#define Buzzer 10
int intBuzzer = 500;
bool poolBuzzer = true;

// Receive data
byte receive[3];

// Elapsed time
unsigned long elapsedTime;

//Drive the car
boolean driveStopBool = false;
boolean centerBool = false;
int lastButton = 0;

boolean newValue = false;


void setup() {
  // put your setup code here, to run once:
  // put your setup code here, to run once:
  Serial.begin(115200);  // Debugging only
  while (!Serial);


  vw_setup(2000);
  vw_set_rx_pin(RX_PIN);
  vw_rx_start();
  

  
  // Initialisation

  // Drive
  pinMode(PRO_A, OUTPUT);
  pinMode(PRO_R, OUTPUT);
  analogWrite(PRO_A, LOW);
  analogWrite(PRO_R, LOW);

  pinMode(DIR_G, OUTPUT);
  pinMode(DIR_D, OUTPUT);
  analogWrite(DIR_G, LOW);
  analogWrite(DIR_D, LOW);

  // Stop
  pinMode(STOP, OUTPUT);
  pinMode(NO_STOP, OUTPUT);
  digitalWrite(STOP, LOW);
  digitalWrite(NO_STOP, LOW);
  

  // Button
  pinMode(bLeft, INPUT_PULLUP);
  pinMode(bCenter, INPUT_PULLUP);
  pinMode(bRight, INPUT_PULLUP);

  receive[0] = (byte)128;
  receive[1] = (byte)128;
  receive[2] = (byte)1;

  
  // Now set up two Tasks to run independently.
  xTaskCreate(
    TaskBuzzer
    ,  "Buzzer"  // A name just for humans
    ,  64  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  configMAX_PRIORITIES - 3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &TaskReceiveHandle );

   xTaskCreate(
    TaskReceive
    ,  "Receive information"
    ,  128  // Stack size
    ,  NULL
    ,  configMAX_PRIORITIES - 1  // Priority
    ,  &TaskReceiveHandle );

    xTaskCreate(
    TaskDrive
    ,  "Drive the car"
    ,  160  // Stack size
    ,  NULL
    ,  configMAX_PRIORITIES - 2  // Priority
    ,  &TaskDriveHandle );

    xTaskCreate(
    TaskStopDirection
    ,  "Stop direction"
    ,  128  // Stack size
    ,  NULL
    ,  configMAX_PRIORITIES - 3  // Priority
    ,  &TaskStopDirectionHandle );
    

    //Serial.println("Start");
}

void loop() {}


// Stop direction
void TaskStopDirection(void* pvParameters)
{

  (void) pvParameters;
  
  for (;;)
  {
    //Serial.println("Stop direction");
    
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
    #ifdef DEBUG_STACK
      Serial.print("- TASK ");
      Serial.print(pcTaskGetName(TaskStopDirectionHandle));
      Serial.print(", High Watermark: ");
      Serial.print(uxTaskGetStackHighWaterMark(TaskStopDirectionHandle));
      Serial.println();
     #endif
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}


// Drive
void TaskDrive(void* pvParameters)
{

  (void) pvParameters;
  
  for (;;)
  {
    
    byte x,y;
    int pro_pwm;

    if (newValue)
    {

      //Serial.println("Drive");

      x = receive[0];
      y = receive[1];
  
      if (x >= 120 and x <= 140) {
        analogWrite(PRO_A, LOW);
        analogWrite(PRO_R, LOW);
        if (driveStopBool) {
          digitalWrite(STOP, HIGH);
          digitalWrite(NO_STOP, LOW);
          delay(start_delay);
          //vTaskDelay(start_delay / portTICK_PERIOD_MS);
          digitalWrite(STOP, LOW);
          digitalWrite(NO_STOP, LOW);
          delay(stop_delay);
          //vTaskDelay(stop_delay / portTICK_PERIOD_MS);
          digitalWrite(STOP, LOW);
          digitalWrite(NO_STOP, HIGH);
          delay(return_delay);
          //vTaskDelay(return_delay / portTICK_PERIOD_MS);
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
          //vTaskDelay(start_delay / portTICK_PERIOD_MS);
          digitalWrite(STOP, LOW);
          digitalWrite(NO_STOP, LOW);
          delay(stop_delay);
          //vTaskDelay(stop_delay / portTICK_PERIOD_MS);
          digitalWrite(STOP, LOW);
          digitalWrite(NO_STOP, HIGH);
          delay(return_delay);
          //vTaskDelay(return_delay / portTICK_PERIOD_MS);
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
          //vTaskDelay(start_delay / portTICK_PERIOD_MS);
          digitalWrite(STOP, LOW);
          digitalWrite(NO_STOP, LOW);
          delay(stop_delay);
          //vTaskDelay(stop_delay / portTICK_PERIOD_MS);
          digitalWrite(STOP, LOW);
          digitalWrite(NO_STOP, HIGH);
          delay(return_delay);
          //vTaskDelay(return_delay / portTICK_PERIOD_MS);
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

      newValue = false;
    }
    #ifdef DEBUG_STACK
      Serial.print("- TASK ");
      Serial.print(pcTaskGetName(TaskDriveHandle));
      Serial.print(", High Watermark: ");
      Serial.print(uxTaskGetStackHighWaterMark(TaskDriveHandle));
      Serial.println();
     #endif
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Receive data
void TaskReceive(void* pvParameters)
{
  (void) pvParameters;
  
  for (;;)
  {
    //Serial.println("Received");
    
    if (vw_get_message(receive, 3)) // Non-blocking
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

       //Serial.println("Receive");
       newValue = true;

      #ifdef DEBUG_STACK
        Serial.print("- TASK ");
        Serial.print(pcTaskGetName(TaskReceiveHandle));
        Serial.print(", High Watermark: ");
        Serial.print(uxTaskGetStackHighWaterMark(TaskReceiveHandle));
        Serial.println();
       #endif
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// Buzzer
void TaskBuzzer(void* pvParameters)
{

  (void) pvParameters;

  for (;;) 
  {
    //Serial.println("Buzzer");
    
    if (receive[2] == 0) {
      int i;
      if (poolBuzzer) {
        for (i=intBuzzer; i<intBuzzer+20; i++) {
          tone(Buzzer, i);
          vTaskDelay(5 / portTICK_PERIOD_MS);
        }
        intBuzzer += 100;
        if (intBuzzer == 1000)
          poolBuzzer = false;
      } else {
        for (i=intBuzzer; i>intBuzzer-20; i--) {
          tone(Buzzer, i);
          vTaskDelay(5 / portTICK_PERIOD_MS);
        }
        intBuzzer -= 100;
        if (intBuzzer == 500) {
          poolBuzzer = true;
          noTone(Buzzer);
        }
      }
    } else {
      intBuzzer = 0;
      poolBuzzer = true;
    }
    #ifdef DEBUG_STACK
      Serial.print("- TASK ");
      Serial.print(pcTaskGetName(TaskBuzzerHandle));
      Serial.print(", High Watermark: ");
      Serial.print(uxTaskGetStackHighWaterMark(TaskBuzzerHandle));
      Serial.println();
     #endif
    vTaskDelay(100 / portTICK_PERIOD_MS);
    noTone(Buzzer);
  }
}
