#include <IRremote.h>
#define PIN_IR_RECEIVE 2
#define PIN_MOTOR_LEFT_FORWARD 10
#define PIN_MOTOR_LEFT_BACKWARD 9
#define PIN_MOTOR_RIGHT_FORWARD 6
#define PIN_MOTOR_RIGHT_BACKWARD 5
#define PIN_LINE_DETECT_RIGHT A0
#define PIN_LINE_DETECT_LEFT A1

#define REF_VALUE_LINE_DETECT 500

int motor_speed_left = 0;
int motor_speed_right = 0;
int is_start_line_tracing = 0;
int detect_value_left = 0;
int detect_value_right = 0;

void process_motor(void);

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

void process_motor(void) {
	if(motor_speed_left >= 0) {
		analogWrite(PIN_MOTOR_LEFT_FORWARD, motor_speed_left);
		analogWrite(PIN_MOTOR_LEFT_BACKWARD, 0);
	}
	else {	
		analogWrite(PIN_MOTOR_LEFT_FORWARD, 0);
		analogWrite(PIN_MOTOR_LEFT_BACKWARD, motor_speed_left*(-1));
	}
	if(motor_speed_right >= 0) {
		analogWrite(PIN_MOTOR_RIGHT_FORWARD, motor_speed_right);
		analogWrite(PIN_MOTOR_RIGHT_BACKWARD, 0);
	}
	else {	
		analogWrite(PIN_MOTOR_RIGHT_FORWARD, 0);
		analogWrite(PIN_MOTOR_RIGHT_BACKWARD, motor_speed_right*(-1));
	}
}

void loop() {
	if(IrReceiver.decode() == true) {
		if(IrReceiver.decodedIRData.protocol == NEC) {
			if(IrReceiver.decodedIRData.command == 0x16) {
				is_start_line_tracing = 1;
			}
			else if(IrReceiver.decodedIRData.command == 0x0D) {
				is_start_line_tracing = 0;
			}
		}
		IrReceiver.resume();
	}
	if(is_start_line_tracing == 0) {
		motor_speed_left = 0;
		motor_speed_right = 0;
	}
	else {
		detect_value_left = analogRead(PIN_LINE_DETECT_LEFT);
		detect_value_right = analogRead(PIN_LINE_DETECT_RIGHT);
		if((detect_value_left > REF_VALUE_LINE_DETECT) && (detect_value_right > REF_VALUE_LINE_DETECT)) {
			motor_speed_left = 150;
			motor_speed_right = 150;
		}
		else if((detect_value_left > REF_VALUE_LINE_DETECT) && (detect_value_right < REF_VALUE_LINE_DETECT)) {
			motor_speed_left = -255;
			motor_speed_right = 150;
		}
		else if((detect_value_left < REF_VALUE_LINE_DETECT) && (detect_value_right > REF_VALUE_LINE_DETECT)) {
			motor_speed_left = 150;
			motor_speed_right = -255;
		}
		else {
			motor_speed_left = -255;
			motor_speed_right = -255;
		}
	}
	process_motor();
}
