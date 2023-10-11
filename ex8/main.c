#include <msp430.h> 


/*
1. Set up the ADC to sample from the temperature sensor (NTC) port (ensure power is provided to the NTC
using P2.7 pin).
2. Write code to sample data from the NTC. Bit shift the 10-bit result to an 8-bit value.
3. Transmit the result using the UART. Place your finger on the NTC and observe the range of values that can
be obtained using the CCS Terminal/PuTTY/MECH 423 Serial Communicator.
4. Write code to turn the EXP board into a digital thermometer, using LED1-LED8 as readouts. At room
temperature, only LED1 should be lit (having both LED1 & LED2 lit is also acceptable). The higher the
temperature, the more LEDs will be lit. Resting your finger on the NTC should light all the LEDs.
5. As a test, place your finger in the NTC for 15 seconds, then release. The LEDs should all light up. After
removing your finger, the LEDs should turn off 1 by 1.
 */

#define X_CH   ADC10INCH_12
#define Y_CH   ADC10INCH_13
#define Z_CH   ADC10INCH_14
#define NTC_CH ADC10INCH_4

volatile unsigned char ax = 0;
volatile unsigned char ay = 0;
volatile unsigned char az = 0;
unsigned char datapacket = 255;

volatile unsigned int temp = 0;
volatile unsigned int tempThresh = 194;
static const int numLEDs = 8;

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

// display temp on LEDs
// LED1-4 is PJ0-3
// LED5-8 is P34-7
void displayTemp()
{
    int lightUpTo = (tempThresh - temp);
    if(lightUpTo < 1)
    {
        lightUpTo = 1;
    }
    if(lightUpTo > numLEDs)
    {
        lightUpTo = numLEDs;
    }

    // PJx = 1<<x * (lightUpTo>=x) x from 1 to 4
    // P3x = 1<<x * (lightUpTo>=x) x from 5 to 8
    PJOUT_L = 0b00001000*(lightUpTo>=4) + 0b00000100*(lightUpTo>=3) + 0b00000010*(lightUpTo>=2) + 0b00000001*(lightUpTo>=1);
    P3OUT   = 0b10000000*(lightUpTo>=8) + 0b01000000*(lightUpTo>=7) + 0b00100000*(lightUpTo>=6) + 0b00010000*(lightUpTo>=5);
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    /////////////////////////////////////////////////
    // Power the Accelerometer and/or NTC
    // configure P2.7 as output high to power the NTC/Accelerometer
    P2DIR |= BIT7;
    P2OUT |= BIT7;

    /////////////////////////////////////////////////
    // Set up ADC
    // set up A4 (NTC) as input for ADC on P1.4
    P1SEL0 |= BIT4;
    P1SEL1 |= BIT4;

    //// set up A12, A13, A14 (accelerometer X, Y, Z) as input for ADC
    //P3SEL0 |= BIT0 + BIT1 + BIT2;
    //P3SEL1 |= BIT0 + BIT1 + BIT2;

    ADC10CTL0 |= ADC10ON    + // turn on ADC
                 ADC10SHT_2 ; // sample & hold 16 clks
    ADC10CTL1 |= ADC10SHP;    // sampling input is sourced from timer
    ADC10CTL2 |= ADC10RES;    // 10-bit conversion results

    /////////////////////////////////////////////////
    // Set up S1 as temperature calibration button
    P4DIR &= ~BIT0;         // P4.0 as input (button S1)
    P4OUT |= BIT0;          // resistor pull up
    P4REN |= BIT0;          // resistor pull up
    P4IES |= BIT0;          // falling edge interrupt
    P4IE |= BIT0;           // enable interrupt

    /////////////////////////////////////////////////
    // Set up LED outputs
    PJDIR |= BIT0 + BIT1 + BIT2 + BIT3;
    P3DIR |= BIT4 + BIT5 + BIT6 + BIT7;
    PJOUT = 0;
    P3OUT = 0;

    /////////////////////////////////////////////////
    // Set up UART
    // Configure P2.0 and P2.1 ports for UART
    P2SEL0 &= ~(BIT0 + BIT1); // redundant
    P2SEL1 |= BIT0 + BIT1;

    UCA0CTL1 = UCSWRST;                 // hold UCA in software reset
    UCA0CTL1 |= UCSSEL0|UCSSEL1;        // set source to SMCLK 1MHz
    UCA0BRW = 104;                      // Baud rate = 9600 from an 1 MHz clock
    UCA0CTL1 &= ~UCSWRST;               // take UCA out of software reset

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
    // enable global interrupt
    _EINT();

    /////////////////////////////////////////////////
    // initialize temperature threshold
    __delay_cycles(100000);
    tempThresh = adcReadChannel(ADC10INCH_4);

    while(1)
    {
        txUART(temp);
        displayTemp();
    }
    return 0;
}

#pragma vector = TIMER0_A0_VECTOR       // CCR0 overflow only
__interrupt void timerA0()
{
    temp = adcReadChannel(ADC10INCH_4);
    //ax = adcReadChannel(X_CH) >> 2;
    //ay = adcReadChannel(Y_CH) >> 2;
    //az = adcReadChannel(Z_CH) >> 2;
    TA0CCTL0 &= ~CCIFG; // clear IFG
}

// Temperature threshold calibration using button S1
#pragma vector = PORT4_VECTOR
__interrupt void P4()
{
    tempThresh = adcReadChannel(ADC10INCH_4);
    P4IFG = 0; // clear interrupt
}
