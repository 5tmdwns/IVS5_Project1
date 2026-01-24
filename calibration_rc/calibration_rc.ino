#include <IRremote.h>
#define PIN_IR_RECEIVE 2
#define PIN_MOTOR_LEFT_FORWARD 10
#define PIN_MOTOR_LEFT_BACKWARD 9
#define PIN_MOTOR_RIGHT_FORWARD 6
#define PIN_MOTOR_RIGHT_BACKWARD 5

int motor_speed_left = 0;
int motor_speed_right = 0;

int calibration_left = 0;
int calibration_right = 0;

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
  if(IrReceiver.decode() == true) {
    if(IrReceiver.decodedIRData.protocol == NEC) {
      if(IrReceiver.decodedIRData.command == 0x18) {
        motor_speed_left = 200;
        motor_speed_right = 200;
      }
      else if(IrReceiver.decodedIRData.command == 0x52) {
        motor_speed_left = -200;
        motor_speed_right = -200;
      }
      else if(IrReceiver.decodedIRData.command == 0x5A) {
        motor_speed_left = 200;
        motor_speed_right = 150;
      }
      else if(IrReceiver.decodedIRData.command == 0x08) {
        motor_speed_left = 150;
        motor_speed_right = 200;
      }
      else if(IrReceiver.decodedIRData.command == 0x1C) {
        motor_speed_left = 0;
        motor_speed_right = 0;
      }
      else if(IrReceiver.decodedIRData.command == 0x19) {
        calibration_left = 0;
        calibration_right = 0;
      }
      else if(IrReceiver.decodedIRData.command == 0x45) {
        calibration_left = 10;
      }
      else if(IrReceiver.decodedIRData.command == 0x46) {
        calibration_left = 20;
      }
      else if(IrReceiver.decodedIRData.command == 0x47) {
		  calibration_left = 30;
		}
      else if(IrReceiver.decodedIRData.command == 0x44) {
        calibration_right = 10;
		}
      else if(IrReceiver.decodedIRData.command == 0x40) {
        calibration_right = 20;
      }
      else if(IrReceiver.decodedIRData.command == 0x43) {
        calibration_right = 30;
      }
    }
    IrReceiver.resume();
  }
  if(motor_speed_left >= 0) {
    analogWrite(PIN_MOTOR_LEFT_FORWARD, (motor_speed_left + calibration_left));
    analogWrite(PIN_MOTOR_LEFT_BACKWARD, 0);
  }
  else {
    analogWrite(PIN_MOTOR_LEFT_FORWARD, 0);
    analogWrite(PIN_MOTOR_LEFT_BACKWARD, (motor_speed_left - calibration_left)*(-1));
  }
  if(motor_speed_right >= 0) {
    analogWrite(PIN_MOTOR_RIGHT_FORWARD, (motor_speed_right + calibration_right));
    analogWrite(PIN_MOTOR_RIGHT_BACKWARD, 0);
  }
  else {
    analogWrite(PIN_MOTOR_RIGHT_FORWARD, 0);
    analogWrite(PIN_MOTOR_RIGHT_BACKWARD, (motor_speed_right - calibration_right)*(-1));
  }
}
/*
#include <IRremote.h>
#include <EEPROM.h>  // EEPROM library

#define PIN_IR_RECEIVE 2
#define PIN_MOTOR_LEFT_FORWARD 10
#define PIN_MOTOR_LEFT_BACKWARD 9
#define PIN_MOTOR_RIGHT_FORWARD 6
#define PIN_MOTOR_RIGHT_BACKWARD 5

int motor_speed_left = 0;
int motor_speed_right = 0;

int calibration_left = 0;
int calibration_right = 0;

// ---- EEPROM data layout ----
struct CalibrationData {
  uint16_t magic;   // To validate stored data
  int16_t left;     // calibration_left
  int16_t right;    // calibration_right
};

static const uint16_t CAL_MAGIC = 0xBEEF;
static const int EEPROM_ADDR = 0;

// Save only when changed
int last_saved_left = 0;
int last_saved_right = 0;

// Clamp helper (avoid PWM overflow)
static inline int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void loadCalibrationFromEEPROM() {
  CalibrationData data;
  EEPROM.get(EEPROM_ADDR, data);

  if (data.magic == CAL_MAGIC) {
    // Basic sanity check (your calibration range is 0~30 in current logic)
    if (data.left >= 0 && data.left <= 60 && data.right >= 0 && data.right <= 60) {
      calibration_left = data.left;
      calibration_right = data.right;
      last_saved_left = calibration_left;
      last_saved_right = calibration_right;
      return;
    }
  }

  // If invalid -> reset to defaults and write once
  calibration_left = 0;
  calibration_right = 0;
  last_saved_left = 0;
  last_saved_right = 0;

  CalibrationData initData;
  initData.magic = CAL_MAGIC;
  initData.left = calibration_left;
  initData.right = calibration_right;
  EEPROM.put(EEPROM_ADDR, initData);
}

void saveCalibrationToEEPROMIfChanged() {
  if (calibration_left == last_saved_left && calibration_right == last_saved_right) {
    return; // no change -> no write
  }

  CalibrationData data;
  data.magic = CAL_MAGIC;
  data.left = (int16_t)calibration_left;
  data.right = (int16_t)calibration_right;

  EEPROM.put(EEPROM_ADDR, data);

  last_saved_left = calibration_left;
  last_saved_right = calibration_right;
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

  // Load saved calibration values on boot/reset
  loadCalibrationFromEEPROM();
}

void loop() {
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.protocol == NEC) {
      uint8_t cmd = IrReceiver.decodedIRData.command;

      if (cmd == 0x18) {            // forward
        motor_speed_left = 200;
        motor_speed_right = 200;
      } else if (cmd == 0x52) {     // backward
        motor_speed_left = -200;
        motor_speed_right = -200;
      } else if (cmd == 0x5A) {     // turn right?
        motor_speed_left = 200;
        motor_speed_right = 150;
      } else if (cmd == 0x08) {     // turn left?
        motor_speed_left = 150;
        motor_speed_right = 200;
      } else if (cmd == 0x1C) {     // stop
        motor_speed_left = 0;
        motor_speed_right = 0;
      } else if (cmd == 0x19) {     // reset calibration
        calibration_left = 0;
        calibration_right = 0;
        saveCalibrationToEEPROMIfChanged();
      } else if (cmd == 0x45) {     // left cal = 10
        calibration_left = 10;
        saveCalibrationToEEPROMIfChanged();
      } else if (cmd == 0x46) {     // left cal = 20
        calibration_left = 20;
        saveCalibrationToEEPROMIfChanged();
      } else if (cmd == 0x47) {     // left cal = 30
        calibration_left = 30;
        saveCalibrationToEEPROMIfChanged();
      } else if (cmd == 0x44) {     // right cal = 10
        calibration_right = 10;
        saveCalibrationToEEPROMIfChanged();
      } else if (cmd == 0x40) {     // right cal = 20
        calibration_right = 20;
        saveCalibrationToEEPROMIfChanged();
      } else if (cmd == 0x43) {     // right cal = 30
        calibration_right = 30;
        saveCalibrationToEEPROMIfChanged();
      }
    }

    IrReceiver.resume();
  }

  // ---- Motor output ----
  if (motor_speed_left >= 0) {
    int pwmL = clampInt(motor_speed_left + calibration_left, 0, 255);
    analogWrite(PIN_MOTOR_LEFT_FORWARD, pwmL);
    analogWrite(PIN_MOTOR_LEFT_BACKWARD, 0);
  } else {
    int pwmL = clampInt((-motor_speed_left) + calibration_left, 0, 255);
    analogWrite(PIN_MOTOR_LEFT_FORWARD, 0);
    analogWrite(PIN_MOTOR_LEFT_BACKWARD, pwmL);
  }

  if (motor_speed_right >= 0) {
    int pwmR = clampInt(motor_speed_right + calibration_right, 0, 255);
    analogWrite(PIN_MOTOR_RIGHT_FORWARD, pwmR);
    analogWrite(PIN_MOTOR_RIGHT_BACKWARD, 0);
  } else {
    int pwmR = clampInt((-motor_speed_right) + calibration_right, 0, 255);
    analogWrite(PIN_MOTOR_RIGHT_FORWARD, 0);
    analogWrite(PIN_MOTOR_RIGHT_BACKWARD, pwmR);
  }
}
*/
