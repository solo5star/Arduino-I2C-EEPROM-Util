#include <Wire.h>

// define your device address
#define DEVICE 0x10

// set your eeprom size
#define SIZE 256

// set address mode
// 1=Using MSB/LSB. Send address to EEPROM twice (MSB(8bit) + LSB(8bit) = 16bit)
// 0=Using Page/Address. (EEPROM Address | 3bit(page) + 8bit = 11bit)
#define IS_LARGE 0

// set eeprom clock. available : 50kHz / 100kHz / 200kHz / 250kHz / 400kHz / 800kHz
#define CLOCK 100000

int SelectedAddress = -1;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(CLOCK);
    
  help();
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    execCommand(command);
  }
}

void help(){
  Serial.println("");
  Serial.println("----------------------------------");
  Serial.println("        Arduino EEPROM Util       ");
  Serial.println("----------------------------------");
  Serial.println("");
  Serial.println("* current configure");
  Serial.print("  SIZE=");
  Serial.println(SIZE);
  Serial.print("  IS_LARGE=");
  Serial.println(IS_LARGE);
  Serial.println("");
  Serial.println("* command help");
  Serial.println("  d - dump eeprom");
  Serial.println("  p - print current address value");
  Serial.println("  t - test function");
  Serial.println("  f[address] - set current address as input");
  Serial.println("  w[value] - write value at current address");
  Serial.println("  h - show help");
  Serial.println("");
}

void execCommand(char c){
  if(c < 'a' || c > 'z'){
    Serial.readString();
    return;
  }
  
  Serial.print("> ");
  Serial.println(c);
  
  switch(c){
    case 'd':
      eepromRead();
      break;
      
    case 'p':
      if(SelectedAddress < 0){
        Serial.println("Select address using command f");
        break;
      }
      Serial.print("Value at ");
      Serial.print(SelectedAddress);
      Serial.print(" is ");
      setCurrentAddress(SelectedAddress);
      printCurrentAddress();
      Serial.println();
      break;
      
    case 'f':
      SelectedAddress = Serial.readString().toInt();
      Serial.print("Set address to ");
      Serial.print(SelectedAddress);
      Serial.print(", value=");
      setCurrentAddress(SelectedAddress);
      printCurrentAddress();
      Serial.println();
      break;

    case 'w':
      if(SelectedAddress < 0){
        Serial.println("Select address using command f");
        break;
      }
      int value;
      value = Serial.readString().toInt();
      Serial.print("Write value ");
      Serial.print(value);
      Serial.print(" to address ");
      Serial.println(SelectedAddress);
      randomWrite(SelectedAddress, value);
      
    case 't':
      test();
      break;
      
    case 'h':
      help();
      break;
      
    default:
      Serial.println("Unknown command.");
      help();
      break;
  }
  Serial.readString(); // clear buffer
  Serial.println();
}

void test(){
  Serial.println("Test function is not defined");
}

// display the content of the eeprom
void eepromRead() {
  Serial.println("------ START DUMP ------");

  unsigned int address;
  for (address = 0; address < SIZE; address++) {
    if(address % 16 == 0){
      if(address != 0) Serial.println();
      Serial.print("0x");
      if(address < 16) Serial.print("0");
      Serial.print(address, HEX);
    }
    Serial.print(" ");
    
    if (printRandomAddress(address) != 0) {
      Serial.print("Read failed at ");
      Serial.print(address, HEX);
      Serial.println("!");
      break;
    }
  }
  Serial.println();
  Serial.println("------ END DUMP ------");
}

// Set current address:
//  master send start condition
//  master send eeprom address + read bit
//  master send data address
//  master send start condition
unsigned int setCurrentAddress(unsigned int address) {
  if(IS_LARGE){
    Wire.beginTransmission(DEVICE);
    Wire.write(byte(address >> 8)); // MSB
  }else{
    Wire.beginTransmission(byte(DEVICE | ((address >> 8) & 0x07)));
  }
  byte size = Wire.write(byte(address & 0xFF)); // LSB

  if (size == 0) {
    Serial.println("Failed to write address");
    return 10;
  }
  byte error = Wire.endTransmission(false);
  if (error == 0) {
    // Serial.println("tranmission success");
  } else if (error == 1) {
    Serial.println("data too long to fit in buffer");
  } else if (error == 2) {
    Serial.println("receive NAK when transmiting address");
  } else if (error == 3) {
    Serial.println("receive NAK when transmiting data");
  } else if (error == 4) {
    Serial.println("other error");
  } else {
    Serial.println("unknown error");
  }

  // return error value
  return error;
}

// Current read:
//   master send eeprom address + read bit
//  device respond with data
//  master send stop condition
unsigned int printCurrentAddress() {
  byte size = Wire.requestFrom(DEVICE, 1);
  if (size == 0) {
  }
  if (Wire.available()) {
    byte rdata = Wire.read();
    if(rdata < 16) Serial.print("0");
    Serial.print(rdata, HEX);
    return 0;
  } else {
    Serial.println("no data available from device");
    return 1;
  }
}

// Random read:
//  1. set current address
//  2. read current address
unsigned int printRandomAddress(unsigned int address) {
  if (setCurrentAddress(address) != 0) {
    Serial.println("failed to set current address");
    return 1;
  }
  delay(50); // wait 50 ms between write and read

  if (printCurrentAddress() != 0) {
    Serial.println("failed to read current address");
    return 2;
  }
  return 0;
}

unsigned int randomWrite(unsigned int address, byte data) {
  if(IS_LARGE){
    Wire.beginTransmission(DEVICE);
    Wire.write(byte(address >> 8)); // MSB
  }else{
    Wire.beginTransmission(byte(DEVICE | ((address >> 8) & 0x07)));
  }
  byte size = Wire.write(byte(address & 0xFF)); // LSB
  
  if (size == 0) {
    Serial.println("Failed to write address");
    return 1;
  }
  size= Wire.write(data); 
  if (size == 0) {
    Serial.println("Failed to write data");
    return 2;
  }

  byte error = Wire.endTransmission();
  if (error == 0) {
    // Serial.println("tranmission success");
  } else if (error == 1) {
    Serial.println("data too long to fit in buffer");
  } else if (error == 2) {
    Serial.println("receive NAK when transmiting address");
  } else if (error == 3) {
    Serial.println("receive NAK when transmiting data");
  } else if (error == 4) {
    Serial.println("other error");
  } else {
    Serial.println("unknown error");
  }

  delay(50); // wait 50 ms, a write cycle

  return error;
}
