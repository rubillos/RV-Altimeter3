#include "packets.h"

#include "SimplyAtomic.h"
#include "defs.h"

constexpr uint32_t duplicateTime = 2000;

// https://github.com/cterwilliger/tst_tpms

// Heltec WiFi LoRa has the following connections:
// NSS pin:   18 (CS)        GPIO18
// DIO0 pin:  26 (irq)       GPIO26
// RESET pin: 14 (RST 1278)  GPIO14
// DIO1 pin:  35 (clk)       GPIO35
// DIO2 pin:  34 (data)      GPIO34
SX1278 radio = new Module(18, 26, 14, 35);

uint16_t receivedPacket = 0; // count packets

volatile uint32_t packetCount = 0;

// this function is called when a complete packet is received by the module
void IRAM_ATTR setFlag(void) {
    packetCount++;
}

void PacketMonitor::begin() {
    _packetLog = new PacketBuff(100);

    radio.beginFSK();
    radio.setOOK(true);
    radio.setFrequency(433.92);
    radio.setBitRate(19.2);
    radio.setFrequencyDeviation(50);
    radio.setRxBandwidth(125);
    radio.setCRC(false);
    radio.setAFCAGCTrigger(RADIOLIB_SX127X_RX_TRIGGER_RSSI_INTERRUPT);

    byte syncWord[] = {0xA9};
    radio.setSyncWord(syncWord, 1);

    radio.fixedPacketLengthMode(16);
    radio.disableAddressFiltering();
    radio.setEncoding(RADIOLIB_ENCODING_NRZ);
    radio.setOokThresholdType(RADIOLIB_SX127X_OOK_THRESH_PEAK);
    radio.setOokFixedOrFloorThreshold(0x50);   
    radio.setOokPeakThresholdDecrement(RADIOLIB_SX127X_OOK_PEAK_THRESH_DEC_1_4_CHIP);
    radio.setOokPeakThresholdStep(RADIOLIB_SX127X_OOK_PEAK_THRESH_STEP_1_5_DB);
    radio.setRSSIConfig(RADIOLIB_SX127X_RSSI_SMOOTHING_SAMPLES_8);
    radio.setDio0Action(setFlag);

    // start listening for packets
    Serial.print(F("PacketMonitor: Starting to listen ...\n"));
    radio.startReceive();
}

bool computeChecksum(uint8_t *array, bool show=false) {
    uint16_t byteSum = array[1] + array[2] + array[3] + array[4] + array[5] + array[6] + array[7];
    uint16_t checkSum = ((byteSum & 0xFF00) ? 0x80 : 0x00) | (byteSum & 0x7F);
    bool match = checkSum == array[0];

    if (show) {
        Serial.printf("Checksum - %s: byteSum=0x%X, checkSum=0x%X, byte0=0x%X\n", (match) ? "Match":"FAIL", byteSum, checkSum, array[0]);
    }

    return match;
}

void decodeManI(byte *array, int size) { 
    // array of all possible Manchester I values
    byte manIarray[16] = {0xAA, 0xA9, 0xA6, 0xA5, 0x9A, 0x99, 0x96, 0x95, 0x6A, 0x69, 0x66, 0x65, 0x5A, 0x59, 0x56, 0x55};
    int k = 0;

    // convert each Manchester I byte to it's NRZ equivalent
    for ( int i = 0; i < size; i=i+2 ) { // look at every 2 bytes in the payload
        for ( int j = 0; j < 16; j++ ) {
            if ( array[i] == manIarray[j] ) { // search for ManI pattern
                array[k] = j << 4; // shift left one nibble, put back into passed array
            }
        } 

        for ( int j = 0; j < 16; j++ ) {
            if ( array[i+1] == manIarray[j] ) {
                array[k] = array[k] | j; // combine previous nibble with this one
            }
        }
        k++;
    }
}

// shiftBlockRight: bit shift entire payload right by 2 bits and pre-pend 0b01 to restore what the packet engine removed (sync word)
void shiftBlockRight(byte *inBytes, byte *outBytes, int size, short bitShift) {
    for (int index=0; index < size; index++) {
        byte newByteMSB = (byte)(inBytes[index] >> bitShift);
        byte newByteLSB = (byte)((index==0)?((byte)0x01):(inBytes[index-1]));       
        newByteLSB = (byte) (newByteLSB << (8-bitShift));
        outBytes[index] = (byte) ( newByteMSB | newByteLSB);
    }
}

