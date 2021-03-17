#include <Arduino.h>
#include "GTTurboEcu.h"
#include "definitions.h"

GTTurboEcu gtTurboEcu;

/**
 * definition of a provider for fake sensor values
 */
uint32_t FakeSensorValueProvider();


void setup() {

    // init serial communication, is only need if there's a need
    //  to read the debug info on serial port of the arduino
    Serial.begin(9600);
    gtTurboEcu.init(9600L);
    pinMode(LED_BUILTIN, OUTPUT);
    //delay(10000);

    /**
     * https://en.wikipedia.org/wiki/OBD-II_PIDs
     *
     * When the OBD client (ex: torque)
     * init's connection it will ask what sensors our "ecu" supports
     *
     * Register the PID(sensors) we are going to support
     */
    gtTurboEcu.registerMode01Pid(0x05);
    gtTurboEcu.registerMode01Pid(0x0B);
    gtTurboEcu.registerMode01Pid(0x0C);
    gtTurboEcu.registerMode01Pid(0x0D);
    gtTurboEcu.registerMode01Pid(0x33);
    gtTurboEcu.registerMode01Pid(0x46);
    gtTurboEcu.registerMode01Pid(0x5C);
    gtTurboEcu.registerMode01Pid(0x70);
    Serial.println("Finished setup");
}

int loopCnt=0;

// Measured 25uS per loop pass. With or without debug prints
// Assume that the scope was unable to capture the occasionally longer passes
// Replaced digitalWrite and removed serial flush from WriteToEnd, and still 25uS per pass.
// I do not understand what is happening.
void loop() {
   
   if (loopCnt == 0) {
      //digitalWrite(LED_BUILTIN, HIGH);
      //PIOB->PIO_SODR = PIO_SODR_P26;  // much faster than digitalWrite(LED_BUILTIN, HIGH);
      loopCnt = 1;
   }
   else {
      //digitalWrite(LED_BUILTIN, LOW);
      //PIOB->PIO_CODR = PIO_CODR_P26;
      loopCnt = 0;
   }

 
    /**
     * The OBD client software (ex: torque),
     * will frequently request values for the PID(sensors)
     * we have register in setup
     *
     * use readPidRequest to listen for client pid request's
     *
     * GTTurboEcu receives Pid Request
     */
    String pidRequest = gtTurboEcu.readPidRequest();

    /**
     * After receiving a PID(sensor) request
     * read the sensor value (in this case is a fake value)
     *
     * and send a response containing the sensor value
     */

    if (!pidRequest.equals("")) {

      /**
       * 0105 Engine coolant temperature
      */
      if (pidRequest.equalsIgnoreCase("0105")) {

          uint8_t numberOfBytes = 1;
          uint32_t sensorValue = FakeSensorValueProvider();

          /**
           * Response parameters example:
           * pidRequest - we send the pid we received to identify it
           * numberOfBytes - the number of bytes this PID value has, see OBDII PID specifications
           * sensorValue - the value of the sensor
           */
          gtTurboEcu.writePidResponse(pidRequest, numberOfBytes, sensorValue);
          return;
      }

      /**
       * 010B intake manifold abs pressure
       */
      if (pidRequest.equalsIgnoreCase("010B")) {
          gtTurboEcu.writePidResponse(pidRequest, 1, FakeSensorValueProvider());
          return;
      }

      /**
       * 010C engine rpm
       */
      if (pidRequest.equalsIgnoreCase("010C")) {
          uint16_t rpmValue = FakeSensorValueProvider() * 100;

          // Note: this time the PID value has two bytes
          gtTurboEcu.writePidResponse(pidRequest, 2, rpmValue);
          return;
      }

      /**
       * 0D Vehicle speed
       */
      if (pidRequest.equalsIgnoreCase("010D")) {
          gtTurboEcu.writePidResponse(pidRequest, 1, FakeSensorValueProvider());
          return;
      }

      /**
       * 33 Absolute Barometric Pressure
       */
      if (pidRequest.equalsIgnoreCase("0133")) {
          gtTurboEcu.writePidResponse(pidRequest, 1, FakeSensorValueProvider());
          return;
      }

      /**
       * 0146 Ambient air temperature
       */
      if (pidRequest.equalsIgnoreCase("0146")) {
          gtTurboEcu.writePidResponse(pidRequest, 1, FakeSensorValueProvider());
          return;
      }

      /**
       * 015C Engine oil temperature
       */
      if (pidRequest.equalsIgnoreCase("015C")) {
          gtTurboEcu.writePidResponse(pidRequest, 1, FakeSensorValueProvider());
          return;
      }

      /**
       * 015C Engine oil temperature
       */
      else if (pidRequest.equalsIgnoreCase("0170")) {
          gtTurboEcu.writePidResponse(pidRequest, 9, FakeSensorValueProvider());
          return;
      }


      /**
       * If pid not implemented, report it as not implemented
       */
      gtTurboEcu.writePidNotSupported();
    }
    gtTurboEcu.serStreamWrite();
     
} // loop end




/**
 * Fake some data
 * start counter = 0
 * every call to getFakeSensorData, will increment the counter to a max of 255 (0xff)
 * When 0xff is reached the cycle is reverted
 * and the counter will decrement by one until 0 is reached
 */
bool isCycleUp = true;
uint32_t cycle = 0;
uint32_t FakeSensorValueProvider() {

    if(isCycleUp) {
        cycle++;
        if(cycle == 0xff) {
            isCycleUp = false;
        }
    } else {
        cycle--;
        if (cycle == 0) {
            isCycleUp = true;
        }
    }

    return cycle;
}
