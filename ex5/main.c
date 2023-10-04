#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// Configure clocks
    CSCTL0 = 0xA500;                        // Write password to modify CS registers
    CSCTL1 = DCOFSEL0 + DCOFSEL1;           // DCO = 8 MHz
    CSCTL2 = SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // MCLK = DCO, ACLK = DCO, SMCLK = DCO

	// configure P3.4 as output
	P3DIR |= BIT4;
	//P3OUT &= ~BIT4; // clear P3.4

	// configure TB1.1 to output 500 Hz square wave on P3.4
	// 8Mhz/500Hz = 16000   = TB0CCR0  clear output
	//              16000/2 = TB0CCR1  set   output
	TB1CTL |= BIT9; // select source clock as SMCLK (bit 9-8 = 10b)
	TB1CTL |= BIT4; // up mode: Timer counts up to TAxCCR0

	TB1CCTL1 |= BIT6 + BIT5; // set/reset mode
							 // set output when TB1R == TB1CCR0
							 // clr output when TB1R == TB1CCR1
	TB1CCR0 = 16000;
	TB1CCR1 = 8000;

	// set timer output to output on P3.4
	P3SEL0 |= BIT4;

	while(1);
	
	return 0;
}
