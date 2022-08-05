#include "Packets.h"

// Heltec WiFi LoRa has the following connections:
// NSS pin:   18 (CS)        GPIO18
// DIO0 pin:  26 (irq)       GPIO26
// RESET pin: 14 (RST 1278)  GPIO14
// DIO1 pin:  35 (clk)       GPIO35
// DIO2 pin:  34 (data)      GPIO34
SX1278 radio = new Module(18, 26, 14, 35);

uint16_t receivedPacket = 0; // count packets
uint16_t goodCRC = 0;  // good checksums

volatile bool receivedFlag = false;    // flag to indicate that a packet was received
volatile bool enableInterrupt = true;  // disable interrupt when it's not needed

// this function is called when a complete packet is received by the module
void ICACHE_RAM_ATTR setFlag(void) 
{
    if ( !enableInterrupt ) {
        return;
    }
    receivedFlag = true;
}

void packetsBegin() {
  // initialize SX1278
  Serial.println("\nInitializing SX1278... ");
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
  Serial.print(F("Starting to listen ...\n"));
  radio.startReceive();
}

bool computeChecksum(uint8_t *array) {
    uint16_t byteSum = array[1] + array[2] + array[3] + array[4] + array[5] + array[6] + array[7];

    if ( (byteSum & 0x7F) == (array[0] & 0x7F) ) {
        goodCRC++;
        return true;
    }
    return false;
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

bool readPacket(TPMS_Packet* packet) {
    bool result = false;

    if ( receivedFlag ) {
        enableInterrupt = false;
        receivedFlag = false;

        byte byteArr[16];
        int state = radio.readData(byteArr, 16);

        if ( state == RADIOLIB_ERR_NONE ) {
            receivedPacket++;

            byte newArr[16];
            shiftBlockRight(byteArr, newArr, 16, 2);      
            decodeManI(newArr, 16);

            if ( computeChecksum(newArr) ) {
                packet->id = newArr[1]<<16 | newArr[2]<<8 | newArr[3];

                float press = (newArr[7] & 0x0F) << 8 | newArr[4];
                packet->pressure = (press / 0.4) / 6.895;  // kPa then psi

                packet->temperature = ((newArr[5] - 50) * 1.8) + 32;  // deg C to F

                packet->low_battery = (newArr[6] & 0x20) != 0;
                packet->fast_leak = (newArr[6] & 0x10) != 0;

                goodCRC++;
                result = true;
            } 
        }
        else {
            Serial.printf("Failed, code ", state);
        }

        enableInterrupt = true;

        radio.startReceive();
    }
    return result;
}
