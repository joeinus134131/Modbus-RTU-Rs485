/*
 *  Research and Development for MikroAmpere Monitoring with SDcard
 *  modified 14 August 2020
 *  Develop by IDNmakerspace Algorithm Factory
 */

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <ModbusRtu.h>

#define pinModBusTX         5
#define Pin13LED          10

File fileku;
SoftwareSerial pzem(2, 3); // RX, TX
 
byte perintah[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x08, 0x2341};
byte bufferDataModbus[100];
byte *ptr;
 
void setup() {
  pinMode(pinModBusTX, OUTPUT);
 
  Serial.begin(9600);
  Serial.println("INSITUT TEKNOLOGI SUMATERA");
  Serial.println("AMMETER RS485");
  while (!Serial)
  {
    ;
  }
  Serial.println ("Pengenalan SD card...");
  if (!SD.begin(4)) 
  {
    Serial.println("Pengenalan gagal!");
    while (1);
  }
  
  Serial.println("Pengenalan berhasil.");
  digitalWrite(pinModBusTX, HIGH);  
  pzem.write((byte)0x00);         
  pzem.write((byte)0x03);                 
  pzem.write((byte)0x00);                 
  pzem.write((byte)0x00);                   
  pzem.write((byte)0x00);                 
  pzem.write((byte)0x01);         
  pzem.write((byte)0x85);                
  pzem.write((byte)0xDB); 
  Serial.println(" "); 
  
  delay(10);
  digitalWrite(pinModBusTX, LOW);
  digitalWrite(Pin13LED, HIGH);
  pzem.begin(9600);
  ptr = bufferDataModbus;
  
}
 
void loop()
{
  uint16_t crc = calcCRC(perintah, sizeof(perintah));
 
  digitalWrite(pinModBusTX, HIGH);
  delay(1);
  
  pzem.write(perintah, sizeof(perintah));
  pzem.write(lowByte(crc));
  pzem.write(highByte(crc));
  delay(10);
  digitalWrite(pinModBusTX, LOW);
 
  long millisResponModbus = millis() + 1000;
  while (!pzem.available())
  {
    if (millisResponModbus < millis())
    {
      break;//timeout
    }
  }
 
  while (pzem.available())
  {
    byte b = pzem.read();
    *ptr++ = b;
    delay(2);
  }
 
  //if (memcmp(bufferDataModbus, perintah, 2) == 0)
  //{
    ptr = bufferDataModbus;
 
    float tegangan      = ((ptr[0 + 3] << 8) + ptr[1 + 3]) * 0.01;
    float arus          = ((ptr[2 + 3] << 8) + ptr[3 + 3]) * 0.01;
    float daya          = (((uint32_t)ptr[6 + 3] << 24) + ((uint32_t)ptr[7 + 3] << 16) + (ptr[4 + 3] << 8) + ptr[5 + 3]) * 0.1;
 
    memset(bufferDataModbus, 0x00, sizeof(bufferDataModbus));
 
    Serial.println("==========");
    Serial.print("tegangan      = ");
    Serial.println(tegangan);
    Serial.print("arus          = ");
    Serial.println(arus);
    Serial.print("daya          = ");
    Serial.println(daya);
  //}
  tulisdata();
  bacadata();
  
  Serial.println();
  delay(1000);
}
 
uint16_t calcCRC(byte *data, byte panjang)
{
  int i;
  uint16_t crc = 0xFFFF;
  for (byte p = 0; p < panjang; p++)
  {
    crc ^= data[p];
    for (i = 0; i < 8; ++i)
    {
      if (crc & 1)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc = (crc >> 1);
    }
  }
  return crc;
}

void tulisdata()
{
  fileku = SD.open("Hasil.csv", FILE_WRITE);

  if (fileku) 
  {
    Serial.println("Menulis ke file...");
    delay(100);
    
    int tegangan, arus, daya;
    //untuk menulis ke file
    fileku.print("tegangan      = ");
    fileku.print(tegangan);
    fileku.print(",");
    fileku.print("arus          = ");
    fileku.print(arus);
    fileku.print("daya          = ");
    fileku.println(daya);
    delay(1000);
    //tutup file:
    fileku.close();
    Serial.println("Berhasil");
  } 
  else 
  {
    Serial.println("error untuk membuka file csv");
  }
}

void bacadata()
{
  fileku = SD.open("Hasil.csv");
  if (fileku) 
  {
    Serial.println("Data Input : ");
    while (fileku.available()) 
    {
      Serial.write(fileku.read());
    }
    fileku.close();
  } 
  else 
  {
    Serial.println("error untuk membuka file csv");
  }
}
 
