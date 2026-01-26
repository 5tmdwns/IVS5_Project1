#include <IRremote.h>

#define PIN_IR_RECEIVE 2
#define PIN_MOTOR_LEFT_FORWARD 10
#define PIN_MOTOR_LEFT_BACKWARD 9
#define PIN_MOTOR_RIGHT_FORWARD 6
#define PIN_MOTOR_RIGHT_BACKWARD 5
#define PIN_LINE_DETECT A0

#define REF_VALUE_LINE_DETECT 300

int motor_speed_left = 0;
int motor_speed_right = 0;
int is_start_line_tracing = 0;
int detect_value = 0;

void processMotor(void);

const int BASE_L = 105;
const int BASE_R = BASE_L + 20;

const int TH_WHITE = 80;
const int TH_BLACK = 300;

float Kp = 0.45f;
float Ki = 0.0f;
float Kd = 2.80f;

float iLimit = 1000.0f;
int deadband = 6;

int steerLimit = 55;

float d_filt = 0.0f;
float d_alpha = 0.15f;

float pid_i = 0.0f;
float prev_error = 0.0f;
unsigned long prev_ms = 0;

static bool on_black = false;

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

  prev_ms = millis();
}

void processMotor(void) {
  motor_speed_left  = constrain(motor_speed_left,  0, 255);
  motor_speed_right = constrain(motor_speed_right, 0, 255);

  analogWrite(PIN_MOTOR_LEFT_FORWARD, motor_speed_left);
  analogWrite(PIN_MOTOR_LEFT_BACKWARD, 0);

  analogWrite(PIN_MOTOR_RIGHT_FORWARD, motor_speed_right);
  analogWrite(PIN_MOTOR_RIGHT_BACKWARD, 0);
}

void resetPID() {
  pid_i = 0.0f;
  prev_error = 0.0f;
  d_filt = 0.0f;
  prev_ms = millis();
}

void loop() {
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.protocol == NEC) {
      if (IrReceiver.decodedIRData.command == 0x16) {
        is_start_line_tracing = 1;
        resetPID();
      } else if (IrReceiver.decodedIRData.command == 0x0D) {
        is_start_line_tracing = 0;
        resetPID();
      }
    }
    IrReceiver.resume();
  }

  if (!is_start_line_tracing) {
    motor_speed_left = 0;
    motor_speed_right = 0;
    processMotor();
    return;
  }

  detect_value = analogRead(PIN_LINE_DETECT);

  if (detect_value >= TH_BLACK) on_black = true;
  else if (detect_value <= TH_WHITE) on_black = false;

  float error = (float)(detect_value - REF_VALUE_LINE_DETECT);

  if (error > -deadband && error < deadband) error = 0.0f;

  unsigned long now = millis();
  float dt = (now - prev_ms) / 1000.0f;
  if (dt <= 0.0f) dt = 0.001f;
  prev_ms = now;

  pid_i += error * dt;
  pid_i = constrain(pid_i, -iLimit, iLimit);

  float d_raw = (error - prev_error) / dt;
  prev_error = error;
  d_filt = (1.0f - d_alpha) * d_filt + d_alpha * d_raw;

  float steer_f = (Kp * error) + (Ki * pid_i) + (Kd * d_filt);

  steer_f = constrain(steer_f, -steerLimit, steerLimit);
  int steer = (int)steer_f;

  int L = BASE_L + steer;
  int R = BASE_R - steer;

  motor_speed_left  = constrain(L, 0, 255);
  motor_speed_right = constrain(R, 0, 255);

  processMotor();

  Serial.print("val=");
  Serial.print(detect_value);
}
