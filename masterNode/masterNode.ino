 
 
 /** RF24Mesh_Example_Master.ino by TMRh20
  * 
  *
  * This example sketch shows how to manually configure a node via RF24Mesh as a master node, which
  * will receive all data from sensor nodes.
  *
  * The nodes can change physical or logical position in the network, and reconnect through different
  * routing nodes as required. The master node manages the address assignments for the individual nodes
  * in a manner similar to DHCP.
  *
  */
  
  
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>
//Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
#include <EEPROM.h>

#define pi8 0.3927

/***** Configure the chosen CE,CS pins *****/
RF24 radio(7,8);
RF24Network network(radio);
RF24Mesh mesh(radio,network);

uint32_t displayTimer = 0;
uint8_t cnt = 0;

typedef struct {
  uint8_t id;
  float temp;
  float lux;
  uint32_t count;
}NodeBasicData;

typedef struct {
  uint8_t id;
  float temp;
  float lux;
  float gas;
  uint32_t count;
}NodeComplexData;
float a, b, c;
NodeComplexData dummy1 = {100, 25.4, 13.7, 53.7, 0};
NodeComplexData dummy2 = {101, 27.3, 16.5, 49.3, 0};
NodeComplexData dummy3 = {102, 24.8, 18.4, 57.1, 0};
void setup() {
  Serial.begin(57600);

  // Set the nodeID to 0 for the master node
  mesh.setNodeID(0);
  //Serial.println(mesh.getNodeID());
  // Connect to the mesh
  mesh.begin();

}


void loop() {    

  // Call mesh.update to keep the network updated
  mesh.update();
  
  // In addition, keep the 'DHCP service' running on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();
  
  
  // Check for incoming data from the sensors
  if(network.available()){
    RF24NetworkHeader header;
    network.peek(header);
    
    static uint32_t dat=0;
    static NodeBasicData basicData;
    static NodeComplexData complexData;
    switch(header.type){
      // Display the incoming millis() values from the sensor nodes
      case 'M':
        network.read(header,&dat,sizeof(dat));
        Serial.println(dat);
        break;
      case 'B': 
        network.read(header,&basicData,sizeof(basicData));
        Serial.print("node ");
        Serial.print(basicData.id);
        Serial.print(" data ");
        Serial.print(basicData.count);
        Serial.print(" temp ");
        Serial.print(basicData.temp);
        Serial.print(" gas 0.0 lux ");
        Serial.println(basicData.lux);
        break;
      case 'C': 
        network.read(header,&complexData,sizeof(basicData));
        Serial.print("node ");
        Serial.print(complexData.id);
        Serial.print(" data ");
        Serial.print(complexData.count);
        Serial.print(" temp ");
        Serial.print(complexData.temp);
        Serial.print(" gas ");
        Serial.print(complexData.gas);
        Serial.print(" lux ");
        Serial.println(complexData.lux);
        break;
      default: 
        network.read(header,0,0);
        Serial.println(header.type);
        break;
    }
  }
  
  if(millis() - displayTimer > 2000){
    /*displayTimer = millis();
    Serial.println(" ");
    Serial.println(F("********Assigned Addresses********"));
     for(int i=0; i<mesh.addrListTop; i++){
       Serial.print("NodeID: ");
       Serial.print(mesh.addrList[i].nodeID);
       Serial.print(" RF24Network Address: 0");
       Serial.println(mesh.addrList[i].address,OCT);
     }
    Serial.println(F("**********************************"));
    */
    
    displayTimer = millis();
    cnt++;
    float aux1 = cos(cnt*pi8);
    float aux2 = sin(cnt*pi8);
    a = aux1 + aux2;
    b = aux1*aux2;
    c = aux1 - aux2;
    
    Serial.print("node ");
    Serial.print(dummy1.id);
    Serial.print(" data ");
    Serial.print(dummy1.count + cnt);
    Serial.print(" temp ");
    Serial.print(dummy1.temp + a);
    Serial.print(" gas ");
    Serial.print(dummy1.gas + b);
    Serial.print(" lux ");
    Serial.println(dummy1.lux + c);

    Serial.print("node ");
    Serial.print(dummy2.id);
    Serial.print(" data ");
    Serial.print(dummy2.count + cnt);
    Serial.print(" temp ");
    Serial.print(dummy2.temp + c);
    Serial.print(" gas ");
    Serial.print(dummy2.gas + a);
    Serial.print(" lux ");
    Serial.println(dummy2.lux + b);

    Serial.print("node ");
    Serial.print(dummy3.id);
    Serial.print(" data ");
    Serial.print(dummy3.count + cnt);
    Serial.print(" temp ");
    Serial.print(dummy3.temp + b);
    Serial.print(" gas ");
    Serial.print(dummy3.gas + c);
    Serial.print(" lux ");
    Serial.println(dummy3.lux + a);
    
  }
}
