#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

//DEFINES
#define RXD BIT0
#define TXD BIT1


//VARIABLES
volatile unsigned int force = 1023; // force gauge ADC value
volatile char result[5];


//FUNCTION DEFINITIONS
void setup(void);
void setClk(void);
void setUART(void);
void setADC(void);
void setPotInput(void);
void readForce(void);
void transmitData(int data);
void setForceInput(void);
void sendInt(int num);
void sendComma();
void newLine();
void itoa(long unsigned int value, volatile char* result, int base);



//MAIN----------------------------------------------------------------------
int main(void)
{

    //Setup
    setup();
    setClk();
    setUART();
    setADC();
    setForceInput();

    //Main loop
    while (1) {

        readForce();

        transmitData(force);

        __delay_cycles(100000); // to avoid spamming serial reader

    }

    return 0;

}



//FUNCTIONS --------------------------------------------------------------------------

void setup(){
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    _EINT(); // enable global interrupts
}

void transmitData(int data){
    while (!(UCA0IFG & UCTXIFG));

        PJOUT &= ~BIT0;
//        while (!(UCA0IFG & UCTXIFG));
//        UCA0TXBUF = force;
        //ITOA METHOD BELOW
        sendInt(force);
        sendComma();

}

void readForce(){
    ADC10MCTL0 = ADC10INCH_12; // use P3.0 (A12)
    ADC10CTL0 |= ADC10ENC + ADC10SC; // enable and start conversion
    ADC10CTL0 &= ~(ADC10SC);
    while((ADC10IFG & ADC10IFG0) == 0); // wait for flag
    ADC10CTL0 &= ~(ADC10ENC);
    force = ADC10MEM0; // get voltage from ADC
}

void setClk() {
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;           // DCO = 8 MHz
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // MCLK = DCO, ACLK = DCO, SMCLK = DCO
}


void setUART() {
    UCA0CTLW0 = UCSSEL0;                    // Run the UART using ACLK
    UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900;   // Baud rate = 9600 from an 8 MHz clock
    UCA0BRW = 52;
    P2SEL0 &= ~(RXD + TXD); // set to 00 ds74
    P2SEL1 |= RXD + TXD;    // set to 11 ds74
    PJDIR |= BIT0; //PJ.0
}

void setADC() {
    // Enable ADC_B (ug449)
    ADC10CTL0 &= ~ADC10ENC;           // ensure ENC is clear
    ADC10CTL0 = ADC10ON + ADC10SHT_2; //ON -> Turns on //SHT -> define the number of ADC10CLK cycles in the sampling period for the ADC10 (ENC must be zero to change this)
    ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
    ADC10CTL2 &= ~(ADC10RES); // 8 or 10 bit ADC out // unsigned int
    ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_12; // ADC10INCH_12 for potentiometer

    // Set internal voltage reference
    while(REFCTL0 & REFGENBUSY);
    REFCTL0 |= REFVSEL_3 + REFON; // ug431
}

void setForceInput() {
    // Input pot on P3.0 ds80
    P3DIR |= BIT0;
    P3SEL1 |= BIT0;
    P3SEL0 |= BIT0;
}

//ITOA SHIT --------------------------------------------------------------------------------

//INT TO STRING (Found online)
void itoa(long unsigned int value, volatile char* result, int base) {
      // check that the base if valid
      if (base < 2 || base > 36) { *result = '\0';}

      char* ptr = result, *ptr1 = result, tmp_char;
      int tmp_value;

      do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
      } while ( value );

      // Apply negative sign
      if (tmp_value < 0) *ptr++ = '-';
      *ptr-- = '\0';
      while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
      }

}

//SEND INT to TX Buffer
void sendInt(int num) {

    itoa(num, result, 10);
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[0];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[1];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[2];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[3];

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = result[4];

}

//SEND ',' to TX Buffer
void sendComma() {

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = ',';
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = ' ';

}

//SEND '\n' to TX Buffer
void newLine() {

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\n';
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\r';

}
