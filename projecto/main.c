#include <p18f4580.h>
#include <adc.h>
#include <timers.h>
#include <usart.h>

/******************************/

int resultado;

/******************************/

void low_ISR(void);
void high_ISR(void);

#pragma code high_vector = 0x08 // force the following statement to start
void high_interrupt (void) // at 0x08
{
	_asm
	goto high_ISR
	_endasm
}

#pragma code low_vector = 0x18 //force the following statements to start
void low_interrupt (void) // at 0x18
{
	_asm
	goto low_ISR
	_endasm
}

/*****************************/

#pragma code //return to the default code section
#pragma interrupt high_ISR
void high_ISR (void)
{
	if(INTCONbits.TMR0IF) //handle high-priority interrupts
		{
			OpenADC( ADC_FOSC_64 & ADC_RIGHT_JUST & ADC_1ANA , ADC_CH0 & ADC_INT_OFF , 0 );
			ConvertADC();
			while(busyADC());
			resultado= ReadADC();

			OpenTimer0( TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_64 );
			WriteTimer0( 0x7A & 0xE1 );
			INTCONbits.TMR0IF=0;	
		}
}


#pragma interruptlow low_ISR
void low_ISR (void)
{
	_asm // handle low-priority interrupts
	retfie 0 // simply return
	_endasm
}

/*****************************/

unsigned char getc_usart(void) {
    while (!PIR1bits.RCIF);
    PIR1bits.RCIF = 0;
    return RCREG;
}

/*********************************/


 void main (void)
{
	char c;
  	char str[7] = "ECHO:x\0";

	RCONbits.IPEN = 1; // Enable priority interrupt
	INTCON = 0b10100000;

	TRISD = 0x00; // Configure PORTD for output
	PORTD = 0b00000000; // turn off all LEDs initially
	TRISB = 0b00000000;
     PORTB = 0b00000000;

	//TMR0H=0x67;
	//TMR0L=0x69;
	//T0CON=0b10000110;

//    configure USART
    OpenUSART(USART_TX_INT_OFF &
            USART_RX_INT_OFF &
            USART_ASYNCH_MODE &
            USART_EIGHT_BIT &
            USART_CONT_RX,
            129);


	OpenADC( ADC_FOSC_64 & ADC_RIGHT_JUST & ADC_1ANA , ADC_CH0 & ADC_INT_OFF , 0 );
	OpenTimer0( TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_64 );
	WriteTimer0( 0x7A & 0xE1 );
	

	//CONFIGURAR AQUI AS INTERRUPÇÕES

	while (1); // stay in an infinite loop
	{
		c = getc_usart();
        if (c == 'l') {
            PORTB = 0b00000001;
        } else if (c == 'd') {
            PORTB = 0b00000000;
        }


        // get char from USART by polling method
        str[5] = c;
        putsUSART(str); // send string to USART

	}
}