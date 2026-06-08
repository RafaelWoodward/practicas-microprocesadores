# Practica 1 - Manejo de Salidas Digitales
Manejo de salidas digitales mediante el microcontrolador PIC16F887, implementando secuencias de parpadeo, conteo binario y caminata en LEDs.
<br>**Esquematico:** <br>
![Esquematico Practica 1](./Practica1_esquematico.png)

<br> **Circuito:** <br>
![Circuito](./Practica1_circuito.jpg)

---
## Actividades
### Clase - Blink (Parpadeo de 4 LEDs)
Programa de parpadeo de 4 LEDs conectados al puerto del PIC16F887, introducción al manejo de salidas digitales y configuración de registros TRIS y PORT.
<br> **Codigo:** <br> [Blink.c](./Blink.c)

---
### Actividad 1 - Contador Binario de 6 bits
Contador binario de 6 bits desplegado en LEDs, incrementando de 0 a 63 de forma secuencial usando los puertos D del PIC16F887.
<br> **Codigo:** <br> [Contador_6_bits.c](./Contador_6_bits.c)

---
### Actividad 2 - Caminata de 8 bits
Secuencia tipo caminata sobre 8 LEDs conectados al puerto B del PIC16F887, desplazando un bit encendido de izquierda a derecha y de regreso de forma continua.
<br> **Codigo:** <br> [Caminata.c](./Caminata.c)

---
## Observaciones
- Los registros TRIS deben configurarse en 0x00 para definir el puerto como salida antes de cualquier operacion.
- Se utilizo la funcion __delay_ms() para controlar los tiempos de cada secuencia.
- 0x00 = 0x00000000 = 0
