#include <msp430.h> 

/**
 * main.c
 */

volatile int byte = 0;
unsigned volatile int size = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Timer A0 Setup
    TA0CTL |= TASSEL_2 + MC_1;                  // Setup Timer A0 in UP mode w/ SMCLK
    TA0CCR0 = 256;                              // Set TA0CCR0 to 255

    TA0CCTL1 |= OUTMOD_3;                       // Enable Output
    TA0CCR1 = 0x00;                                // Set Duty Cycle to 0%

    TA0CCTL2 |= OUTMOD_3;                       // Enable Output
    TA0CCR2 = 0x00;                                // Set Duty Cycle to 0%

    TA0CCTL3 |= OUTMOD_3;                       // Enable Output
    TA0CCR3 = 0x00;                                // Set Duty Cycle to 0%

    // RGB Setup
    // RED
    P1OUT |= BIT2;                              // Set Pin 1.2 to High
    P1DIR |= BIT2;                              // Set Pin 1.2 to Output
    P1SEL |= BIT2;                              // Enable TA0.1 Output

    // GREEN
    P1OUT |= BIT3;                              // Set Pin 1.3 to High
    P1DIR |= BIT3;                              // Set Pin 1.3 to Output
    P1SEL |= BIT3;                              // Enable TA0.2 Output

    // BLUE
    P1OUT |= BIT4;                              // Set Pin 1.4 to High
    P1DIR |= BIT4;                              // Set Pin 1.4 to Output
    P1SEL |= BIT4;                              // Enable TA0.3 Output

    // UART Setup

    P4SEL |= BIT4+BIT5;
    // TX
    P3SEL |= BIT3;                              // Enable TX on pin 3.3

    // RX
    P3SEL |= BIT4;                              // Enable RX on pin 3.4

    // UART Initialization
    UCA1CTL1 |= UCSWRST;                        // **Put state machine in reset**
    UCA1CTL1 |= UCSSEL_2;
    UCA1BR0 = 104;
    UCA1BR1 = 0;
    UCA1MCTL |= UCBRS_1 + UCBRF_0;
    UCA1CTL1 &= ~UCSWRST;                       // **Initialize USCI state machine**
    UCA1IE |= UCRXIE;                           // Enable Interrupt on RX
    UCA1IFG &= ~UCRXIFG;                        // Clear Interrupt Flag

    __bis_SR_register(GIE);       // Enter LPM0, interrupts enabled

    while(1);

    return 0;
}


#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
  switch(byte)
  {
  case 0:
      size = UCA1RXBUF;                    // Save Packet Size
      break;                                    // Vector 0 - no interrupt
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
