/*
 * Extract accelerometer and force sensor data and send over UART to ESP32
 */

#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

volatile unsigned int xAcc = 0;
volatile unsigned int yAcc = 0;
volatile unsigned int zAcc = 0;
volatile unsigned int ADC_counter = 0; // increment x, y, z axis
volatile unsigned int force = 1023; // force gauge ADC value

void setup(void);
void setClk(void);
void setTimer(void);
void setUART(void);
void setADC(void);
void setForceInput(void);
void setAccel(void);
void setLEDs();

int main(void)
{
	setup();
	setLEDs();
    setClk();
    setTimer();
    setUART();
    setADC();
    setForceInput();
    setAccel();

    while(1) {
        if (force > 50)
            P1OUT = BIT7;
        if (force > 115)
            P1OUT = BIT6 + BIT7;
        if (force > 165) {
            P1OUT = BIT6 + BIT7;
            P3OUT = BIT7;
        }
        if (force > 200) {
            P1OUT = BIT6 + BIT7;
            P3OUT = BIT6 + BIT7;
        }
        else
        {
            P1OUT &= ~(BIT7 + BIT6);
            P3OUT &= ~(BIT7 + BIT6);
        }

    }
	
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
 else if (ADC_counter == 2) // Z-axis
 {
   ADC10CTL0 &= ~ADC10ENC;
   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_15;  // next channel is the force sensor
   zAcc = ADC10MEM0;
   ADC_counter++;
   ADC10CTL0 |= ADC10ENC | ADC10SC;
 }
 else  // Force sensor
 {
   ADC10CTL0 &= ~ADC10ENC;
   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;  // next channel is the X-axis
   force = ADC10MEM0;
   ADC_counter = 0;
   ADC10CTL0 |= ADC10ENC | ADC10SC;
 }
}

#pragma vector = TIMER1_B1_VECTOR
__interrupt void TIMER1_B1_ISR(void) {

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
    UCA0TXBUF = force;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 0;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 0;

    TB1CCTL1 &= ~CCIFG; // reset flag

    // Flash LED for verification
    PJOUT ^= BIT0;
}

void setup(){
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    _EINT(); // enable global interrupts
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
    ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_12; // ADC10INCH_12 for xAccel (cycles through using ADC_counter)
    ADC10IV = 0x00;    // clear all ADC12 channel int flags
    ADC10IE |= ADC10IE0;  // enable ADC10 interrupts

    ADC10CTL0 |= ADC10ENC | ADC10SC; // start the first sample. If this is not done the ADC10 interrupt will not trigger.

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

    // Set force indication LEDs
    P1DIR |= BIT7 + BIT6;
    P3DIR |= BIT7 + BIT6;

}
/*
 * Extract accelerometer and force sensor data and send over UART to ESP32
 */

#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

volatile unsigned int xAcc = 0;
volatile unsigned int yAcc = 0;
volatile unsigned int zAcc = 0;
volatile unsigned int ADC_counter = 0; // increment x, y, z axis
volatile unsigned int force = 1023; // force gauge ADC value

void setup(void);
void setClk(void);
void setTimer(void);
void setUART(void);
void setADC(void);
void setForceInput(void);
void setAccel(void);
void setLEDs();

int main(void)
{
	setup();
	setLEDs();
    setClk();
    setTimer();
    setUART();
    setADC();
    setForceInput();
    setAccel();

    while(1) {
        if (force > 50)
            P1OUT = BIT7;
        if (force > 115)
            P1OUT = BIT6 + BIT7;
        if (force > 165) {
            P1OUT = BIT6 + BIT7;
            P3OUT = BIT7;
        }
        if (force > 200) {
            P1OUT = BIT6 + BIT7;
            P3OUT = BIT6 + BIT7;
        }
        else
        {
            P1OUT &= ~(BIT7 + BIT6);
            P3OUT &= ~(BIT7 + BIT6);
        }

    }
	
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
 else if (ADC_counter == 2) // Z-axis
 {
   ADC10CTL0 &= ~ADC10ENC;
   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_15;  // next channel is the force sensor
   zAcc = ADC10MEM0;
   ADC_counter++;
   ADC10CTL0 |= ADC10ENC | ADC10SC;
 }
 else  // Force sensor
 {
   ADC10CTL0 &= ~ADC10ENC;
   ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;  // next channel is the X-axis
   force = ADC10MEM0;
   ADC_counter = 0;
   ADC10CTL0 |= ADC10ENC | ADC10SC;
 }
}

#pragma vector = TIMER1_B1_VECTOR
__interrupt void TIMER1_B1_ISR(void) {

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
    UCA0TXBUF = force;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 0;
    while (!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = 0;

    TB1CCTL1 &= ~CCIFG; // reset flag

    // Flash LED for verification
    PJOUT ^= BIT0;
}

void setup(){
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    _EINT(); // enable global interrupts
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
    ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_12; // ADC10INCH_12 for xAccel (cycles through using ADC_counter)
    ADC10IV = 0x00;    // clear all ADC12 channel int flags
    ADC10IE |= ADC10IE0;  // enable ADC10 interrupts

    ADC10CTL0 |= ADC10ENC | ADC10SC; // start the first sample. If this is not done the ADC10 interrupt will not trigger.

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

    // Set force indication LEDs
    P1DIR |= BIT7 + BIT6;
    P3DIR |= BIT7 + BIT6;

}
