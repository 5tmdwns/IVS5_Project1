#include <IRremote.h>
#include <EEPROM.h>
#define PIN_IR_RECEIVE            2
#define PIN_MOTOR_LEFT_FORWARD    10
#define PIN_MOTOR_LEFT_BACKWARD   9
#define PIN_MOTOR_RIGHT_FORWARD   6
#define PIN_MOTOR_RIGHT_BACKWARD  5

int16_t motor_speed_left  = 0;   
int16_t motor_speed_right = 0;

static inline uint8_t clampPWM(int v) {
  if (v < 0) return 0;
  if (v > 255) return 255;
  return (uint8_t)v;
}

void processMotors() {
  if(motor_speed_left >= 0) {
    uint16_t pwmL = clampPWM((int)motor_speed_left);
    analogWrite(PIN_MOTOR_LEFT_FORWARD, pwmL);
    analogWrite(PIN_MOTOR_LEFT_BACKWARD, 0);
  } 
  else{
    uint16_t pwmL = clampPWM((int)(-motor_speed_left));
    analogWrite(PIN_MOTOR_LEFT_FORWARD, 0);
    analogWrite(PIN_MOTOR_LEFT_BACKWARD, pwmL);
  }
  if(motor_speed_right >= 0) {
    uint16_t pwmR = clampPWM((int)motor_speed_right);
    analogWrite(PIN_MOTOR_RIGHT_FORWARD, pwmR);
    analogWrite(PIN_MOTOR_RIGHT_BACKWARD, 0);
  } 
  else{
    uint16_t pwmR = clampPWM((int)(-motor_speed_right);
    analogWrite(PIN_MOTOR_RIGHT_FORWARD, 0);
    analogWrite(PIN_MOTOR_RIGHT_BACKWARD, pwmR);
  }
}

void setup() {
  IrReceiver.begin(PIN_IR_RECEIVE, ENABLE_LED_FEEDBACK);

  pinMode(PIN_MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_BACKWARD, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_BACKWARD, OUTPUT);

  analogWrite(PIN_MOTOR_LEFT_FORWARD, 0);
  analogWrite(PIN_MOTOR_LEFT_BACKWARD, 0);
  analogWrite(PIN_MOTOR_RIGHT_FORWARD, 0);
  analogWrite(PIN_MOTOR_RIGHT_BACKWARD, 0);
}

void loop() {
  uint8_t cmd = IrReceiver.decodedIRData.command;
  if(IrReceiver.decode()) {
    if(IrReceiver.decodedIRData.protocol == NEC) {
      switch (cmd) {
        case 0x18: // Forward
          motor_speed_left  = 230;
          motor_speed_right = 255;
          break;
        case 0x52: // Backward
          motor_speed_left  = -230;
          motor_speed_right = -255;
          break;
        case 0x5A: // Right
          motor_speed_left  = 230;
          motor_speed_right = 205;
          break;
        case 0x08: // Left
          motor_speed_left  = 180;
          motor_speed_right = 255;
          break;
        case 0x1C: // Stop
          motor_speed_left  = 0;
          motor_speed_right = 0;
          break;
        default:
          break;
    }
    IrReceiver.resume();
  }
  processMotors();
}
