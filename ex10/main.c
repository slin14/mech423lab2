#include <msp430.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * main.c
 *
 */

#define PRINTBUFSIZE 4
#define maxSize 50
#define LOWMASK 0x0F
#define HIGHMASK 0XF0

volatile unsigned char buf[maxSize];    // main input buffer acts like a queue
volatile unsigned int head = 0;         // queue head pointer
volatile unsigned int counter = 0;      // queue counter
volatile unsigned int tail = 0;         // tail pointer

volatile int timerBCCR0 = 2000-1;       // default timer b CCR0

volatile unsigned char deQueueOut;      // temporary target for dequeue value
volatile unsigned char command;         // temporary storage for command
volatile unsigned int data;             // temporary storage for data 16bit
volatile unsigned char data_L;          // temporary storage for low data nibble
volatile unsigned char data_H;          // temporary storage for high data nibble
volatile unsigned char esc;             // temporary storage for escape byte
volatile unsigned char byteState = 0;   // byte state tracker


void byteDisplayLED(unsigned char in)
{
    char high = in & ~LOWMASK;
    char low = in & ~HIGHMASK;
    PJOUT &= ~LOWMASK;
    P3OUT &= ~HIGHMASK;
    PJOUT |= low;
    P3OUT |= high;
}

void printUARTint_ASCII(int in)
{
    char printbuffer[PRINTBUFSIZE];
    sprintf(printbuffer, "%d", in);
    unsigned int i;
    for(i = 0; i < PRINTBUFSIZE-1; i++)
    {
        while(!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = printbuffer[i];
    }
}
void printUARTstring(char * in)
{
    unsigned int i;
    for(i = 0; i < strlen(in); i++)
    {
        while(!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = in[i];
    }
}

void waitForUARTbusy()
{
    while((UCA0STATW & UCBUSY));        // wait until UART is done doing whatever it's doing.
}

void deQueue()
{
    if(counter > 0)
    {
       while(!(UCA0IFG & UCTXIFG));
       UCA0TXBUF = buf[head];       // transmit dequeued byte for debugging

       deQueueOut = buf[head];
       if(counter > 1)
       {
           head = (head+1)%maxSize;
       }
       counter--;
    }
    else
    {
       printUARTstring(" Empty ");
    }

}

void enQueue(unsigned char in)
{
    if(counter != 0)
    {
        tail = (tail+1)%maxSize;
    }
    if(tail==head && counter != 0)
    {
       printUARTstring(" Overflow ");
       head = (head+1)%maxSize;
    }
    buf[tail] = in;
    counter = ++counter > maxSize ? maxSize : counter;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    PJDIR |= BIT0 + BIT1 + BIT2 + BIT3; // LED display bits
    P3DIR|= BIT4 + BIT5 + BIT6 + BIT7;


    CSCTL0_H = CSKEY_H;                 // write CS password. OK. So when you're using these macros, like CSKEY. Just set the register with =
    CSCTL1 |= DCOFSEL_3;                // select 8HMz DCO
    CSCTL2 |= SELS__DCOCLK;             // select DCO for SMCLK
    CSCTL3 |= DIVS__8;                  // select div 8 for 1MHz clock on SMCLK

    UCA0CTL1 = UCSWRST;                 // hold UCA in software reset
    UCA0CTL1 |= UCSSEL0|UCSSEL1;        // set source to SMCLK 1MHz
    UCA0BR0 = 104;                      // 9600 baud from 1Mhz
    UCA0BR1 = 0;                        // ^
                                        // what about clock modulation for UART

    P2SEL1 |= BIT0|BIT1;                // route to 2.0 2.1
    UCA0CTL1 &= ~UCSWRST;               // take UCA out of software reset

    UCA0IE |= UCRXIE;                   // enable RX interrupt on UCA0


    P1DIR |= BIT6+BIT7;               // Set P1.6 and 1.7 output
    P1SEL0 |= BIT6+BIT7;              // Select TB1.1 on P1.6, TB1.2 on P1.7
    TB1CCR0 = timerBCCR0;             // TB1CCR0 for up count target across TB11
    TB1CCTL1 |= OUTMOD_7;             // TB1CCR1 mode 7 = reset/set
    TB1CCR1 = 1000;                   // TB1CCR1 PWM duty cycle = 50%
    TB1CCTL2 |= OUTMOD_7;             // TB1CCR2 mode 7 = reset/set
    TB1CCR2 = 500;                    // TB1CCR2 PWM duty cycle = 25%
    TB1CTL = TBSSEL_2 + MC_1;         // TB1 SMCLK, UP mode

    P4DIR &= ~BIT0;         // let's use S1 as a single packet process button
    P4OUT |= BIT0;          // resistor pull up
    P4REN |= BIT0;          // resistor pull up
    P4IES |= BIT0;          // falling edge
    //P4IE |= BIT0;           // enable interrupt
    __bis_SR_register(GIE);             // enable global interrupts


    while(1)
    {
        if(counter>0)       // keep dequeueing as long as stuff is in the buffer.
        {
             waitForUARTbusy();   // maybe wait for UART
             deQueue();
             switch(byteState)
             {
             case 0:
                 if(deQueueOut == 0xFF) byteState = 1;
                 break;
             case 1:
                 command = deQueueOut;
                 byteState = 2;
                 break;
             case 2:
                 data_H = deQueueOut;
                 byteState = 3;
                 break;
             case 3:
                 data_L = deQueueOut;
                 byteState = 4;
                 break;
             case 4:
                 esc = deQueueOut;
             // If we're here, we have the whole packet. Process it
                byteState = 0;
                switch(esc)            // adjust data according to escape
                {
                case 0x02:
                    data_L = 0xFF;break;
                case 0x04:
                    data_H = 0xFF;break;
                case 0x06:
                    data_L = 0xFF;
                    data_H = 0xFF;break;
                default:
                    break;
                }

                data = data_H<<8 | data_L;      // combine data

                switch(command)        // main command execution happens here.
                {
                case 0x01:
                    TB1CCR0 = data;      // Command 1: set timer B CCR0 (period)
                    break;
                case 0x02:
                    byteDisplayLED(data_L);   // Command 2: set LEDs
                    break;
                case 0x03:
                    TB1CCR1 = data;     // Command 3: change timerB CCR1 (duty cycle)
                default:
                    break;
                }

             default:
                break;
             }
        }
    }
    return 0;
}

#pragma vector=USCI_A0_VECTOR
__interrupt void UCA0RX_ISR(void)
{
    while(!(UCA0IFG&UCTXIFG));        // wait for cleared USCI A0 TX IFG
    char in = UCA0RXBUF;
    //UCA0TXBUF = in;        // send back received char
    switch(in)
    {
    case 13:
        //deQueue();break;
    default:
        enQueue(in);
        break;
    }

}

