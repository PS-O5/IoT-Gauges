// pins - voltage on outputs are ~1.5V so we use the ADC but for voltages > 3.3v, digital pins are recommended.
#define DataPin A0
#define ClkPin  A1
#define LedPin  13  //For debugging
#define LATCHER 5   //Latches and sends current value displayed on gauge.

// Dial Indicator resolution: 100 - 0.01mm, 1000 - 0.001mm. Uncommment as per the Least Count offered by your gauge.
//#define Resolution 100
#define Resolution 1000

// UART speed. We are communicating this board as a 1-to-1 communicator with the ESP8266 in this code. For more complex data transfers, SPI or I2C is recommended. Basic comms codes can be found in my git: www.github.com/ps-o5
#define UARTBaudRate 115200

// ADC threshold, ADC values greater than this are interpreted as logical 1, see loop(). An oscilloscope is recommended for byte analysis if the values aren't as per display/requirement.
#define ADC_Threshold 140

// data format. Again, obtained from an oscilloscope.
#define DATA_BITS_LEN 24
#define INCH_BIT 23
#define SIGN_BIT 20
#define START_BIT -1 // -1 - no start bit

// data capture and decode functions
bool getRawBit() {
    bool data;
    while (analogRead(ClkPin) > ADC_Threshold)
        ;
    while (analogRead(ClkPin) < ADC_Threshold)
        ;
    data = analogRead(DataPin) > ADC_Threshold;
    return data;
}

long getRawData() {
    long out = 0;
    for (int i = 0; i < DATA_BITS_LEN; i++) {
        out |= getRawBit() ? 1L << DATA_BITS_LEN : 0L;
        out >>= 1;
    }
    return out;
}

long getValue(bool &inch) {
    long out = getRawData();
    inch = out & (1L << INCH_BIT);
    bool sign = out & (1L << SIGN_BIT);
    out &= (1L << SIGN_BIT) - 1L;
    out >>= (START_BIT+1);
    if (sign)
        out = -out;
    return out;
}

// printing functions
void printBits(long v) {
    char buf[DATA_BITS_LEN + 1];
    for (int i = DATA_BITS_LEN - 1; i >= 0; i--) {
        buf[i] = v & 1 ? '1' : '0';
        v >>= 1;
    }
    buf[DATA_BITS_LEN] = 0;
    Serial.print(buf);
}

void prettyPrintValue(long value, bool inch) {
    double v = value;
#if Resolution == 100
    if (inch) {
        Serial.print(v / 2000, 4);
        Serial.print(" in");
    } else {
        Serial.print(v / 100, 2);
        Serial.print(" mm");
    }
#else
    if (inch) {
        Serial.print(v / 20000, 5);
        Serial.print(" in");
    } else {
        Serial.print(v / 1000, 3);
        Serial.print(" mm");
    }
#endif
}

void toggleLed() {
#ifdef LedPin
  static bool state = false;
  state = !state;
  digitalWrite(LedPin, state);
#endif
}

// Arduino setup and main loop

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void setup() {
    // set ADC prescale to 16 (set ADC clock to 1MHz)
    // this gives as a sampling rate of ~77kSps
    sbi(ADCSRA, ADPS2);
    cbi(ADCSRA, ADPS1);
    cbi(ADCSRA, ADPS0);

    Serial.begin(UARTBaudRate);
    pinMode(LATCHER, INPUT_PULLUP)    //The INPUT_PULLUP can be replaced with an INPUT if 20k to 50k resistors are used between Vcc and Gnd.
#ifdef LedPin
    pinMode(LedPin, OUTPUT);
#endif
}

void loop() {
    bool inch;
    long value;

    value = getValue(inch);

    //If time analysis is required, uncommment below to print time.
    //Serial.print(millis());
    //Serial.print(" ms ");

    // When the button attached as Latcher is pressed then goes high then the current value is send to UART.
    while(!digitalRead(LATCHER)){
      prettyPrintValue(value, inch);
      toggleLed();
      //This if loop foolproofs in case the button is held for longer duration    
      if(!digitalRead(LATCHER)){
        serial.Println();
      }    
            }
  
    // uncomment if you are interested in optimal ADC_Threshold value
    //Serial.print(' '); Serial.print(analogRead(ClkPin)/2);

    // uncomment if you are interested in raw data bits
    //Serial.print(' '); printBits(getRawData());

    // uncomment if you are interested in raw data
    //Serial.print(' '); Serial.print(value);

    Serial.println(); // To generate a blank line. No actual purpose.
}