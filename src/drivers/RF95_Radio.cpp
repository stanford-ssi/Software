/*
 * Most recent update: Jan 17, 2025 by Sam C.
 * Fixed function definitions to be consistent with header file and resolved compilation issues.
 * This should allow the main.cpp file to compile and run on any platform.
 * The issues with RF module initialization still persists.
 */

#include "RF95_Radio.h"

// Custom pinout for teensy 4.1
#define RFM95_CS     10 // CS on module
#define RFM95_INT    0 // G0 on module
#define RFM95_RST    1 // RST on module
#define RFM95_IRQN   RFM69_INT

/*
 * the rest of the pins (Teensy --> Radio Module):
 * 3V3 --> Vin
 * G --> GND
 * EN can be left unconnected (defaults to be shorted with Vin)
 * SCK --> 13
 * MISO --> 12
 * MOSI --> 11
 */

#define BAUD_RATE 9600
  
// Constructor
RF95_Radio::RF95_Radio(){
    Serial.begin(BAUD_RATE);

    RHHardwareSPI hardware_spi;
    RH_RF95 rf95 = RH_RF95(RFM95_CS, RFM95_INT, hardware_spi);  
    RF95_FREQ = 915.0;
    
    // this is not part of the original code and does nothing at the moment.
    // pin 1 is RFM95_RST (reset pin), which, when pulled low, should activate the radio module
    // but here it's not working for some reason
    digitalWrite(1, LOW);   // test

    // Initialize RF95
    // THIS IS WHERE THE CODE IS FAILING
    while (!rf95.init()) {
        Serial.println("LoRa radio init failed");
        Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    }
    Serial.println("LoRa radio init OK!");

    // Initialize frequency
    while (!rf95.setFrequency(RF95_FREQ)){
        Serial.println("Frequency was not set, error!"); 
    }
    Serial.println("Succesfully set frequency to: ");
    Serial.print(RF95_FREQ);

    // Setting Transmit power
    rf95.setTxPower(23, false);

    /* Note:
        * Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
        * The default transmitter power is 13dBm, using PA_BOOST.
        * If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
        * you can set transmitter powers from 5 to 23 dBm.
        */
}

void RF95_Radio::_getMessage(int bufferSize){
    const int secondBufferSize = 140;
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
        digitalWrite(LED_BUILTIN, HIGH);
        RH_RF95::printBuffer("Received: ", buf, len);
        Serial.print("Got: ");

        Serial.println((char*)buf);

        char secondBuffer[secondBufferSize];
        snprintf(secondBuffer, sizeof(secondBuffer), "The value of the received message is: %s\n", (char*)buf);

        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);

        _sendACK();
    } else {
        Serial.println("Receive failed!");
    } 
}

void RF95_Radio::_sendMessage(uint8_t packetLength, std::string message){
    const uint8_t NULL_SPACE = 1;

    delay(1000); // Delay added for each transmission
    
    char packet[packetLength + NULL_SPACE] = "";
    for (int i = 0; i < packetLength; i++){
        packet[i] = message[i];
    }
    
    Serial.println("Transmitting...");
    

}

void RF95_Radio::_changeFrequency(double freq){
        RF95_FREQ = freq;
    if (!rf95.setFrequency(RF95_FREQ)) {
        Serial.println("Setting new frequency failed!");
    }
}

void RF95_Radio::_sendACK(){
    // ACK message
    uint8_t data[] = "And hello back to you";
    
    // Sending the packet
    rf95.send(data, sizeof(data));
    rf95.waitPacketSent();
    Serial.println("Sent a reply");
    digitalWrite(LED_BUILTIN, LOW);
}