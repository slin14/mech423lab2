#include <msp430.h> 

volatile unsigned int prevCap = 0; // volatile tells compiler the variable value can be modified at any point outside of this code
volatile unsigned int cap = 0;
unsigned short measurement = 0;

/**
 * main.c - ex6
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    //////////////////////////////////////////
    // ex5

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

    //////////////////////////////////////////
    // ex6

    // configure P1.0 as input to timerA TA0.1
    P1DIR &= ~BIT0;
    P1SEL1 &= ~BIT0;
    P1SEL0 |= BIT0;

    // configure Timer A
    TA0CTL   = TASSEL_2 + // Timer A clock source select: 2 - SMCLK
               MC_2     + // Timer A mode control: 2 - Continuous up
               TACLR    ; // Timer A counter clear
    // Note:   TAIE (overflow interrupt is NOT enabled)

    // configure TA0.1
    TA0CCTL1 |= CM_3  + // Capture mode: 1 - both edges
                CAP   + // Capture mode: 1
                SCS   + // Capture synchronize to timer clk (recommended)
                CCIE  + // Capture/compare interrupt enable
                CCIS_0; // Capture input select: 0 - CCIxA

    // enable global interrupt
    _EINT();

    while(1);

    return 0;
}

// ISR for capture from TA0.1
// note: overflow is NOT enabled, so this will NOT fire when TAR overflows
#pragma vector=TIMER0_A1_VECTOR
__interrupt void timerA(void)
{
    if (TA0IV & TA0IV_TACCR1){ // TA0CCR1_CCIFG is set
        cap = TA0CCR1;
        if(!(TA0CCTL1 & CCI)){ // current output is low (it was previously high)
            measurement = cap - prevCap; // save the measurement (time now - starting time)
            // TA0CCR2 = measurement; // save to a register (trying to see it in the debugger)
        }
        else if (TA0CCTL1 & CCI) { // current output is high (it was previously low)
            prevCap = cap; // reset the measurement starting time
        }
    }
    TA0CCTL1 &= ~CCIFG; // clear IFG
}
