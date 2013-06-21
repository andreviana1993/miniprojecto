#include <p18f4580.h>
#include <adc.h>
#include <timers.h>
#include <usart.h>

/******************************/
// Global variables
int resultado;
int contagem=0;

/*****************************/
//timer related code

void timer10us( void)
{
		WriteTimer0( 65336 );
		contagem++;
}


/******************************/
// Function prototypes

void low_ISR(void);
void high_ISR(void);

#pragma code high_vector = 0x08 // force the following statement to start

void high_interrupt(void) // at 0x08
{
    _asm
    goto high_ISR
            _endasm
}

#pragma code low_vector = 0x18 //force the following statements to start

void low_interrupt(void) // at 0x18
{
    _asm
    goto low_ISR
            _endasm
}

/*****************************/




#pragma code //return to the default code section
#pragma interrupt high_ISR

void high_ISR(void) {
    if (INTCONbits.INT0IF) {
		INTCON2bits.INTEDG0 = !INTCON2bits.INTEDG0;
		//PORTDbits.RD7 = ~(PORTDbits.RD7 );
		contagem=0;
		timer10us();
		INTCONbits.INT0IF = 0;		
    } 
    if (INTCONbits.TMR0IF) //handle high-priority interrupts
    {
        // Timer0 handler
		if (contagem<898){
		timer10us();
		}
		else if (contagem>= 898 && contagem <908){
		PORTBbits.RB1 = 1;
		PORTDbits.RD7 = 1;
		timer10us();
		}
		else if (contagem = 908){
		PORTBbits.RB1 = 0;
		PORTDbits.RD7 = 0;
		contagem=0;
		}
        INTCONbits.TMR0IF = 0;
    }
}

#pragma interruptlow low_ISR

void low_ISR(void) {
    _asm // handle low-priority interrupts
    retfie 0 // simply return
            _endasm
}

/*****************************/
// Usart related code

unsigned char getc_usart(void) {
    while (!PIR1bits.RCIF);
    PIR1bits.RCIF = 0;
    return RCREG;
}

/*********************************/

void main(void) {
    char c;
    char str[7] = "ECHO:x";

    RCONbits.IPEN = 1; // Enable priority interrupt
    INTCON = 0b10100000;

    TRISD = 0b00000000; // Configure PORTD for output
    PORTD = 0b00000000; // turn off all LEDs initially
    TRISB = 0b00000001; // Configure PORTB for output except RB1 and RB0 (INT0)
    PORTB = 0b00000000; // turn off all LEDs initially

    OpenUSART(USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX, 129);


    //OpenADC(ADC_FOSC_64 & ADC_RIGHT_JUST & ADC_1ANA_2REF, ADC_CH0 & ADC_INT_OFF, 0);
    ADCON0 = 0b00000001;
    ADCON1 = 0b00111110;
    ADCON2 = 0b10000110;
    OpenTimer0(TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_2);
    //WriteTimer0( 6 );

    while (1) {
        c = getc_usart();
        if (c == 'l') {
            PORTBbits.RB2 = 1;
        } else if (c == 'd') {
            PORTBbits.RB2 = 0;
        } else if (c == 'q') {
            resultado = resultado + 50;
        } else if (c == 'f') {
            resultado = resultado - 50;
        }
        str[5] = c;
        putsUSART(str);

        ConvertADC();
        while (BusyADC());
        resultado = ReadADC();

    }
}