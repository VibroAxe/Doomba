// CONFIG SETTINGS
int deadzone = 10;
bool debugging = false;

int leftThrottlePin = 10;
int leftDirectionPin = 11;
int rightThrottlePin = 8;
int rightDirectionPin = 9;

int leftYPin = 21;
int rightXPin = 20;
int rightYPin = 19;
int inhibitorPin = 2;
int tankSteerPin = 3;

// NOT CONFIG SETTINGS
int leftYPulseWidth = 0;
int rightXPulseWidth = 0;
int rightYPulseWidth = 0;
int inhibitorPulseWidth = 0;
int tankSteerPulseWidth = 0;

int leftYRisingEdge = 0;
int rightXRisingEdge = 0;
int rightYRisingEdge = 0;
int inhibitorRisingEdge = 0;
int tankSteerRisingEdge = 0;

bool armed = false;
bool carSteer = true;
bool tankSteer = false;

void setup() {
  pinMode(leftThrottlePin, OUTPUT);
  pinMode(leftDirectionPin, OUTPUT);
  pinMode(rightThrottlePin, OUTPUT);
  pinMode(rightDirectionPin, OUTPUT);

  setPwmFrequency(0, 1);
  setPwmFrequency(1, 1);

  pinMode(leftYPin, INPUT);
  pinMode(rightXPin, INPUT);
  pinMode(inhibitorPin, INPUT);
  pinMode(tankSteerPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(leftYPin), leftY, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rightXPin), rightX, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rightYPin), rightY, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inhibitorPin), inhibitor, CHANGE);
  attachInterrupt(digitalPinToInterrupt(tankSteerPin), tankSteer, CHANGE);

  Serial.begin(9600);
}

void leftY() {
  long timestamp = micros();
  if (digitalRead(leftYPin) == HIGH) {
    leftYRisingEdge = timestamp;
  } else {
    leftYPulseWidth = (uint16_t)(timestamp - leftYRisingEdge);
  }
}

void rightX() {
  long timestamp = micros();
  if(digitalRead(rightXPin) == HIGH) {
    rightXRisingEdge = timestamp;
  } else {
    rightXPulseWidth = (uint16_t)(timestamp - rightXRisingEdge);
  }
}

void rightY() {
  long timestamp = micros();
  if(digitalRead(rightYPin) == HIGH) {
    rightYRisingEdge = timestamp;
  } else {
    rightYPulseWidth = (uint16_t)(timestamp - rightYRisingEdge);
  }
}

void inhibitor() {
  long timestamp = micros();
  if(digitalRead(inhibitorPin) == HIGH) {
    inhibitorRisingEdge = timestamp;
  } else {
    inhibitorPulseWidth = (uint16_t)(timestamp - inhibitorRisingEdge);
  }
}

void tankSteer() {
}

int readPWM(pin) {
  long timestamp = micros();
  if(digitalRead(pin) == HIGH) {
    risingEdge = timestamp;
  } else {
    tankSteerPulseWidth = (uint16_t)(timestamp - tankSteerRisingEdge);
  }
}

void loop() {
  armed = (inhibitorPulseWidth > 1500);
  tankMode = (tankSteerPulseWidth > 1500);

  if(!armed) {
    leftThrottle = 0;
    rightThrottle = 0;
  }
}

void carSteer() {
  int leftY = map(leftYPulseWidth, 1000, 1990, -255, 255);
  if (
    leftY < deadzone
    && leftY > (deadzone*-1)
  ) {
    leftY = 0;
  }

  int rightX = map(rightXPulseWidth, 986, 1990, -255, 255);
  if (
    rightX < deadzone
    && rightX > (deadzone*-1)
  ) {
    rightX = 0;
  }

  int carThrottle = leftY;
  int carSteer = rightX;

  Serial.print(leftThrottle);
  Serial.print(",");
  Serial.print(rightThrottle);
  Serial.print(",");
  if(tankMode) {
    Serial.print(100);
  } else {
    Serial.print(-100);
  }
  Serial.print(",");
  if(armed) {
    Serial.println(100);
  } else {
    Serial.println(-100);
  }
}

void tankSteer() {
  int leftY = map(leftYPulseWidth, 1000, 1990, -255, 255);
  if (
    leftY < deadzone
    && leftY > (deadzone*-1)
  ) {
    leftY = 0;
  }

  int rightY = map(rightYPulseWidth, 986, 1990, -255, 255);
  if (
    rightY < deadzone
    && rightY > (deadzone*-1)
  ) {
    rightY = 0;
  }

  int tankLeft = leftY;
  int tankRight = rightY;

  motors(tankLeft, tankRight);
}

void motors(left, right) {
  bool leftReverse = (left < 0);
  bool rightReverse = (right < 0);

  if (!armed) {
    leftThrottle = 0;
    rightThrottle = 0;
  }

  analogWrite(leftThrottlePin, abs(leftThrottle));
  analogWrite(rightThrottlePin, abs(rightThrottle));

  digitalWrite(leftDirectionPin, leftReverse);
  digitalWrite(rightDirectionPin, rightReverse);

  int leftThrottle = constrain(carThrottle - carSteer, -255, 255);
  int rightThrottle = constrain(carThrottle + carSteer, -255, 255);

  bool leftReverse = (leftThrottle < 0);
  bool rightReverse = (rightThrottle > 0);
}

void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
