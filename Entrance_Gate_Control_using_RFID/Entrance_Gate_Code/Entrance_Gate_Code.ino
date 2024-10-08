#include <SPI.h>                // Include the SPI library for communication
#include <MFRC522.h>           // Include the MFRC522 library for RFID
#include <ESP32Servo.h>        // Include the ESP32-compatible Servo library

// Define the pins for RFID and Servo
#define RST_PIN 22             // Reset pin for RFID
#define SS_PIN  21             // Slave Select pin for RFID
#define SERVO_PIN 4            // Define the pin connected to the servo motor

MFRC522 rfid(SS_PIN, RST_PIN);  // Create an instance of the MFRC522 class
Servo gateServo;                 // Create an instance of the Servo class

// Define the authorized RFID card UID
byte authorizedUID[] = {0xFA, 0x89, 0xAF, 0x02}; // Replace with your RFID card UID

// Variables to store entry and exit times
unsigned long inTime = 0;       // Variable to store the in-time (when the card is scanned)
unsigned long outTime = 0;      // Variable to store the out-time (when the card is scanned)

bool hasEntered = false;         // Flag to track if the vehicle has entered

void setup() {
  Serial.begin(9600);           // Start serial communication at 9600 baud rate
  SPI.begin();                  // Initialize the SPI bus
  rfid.PCD_Init();              // Initialize the RFID reader
  
  // Initialize the servo and set it to the closed position
  gateServo.attach(SERVO_PIN);  // Attach the servo to the defined pin
  gateServo.write(0);           // Set the servo to 0 degrees (closed gate)
}

void loop() {
  // Look for an RFID card
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.print("Card UID: ");  // Print the detected card UID
    bool isAuthorized = true;     // Flag to check if the card is authorized

    // Compare scanned UID with the authorized UID
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i], HEX); // Print each byte of the UID in HEX format
      Serial.print(" ");
      
      // Check if the scanned UID matches the authorized UID
      if (rfid.uid.uidByte[i] != authorizedUID[i]) {
        isAuthorized = false; // If any byte does not match, set isAuthorized to false
      }
    }
    Serial.println(); // Print a new line after displaying UID

    // If the card is authorized, handle entry or exit
    if (isAuthorized) {
      Serial.println("Authorized card detected."); // Notify that the card is authorized

      if (!hasEntered) {
        // Entry case
        inTime = millis() / 60000; // Capture in-time in minutes since the program started
        Serial.print("In-Time (minutes since start): ");
        Serial.println(inTime); // Print in-time

        hasEntered = true; // Set flag indicating vehicle has entered

        // Open the gate for entry
        openGate(); // Call the function to open the gate
      } else {
        // Exit case
        outTime = millis() / 60000; // Capture out-time in minutes since the program started
        Serial.print("Out-Time (minutes since start): ");
        Serial.println(outTime); // Print out-time

        // Calculate parking fee based on duration
        unsigned long parkingDuration = outTime - inTime; // Calculate duration of parking
        unsigned long parkingFee = parkingDuration * 10;   // Assuming fee rate is 10 units per minute

        Serial.print("Parking Duration (minutes): ");
        Serial.println(parkingDuration); // Print duration
        Serial.print("Parking Fee: ");
        Serial.println(parkingFee); // Print calculated parking fee

        hasEntered = false; // Reset flag, indicating vehicle has exited

        // Open the gate for exit
        openGate(); // Call the function to open the gate
      }
    } else {
      Serial.println("Unauthorized card."); // Notify if the card is unauthorized
    }

    rfid.PICC_HaltA(); // Halt the card to save power
  }
}

// Function to open the gate
void openGate() {
  Serial.println("Opening gate..."); // Print message to indicate the gate is opening
  gateServo.write(90);   // Open the gate (rotate the servo to 90 degrees)
  delay(3000);           // Keep the gate open for 3 seconds
  gateServo.write(0);    // Close the gate (rotate the servo back to 0 degrees)
  Serial.println("Gate closed."); // Print message to indicate the gate is closed
}