constexpr uint32_t minFakeTime = 2500;
constexpr uint32_t maxFakeTime = 5000;

void PacketMonitor::queueNextFake() {
    if (_doFakePackets) {
        _fakePacketTime = 0;
        _fakePacketDelay = random(minFakeTime, maxFakeTime);
    }
}

void PacketMonitor::setFakePackets(bool doFakes) {
    _doFakePackets = doFakes;
    queueNextFake();
}

constexpr uint32_t fakeIDs[] = { 0xA2AF7F, 0xA29D51, 0xA2AFC7, 0x75EBD7, 0xA37072, 0x75E6FD, 0x223344, 0x887766 };
float pressureList[] = { 90, 95, 90, 100, 105, 76, 124 };
float tempList[] = { 65, 70, 80, 85, 80, 150, 80, 30 };

void PacketMonitor::makeFakePacket(TPMSPacket* packet) {
    static int16_t idIndex = 0;
    static float tireP[] = { 0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35 };

    packet->timeStamp = millis();
    packet->rssi = random(-50, -3);
    packet->id = fakeIDs[idIndex];
    packet->pressure = sequenceInterp(pressureList, countof(pressureList), tireP[idIndex]);
    packet->temperature = sequenceInterp(tempList, countof(tempList), tireP[idIndex]);
    packet->lowBattery = false;
    packet->fastLeak = false;
    packet->duplicateCount = random(0, 5) < 4 ? 1:2;

    tireP[idIndex] += 0.01;
    if (tireP[idIndex]>=1.0) {
        tireP[idIndex] -= 1.0;
    }
    idIndex = (idIndex+1) % countof(fakeIDs);
}

bool packetsEqual(TPMSPacket* p1, TPMSPacket* p2) {
    return p1->id==p2->id && p1->pressure==p2->pressure && p1->temperature==p2->temperature && p1->lowBattery==p2->lowBattery && p1->fastLeak==p2->fastLeak;
}

bool PacketMonitor::getPacket(TPMSPacket* packet) {
    static TPMSPacket lastPacket;
    bool result = false;

    if (_doFakePackets && _fakePacketTime > _fakePacketDelay) {
        makeFakePacket(packet);
        _packetLog->addSample(*packet);
        queueNextFake();
        result = true;
    }

    while (!result && packetCount) {
        // Serial.printf("getPacket: %d packets available\n", packetCount);
        ATOMIC() {
            packetCount--;
        }

        byte byteArr[16];
        int state = radio.readData(byteArr, 16);

        if (state == RADIOLIB_ERR_NONE) {
            // Serial.printf("Received data\n");
            receivedPacket++;

            byte newPacket[16];
            shiftBlockRight(byteArr, newPacket, 16, 2);      
            decodeManI(newPacket, 16);

            if (computeChecksum(newPacket, false)) {
                packet->timeStamp = millis();
                packet->rssi = radio.getRSSI(true);
                packet->duplicateCount = 1;

                packet->id = newPacket[1]<<16 | newPacket[2]<<8 | newPacket[3];

                float press = (newPacket[7] & 0x0F) << 8 | newPacket[4];
                packet->pressure = (press / 0.4) / 6.895;  // kPa then psi

                packet->temperature = ((newPacket[5] - 50) * 1.8) + 32;  // deg C to F

                packet->lowBattery = (newPacket[6] & 0x20) != 0;
                packet->fastLeak = (newPacket[6] & 0x10) != 0;

                if (packet->id!=0 && packet->pressure<180 && packet->temperature>-20 && packet->temperature<180) {
                    TPMSPacket lastPacket = _packetLog->getSample(0);
                    uint32_t timeDiff = packet->timeStamp - lastPacket.timeStamp;

                    if (timeDiff<duplicateTime && packetsEqual(packet, &lastPacket)) {
                        lastPacket.duplicateCount++;
                        _packetLog->replaceSample(0, lastPacket);
                    }
                    else {
                        _packetLog->addSample(*packet);
                        result = true;
                    }
                }
            }
        }
        else {
            Serial.printf("Failed, code ", state);
        }

        radio.startReceive();
    }
    return result;
}

PacketMonitor _packetMonitor;
