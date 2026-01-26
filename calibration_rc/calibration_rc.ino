#include <IRremote.h>
#include <EEPROM.h>
#define PIN_IR_RECEIVE            2
#define PIN_MOTOR_LEFT_FORWARD    10
#define PIN_MOTOR_LEFT_BACKWARD   9
#define PIN_MOTOR_RIGHT_FORWARD   6
#define PIN_MOTOR_RIGHT_BACKWARD  5

int16_t motor_speed_left  = 0;   
int16_t motor_speed_right = 0;

int16_t calibration_left  = 0;
int16_t calibration_right = 0;

static const int EEPROM_LEFT_ADDR  = 0; 
static const int EEPROM_RIGHT_ADDR = 2; 

static const int16_t CAL_STEP = 5;
static const int16_t CAL_MIN  = -255;
static const int16_t CAL_MAX  =  255;

bool pending_save = false;
unsigned long last_cal_change_ms = 0;
static const unsigned long SAVE_DELAY_MS = 600;

static inline int16_t clamp16(int16_t v, int16_t lo, int16_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static inline uint8_t clampPWM(int v) {
  if (v < 0) return 0;
  if (v > 255) return 255;
  return (uint8_t)v;
}

void writeEEPROM(int addr, int16_t value) {
  uint8_t low_byte  = (uint8_t)(value & 0xFF);
  uint8_t high_byte = (uint8_t)((value >> 8) & 0xFF);

  EEPROM.update(addr, low_byte);
  EEPROM.update(addr + 1, high_byte);
}

int16_t readEEPROM(int addr) {
  uint8_t low_byte  = EEPROM.read(addr);
  uint8_t high_byte = EEPROM.read(addr + 1);
  return (int16_t)((high_byte << 8) | low_byte);
}

void scheduleSave() {
  pending_save = true;
  last_cal_change_ms = millis();
}

void saveIfDue() {
  if(!pending_save) return;
  if(millis() - last_cal_change_ms < SAVE_DELAY_MS) return;

  writeEEPROM(EEPROM_LEFT_ADDR, calibration_left);
  writeEEPROM(EEPROM_RIGHT_ADDR, calibration_right);

  pending_save = false;
}

void processMotors(uint8_t cmd) {
  if(cmd == 0x1C) {
    analogWrite(PIN_MOTOR_LEFT_FORWARD, 0);
    analogWrite(PIN_MOTOR_LEFT_BACKWARD, 0);
    analogWrite(PIN_MOTOR_RIGHT_FORWARD, 0);
    analogWrite(PIN_MOTOR_RIGHT_BACKWARD, 0);
  }
  else {
  
    if(motor_speed_left >= 0) {
      uint16_t pwmL = clampPWM((int)motor_speed_left + (int)calibration_left);
      analogWrite(PIN_MOTOR_LEFT_FORWARD, pwmL);
      analogWrite(PIN_MOTOR_LEFT_BACKWARD, 0);
    } 
    else{
      uint16_t pwmL = clampPWM((int)(-motor_speed_left) + (int)calibration_left);
      analogWrite(PIN_MOTOR_LEFT_FORWARD, 0);
      analogWrite(PIN_MOTOR_LEFT_BACKWARD, pwmL);
    }
    if(motor_speed_right >= 0) {
      uint16_t pwmR = clampPWM((int)motor_speed_right + (int)calibration_right);
      analogWrite(PIN_MOTOR_RIGHT_FORWARD, pwmR);
      analogWrite(PIN_MOTOR_RIGHT_BACKWARD, 0);
    } 
    else{
      uint16_t pwmR = clampPWM((int)(-motor_speed_right) + (int)calibration_right);
      analogWrite(PIN_MOTOR_RIGHT_FORWARD, 0);
      analogWrite(PIN_MOTOR_RIGHT_BACKWARD, pwmR);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  IrReceiver.begin(PIN_IR_RECEIVE, ENABLE_LED_FEEDBACK);

  pinMode(PIN_MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_BACKWARD, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_BACKWARD, OUTPUT);

  analogWrite(PIN_MOTOR_LEFT_FORWARD, 0);
  analogWrite(PIN_MOTOR_LEFT_BACKWARD, 0);
  analogWrite(PIN_MOTOR_RIGHT_FORWARD, 0);
  analogWrite(PIN_MOTOR_RIGHT_BACKWARD, 0);

  calibration_left  = readEEPROM(EEPROM_LEFT_ADDR);
  calibration_right = readEEPROM(EEPROM_RIGHT_ADDR);

  calibration_left  = clamp16(calibration_left,  CAL_MIN, CAL_MAX);
  calibration_right = clamp16(calibration_right, CAL_MIN, CAL_MAX);

  Serial.println("===== Loaded EEPROM Calibration =====");
  Serial.print("Left  : "); Serial.println(calibration_left);
  Serial.print("Right : "); Serial.println(calibration_right);
  Serial.println("=====================================");
}

void loop() {
  uint8_t cmd = IrReceiver.decodedIRData.command;
  if(IrReceiver.decode()) {
    if(IrReceiver.decodedIRData.protocol == NEC) {
      switch (cmd) {
        case 0x18: // Forward
          motor_speed_left  = 200;
          motor_speed_right = 200;
          break;

        case 0x52: // Backward
          motor_speed_left  = -200;
          motor_speed_right = -200;
          break;

        case 0x5A: // Right
          motor_speed_left  = 200;
          motor_speed_right = 150;
          break;

        case 0x08: // Left
          motor_speed_left  = 150;
          motor_speed_right = 200;
          break;

        case 0x1C: // Stop
          motor_speed_left  = 0;
          motor_speed_right = 0;
          break;

        case 0x45: // Left +10
          calibration_left = clamp16(calibration_left + CAL_STEP, CAL_MIN, CAL_MAX);
          scheduleSave();
          break;

        case 0x07: // Left -10
          calibration_left = clamp16(calibration_left - CAL_STEP, CAL_MIN, CAL_MAX);
          scheduleSave();
          break;

        case 0x47: // Right +10
          calibration_right = clamp16(calibration_right + CAL_STEP, CAL_MIN, CAL_MAX);
          scheduleSave();
          break;

        case 0x09: // Right -10
          calibration_right = clamp16(calibration_right - CAL_STEP, CAL_MIN, CAL_MAX);
          scheduleSave();
          break;

        default:
          break;
      }
    }

    IrReceiver.resume();
  }

  processMotors(cmd);

  saveIfDue();
}
