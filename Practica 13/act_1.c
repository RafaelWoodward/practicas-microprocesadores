// ACT_A
#include <xc.h>

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 8000000

#define MOTOR PORTDbits.RD0  // salida PWM al BJT

// duty: controlado por el ADC, leido por la ISR del Timer2

volatile unsigned char duty    = 0;
volatile unsigned char pwm_cnt = 0;

// TIMER 2 - PWM software igual que en la practica 11
// Fosc=8MHz, prescaler 1:4, PR2=99
// Frecuencia PWM = 8,000,000 / (4 * 4 * 100) = 5,000 Hz
// Suficiente para que el motor no vibre por el switching

void Timer2_Init() {
    TRISDbits.TRISD0 = 0;   // RD0 como salida al BJT
    PR2     = 99;            // periodo: 0 a 99 = 100 pasos de resolucion
    T2CON   = 0b00000101;   // prescaler 1:4, Timer2 ON, postscaler 1:1
    TMR2IE  = 1;             // interrupcion de Timer2
    PEIE    = 1;             // interrupciones de perifericos
    GIE     = 1;             // interrupciones globales
}

void __interrupt() ISR(void) {
    if (TMR2IF) {
        pwm_cnt++;
        if (pwm_cnt >= 100) pwm_cnt = 0;
        MOTOR = (pwm_cnt < duty) ? 1 : 0;  // encendido si dentro del duty
        TMR2IF = 0;
    }
}

// ADC
void ADC_Init() {
    ANSEL  = 0x01;  // solo RA0 analogico
    ANSELH = 0x00;
    ADCON0 = 0x01;  // canal AN0, ADC habilitado
    ADCON1 = 0x80;  // right-justified
}

unsigned int ADC_Read() {
    __delay_us(5);
    GO_nDONE = 1;
    while(GO_nDONE);
    return (unsigned int)((ADRESH << 8) | ADRESL);
}

// MAIN
void main(void) {
    ADC_Init();
    Timer2_Init();

    while(1) {
        unsigned int adc = ADC_Read();

        // Mapear ADC (0-1023) a duty (0-99)
        // unsigned long para evitar overflow: 1023 * 99 = 101,277 > 65535
        duty = (unsigned char)(((unsigned long)adc * 99) / 1023);

        __delay_ms(20); // refresco cada 20ms
    }
}
