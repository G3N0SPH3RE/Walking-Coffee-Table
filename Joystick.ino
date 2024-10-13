#include <ArduinoBLE.h>

// variables for button and joystick pins
const int buttonPin = 2;
const int yAxisPin = A0;
const int xAxisPin = A2; // Add the X-axis pin

int oldButtonState = LOW;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // configure the button pin as input
  pinMode(buttonPin, INPUT);

  // initialize the Bluetooth® Low Energy hardware
  BLE.begin();

  Serial.println("Bluetooth® Low Energy Central - LED control");

  // start scanning for peripherals
  BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "LED") {
      return;
    }

    // stop scanning
    BLE.stopScan();

    controlLed(peripheral);

    // peripheral disconnected, start scanning again
    BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
  }
}

void controlLed(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the LED characteristic
  BLECharacteristic ledCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

  if (!ledCharacteristic) {
    Serial.println("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    return;
  } else if (!ledCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable LED characteristic!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected()) {
    // while the peripheral is connected

    // read the button pin
    int buttonState = digitalRead(buttonPin);
    
    // read the joystick X and Y axes
    int yValue = analogRead(yAxisPin);
    int xValue = analogRead(xAxisPin);
    Serial.print("Joystick Y-axis RAW: ");
    Serial.print(yValue);
    Serial.print(" | Joystick X-axis RAW: ");
    Serial.print(xValue);
    
    // map both values to 0-255
    byte motorValueY = map(yValue, 0, 4095, 0, 255);
    byte motorValueX = map(xValue, 0, 4095, 0, 255);

    // Debugging output
    Serial.print(" | Joystick Y-axis: ");
    Serial.print(yValue);
    Serial.print(" | Motor value Y: ");
    Serial.print(motorValueY);
    Serial.print(" | Joystick X-axis: ");
    Serial.print(xValue);
    Serial.print(" | Motor value X: ");
    Serial.println(motorValueX);

    // Create a buffer to hold both X and Y values
    byte motorValues[2] = {motorValueX, motorValueY};

    // Send both values over BLE
    ledCharacteristic.writeValue(motorValues, 2);

    delay(100);
  }

  Serial.println("Peripheral disconnected");
}
