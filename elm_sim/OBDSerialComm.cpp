#include "OBDSerialComm.h"


OBDSerialComm::OBDSerialComm(uint32_t baudRate) {
    setBaudRate(baudRate);
    //serial = new SoftwareSerial(rxPin, txPin);
    //serial->begin(getBaudRate());
    Serial2.begin(getBaudRate());
    //serial->setTimeout(SERIAL_READ_TIMEOUT);
    Serial2.setTimeout(SERIAL_READ_TIMEOUT);
    //Serial2.println("SerComs is alive1");
    setToDefaults();
    //while (true) {
      //Serial2.println("Hello");
    //  Serial.println("Hello");
    //  writeTo("Hello\n");
    //  delay(1000);
    //}
}

OBDSerialComm::~OBDSerialComm() {
    //operator delete(serial);
    Serial2.end();
}


void OBDSerialComm::writeEnd() {

    // 1 - write carriage return
    //    writeTo(0x0D);
    writeTo("\r");
    //    writeTo(13);


    // 2- (optional ) write linefeed
    if (lineFeedEnable) {
        writeTo("\n");
        // writeTo(10);
    }

    // 3 - Write prompt
    //    writeTo(0x3E);
    writeTo('>');

    //serial->flush();
    Serial2.flush();
};


void OBDSerialComm::writeEndOK() {
    writeTo("OK");
    writeEnd();
}

void OBDSerialComm::writeEndERROR() {
    writeTo("ERROR");
    writeEnd();
}

void OBDSerialComm::writeEndNoData() {
    writeTo("NO DATA");
    writeEnd();
}

void OBDSerialComm::writeEndUnknown() {
    writeTo("?");
    writeEnd();
}

void OBDSerialComm::setToDefaults() {
    setEcho(true);
    setStatus(READY);
    setWhiteSpaces(true);
    setHeaders(false);
    setLineFeeds(false);
    setMemory(false);
}

void OBDSerialComm::writeTo(char const *response) {
    //serial->write(response);
    Serial2.write(response);
}


void OBDSerialComm::writeTo(uint8_t cChar) {
    //serial->write(cChar);
    Serial2.write(cChar);
}

void OBDSerialComm::writeEndPidTo(char const *response) {
    if (whiteSpacesEnabled) {
        uint8_t len = strlen(response);
        char spacedResponse[len + len / 2 + 1];
        addSpacesToResponse(response, spacedResponse);
        writeTo(spacedResponse);
    } else {
        writeTo(response);
    }
    writeEnd();
}

String OBDSerialComm::readData() {
    char str[40];
    char rxChars[3];
    int serInt = 0;
    int i=0;
    //DEBUG("baudrate:");
    //DEBUG(getBaudRate());
    //DEBUG("serial timeout:");
    //DEBUG(SERIAL_READ_TIMEOUT);
    //Serial2.println("SerComs is alive2");
    //serial->flush(); // temp remove this
    //Serial2.flush();
    //String rxData = serial->readStringUntil(SERIAL_END_CHAR);
    // Read serial chars as they become available.
    // Discard all control chars and white space
    while (Serial2.readBytes(rxChars,1) != 0) {
      serInt = (int) rxChars[0];
      if (serInt == 0x0d)
        break;
      if (serInt >= 0x21 && serInt <= 0x7e) {
        str[i++] = rxChars[0];
      }
    }
    // Special case of lone CR. Return the lone CR, as this is used to trigger a 
    // repeat of the last command.
    if (i == 0 && serInt == 0x0d) {
      str[0] = '\r';
      str[1] = '\0';
    }
    else
      str[i] = '\0';
    String rxData = String(str);
    //String rxData = Serial2.readStringUntil(SERIAL_END_CHAR);
    //DEBUG("OBDSerialComm:readData: text begin");
    //DEBUG(rxData); 
    //DEBUG("OBDSerialComm:readData: text end");
    if (isEchoEnable()) {
        writeTo(rxData.c_str());
    }
    return rxData;
}


void OBDSerialComm::setBaudRate(uint32_t rate) {
    this->boudRate = rate;
}

long OBDSerialComm::getBaudRate() {
    return boudRate;
}

bool OBDSerialComm::isEchoEnable() {
    return echoEnable;
}

void OBDSerialComm::setEcho(bool echo) {
    this->echoEnable = echo;
}

void OBDSerialComm::setStatus(STATUS status) {
    this->status = status;
}

void OBDSerialComm::setLineFeeds(bool status) {
    this->lineFeedEnable = status;
}

void OBDSerialComm::setMemory(bool status) {
    this->memoryEnabled = status;
}

void OBDSerialComm::setWhiteSpaces(bool status) {
    this->whiteSpacesEnabled = status;
}

void OBDSerialComm::setHeaders(bool status) {
    this->headersEnabled = status;
}

void OBDSerialComm::addSpacesToResponse(const char *response, char spacedRes[]) {
    uint8_t len = strlen(response);
    int j = 0;
    for (int i = 0; i < len;) {
        *(spacedRes + j++) = *(response + i++);
        *(spacedRes + j++) = *(response + i++);
        if (i < len) {
            *(spacedRes + j++) = 0x20;
        }
    }
    *(spacedRes + j) = '\0';
}
