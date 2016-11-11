#include <stdarg.h>
/*
  Listener is part of the VCoS NSF project, and is intended to listen for incoming serial bytes. 
 These bytes indicate which motor(s) should vibrate to provide haptic feedback to the user.
 The current interface is as follows:
 [Motor (0..3 or 5). Each number corresponds to a motor, or 5 for all motors. 8 bit unsigned int.)]
 [Intensity (0..100 percent). 8 bit unsigned int.]
 [Duration (1..100 percent of a second) 8-bit unsigned int.]
 These values are sent in binary packets using unsigned integer values, as follows:
 [0xFF][orientation][intensity][duration][checksum - never 255. sender will truncate to 254 rather than send a 255]
 If a new packet is received, it will overwrite the duration of the previous one, terminating it immediately.

 Authors: Greg Link
          Gus Smith <hfs5022@psu.edu>
          Chris Pratt <cmp6048@psu.edu>
 */
 
// Play with these (INTENSITY_SCALE has minimum of 3)
#define INTENSITY_SCALE 70
#define INTENSITY_MIN 25
#define INTENSITY_MAX 100

// Pin 13 has an LED connected
int ledpin = 13;

// Pin Mapping for IST Glove: MOT0 - 3 MOT1 - 8 MOT2 - 12 MOT3 - 23
int motorpins[6] = {
  3,8,12,23,19,14 }; // motor driver pins 0 through 5, ideally spaced 60 degrees apart 

/* Pin Mapping for Dev Glove: MOT0 - 3 MOT1 - 8 MOT2 - 12 MOT4 - 19
int motorpins[6] = {
    3,8,12,19,23,14 };
*/

char print_buffer[64];
float distance_threshold = 40;

uint8_t motor = 7;
uint8_t intensity = 0;
word duration = 0;

// the setup routine runs once when you press reset:
void setup() {
                   
  Serial.begin(9600); // Serial USB is always 12MBit/sec
  Serial.setTimeout(250);

  pinMode(ledpin, OUTPUT);     
  for(int i = 0; i < 6; i++){
    pinMode(motorpins[i],OUTPUT);
    digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);               // wait for a second
    digitalWrite(ledpin, LOW);    // turn the LED off by making the voltage LOW
    delay(100);               // wait for a second
  }
  digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);               // wait for a second
  digitalWrite(ledpin, LOW);    // turn the LED off by making the voltage LOW
  delay(500);  // wait for a second

  digitalWriteAll(LOW);
  
// TEST used to determine which motors are which
//  while(1){
//    for (int i = 0; i < 4; i++){
//      digitalWrite(motorpins[i],HIGH);
//      delay(250);
//      digitalWrite(motorpins[i],LOW);
//    }
//  }
  
}

void pulseLED(int durationms){
  digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(durationms);               // wait for a second
  digitalWrite(ledpin, LOW);    // turn the LED off by making the voltage LOW
  delay(durationms);               // wait for a second
}

void p(char const *fmt, ... ){
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  Serial.print(buf);
}

void check_bytes(void)
{
  static int8_t received_packet[5] = {0,0,0}; //array for the getc
  static int8_t bytes_seen = 0;
  static bool seenFF = false;
  int8_t num_characters = 5;
  
  // While there's bits to be read...
  while(Serial.available() > 0){  
    
    uint8_t byte = Serial.read();

    //p("R:[%d]@%d\n",byte, bytes_seen);
    if(byte == 0xFF) { // preamble byte
      // This happens at the start of a set of numbers, so we prepare to take in a new data set. 
      bytes_seen = 0;
      seenFF = true;
    }
    // If we've started, but not finished, a new set of data...
    if(seenFF && bytes_seen < num_characters) {
      received_packet [bytes_seen] = byte;
      bytes_seen++;
    }
    // If we found the full number of bytes expected.
    if(seenFF && bytes_seen == num_characters) {
      // Generate the checksum to check that our data is correct.
      int checksum_int = (int) received_packet[0] + (int) received_packet[1] + (int) received_packet[2] + (int) received_packet[3];
      uint8_t checksum = checksum_int & 0xFF;
      
      if(checksum == 255) { 
        // Sender will truncate to 254 in this case; so must we.
        checksum = 254; 
      }
      
      
      if(/*checksum == received_packet[4]*/1==1){
        motor = constrain(received_packet[1],0,5);
        
        intensity = constrain(received_packet[2],0,100);
        // Scale the intensity to fit between INTENSITY_MIN and INTENSITY_MAX
        intensity = map(intensity, 0, 100, INTENSITY_MIN, INTENSITY_MAX);
        
        duration = constrain(received_packet[3],1,100);
        // Convert 1/100 of a second into ms, then to us
        duration *= 10 * 1000;
        
        p("PR: [%d][%d][%d][%d][%d]\n", received_packet[0], received_packet[1], received_packet[2], received_packet[3], received_packet[4]);
      } 
      else {
        p("ERR: [%d][%d][%d][%d][%d]\n", received_packet[0], received_packet[1], received_packet[2], received_packet[3], received_packet[4]);
      }
      bytes_seen = 0;
      seenFF = false;
      received_packet[0] = 0;
      // Flush the rest of it so we get a fresh packet next time.
      Serial.flush();
    }
  } 
}

// Write to every motor
void digitalWriteAll(uint8_t state)
{
  for (int m = 0; m < 5; m++)
  {
    digitalWriteFast(motorpins[m], state);
  }
}

// Run motors with individual PWM
void runMotors(uint8_t motor, int intensity, word duration)
{
  unsigned long start_time = micros();
  
  // Calculate length of the duty cycle
  word cycle_time = INTENSITY_SCALE * intensity;

  // Make sure every motor is off when running a new motor
  digitalWriteAll(LOW);

  // Keep running until the full duration has passed
  while(start_time + duration > micros()) { 
    // Pulse the motors based on our duty cycle
    if(motor == 5) {
      digitalWriteAll(HIGH);
    }
    else {
      digitalWriteFast(motor, HIGH);
    }
    delayMicroseconds(cycle_time);

    // If there's another byte waiting, read it right away
    if(Serial.available() > 0) {
      break;
    }

    // Pulse the motors based on our duty cycle
    if(motor == 5) {
      digitalWriteAll(LOW);
    }
    else {
      digitalWriteFast(motor, LOW);
    }

    // This algorithm breaks up the delays into chunks to keep checking
    // for the right time to stop the loop.
    // The shorter our delay, the less we have to break up the delays
    int iterations = map(intensity, INTENSITY_MIN, INTENSITY_MAX, 
                         INTENSITY_MIN, INTENSITY_MAX * 2);
    for(uint8_t i = 0; i < iterations; i++)
    {
      delayMicroseconds(((100 * INTENSITY_SCALE) - cycle_time) / iterations);
      
      if(start_time + duration < micros()) {
        break;
      }
    }

    // If the above breaks, just use this:
    // delayMicroseconds(((100 * INTENSITY_SCALE) - cycle_time));
  }
}

// the loop routine runs over and over again forever:
void loop() {
  motor = 7;
  intensity = 0;
  duration = 0;
  
  check_bytes(); // scan USB incoming buffer for more bytes. If a complete packet, update the three variables and continue.
  pulseLED(10);

  // Run the motors
  if (motor != 7) {
    // Set the proper motor based on the motor pins
    motor = (motor != 5 ? motorpins[motor] : 5);
  
    runMotors(motor, intensity, duration);
  }
  // Make sure our motors are off if there are no bytes waiting
  if(!(Serial.available() > 0))
  {
    digitalWriteAll(LOW);
  }
}
