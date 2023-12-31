#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
    int i;
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Configure PJ.0, PJ.1, PJ.2, PJ.3, P3.4, P3.5, P3.6, and P3.7 as digital outputs
    PJDIR |= BIT3 + BIT2 + BIT1 + BIT0;
    P3DIR |= BIT7 + BIT6 + BIT5 + BIT4;

    // Set the LEDs 1 to 8 to output 10010011
    PJOUT |= BIT1 + BIT0;
    PJOUT &= ~(BIT3 + BIT2);

    P3OUT |= BIT7 + BIT4;
    P3OUT &= ~(BIT6 + BIT5);

    while (1) {
        PJOUT ^= (BIT3 + BIT2);
        P3OUT ^= (BIT6 + BIT5);
        for (i = 0; i < 20000; i++) {
            _NOP();
        }
    }

    return 0;
}
