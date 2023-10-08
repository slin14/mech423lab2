#include <msp430.h> 
volatile unsigned int timerFlag = 0;
volatile unsigned char ax = 0;
volatile unsigned char ay = 0;
volatile unsigned char az = 0;

#define X_CH ADC10INCH_12
#define Y_CH ADC10INCH_13
#define Z_CH ADC10INCH_14

/**
 * main.c - ex7
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // configure P2.7 as output high to power the accelerometer
    P2DIR |= BIT7;
    P2OUT |= BIT7;

    /////////////////////////////////////////////////
    // Set up ADC
    // set up the ADC to sample from ports A12, A13, and A14
    P3SEL0 |= BIT0 + BIT1 + BIT2;
    P3SEL1 |= BIT0 + BIT1 + BIT2;

    ADC10CTL0 |= ADC10ON; // turn on ADC

    /////////////////////////////////////////////////
    // Set up Timer A to interrupt every 40ms (25Hz)
    // configure Timer A
    TA0CTL  |= TASSEL_2 + // Timer A clock source select: 2 - SMCLK
               MC_1     ; // Timer A mode control: 1 - Up Mode
    // Note:   TAIE (overflow interrupt is NOT enabled)

    // configure TA0.1
    TA0CCTL1 |= CCIE;    // Capture/compare interrupt enable
    TA0CCR0 = 40000 - 1; // overflow at 40ms if 1MHz in

    /////////////////////////////////////////////////
    // Set up UART
    // Configure P2.0 and P2.1 ports for UART
    P2SEL0 &= ~(BIT0 + BIT1);
    P2SEL1 |= BIT0 + BIT1;

    UCA0CTLW0 |= UCSWRST;                   // Put the UART in software reset
    UCA0CTLW0 |= UCSSEL1;                   // Run the UART using SMCLK
    UCA0MCTLW = UCOS16 + UCBRF3 + 0x2000;   // Baud rate = 9600 from an 1 MHz clock
    UCA0BRW = 6;
    UCA0CTLW0 &= ~UCSWRST;                  // release UART for operation
    UCA0IE |= UCRXIE;                       // Enable UART Rx interrupt

    /////////////////////////////////////////////////
    // TESTING
    // configure P3.4 as output
    P3DIR |= BIT4;
    P3OUT &= ~BIT4;

    // END TESTING
    /////////////////////////////////////////////////

    // enable global interrupt
    _EINT();

    /////////////////////////////////////////////////
    while(1){

        if (timerFlag == 1){ // ISR every 40ms
            P3OUT ^= BIT4; // TESTING

            // UART transmit 255, Ax, Ay, Az, 255 ...

            while (!(UCA0IFG & UCTXIFG)); // wait until UART not transmitting
            UCA0TXBUF = 255;
            while (!(UCA0IFG & UCTXIFG)); // wait until UART not transmitting
            UCA0TXBUF = ax;
            while (!(UCA0IFG & UCTXIFG)); // wait until UART not transmitting
            UCA0TXBUF = ay;
            while (!(UCA0IFG & UCTXIFG)); // wait until UART not transmitting
            UCA0TXBUF = az;

            timerFlag = 0;
        }
    }

    return 0;
}

// ISR for compare from TA0.1
// note: overflow is NOT enabled, so this will NOT fire when TAR overflows
#pragma vector=TIMER0_A1_VECTOR
// Sample ADC every 40ms
__interrupt void timerA(void)
{
    if (TA0IV & TA0IV_TACCR1){ // TA0CCR1_CCIFG is set

        // ADC disable conversion to switch channel
        ADC10CTL0 &= ~ADC10ENC;

        // channel A12 (Ax)
        ADC10MCTL0 |= ADC10INCH3 + ADC10INCH2;
        ADC10MCTL0 &= ~(ADC10INCH1 + ADC10INCH0);
        // sample ADC (enable conversion)
        ADC10CTL0 |= ADC10ENC;
        // wait until ADC is done and store result
        while((ADC10IFG & ADC10IFG0) == 0);
        ax = ADC10MEM0 >> 2;

        // channel A13 (Ay)
        ADC10MCTL0 |= ADC10INCH3 + ADC10INCH2 + ADC10INCH0;
        ADC10MCTL0 &= ~(ADC10INCH1);
        // sample ADC (enable conversion)
        ADC10CTL0 |= ADC10ENC;
        // wait until ADC is done and store result
        while((ADC10IFG & ADC10IFG0) == 0);
        ay = ADC10MEM0 >> 2;

        // channel A14 (Az)
        ADC10MCTL0 |= ADC10INCH3 + ADC10INCH2 + ADC10INCH1;
        ADC10MCTL0 &= ~(ADC10INCH0);
        // sample ADC (enable conversion)
        ADC10CTL0 |= ADC10ENC;
        // wait until ADC is done and store result
        while((ADC10IFG & ADC10IFG0) == 0);
        az = ADC10MEM0 >> 2;


        // set flag for main function
        timerFlag = 1;
    }

    TA0CCTL1 &= ~CCIFG; // clear IFG
}

// ISR for UART Receive
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    unsigned char RxByte = 0;
    RxByte = UCA0RXBUF;                     // Get the new byte from the Rx buffer
    // UART RX IFG self clearing
}

