/*
 * Code to test ESP32 UART to send BT to phone
 *
 *
 */

#include <msp430.h> 

volatile unsigned int xAcc = 0;
volatile unsigned int yAcc = 0;
volatile unsigned int zAcc = 0;
volatile unsigned int ADC_counter = 0; // increment x, y, z axis
volatile char result[4];

void itoa(long unsigned int value, volatile char* result, int base) {
      // Check that base is valid
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
}

void sendComma() {
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = ',';
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = ' ';
}

void newLine() {
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\n';
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = '\r';
}

void setClk(void);
void setTimer(void);
void setUART(void);
void setADC(void);
void setAccel(void);
void setLEDs();

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	setLEDs();
    setClk();
    setTimer();
    setUART();
    setADC();
    setAccel();

    _EINT(); // enable global interrupts

    while(1) {}
	
	return 0;
}

// ADC10 interrupt routine
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
 if (ADC_counter == 0) // X-axis
 {
   ADC10CTL0 &= ~ADC10ENC;
   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_13;  // next channel is the Y-axis
   xAcc = ADC10MEM0; // already 8 bits set by ADC10CTL2
   ADC_counter++;
   ADC10CTL0 |= ADC10ENC | ADC10SC;
 }
 else if (ADC_counter == 1)  // Y-axis
 {
   ADC10CTL0 &= ~ADC10ENC;
   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_14;  // next channel is the Z-axis
   yAcc = ADC10MEM0;
   ADC_counter++;
   ADC10CTL0 |= ADC10ENC | ADC10SC;
 }
 else  // Z-axis
 {
   ADC10CTL0 &= ~ADC10ENC;
   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;  // next channel is the X-axis
   zAcc = ADC10MEM0;
   ADC_counter = 0;
   ADC10CTL0 |= ADC10ENC | ADC10SC;
 }
}

#pragma vector = TIMER1_B1_VECTOR
__interrupt void TIMER1_B1_ISR(void) {
//    sendInt(255); // start bit 255
////    sendComma();
//    sendInt(xAcc);
////    sendComma();
//    sendInt(yAcc);
////    sendComma();
//    sendInt(zAcc);
//    newLine();

    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 255;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = xAcc;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = yAcc;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = zAcc;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 0;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 0;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 0;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 0;

    TB1CCTL1 &= ~CCIFG; // reset flag

    // Flash LED for verification
    PJOUT ^= BIT0;
}

void setClk() {
    CSCTL0_H = CSKEY >> 8; // enables CS registers, can also do = 0xA5 (pg80 ug [ug = user guide])
    CSCTL1 &= ~DCORSEL; // DCORSEL set to 0 ug72
    CSCTL1 |= DCOFSEL0 + DCOFSEL1; // (pg81 ug) for 8MHz 11b
    CSCTL2 |= SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // set all CLK to run off DCO; (ug82)
    CSCTL3 |= DIVS__32; // set SMCLK divider to /32
}

void setTimer() {
    // Set P3.4 and P3.5 to be Timer B output and LED output; um14, 17
    P3DIR |= BIT4 + BIT5;
    P3OUT &= ~(BIT4 + BIT5);
    P3SEL0 |= BIT4 + BIT5;
    P3SEL1 &= ~(BIT4 + BIT5);

    // Set timer B
    TB1CTL |= TBSSEL1 + MC0; // select SMCLK source, initialize up mode (ug372)
    TB1CCTL1 = OUTMOD_3 + CCIE; // set/reset and interrupt enable (ug375, ug366 diagrams)

    // Set 25Hz waves (draw up graph to show)
    TB1CCR0 = 40000 - 1; // = (CLK/divider)/target = (8E6/8)/500 aka 4x divisions; subtract one since it counts more
    TB1CCR1 = 20000; // 50% duty cycle
}

void setUART() {
    // Configure UART on P2.0 and 2.1
    P2SEL0 &= ~(BIT0 + BIT1); // set to 00 ds74
    P2SEL1 |= BIT0 + BIT1; // set to 11 ds74
    UCA0CTLW0 = UCSSEL0; // 01b for ACLK (pg495 ug)
    UCA0MCTLW = UCOS16 + UCBRF0 + 0x4900; // 9600 baud from 8MHz ug490; UCOS16 = oversampling enabled, UCBRF0 = modulation stage
//    UCA0MCTLW = UCOS16 + UCBRF3 + UCBRF1 + 0xF700; // 57600 baud; UCBRFx = decimal 10 = 1010 hex = high low high low
    UCA0BRW = 52; // ug490 and ug497, bit clock prescaler ***Why is this 52 for both 9600 and 57600 baud?
    UCA0IE |= UCRXIE; // enable UART RX interrupt
}

void setADC() {
    // Enable ADC_B (ug449)
    ADC10CTL0 &= ~ADC10ENC; // ensure ENC is clear
    ADC10CTL0 = ADC10ON + ADC10SHT_5;
    ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
    ADC10CTL2 &= ~ADC10RES; // 8 bit ADC out
    ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;
    ADC10IV = 0x00;    // clear all ADC12 channel int flags
    ADC10IE |= ADC10IE0;  // enable ADC10 interrupts

    ADC10CTL0 |= ADC10ENC | ADC10SC; // start the first sample. If this is not done the ADC10 interrupt will not trigger.
}

void setAccel() {
    // Set P2.7 to output HIGH to power accel
    P2DIR |= BIT7;
    P2OUT |= BIT7;

    // Set P3.0, 3.1, 3.2 to output A12, A13, A14
    P3DIR &= ~(BIT0 + BIT1 + BIT2);
    P3SEL0 |= BIT0 + BIT1 + BIT2; // ds80
    P3SEL1 |= BIT0 + BIT1 + BIT2;
}

void setLEDs() {
    PJDIR |= 0x0F; // 00001111 // unsure LSB
    P3DIR |= 0xF0; // 11110000

    // Toggle to show ready
    PJOUT |= 0x0F;
    P3OUT |= 0xF0;
    __delay_cycles(2000000);
    PJOUT &= ~0x0F; // turn off
    P3OUT &= ~0xF0; // turn off
}
