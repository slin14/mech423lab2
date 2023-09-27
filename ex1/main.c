#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// set up SMCLK to run on the DCO
    CSCTL0_H = 0xA5;            // CSKEY password
    CSCTL2 |= BIT5 + BIT4;
    CSCTL2 &= ~ BIT6;

    // configure the DCO to run at 8 MHz
    CSCTL1 |= BIT2 + BIT1;
    CSCTL1 &= ~BIT7;

    // set up SMCLK with a divider of 32
    CSCTL3 |= BIT6 | BIT4;
    CSCTL3 &= ~ BIT5;

    // set up P3.4 as output and set it to SMCLK
    P3DIR |= BIT4;
    P3SEL1 |= BIT4;
    P3SEL0 |= BIT4;

	return 0;
}
