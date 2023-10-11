#include <msp430.h> 

#define X_MILLISECONDS 10
#define NTC_CH ADC10INCH_10

// Temperature Sensor Calibration-30 C
//See device datasheet for TLV table memory mapping
#define CALADC10_15V_30C  *((unsigned int *)0x1A1A)

// Temperature Sensor Calibration-85 C
#define CALADC10_15V_85C  *((unsigned int *)0x1A1C)

volatile unsigned int ntc = 0;
volatile unsigned int timerFlag = 0;

/**
 * main.c - ex8
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // configure P2.7 as output high to power the NTC
    P2DIR |= BIT7;
    P2OUT |= BIT7;

    /////////////////////////////////////////////////
    // Set up ADC
    ADC10CTL0 |= ADC10ON    + // turn on ADC
                 ADC10SHT_2 ; // sample & hold 16 clks
    ADC10CTL1 |= ADC10SHP;    // sampling input is sourced from timer
    ADC10CTL2 &= ~ADC10RES;   // 8-bit conversion results

    // Configure ADC10 - Pulse sample mode; ADC10SC trigger
    ADC10MCTL0 = ADC10INCH_10 + // ADC input ch A10 => temp sense
                 ADC10SREF_1  ; // VR+ = VREF, VR- = AVSS

    // Configure internal reference
    while(REFCTL0 & REFGENBUSY);              // If ref generator busy, WAIT
    REFCTL0 |= REFVSEL_0+REFON;               // Select internal ref = 1.5V
    __delay_cycles(400);                      // Delay for Ref to settle

    // sample ADC (enable conversion)
    ADC10CTL0 |= ADC10ENC + // enable conversion
                 ADC10SC  ; // start conversion

    /////////////////////////////////////////////////
    // Set up Timer A to interrupt every X ms (1000/X Hz)
    // configure TA0.1
    TA0CCTL0 |= CCIE;    // Capture/compare interrupt enable
    TA0CCR0 = X_MILLISECONDS * 1000 - 1; // overflow at X ms if 1MHz in

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

    UCA0CTL1  = UCSWRST;                   // Put the UART in software reset
    UCA0CTL1 |= UCSSEL1;                   // Run the UART using SMCLK
    UCA0MCTLW = UCOS16 + UCBRF1 + 0xD600;  // Baud rate = 9600 from an 1 MHz clock
    //UCA0BRW = 104;
    UCA0BRW = 6;
    UCA0CTL1 &= ~UCSWRST;                  // release UART for operation
    //UCA0IE |= UCRXIE;                       // Enable UART Rx interrupt

    /////////////////////////////////////////////////
    // enable global interrupt
    _EINT();

    while(1){
        if (timerFlag == 1) { // every X ms
            while (!(UCA0IFG & UCTXIFG)); // wait until UART not transmitting
            UCA0TXBUF = ntc << 2;         // transmit NTC reading over UART
                                          // left shift for more precision?
            timerFlag = 0;
        }
    }

    return 0;
}

// CCR0 vector for TA0
// note: overflow is NOT enabled, so this will NOT fire when TAR overflows
#pragma vector=TIMER0_A0_VECTOR
// UART transmit the sampled ADC every 40ms
__interrupt void timerA(void)
{
    // wait for ADC conversion complete
    while(ADC10CTL1 & ADC10BUSY);

    // convert result to 8 bit and store
    ntc = ADC10MEM0 << 2;

    timerFlag = 1;      // set timerFlag (so main can use)
    TA0CCTL0 &= ~CCIFG; // clear IFG
}

// ISR for UART Receive
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    unsigned char RxByte = 0;
    RxByte = UCA0RXBUF;                     // Get the new byte from the Rx buffer
    // UART RX IFG self clearing
}

