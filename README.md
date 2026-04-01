# Nanoosciloscopio con ATtiny85
![Prototipo](Fotos/prototipo.jpg)

Osciloscopio digital simple basado en ATtiny85 con visualización en OLED.

## Características
- Muestreo rápido por ADC
- Visualización en pantalla
- Generador de señal integrado
- Escala de tiempo: 10 µs a 8160 µs por punto.

## Hardware necesario
- ATtiny85 (principal), adaptable a ATmega328P
- OLED SSD1306

### Componentes

- R1 10 kΩ
- R2 10 kΩ
- R3 10 kΩ
- R4 100 kΩ
- R5 47 kΩ   (PUL1)
- R6 4,7 kΩ
- R7 12 kΩ
- R8 47 kΩ
- R9 8,2 k
- R10 6,8 kΩ (PUL 2) Solo para placa de 3 pulsadores
- R11 22 kΩ  (Pul 3) Solo para placa de 3 pulsadores
- C1 15 pF
- C2 15 pF 
- C3 10 µF
- C4 100 nF
- DS1=DS2 Schottky
- DZ1 Zener 3,3 V.
- Conectores varios.
- Pulsadores.

 Opcionales:
- Interruptor.
- Circuito impreso
- Módulo de carga tipo TP4056
- Batería
- Cristal. (Preferente de 8 MHz funciona con batería de litio,
           y a 16 MHz alimentada a 5 V, pero dan precisión)

## Conexiones
(ver hardware/Esquematico_Nano-Osciloscopio_2026-04-01.png)

## Cómo usar
1. Cargar el código
2. Conectar la señal a la entrada (rangos soportados: 1 V, 3.3/5 V o 12 V según configuración)
3. Ajustar parámetros

## Limitaciones
- Ancho de banda limitado
- Resolución ADC

## Autor
Alejandro F. Fernández  
nanoosciloscopio@gmail.com

## Licencia
Uso no comercial.

Si querés usarlo comercialmente, contactame:
nanoosciloscopio@gmail.com

Se agradece reportar mejoras o errores.

