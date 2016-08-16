# CAN-Denial-of-Service

"A Stealth, Selective, Link-layer Denial-of-Service Attack Against Automotive Networks"

Demo video at: https://www.youtube.com/watch?v=PmcqCbRMCCk

Arduino Uno Rev 3 DoS Implementation

Uno R3 specific implementation changes with respect to proposed attack algorithm:
- First RXD falling edge waiting and Attack ISR enabling implemented via another ISR
  (RXD Falling Edge ISR) which monitors the RXD value and triggers on RXD falling edge;
- Buffer is split into two variables due to size requirements reasons;
- In Attack ISR, after dominant bit writing, Attack ISR disables itself for  
  performance optimizations reasons. It is re-enabled by RXD Falling Edge ISR;
- In Attack ISR, slightly increased overwriting time in order to compensate 
  for possible Arduino interrupts timing drifts.
  
Implementation is set to perform the DoS attack against the 2012 Alfa Romeo Giulietta
parking sensors module (identifier 06314018) on CAN B operating at 29 bit / 50 kbps.

CAN target value explaination:
254 -> 11111110 | 832209432 -> 00110001100110101000001000011000

1111111000110001100110101000001000011000

- 1111111 = preceding data EoF/remote EoF/error delimiter/overload
            delimiter + minimum interframe spacing;
- 0 = SoF;
- 00110001100 = Base ID;
- 1 = SRR;
- 1 = IDE;
- 0101000001000011000 = Extended ID + 1 stuff bit.
