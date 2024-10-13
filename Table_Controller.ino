#include <ArduinoBLE.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Update the characteristic to handle 2 bytes (X and Y values)
BLECharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 2);

const int ledPin = LED_BUILTIN; // pin to use for the LED

// Motor 1 (existing)
const int pwmPin1 = 11;
const int in1Pin1 = 8;
const int in2Pin1 = 7;

// Motor 2 (new motor)
const int pwmPin2 = 9;
const int in1Pin2 = 6;
const int in2Pin2 = 5;

// Constants for joystick ranges
const int joystickMin = 0;
const int joystickMax = 4095;
const int joystickCenterX = 2800; // Adjust based on your joystick's idle X value
const int joystickCenterY = 2860; // Adjust based on your joystick's idle Y value
const int deadZone = 50; // Adjust as needed to create a dead zone around the center

void setup() {
  Serial.begin(9600);
  pinMode(pwmPin1, OUTPUT);
  pinMode(in1Pin1, OUTPUT);
  pinMode(in2Pin1, OUTPUT);

  pinMode(pwmPin2, OUTPUT);
  pinMode(in1Pin2, OUTPUT);
  pinMode(in2Pin2, OUTPUT);


  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  // set advertised local name and service UUID (names unchanged)
  BLE.setLocalName("LED"); // Keep as "LED"
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characteristic (initialize with two zeros)
  byte initialValue[2] = {0, 0};
  switchCharacteristic.writeValue(initialValue, 2);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}
void loop() {
  // Listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // If a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // Print the central's MAC address:
    Serial.println(central.address());

    // While the central is still connected to peripheral:
    while (central.connected()) {
      // If the remote device wrote to the characteristic,
      // use the value to control the motors:
      if (switchCharacteristic.written()) {
        // Read the two bytes sent from the joystick (X and Y values)
        byte motorValues[2];
        int bytesRead = switchCharacteristic.readValue(motorValues, 2);

        if (bytesRead == 2) {
          // Read X and Y values (0 - 255)
          int xValue = motorValues[0];
          int yValue = motorValues[1];

          // Map X and Y values to -255 to 255
          int xSpeed = map(xValue, 0, 255, -255, 255);
          int ySpeed = map(yValue, 0, 255, -255, 255);

          // Implement dead zone
          int deadZone = 30; // Adjust the dead zone threshold as needed
          if (abs(xSpeed) < deadZone) xSpeed = 0;
          if (abs(ySpeed) < deadZone) ySpeed = 0;

          // Calculate motor speeds for differential drive
          int leftMotorSpeed = ySpeed + xSpeed;
          int rightMotorSpeed = ySpeed - xSpeed;

          // Constrain motor speeds to -255 to 255
          leftMotorSpeed = constrain(leftMotorSpeed, -255, 255);
          rightMotorSpeed = constrain(rightMotorSpeed, -255, 255);

          // Debugging output
          Serial.print("xValue: ");
          Serial.print(xValue);
          Serial.print(" | yValue: ");
          Serial.print(yValue);
          Serial.print(" | xSpeed: ");
          Serial.print(xSpeed);
          Serial.print(" | ySpeed: ");
          Serial.print(ySpeed);
          Serial.print(" | Left Motor Speed: ");
          Serial.print(leftMotorSpeed);
          Serial.print(" | Right Motor Speed: ");
          Serial.println(rightMotorSpeed);

          // Control Left Motor (Motor 1)
          if (leftMotorSpeed > 0) {
            digitalWrite(in1Pin1, HIGH);
            digitalWrite(in2Pin1, LOW);
          } else if (leftMotorSpeed < 0) {
            digitalWrite(in1Pin1, LOW);
            digitalWrite(in2Pin1, HIGH);
          } else {
            digitalWrite(in1Pin1, LOW);
            digitalWrite(in2Pin1, LOW);
          }
          analogWrite(pwmPin1, abs(leftMotorSpeed));

          // Control Right Motor (Motor 2)
          if (rightMotorSpeed > 0) {
            digitalWrite(in1Pin2, HIGH);
            digitalWrite(in2Pin2, LOW);
          } else if (rightMotorSpeed < 0) {
            digitalWrite(in1Pin2, LOW);
            digitalWrite(in2Pin2, HIGH);
          } else {
            digitalWrite(in1Pin2, LOW);
            digitalWrite(in2Pin2, LOW);
          }
          analogWrite(pwmPin2, abs(rightMotorSpeed));
        }
      }
    }

    // When the central disconnects, print it out:
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}