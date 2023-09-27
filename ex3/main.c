#include <msp430.h> 


/**
 * main.c - ex3
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// configure P4.0 as input (connected to SW1)
	P4DIR &= ~BIT0;

	// enable internal pullup resistor for P4.0
	P4REN |= BIT0;          // pullup: OPEN = 1, CLOSE = 0

	// set P4.0 to get interrupted from a rising edge
	P4IE |= BIT0;           // enable interrupt
	P4IES &= ~BIT0;         // select rising edge
	P4IFG &= ~BIT0;

	// configure P3.7 as output (connected to LED8)
    P3DIR |= BIT7;
    P3OUT &= 0x0000;

	// enable global interrupt
	__bis_SR_register(GIE);
	//_EINT();

	while (1)
	{
	    //__no_operation();   // For debugger
	}

	return 0;
}

// ISR
#pragma vector=PORT4_VECTOR
__interrupt void s1_rising_edge(void)
{
    P3OUT ^= BIT7;    // toggle P3.7 (LED8)
    P4IFG &= ~BIT0;   // clear IFG
}
