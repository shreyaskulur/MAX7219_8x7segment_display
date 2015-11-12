/*
  | Arduino   | MAX7219       |
  | --------- | ------------- |
  | MOSI (11) | DIN (1)       |
  | SCK (13)  | CLK (13)      |
  | I/O (7)   | LOAD/CS (12)  |

 */
#include <SPI.h>   //to communicate with MAX7219
#include <Wire.h>  
#include <Time.h>
#include <DS1307RTC.h>

#define LOAD_PIN 7  //load pin of MAX7219

/*
 * Transfers data to a MAX7219/MAX7221 register.
 * 
 * address The register to load data into
 * value   Value to store in the register
 */
void maxTransfer(int address, int value) {

  // Ensure LOAD/CS is LOW
  digitalWrite(LOAD_PIN, LOW);

  // Send the register address
  SPI.transfer(address);

  // Send the value
  SPI.transfer(value);

  // Tell chip to load in data
  digitalWrite(LOAD_PIN, HIGH);
}

void init_display()
{
  // Set load pin to output
  pinMode(LOAD_PIN, OUTPUT);

  // Reverse the SPI transfer to send the MSB first  
  SPI.setBitOrder(MSBFIRST);
  
  // Start SPI
  SPI.begin();

  // Enable mode B
  maxTransfer(0x09, 0xFF);
  
  // Use lowest intensity
  maxTransfer(0x0A, 0x00);
  
  // Activate display of all 8 digits
  maxTransfer(0x0B, 0x07);
  
  // Turn on chip
  maxTransfer(0x0C, 0x01);
  
  //blank all the digits
  for( int i=1; i<=8; i++)
    maxTransfer(i, 0x0F);
  
}

void print2digits(unsigned int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void setup() {
  
  Serial.begin(9600);
  Serial.println("DS1307RTC + MAX7219 DISPLAY");
  Serial.println("---------------------------");  
  init_display(); //initialise display
}

int date=0;

void loop() {
  
  tmElements_t tm;
  
  if (RTC.read(tm)) 
  {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
    if(date%17<5) //display time for 5 secs
    {
      //blank all the digits
      for( int i=1; i<=8; i++)
        maxTransfer(i, 0x0F);
      //display clock in 24hrs format
      maxTransfer( 0x03, tm.Hour/10 );
      maxTransfer( 0x04, tm.Hour%10 );
      maxTransfer( 0xF5, tm.Minute/10 );
      maxTransfer( 0x06, tm.Minute%10 );
      //maxTransfer( 0x07, tm.Second/10 );
      //maxTransfer( 0x08, tm.Second%10 );
    }
    else //display date for 2 seconds
    {
      //blank all the digits
      for( int i=1; i<=8; i++)
        maxTransfer(i, 0x0F);
      //display date
      maxTransfer( 0x01, tm.Day/10);   
      maxTransfer( 0x02, tm.Day%10);
      maxTransfer( 0x04, tm.Month/10);
      maxTransfer( 0x05, tm.Month%10);
      maxTransfer( 0x07, (tmYearToCalendar(tm.Year)%100)/10);
      maxTransfer( 0x08, tmYearToCalendar(tm.Year)%10);   
    }
  } 
  else //if time is not available
  {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  }
  date++; //to handle switching between date and time
  delay(1000);
}  

