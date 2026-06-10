# Practica 4 - Entradas como señal de inputs
Implementación de contador decimal y control de LEDs mediante el manejo de inputs con botones push con el microcontrolador PIC16F887.

<br>**Esquematico:** <br>
![Esquematico Practica 4](./Esquematico_Semaforo.png)

---
## Actividades
### Clase - Control de LEDs mediante entradas push button.
Programa que implementa un control sobre los LEDs que se encienden en base al boton push que se oprima. Cada botón conectado a RB0–RB2 enciende su LED correspondiente en RD0–RD2 de forma directa.
 - ANSEL y ANSELH Deshabilita las entradas analógicas, por lo que todos los pines son digitales.
 - TRISB define los puertos B como inputs y TRISD define a los puetos D como salidas (LEDs).
 - "OPTION_REG = OPTION_REG & 0b01111111;" Esta línea pone en 0 el bit 7 del registro OPTION_REG, sin modificar los demás bits. Esto nos permite activar las resistencias pull-up internas del Puerto B, lo cual es necesario porque los botones conectados a RB0–RB2 usan lógica negativa.

<br> **Codigo:** <br> [Entradas.c](./Entradas.c)

<br> **Circuito:** <br>
![Circuito](./Entradas.jpeg)

---
### Actividad 1 - Contador de 00 a 99 con control por 3 botones push-button.
El programa nos permite desarrollar un contador de dos digitos de 00 a 99 con 3 botones: sumar, restar y reset, mostrado sobre 2 displays de 7 segmentos.
 - PORTB define las entradas usando 3 botones con pull-up interno
 - PORTC define las salidas al display de decenas
 - PORTD define las salidas al display de unidades
 - La variable "contador" representa el valor que se está mostrando en los dos displays en todo momento. Cada botón lo modifica de una forma distinta para incrementar, reducir o resetarlo con "++", "--", y "contador = 0".
 - "decenas  = contador / 10;"   y "unidades = contador % 10;"   separan los dos digitos en dos variables que posteriormente se usan para determinar que palabra binaria dentro del directorio de "seg7" se usa como salida con "PORTC = seg7[decenas];" y "PORTD = seg7[unidades];"

<br> **Codigo:** <br> [Contador_99.c](./Contador_99.c)

<br> **Esquematico:** <br>
![Esquematico](./Esquematico_Contador_99.png)

<br> **Circuito:** <br>
![Circuito](./Contador_99.jpeg)

---
## Observaciones
- El operador ! invierte la señal, esto porque los botones usan pull-up por la logica de señales en los inputs del microcontrolador.
- El puerto B se usa exclusivamente para entradas, este t
