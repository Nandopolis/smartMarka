
/** RF24Mesh_Example.ino by TMRh20

   This example sketch shows how to manually configure a node via RF24Mesh, and send data to the
   master node.
   The nodes will refresh their network address as soon as a single write fails. This allows the
   nodes to change position in relation to each other and the master node.
*/


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>
//#include <printf.h>


/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(7, 8);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/**
   User Configuration: nodeID - A unique identifier for each radio. Allows addressing
   to change dynamically with physical changes to the mesh.

   In this example, configuration takes place below, prior to uploading the sketch to the device
   A unique value from 1-255 must be configured for each node.
   This will be stored in EEPROM on AVR devices, so remains persistent between further uploads, loss of power, etc.

 **/
#define nodeID 1
#define eeAdr 0


uint32_t displayTimer = 0;

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

#define LDRserRes 1200
#define NTCserRes 10000
float getRes(uint16_t adc, uint32_t serRes) {
  float res = serRes/(1023.0/adc - 1.0);
  return res;
}

#define nomTemp 25 + 273.15
#define nomRes 100000
#define B 4057
float getTemp(float res) {
  float temp = 1.0/(1.0/(nomTemp) + log(res/nomRes)/B);
  return temp;
}

float getLux(float res) {
  float lux = 10000/pow(res/100.0, 4.0/3.0 );
  return lux;
}

typedef struct {
  uint8_t id;
  float temp;
  float lux;
  uint32_t count;
}NodeBasicData;

NodeBasicData data = {nodeID, 0.0, 0.0, 0};

void setup() {

  Serial.begin(57600);
  //printf_begin();
  // Set the nodeID manually
  //mesh.setNodeID(nodeID);
  uint8_t id = EEPROM.read(eeAdr);
  if (id == 0) {
    do {
      // Wait for the nodeID to be set via Serial
      if (Serial.available()) {
        uint8_t newID = Serial.read();
        EEPROM.write(eeAdr, newID);
        mesh.setNodeID(newID);
      }
    } while (!mesh.getNodeID());
  }
  else {
    mesh.setNodeID(id);
  }

  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  mesh.begin();
}



void loop() {

  mesh.update();

  // Send to the master node every second
  if (millis() - displayTimer >= 2000) {
    displayTimer = millis();
    data.count = data.count +1;

    uint16_t ldr = analogRead(A0);
    uint16_t ntc = analogRead(A1);

    data.temp = getTemp(getRes(ntc, NTCserRes)) - 273.15;
    data.lux = getLux(getRes(ldr, LDRserRes));

    // Send an 'M' type message containing the current millis()
    if (!mesh.write(&data, 'B', sizeof(data))) {

      // If a write fails, check connectivity to the mesh network
      if ( ! mesh.checkConnection() ) {
        //refresh the network address
        Serial.println("Renewing Address");
        mesh.renewAddress();
      } else {
        Serial.println("Send fail, Test OK");
      }
    } else {
      Serial.print("Send OK: "); Serial.println(displayTimer);
    }
  }

  while (network.available()) {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print("Received packet #");
    Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);
  }
}






