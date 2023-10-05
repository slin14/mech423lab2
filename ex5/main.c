#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// configure P3.4 as output
	P3DIR |= BIT4;
	// configure P3.5 as output
    P3DIR |= BIT5;

	// set timer output to output on P3.4
	P3SEL0 |= BIT4;
    // set timer output to output on P3.5
    P3SEL0 |= BIT5;

	// configure TB1.1 to output 500 Hz square wave on P3.4
	//TB1CTL |= BIT9; // select source clock as SMCLK (bit 9-8 = 10b)
	//TB1CTL |= BIT4; // up mode: Timer counts up to TAxCCR0
	TB1CTL = TASSEL_2 + MC_1; // SMCLK, UP mode


	TB1CCTL1 |= OUTMOD_7;    // reset/set mode
							 // clr output when TB1R == TB1CCR0
							 // set output when TB1R == TB1CCR1
	TB1CCR0 = 2000;
	TB1CCR1 = 1000; // 50% duty cycle



    // configure TB1.2 to output 500 Hz square wave on P3.4
    TB1CCTL2 |= OUTMOD_7;    // reset/set mode
                             // clr output when TB1R == TB1CCR0
                             // set output when TB1R == TB1CCR2
    TB1CCR2 = 500; // 25% duty cycle


	while(1);
	
	return 0;
}
