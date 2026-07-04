/*
 * ADC Annex Example - Simple Single-Ended and Differential Readings
 * 
 * This example demonstrates how to use the ADC Annex library to read
 * analog values from an ADC chip over SPI.
 * 
 * Hardware Setup:
 * - CS Pin: 10 (SS on Arduino Uno)
 * - SPI Bus: Default Arduino SPI pins (MOSI-11, MISO-12, SCK-13 on Uno)
 * - Reference Voltage: 5V
 */

#include <ADCAnnex.h>

// Create ADC instance with CS pin 10, 5V reference voltage, 2MHz SPI frequency
ADCAnnex adc(10, ADC_VREF_5000MV, 2000000);

void setup() {
    // Initialize Serial for debugging output
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("ADC Annex Example Starting...");
    
    // Initialize the ADC driver
    ADC_Status status = adc.begin();
    if (status != ADC_OK) {
        Serial.println("ERROR: Failed to initialize ADC!");
        while (1);  // Halt
    }
    
    Serial.println("ADC initialized successfully");
    Serial.print("Reference Voltage: ");
    Serial.print(adc.getVref());
    Serial.println(" mV");
    Serial.println();
}

void loop() {
    uint16_t raw_value = 0;
    uint16_t voltage_mv = 0;
    ADC_Status status;
    
    // Print header
    Serial.println("+--------+-------+----------+");
    Serial.println("| Channel| Raw   | Voltage  |");
    Serial.println("+--------+-------+----------+");
    
    // Read all single-ended channels
    for (int ch = ADC_CH0; ch <= ADC_CH3; ch++) {
        // Read raw ADC value
        status = adc.readRaw((ADC_Channel)ch, &raw_value);
        if (status != ADC_OK) {
            Serial.print("CH");
            Serial.print(ch);
            Serial.println(" - Error reading raw value!");
            continue;
        }
        
        // Read voltage in millivolts
        status = adc.readVoltage((ADC_Channel)ch, &voltage_mv);
        if (status != ADC_OK) {
            Serial.print("CH");
            Serial.print(ch);
            Serial.println(" - Error reading voltage!");
            continue;
        }
        
        // Display results
        Serial.print("| CH");
        Serial.print(ch);
        Serial.print("   | ");
        Serial.print(raw_value);
        Serial.print(" | ");
        Serial.print(voltage_mv);
        Serial.println(" mV |");
    }
    
    Serial.println("+--------+-------+----------+");
    
    // Read differential channels
    Serial.println("\nDifferential Readings:");
    Serial.println("+----------+----------+");
    Serial.println("| Channel  | Voltage  |");
    Serial.println("+----------+----------+");
    
    // CH0 - CH1
    status = adc.readDifferentialVoltage(ADC_DIFF_CH01, &voltage_mv);
    Serial.print("| DIFF_0_1 | ");
    Serial.print(voltage_mv);
    Serial.println(" mV |");
    
    // CH1 - CH0
    status = adc.readDifferentialVoltage(ADC_DIFF_CH10, &voltage_mv);
    Serial.print("| DIFF_1_0 | ");
    Serial.print(voltage_mv);
    Serial.println(" mV |");
    
    // CH2 - CH3
    status = adc.readDifferentialVoltage(ADC_DIFF_CH23, &voltage_mv);
    Serial.print("| DIFF_2_3 | ");
    Serial.print(voltage_mv);
    Serial.println(" mV |");
    
    // CH3 - CH2
    status = adc.readDifferentialVoltage(ADC_DIFF_CH32, &voltage_mv);
    Serial.print("| DIFF_3_2 | ");
    Serial.print(voltage_mv);
    Serial.println(" mV |");
    
    Serial.println("+----------+----------+");
    Serial.println();
    
    // Wait before next reading
    delay(2000);
}
