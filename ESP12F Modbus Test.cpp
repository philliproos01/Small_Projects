#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define SLAVE_ID 1
#define FIRST_REG 1
#define REG_COUNT 20

SoftwareSerial S(D1, D2); // RX, TX
ModbusRTU mb;

void setup() {
  Serial.begin(115200);
  S.begin(9600, SWSERIAL_8N1);
  mb.begin(&S);
  mb.master();
}

void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();

    // Read holding registers
    if (!mb.slave()) {
      Serial.println("Reading holding registers...");
      uint16_t res[REG_COUNT];
      if (mb.readHreg(SLAVE_ID, FIRST_REG, res, REG_COUNT, 1000)) {
        for (int i = 0; i < REG_COUNT; i++) {
          Serial.printf("Register %d: %d\n", FIRST_REG + i, res[i]);
        }
      } else {
        Serial.println("Read failed");
      }

      // Write multiple registers
      Serial.println("Writing multiple registers...");
      uint16_t values[REG_COUNT];
      for (int i = 0; i < REG_COUNT; i++) {
        values[i] = random(0, 65535); // Generate random values
      }
      if (mb.writeHreg(SLAVE_ID, FIRST_REG, values, REG_COUNT, 1000)) {
        Serial.println("Write successful");
      } else {
        Serial.println("Write failed");
      }
    }
  }
  mb.task();
  yield();
}
