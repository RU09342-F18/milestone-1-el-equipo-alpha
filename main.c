/* Rowan University
 * File: main.c
 * Author: Jan Garcia
 * Course: Intro to Embedded Systems
 * Section: 1
 * Creation Data: 10/18/18
 * Milestone Project 1
 * Application - UART controlled RGB LEDs for MSP430
 * Board - MSP430F5529
 */

#include <msp430.h>


unsigned int byte = 0;
unsigned int size = 0;

void setUart(void);
void setLeds(void);
void setPWM(void);

/* Initialize UART */

void setUart(void)
{
    P3SEL    |=  BIT3;      // UART TX
    P3SEL    |=  BIT4;      // UART RX
    UCA0CTL1 |=  UCSWRST;   // Resets state machine
    UCA0CTL1 |=  UCSSEL_2;  // SMCLK
    UCA0BR0   =  6;         // 9600 Baud Rate
    UCA0BR1   =  0;         // 9600 Baud Rate
    UCA0MCTL |=  UCBRS_0;   // Modulation
    UCA0MCTL |=  UCBRF_13;  // Modulation
    UCA0MCTL |=  UCOS16;    // Modulation
    UCA0CTL1 &= ~UCSWRST;   // Initializes the state machine
    UCA0IE   |=  UCRXIE;    // Enables USCI_A0 RX Interrupt
}



/* Initialize LEDs */

void setLeds(void)
{
    P1DIR |= BIT2;     // P1.2 output
    P1SEL |= BIT2;     // P1.2 to TA0.1
    P2DIR |= BIT0;     // P2.0 output
    P2SEL |= BIT0;     // P2.0 to TA1.1
    P1DIR |= BIT4;     // P1.4 output
    P1SEL |= BIT4;     // P1.4 to TA0.3

    P1SEL &= ~(BIT1 + BIT5); // Disable unused GPIO
    P1DIR &= ~(BIT1 + BIT5);
}



/* Initialize PWM */

void setPWM(void)
{
    TA0CTL = TASSEL_2 + MC_1 + ID_2; // Configure TA0: Upmode using 1MHz clock / 4 = 250k
    TA0CCR0 = 255; // 250k / 255 = ~1kHz, set compare to 255
    TA0CCTL1 = OUTMOD_7; // Set to the output pin (Hardware PWM)
    TA0CCR1  = 255; // Set duty cycle to 0% as default
    TA0CCTL3 = OUTMOD_7;
    TA0CCR3  = 255;

    TA1CTL = TASSEL_2 + MC_1 + ID_2; // Configure TA1
    TA1CCR0 = 255;
    TA1CCTL1 = OUTMOD_7;
    TA1CCR1 = 255;
}

/* Main */

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;

    setUart();
    setLeds();
    setPWM();

    __bis_SR_register(LPM0_bits + GIE); // Set to low power mode  and enable interrupts in timer
}

/* UART */

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  switch(byte)
  {
  case 0:
      size = UCA0RXBUF;                // Save Packet Size
      break;
  case 1:

      TA0CCR1 = 255 - (int)UCA0RXBUF;  // Sets Red PWM
      break;
  case 2:

      TA1CCR1 = 255 - (int)UCA0RXBUF;  // Sets Green PWM
      break;
  case 3:

      TA0CCR3 = 255 - (int)UCA0RXBUF;  // Sets Blue PWM
      while(!(UCA0IFG & UCTXIFG));
          UCA0TXBUF = size - 3;
      break;
  default:
      if(byte > size)
      {
          byte = -1;
          size = 0;
      }
      else
      {
          while(!(UCA0IFG & UCTXIFG));
              UCA0TXBUF = UCA0RXBUF;
      }
      break;
  }
  byte++;
}
