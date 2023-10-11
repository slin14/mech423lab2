#include <msp430.h> 

#define X_CH ADC10INCH_12
#define Y_CH ADC10INCH_13
#define Z_CH ADC10INCH_14

volatile unsigned char ax = 0;
volatile unsigned char ay = 0;
volatile unsigned char az = 0;
unsigned char datapacket = 255;

void txUART(unsigned char in)
{
    while (!(UCA0IFG & UCTXIFG)); // wait until UART not transmitting
    UCA0TXBUF = in;
}

unsigned int adcReadChannel(int channel)
{
    ADC10MCTL0 |= channel;  // channel select; Vref=AVCC
    // sample ADC (enable conversion)
    ADC10CTL0 |= ADC10ENC + // enable conversion
                 ADC10SC  ; // start conversion

    // wait for ADC conversion complete
    while(ADC10CTL1 & ADC10BUSY);
    int result = ADC10MEM0;

    // ADC disable conversion to switch channel
    ADC10CTL0 &= ~ADC10ENC;

    ADC10MCTL0 &= ~channel; // clear channel selection

    return result;
}

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

    ADC10CTL0 |= ADC10ON    + // turn on ADC
                 ADC10SHT_2 ; // sample & hold 16 clks
    ADC10CTL1 |= ADC10SHP;    // sampling input is sourced from timer
    //ADC10CTL2 |= ADC10RES;  // 10-bit conversion results

    /////////////////////////////////////////////////
    // Set up Timer A to interrupt every 40ms (25Hz)
    // configure TA0.1
    TA0CCTL0 |= CCIE;    // Capture/compare interrupt enable
    TA0CCR0 = 40000 - 1; // overflow at 40ms if 1MHz in

    // start Timer A
    TA0CTL  |= TASSEL_2 + // Timer A clock source select: 2 - SMCLK
               MC_1     + // Timer A mode control: 1 - Up Mode
               TACLR    ; // clear TA0R
    // Note:   TAIE (overflow interrupt is NOT enabled)

    /////////////////////////////////////////////////
    // Set up UART
    // Configure P2.0 and P2.1 ports for UART
    P2SEL0 &= ~(BIT0 + BIT1); // redundant
    P2SEL1 |= BIT0 + BIT1;

    UCA0CTL1 = UCSWRST;                 // hold UCA in software reset
    UCA0CTL1 |= UCSSEL0|UCSSEL1;        // set source to SMCLK 1MHz
    UCA0BRW = 104;                      // Baud rate = 9600 from an 1 MHz clock
    UCA0CTL1 &= ~UCSWRST;               // take UCA out of software reset
    //UCA0IE |= UCRXIE;                       // Enable UART Rx interrupt

    /////////////////////////////////////////////////
    // enable global interrupt
    _EINT();

    while(1){
        txUART(datapacket);
        txUART(ax);
        txUART(ay);
        txUART(az);
    }

    return 0;
}

// CCR0 vector for TA0
// note: overflow is NOT enabled, so this will NOT fire when TAR overflows
#pragma vector=TIMER0_A0_VECTOR
// UART transmit the sampled ADC every 40ms
__interrupt void timerA(void)
{
    ax = adcReadChannel(X_CH) >> 2;
    ay = adcReadChannel(Y_CH) >> 2;
    az = adcReadChannel(Z_CH) >> 2;

    TA0CCTL0 &= ~CCIFG; // clear IFG
}

/////////////////////////////////////////////////
// ISR for UART Receive
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    unsigned char RxByte = 0;
    RxByte = UCA0RXBUF;                     // Get the new byte from the Rx buffer
    // UART RX IFG self clearing
}
