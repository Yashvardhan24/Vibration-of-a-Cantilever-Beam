#include <Wire.h>

// --- ADXL345 Configuration Constants ---
byte ADXL345_I2CADDR = 0x53; 
const byte ADDR_PRIMARY = 0x53;
const byte ADDR_SECONDARY = 0x1D;

#define ADXL345_DEVID 0x00          
#define ADXL345_BW_RATE 0x2C        
#define ADXL345_POWER_CTL 0x2D      
#define ADXL345_DATA_FORMAT 0x31    
#define ADXL345_DATA_REG_START 0x32 

// --- Settings for Sampling ---
const byte DATA_RATE_SETTING = 0x0C; // 400 Hz
const float SCALE_FACTOR = 0.0039;   // Scale factor

void writeRegister(byte address, byte regAddress, byte value);
byte readRegister(byte address, byte regAddress);


void setup() {
  Serial.begin(230400); 
  Wire.begin(); 
  
  // --- Address Scan and Configuration (Same as before) ---
  byte dev_id = readRegister(ADDR_PRIMARY, ADXL345_DEVID);
  
  if (dev_id == 0xE5) {
    ADXL345_I2CADDR = ADDR_PRIMARY;
    Serial.println("SUCCESS! Sensor found at 0x53.");
  } else {
    dev_id = readRegister(ADDR_SECONDARY, ADXL345_DEVID);
    if (dev_id == 0xE5) {
      ADXL345_I2CADDR = ADDR_SECONDARY;
      Serial.println("SUCCESS! Sensor found at 0x1D.");
    } else {
      Serial.println("FATAL ERROR: Sensor not found.");
      while (true); 
    }
  }

  writeRegister(ADXL345_I2CADDR, ADXL345_BW_RATE, DATA_RATE_SETTING); 
  writeRegister(ADXL345_I2CADDR, ADXL345_DATA_FORMAT, 0x0B); 
  writeRegister(ADXL345_I2CADDR, ADXL345_POWER_CTL, 0x08);   

  Serial.println("Configuration Complete. Starting data stream...");
  // *** CRITICAL: Output the CSV header line ***
  Serial.println("Timestamp_micros,Z_Acceleration_g"); 
  delay(100); 
}


void loop() {
  int16_t Z_raw; 
  // CAPTURE TIME
  unsigned long timestamp = micros(); 

  // Read data
  Wire.beginTransmission(ADXL345_I2CADDR);
  Wire.write(ADXL345_DATA_REG_START);
  Wire.endTransmission(false); 
  Wire.requestFrom(ADXL345_I2CADDR, (uint8_t)6, (uint8_t)true); 
  Wire.read(); Wire.read(); Wire.read(); Wire.read(); 
  uint8_t z_lsb = Wire.read();
  uint8_t z_msb = Wire.read();
  
  Z_raw = (z_msb << 8) | z_lsb;
  float Z_g = Z_raw * SCALE_FACTOR;

  // *** OUTPUT CHANGE: Print BOTH columns in CSV format ***
  Serial.print(timestamp);
  Serial.print(",");
  Serial.println(Z_g, 4); 
}

// Utility functions (readRegister and writeRegister) remain the same
byte readRegister(byte address, byte regAddress) {
  byte value = 0;
  Wire.beginTransmission(address); Wire.write(regAddress); Wire.endTransmission(false); 
  Wire.requestFrom(address, (uint8_t)1, (uint8_t)true); 
  if (Wire.available()) { value = Wire.read(); }
  return value;
}

void writeRegister(byte address, byte regAddress, byte value) {
  Wire.beginTransmission(address);
  Wire.write(regAddress);
  Wire.write(value);
  Wire.endTransmission();
}