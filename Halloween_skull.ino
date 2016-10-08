//
// Damien and Dad's halloween project.
//
// 2016 version... Uses Audio Shield, Servo, Ping and couple of LEDs
// This SW is walso now hooked up to a timer for much better simultaneous effects...
//

#include <Servo.h>
#include <ISD1700.h>

// PIN Definition
#define PIN_LED_L    4
#define PIN_LED_R    5
#define PIN_PING     2
#define PIN_ECHO     3
#define PIN_MOTOR   14
#define PIN_AUDIO   10

// Global Objects
Servo myservo;  // create servo object to control a servo
ISD1700 chip(PIN_AUDIO); // Initialize chipcorder (Audio)

#define SERVO_DELAY 150
#define SERVO_ANGLE_SHAKE 20
#define SERVO_SHAKE_COUNT 20

// The infamous set of global variables....

int   servo_pos =90;
int   pos = 0;    // variable to store the servo position
int   loop_cnt = 0;
bool  bShake = false;
bool  bAudio = false;

volatile unsigned long lastUpdate = 0; // last update of position
volatile int      iShakeStep = -1;
volatile int      iShakeCount = 0;


void setup(){
    // initialize serial communication:
    Serial.begin(9600);
    
    pinMode(PIN_LED_L, OUTPUT);
    pinMode(PIN_LED_R, OUTPUT);

    pinMode(PIN_PING, OUTPUT);
    pinMode(PIN_ECHO, INPUT);

    // Initialize the Audio Command
    apc = apc | vol; //D0, D1, D2
   
    //apc = apc | 0x8; //D3 comment to disable output monitor during record
   
    apc = apc | 0x50; // D4& D6 select MIC REC
    //apc = apc | 0x00; // D4& D6 select AnaIn REC
    //apc = apc | 0x10; // D4& D6 select MIC + AnaIn REC
   
    apc = apc | 0x80; // D7 AUX ON, comment enable AUD
    apc = apc | 0x100; // D8 SPK OFF, comment enable SPK
   
    //apc = apc | 0x200; // D9 Analog OUT OFF, comment enable Analog OUT 
    //apc = apc | 0x400; // D10 vAlert OFF, comment enable vAlert
    
    apc = apc | 0x800; // D11 EOM ON, comment disable EOM

    myservo.attach(PIN_MOTOR);  // attaches the servo on pin MOTOR to the servo object
    myservo.write(servo_pos);
  
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function below
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);

    /* select the correct sound...*/
    chip.fwd();
    delay(500);
    chip.fwd();
    delay(500);
    chip.fwd();
    delay(500);

    Serial.println("Sketch is starting up");
  }

void loop(){

  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration, cm;
  long dur2, cm2;
  int i;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(PING, LOW);

  delay(3);
  digitalWrite(PING, HIGH);

  delay(3);
  digitalWrite(PING, LOW);

  duration = pulseIn(ECHO, HIGH);

  delay(3);
  // convert the time into a distance
  cm = microsecondsToCentimeters(duration);
  digitalWrite(PING, LOW);

  delay(3);
  digitalWrite(PING, HIGH);

  delay(3);
  digitalWrite(PING, LOW);

  duration = pulseIn(ECHO, HIGH);

  delay(3);
  // convert the time into a distance
  cm2 = microsecondsToCentimeters(duration);

  cm = max( cm, cm2); // Take the max of the 2 measures...
  
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();

  if( (cm < 140) && !bShake ) {

    bShake = true;
    bAudio = true;
  }
  
  delay(700);

}


long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

void ArmShakeUpdate( unsigned long curMillis ){
  
  if( ((curMillis - lastUpdate) >= SERVO_DELAY) && (iShakeStep == 0) ){
    servo_pos -= SERVO_ANGLE_SHAKE;
    myservo.write(servo_pos);              // tell servo to go to position in variable 'pos'
    iShakeStep++;
           
    digitalWrite(PIN_LED_R, LOW);
    }
  else if( ((curMillis - lastUpdate) >= 2*SERVO_DELAY) && (iShakeStep == 1)){
    servo_pos -= SERVO_ANGLE_SHAKE;
    myservo.write(servo_pos);              // tell servo to go to position in variable 'pos'
    iShakeStep++;    

    digitalWrite(PIN_LED_L, LOW);
    digitalWrite(PIN_LED_R, HIGH);
    }
  else if( ((curMillis - lastUpdate) >= 3*SERVO_DELAY) && (iShakeStep == 2)){
    servo_pos += SERVO_ANGLE_SHAKE;
    myservo.write(servo_pos);              // tell servo to go to position in variable 'pos'
    iShakeStep++;    
    
    digitalWrite(PIN_LED_L, LOW);

    }
  else if( ((curMillis - lastUpdate) >= 4*SERVO_DELAY) && (iShakeStep == 3)){
    servo_pos += SERVO_ANGLE_SHAKE;
    myservo.write(servo_pos);              // tell servo to go to position in variable 'pos'
    /* Re-initialize the update mechanism */
    iShakeStep = 0;    
    iShakeCount++;
    lastUpdate = millis();

     //LED 1: ON, LED 2: OFF, then wait
    digitalWrite(PIN_LED_L, HIGH);
    digitalWrite(PIN_LED_R, HIGH);  
     
    if( bAudio == true)    
      chip.play();
  }
  
  if( iShakeCount >= SERVO_SHAKE_COUNT ) {
    servo_pos = 90;
    myservo.write(servo_pos); 
    bShake = false;
    iShakeCount = 0;
    iShakeStep = -1;

    digitalWrite(PIN_LED_L, LOW);
    digitalWrite(PIN_LED_R, LOW);  
    }

   return;
}

SIGNAL(TIMER0_COMPA_vect) {
  
  unsigned long currentMillis = millis();
  
  if( bShake == true) {
    
    if( iShakeStep == -1 ) {
      digitalWrite(LED_L, HIGH);
      digitalWrite(LED_R, HIGH);
      lastUpdate = currentMillis;
      servo_pos += SERVO_ANGLE_SHAKE;
      myservo.write(servo_pos);
      iShakeStep = 0; 
      }
  
    ArmShakeUpdate(currentMillis);
    
  }
} 



    
