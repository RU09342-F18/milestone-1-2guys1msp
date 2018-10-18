#include <msp430.h> 

volatile int byte = 0;
unsigned volatile int size = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                   // Stop Watchdog Timer

    TA0CTL |= TASSEL_2 + MC_1;                  // Setup Timer A0 in UP mode w/ SMCLK
    TA0CCR0 = 256;                              // Set TA0CCR0 to 255

    TA0CCTL1 |= OUTMOD_3;                       // Enable Output
    TA0CCTL2 |= OUTMOD_3;                       // Enable Output
    TA0CCTL3 |= OUTMOD_3;                       // Enable Output
    
    TA0CCR1 = 0x00;                             // Set Duty Cycle to 0%   
    TA0CCR2 = 0x00;                             // Set Duty Cycle to 0%   
    TA0CCR3 = 0x00;                             // Set Duty Cycle to 0%

    // RED
    P1SEL |= BIT2;                              // Enable TA0.1 Output
    P1OUT |= BIT2;                              // Set Pin 1.2 to High
    P1DIR |= BIT2;                              // Set Pin 1.2 to Output

    // GREEN
    P1SEL |= BIT3;                              // Enable TA0.2 Output
    P1OUT |= BIT3;                              // Set Pin 1.3 to High
    P1DIR |= BIT3;                              // Set Pin 1.3 to Output

    // BLUE
    P1SEL |= BIT4;                              // Enable TA0.3 Output
    P1OUT |= BIT4;                              // Set Pin 1.4 to High
    P1DIR |= BIT4;                              // Set Pin 1.4 to Output
   
    // UART Setup

    P4SEL |= BIT4+BIT5;
    P3SEL |= BIT3;                              // Enable TX on pin 3.3
    P3SEL |= BIT4;                              // Enable RX on pin 3.4

    // UART Initialization
    
    UCA1CTL1 |= UCSWRST;                        // Puts State Machine in Reset
    UCA1CTL1 |= UCSSEL_2;
    
    UCA1BR0 = 104;
    UCA1BR1 = 0;
    
    UCA1MCTL |= UCBRS_1 + UCBRF_0;
    
    UCA1CTL1 &= ~UCSWRST;                       // Initialize USCI state machine
    UCA1IE |= UCRXIE;                           // Enable Interrupt on RX
    UCA1IFG &= ~UCRXIFG;                        // Clear Interrupt Flag

    __bis_SR_register(GIE);                     // Enter LPM0, interrupts enabled

    while(1);

    return 0;
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
  switch(byte)
  {
  case 0:
      size = UCA1RXBUF;                         // Save Packet Size
      break;                                   
  case 1:
          
      TA0CCR1 = UCA1RXBUF;                      // Sets Red PWM
      break;
  case 2:
          
      TA0CCR2 = UCA1RXBUF;                      // Sets Green PWM
      break;
  case 3:
          
      TA0CCR3 = UCA1RXBUF;                      // Sets Blue PWM
      while(!(UCA1IFG & UCTXIFG));
          UCA1TXBUF = size - 3;
      break;
  default:
      if(byte > size)
      {
          byte = -1;
          size = 0;
      }
      else
      {
          while(!(UCA1IFG & UCTXIFG));
              UCA1TXBUF = UCA1RXBUF;
      }
      break;
  }
  byte++;
}
