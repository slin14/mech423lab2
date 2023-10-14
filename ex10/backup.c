#include <msp430.h> 

#define BUF_SIZE   50

volatile unsigned int head = 0;
volatile unsigned int tail = 0;
volatile unsigned int buf[BUF_SIZE];

volatile unsigned int byteIndex = 0; // state
volatile unsigned int cmdByte;
volatile unsigned int dataByte1;
volatile unsigned int dataByte2;
volatile unsigned int escapeByte;

volatile unsigned short data=0;


unsigned int i;


void txUART(unsigned char in)
{
    while (!(UCA0IFG & UCTXIFG)); // wait until UART not transmitting
    UCA0TXBUF = in;
}

void enqueue(int val)
{
    if ((head + 1) % BUF_SIZE == tail) { // buffer FULL (head + 1 == tail)
        // Error: buffer full
        txUART(255);
    }
    else { // enqueue
        buf[head] = val;
        head = (head + 1) % BUF_SIZE; // head++;
    }
}

int dequeue(void) {
    int result = 0;
    if (head == tail) { // buffer empty
        // Error: buffer empty
        txUART(0);
    }
    else { // dequeue
        result = buf[tail];
        tail = (tail + 1) % BUF_SIZE; // tail++;
        //tail = (tail + BUF_SIZE - 1) % BUF_SIZE; // tail--;
    }
    return result;
}

short combine(char dataByte1, char dataByte2) { // combine upper and lower byte into 16 bit

    data = (dataByte1 << 8) | dataByte2;

    return data;
}

void emptyBufMessage()
{
    // remove the processed bytes from the buffer
    unsigned int dequeuedItem = 0;
    dequeuedItem = dequeue();
    dequeuedItem = dequeue();
    dequeuedItem = dequeue();
    dequeuedItem = dequeue();
    dequeuedItem = dequeue();
}

// testing: function to print buffer contents over UART
void printBufUART()
{
    // print circular buffer contents
    //txUART(0); // start
    for (i = tail; i != head; i = (i + 1) % BUF_SIZE) {
        txUART(buf[i]);
    }
    //txUART(0);
}


/**
 * main.c - ex9
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    /////////////////////////////////////////////////
    // Set up UART
    // Configure P2.0 and P2.1 ports for UART
    P2SEL0 &= ~(BIT0 + BIT1); // redundant
    P2SEL1 |= BIT0 + BIT1;

    UCA0CTL1 = UCSWRST;                 // hold UCA in software reset
    UCA0CTL1 |= UCSSEL0|UCSSEL1;        // set source to SMCLK 1MHz
    UCA0BRW = 104;                      // Baud rate = 9600 from an 1 MHz clock
    UCA0CTL1 &= ~UCSWRST;               // take UCA out of software reset
    UCA0IE |= UCRXIE;                   // Enable UART Rx interrupt

    /////////////////////////////////////////////////
    // Set up LED outputs
    PJDIR |= BIT0 + BIT1 + BIT2 + BIT3;
    P3DIR |= BIT4 + BIT5 + BIT6 + BIT7;
    PJOUT = 0;
    P3OUT = 0;

    /////////////////////////////////////////////////
    // configure TB1.1 to output 500 Hz square wave on P3.4

    // configure P3.4 as output (for TB1.1)
    P3DIR |= BIT4;
    // set timer output to output on P3.4 (for TB1.1)
    P3SEL0 |= BIT4;

    TB1CTL = TASSEL_2 + MC_1; // SMCLK, UP mode


    TB1CCTL1 |= OUTMOD_7;    // reset/set mode
                             // clr output when TB1R == TB1CCR0
                             // set output when TB1R == TB1CCR1
    TB1CCR0 = 2000;
    TB1CCR1 = 1000; // 50% duty cycle
    //TB1CCR1 = 500; // 25% duty cycle

    /////////////////////////////////////////////////
    // enable global interrupt
    _EINT();


    txUART(0);

    while(1) {

    }


    return 0;
}

/////////////////////////////////////////////////
// ISR for UART Receive
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    unsigned char RxByte = 0;
    unsigned int dequeuedItem = 0;
    unsigned char dequeueByte = 13; // character to dequeue on

    RxByte = UCA0RXBUF;                     // Get the new byte from the Rx buffer

    if (RxByte == dequeueByte) { // dequeue if receive a carriage return
        dequeuedItem = dequeue();
    }
    else {
        if (RxByte == 255 && byteIndex == 0) { // message start byte
            byteIndex = 1;
        }
        if (byteIndex == 1) { // command byte
            cmdByte = RxByte;
            byteIndex = 2;
        }
        else if (byteIndex == 2) { // upper data byte (1)
            if (RxByte > 255)    dataByte1 = 255;
            else if (RxByte < 0) dataByte1 = 0;
            else                 dataByte1 = RxByte;
            byteIndex = 3;
        }
        else if (byteIndex == 3) { // lower data byte (2)
            if (RxByte > 255)    dataByte2 = 255;
            else if (RxByte < 0) dataByte2 = 0;
            else                 dataByte2 = RxByte;
            byteIndex = 4;
        }
        else if (byteIndex == 4) { // escape byte
            escapeByte = RxByte;
            byteIndex = 0;
        }
        enqueue(RxByte);
    }

    //printBufUART(); // testing

    data = combine(dataByte1,dataByte2); // combine to 2 data bytes
    TB1CCR1 = data; // duty cycle adjusted based on input 16 bit number

    emptyBufMessage(); // remove the processed bytes from the buffer

}

/*look up table for some common duty cycles for byte 1 and 2 (1MHz timer)
DUTY CYCLE  DECIMAL  BYTE1    BYTE2
10%           200      0       200
25%           500      1       244
50%           1000     3       232
75%           1500     5       220
100%          2000     7       208
------------------------------------------------*/
