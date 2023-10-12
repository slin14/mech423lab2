#include <msp430.h> 

#define BUF_SIZE   50

volatile unsigned int head = 0;
volatile unsigned int tail = 0;
volatile unsigned int buf[BUF_SIZE];

unsigned int i;

void enqueue(int val)
{
    if ((head + 1) % BUF_SIZE == tail) { // buffer FULL (head + 1 == tail)
        //
    }
    else { // enqueue
        buf[head] = val;
        head = (head + 1) % BUF_SIZE; // head++;
    }
}

int dequeue(void) {
    int result = 0;
    if (head == tail) { // buffer empty
        //
    }
    else { // dequeue
        result = buf[tail];
        tail = (tail + 1) % BUF_SIZE; // tail++;
        //tail = (tail + BUF_SIZE - 1) % BUF_SIZE; // tail--;
    }
    return result;
}

void txUART(unsigned char in)
{
    while (!(UCA0IFG & UCTXIFG)); // wait until UART not transmitting
    UCA0TXBUF = in;
}

void printBufUART()
{
    // print circular buffer contents
    txUART(0);
    for (i = tail; i != head; i = (i + 1) % BUF_SIZE) {
        txUART(buf[i]);
    }
    //txUART(0);
}


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
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
    unsigned char dequeueByte = 13;

    RxByte = UCA0RXBUF;                     // Get the new byte from the Rx buffer

    if (RxByte == dequeueByte) { // dequeue if receive a carriage return
        dequeuedItem = dequeue();
        P3OUT |= BIT8;
        // transmit
    }
    else {
        enqueue(RxByte);
    }
    printBufUART();

    // UART RX IFG self clearing
}
