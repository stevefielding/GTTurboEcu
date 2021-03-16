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
    rxIndex = 0;
    txInPtr = 0;
    txOutPtr = 0;
}

OBDSerialComm::~OBDSerialComm() {
    //operator delete(serial);
    Serial2.end();
}

void OBDSerialComm::serStreamWrite() {
    char txChar;
    int serTxSpace = Serial2.availableForWrite();
    while (serTxSpace > 0) {
      serTxSpace--;
      if (txOutPtr == txInPtr)
        break;
      txChar = txFifo[txOutPtr++];
      if (txOutPtr == MAX_RX_TX_CMD_LEN)
        txOutPtr = 0;
      Serial2.write(txChar);
    }
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
    setLineFeeds(true);
    setMemory(false);
}

void OBDSerialComm::writeTo(char const *response) {
    //serial->write(response);
    //Serial2.write(response);
    int i;
    char txChar;
    for (i = 0; i < MAX_RX_TX_CMD_LEN; i++) {
      txChar = response[i];
      if (txChar == '\0')
        break;
      txFifo[txInPtr++] = txChar;
      if (txInPtr == MAX_RX_TX_CMD_LEN)
        txInPtr = 0;
    }
}


void OBDSerialComm::writeTo(uint8_t cChar) {
    //serial->write(cChar);
    //Serial2.write(cChar);
    txFifo[txInPtr++] = cChar;
    if (txInPtr == MAX_RX_TX_CMD_LEN)
      txInPtr = 0;
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
    String rxData;
    int serInt = 0;

    // Read serial data whilst available
    while (Serial2.available() > 0) {
      serInt = Serial2.read();
      if (serInt == 0x0d)
        break;
      if (serInt >= 0x21 && serInt <= 0x7e)
        rxString[rxIndex++] = (char) serInt;
    }
    // In most cases the CR, which terminates the serial stream, is discarded. However,
    // there is a special case of a lone CR. Return the lone CR, as this is used to trigger a 
    // repeat of the last command.
    // Check for CR command terminator
    if (serInt == 0x0d) {
      if (rxIndex == 0) {
        rxString[0] = '\r';  // keep lone CR
        rxString[1] = '\0';  // and add null terminator
      }
      else {
        rxString[rxIndex] = '\0';    // add null terminator
      }
      rxData = String(rxString);     // Return the string
      rxIndex = 0;                   // and reset the index ready for the next command
      if (isEchoEnable()) {
        writeTo((rxData + '\r').c_str());
    }
    }
    // CR has not been received, so return an empty string
    else {
      rxData = "";
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

bool OBDSerialComm::isHeaderEnable() {
    return headersEnabled;
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
