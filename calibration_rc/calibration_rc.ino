#include <IRremote.h>
#include <EEPROM.h>
#define PIN_IR_RECEIVE 2
#define PIN_MOTOR_LEFT_FORWARD 10
#define PIN_MOTOR_LEFT_BACKWARD 9
#define PIN_MOTOR_RIGHT_FORWARD 6
#define PIN_MOTOR_RIGHT_BACKWARD 5

int16_t motor_speed_left = 0;
int16_t motor_speed_right = 0;

int16_t calibration_left = 0;
int16_t calibration_right = 0;

static const int EEPROM_LEFT_ADDR = 0;
static const int EEPROM_RIGHT_ADDR = 2;

static const int16_t PWM_STEP = 10;
static const int16_t PWM_MIN = -255;
static const int16_t PWM_MAX = 255;


void writeEEPROM (int addr, int16_t value) {
	uint8_t low_byte = (uint8_t)(value & 0xFF);
	uint8_t high_byte = (uint8_t)((value >> 8) & 0xFF);

	EEPROM.update(addr, low_byte);
	EEPROM.update(addr+1, high_byte);
}

int16_t readEEPROM (int addr) {
	uint8_t low_byte = EEPROM.read(addr);
	uint8_t high_byte = EEPROM.read(addr+1);
	return (int16_t)((high_byte << 8) | low_byte);
}

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(PIN_IR_RECEIVE, ENABLE_LED_FEEDBACK);
  pinMode(PIN_MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_BACKWARD, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_BACKWARD, OUTPUT);

  analogWrite(PIN_MOTOR_LEFT_FORWARD, 0);
  analogWrite(PIN_MOTOR_LEFT_BACKWARD, 0);
  analogWrite(PIN_MOTOR_RIGHT_FORWARD, 0);
  analogWrite(PIN_MOTOR_RIGHT_BACKWARD, 0);

  calibration_left = readEEPROM(EEPROM_LEFT_ADDR);
  calibration_right = readEEPROM(EEPROM_RIGHT_ADDR);

  Serial.print("Calibration Left Motor PWM : ");
  Serial.println(readEEPROM(EEPROM_LEFT_ADDR));
  Serial.print("Calibration Right Motor PWM : ");
  Serial.println(readEEPROM(EEPROM_RIGHT_ADDR));
}

void loop() {
  if(IrReceiver.decode() == true) {
    if(IrReceiver.decodedIRData.protocol == NEC) {
		 switch(IrReceiver.decodedIRData.command) {
			 case 0x18: // Forward
				 motor_speed_left = 200;
				 motor_speed_right = 200;
				 break;
			 case 0x52: // Backward
				 motor_speed_left = -200;
				 motor_speed_right = -200;
				 break;
			 case 0x5A: // Right
				 motor_speed_left = 200;
				 motor_speed_right = 150;
				 break;
			 case 0x08: // Left
				 motor_speed_left = 150;
				 motor_speed_right = 200;
				 break;
			 case 0x1C: // Stop
				 motor_speed_left = 0;
				 motor_speed_right = 0;
				 break;
			 case 0x45: // Calibration Left Motor PWM +10
				 calibration_left = (motor_speed_left + calibration_left < 245) ? calibration_left+10 : 0;
				 writeEEPROM(EEPROM_LEFT_ADDR, calibration_left);
				 break;
			 case 0x07: // Calibration Left Motor PWM -10
				 calibration_left = (motor_speed_left - calibration_left > -245) ? calibration_left-10 : 0;
				 writeEEPROM(EEPROM_LEFT_ADDR, calibration_left);
				 break;
			 case 0x47: // Calibration Right Motor PWM +10
				 calibration_right = (motor_speed_right + calibration_right < 245) ? calibration_right+10 : 0;
				 writeEEPROM(EEPROM_RIGHT_ADDR, calibration_right);
				 break;
			 case 0x09: // Calibration Right Motor PWM -10
				 calibration_right = (motor_speed_right - calibration_right > -245) ? calibration_right-10 : 0;
				 writeEEPROM(EEPROM_RIGHT_ADDR, calibration_right);
				 break;
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

