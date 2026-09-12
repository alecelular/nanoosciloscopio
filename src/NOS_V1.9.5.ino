/*
 Nanoosciloscopio

 Alejandro F. Fernández  
 nanoosciloscopio@gmail.com
 Año: 2026

 Descripción:
 Osciloscopio simple usando ADC y temporización por
 interrupciones.

 CPU:
 - ATtiny85, eventualmente ATmega328P

 Licencia: Uso no comercial
 Ver archivo LICENSE para más detalles

 Notas:
 - Optimizado para bajo consumo de memoria
 - Uso intensivo de interrupciones
*/

// Para usar con ATtiny85 o con ATmega328P
// Las explicaciones son mayormente para ATtiny85 para que
// siga siendo un osciloscopio muy nano.

#define VERSION  "V1.9.5"   // Hasta 9 de largo
#define FECHA    "30/05/26"
#define EQUIPO   "NOS"
#define AUTOR    "alecelular"

// Líneas en la pantalla a visibilizar, por si se quiere
// agregar más información debajo en el modo osciloscopio.
// (7 agrega una, 6 agrega 2), visto en caracteres simples, no
// dobles. En el caso de OLED<=4, usará una pantalla de 128x32.
// Más de 4, usará una pantalla de 128x64. Usar normalmente 8
// para 128x64 y 4 para 128x32. Los otros modos, se verán co-
// rrectamente, ya que no ocupan más de 4 líneas.
#define OLED 8   

// Si quiero el sensor DHT en vez del DS18B20, poner el define.
// Válido para ATtiny85, para elegir entre sensores. Comentar
// la línea si quiero un DS18B20. Para ATmega328P, se puede
// usar uno u otro, sin recompilar. Ignora el define.
//#define SENSOR_DHT

// V1.9.5 30/5/26 En el frecuencímetro con cristal de 20 MHz,
// cambio levemente el ancho de las ventanas de medición, en
// pos de precisión y arreglo la visualización de ese modo. Op-
// timizo algunas cosas más y de paso cambio la manera en que
// detecto el selector 4 para que use los huecos entre resis-
// tencias y ponga que es un selector que no es válido. Más a-
// delante podría usarse como otra opción.

// V1.9.4 29/5/26 Corrijo el cálculo del porcentaje de VPP.

// V1.9.3 28/5/26 Corrección de mensajes. Cambio como presentar
// los valores de ajuste de toletancias de resistencias en el
// selector.

// V1.9.2 28/5/26 Unifico criterio en calibrarRangoGenerico()
// y en calibrarSensor(), y por ello, se han optimizado las
// rutinas. Hago que si la resistencia del selector no es lo
// esperado, dentro de su tolerancia, no se apruebe ese valor.
// Cambios menores. A probar.

// V1.9.1 27/5/26 Arreglo calibraciones. Estaban mal. Y adapto
// el display de 128x32 porque ahora no gira. Y debí modificar
// el valor de la resistencia del pulsador 2 a 2,2 KΩ. Unifico
// criterio en calibrarRangoGenerico() y calibrarSensor(), y
// por ello, se han optimizado las rutinas. A probar.

// ATtiny85 
// Compilar usando ATtinyCore 1.5.2 sin millis()
// En preferencias: http://drazzy.com/package_drazzy.com_index.json
//| --------------------------------------|
//| En ATtiny85 "Es el NanoOsciloscopio"  |
//|--------------------|------|-----------|
//| Función            | Pata |   Señal   |
//|--------------------|------|-----------|
//| RESET / Selector   |   1  |PB5/ADC0/Re|
//| Cristal A          |   2  |PB3/ADC3/XX|
//| Cristal B          |   3  |PB4/ADC2   |
//| MASA               |   4  |  0 V      |
//| SDA'(BitBang)      |   5  |PB0/AIN0   |
//| SCL'(BitBang)      |   6  |PB1/AIN1   |
//| SAL/ENT/Frec/SENSOR|   7  |PB2/ADC1/T0|
//| Alimentación       |   8  |  VCC      |
//|--------------------| -----|-----------|
//|---------------------------------------|
//| En ATtiny84 (Bosquejo, pero no usado) |
//|--------------------|------|-----------|
//| Función            | Pata |   Señal   |
//|--------------------|------|-----------|
//| Alimentación       |   1  | VCC       |
//| Cristal A          |   2  | PB0       |
//| Cristal B          |   3  | PB1       |
//| RESET              |   4  | PB3 Reset |
//| SALIDA GEN         |   5  | PB2       |
//| PUL 1              |   6  | PA7       |
//| SDA'(bitbang)      |   7  | PA6 MOSI  |
//| PUL 2              |   8  | PA5 MISO  |
//| SCL'(bitbang)      |   9  | PA4 SCK   |
//| ENT FREC           |  10  | PA3 T0    |
//| Selector           |  11  | PA2 ADC2  |
//| ENT ANALOGICA      |  12  | PA1 ADC1  |
//| AREF Capacitor     |  13  | PA0 AREF  |
//| MASA               |  14  | 0 V       |
//|--------------------| -----|-----------|
//| --------------------------------------|
//| En ATmega328P probado con Arduino nano|
//|------------------|---------|----------|
//| Función          |  Pata   |  Señal   |
//|------------------|---------|----------|
//| SAL/ENT/SENSOR   | A0      | PC0      |
//| Frecuencímetro   | D5      | PD5      |
//| SDA'(bitbang)    | D9      | PB1      |
//| SCL'(bitbang)    | D10     | PB2      |
//| PUL1             | D2      | PD2      |
//| PUL2             | D7      | PD7      |
//| Selector         | A1      | PC1      |
//| Capacitor        | AREF    | VREF     |
//|------------------|---------|----------|
//| Extras no usadas aquí                 |
//|------------------|---------|----------|
//| DTR              | Reset   |  Reset   |
//| RXI              | D0/RXD  |  PD0     |
//| TXO              | D1/TXD  |  PD1     |
//|------------------|---------|----------|
//| PB0              |  D8/ICP | PB0      |
//| PB3              |MOSI/OC2 | PB3 3/D11|
//| PB4              |  MISO   | PB4 4    |
//| PB5  LED         | PB5/SCK | PB5 5    |
//| PB6              | PB6(XTAL1/TOSC1)   |
//| PB7              | PB7(XTAL2/TOSC2)   |
//| PC2/PC3          |  A2/A3  |  ADC2/3  |
//| PD3              | D3/INT1 | PD3      |
//| PD4              |D4/XCK/T0| PD4      |
//| PD6              | D6/AIN0 | PD6      |
//| SDA              |   A4    | PC4/ADC4 |
//| SCL              |   A5    | PC5/ADC5 |
//| PC6              | RESET   |  PC6     |
//| ADC6/7           |  A6/7   |  ADC6/7  |
//|------------------|---------|----------|

// Para programar con Arduino as ISP:
//|-----------|-------------|-------------|-------------|
//| Señal ISP |  ATtiny85   |  ATtiny84   |  ATmega328P |
//|-----------|-------------|-------------|-------------|
//| RESET     | pin 1 (RES) | Pin 4 (RST) | RST         |
//| VCC       | pin 8 (VCC) | Pin 1 (VCC) | VCC         |
//| SCK       | pin 7 (PB2) | Pin 9 (PA4) | D13  (PB5)  |
//| MISO      | pin 6 (PB1) | Pin 8 (PA5) | D12  (PB4)  |
//| MOSI      | pin 5 (PB0) | Pin 7 (PA6) | D11  (PB3)  |
//| GND       | pin 4 ( 0V) | Pin 14 (0V) | GND         |
//|-----------|-------------|-------------|-------------|

// Sensor DHT11 Azul / DHT22 Blanco
// Visto de frente, agujeros delante, patas abajo:
// 1 VCC / 2 Datos / 3 NC / 4 Masa

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

// Pantalla:
#define ALTURA_MAX (OLED*8-9)
#define PAGINA_ESTADO OLED-1

// CLOCK_SOURCE==0 RC interno
// CLOCK_SOURCE==1 Cristal externo
// CLOCK_SOURCE==2 Oscilador externo baja frecuencia
// CLOCK_SOURCE==3 Oscilador externo alta frecuencia
// CLOCK_SOURCE==4 Cristal (32 kHz) según micro/core
// CLOCK_SOURCE==5 Reservado o variante especial según micro/core
// CLOCK_SOURCE==6 PLL

// Para saber cómo se compila. Solo puede usarse con cristal.
#ifdef CLOCK_SOURCE
 #if CLOCK_SOURCE!=1
 #define SIN_CRISTAL
 #endif
#else
 // No se sabe, entonces será impreciso
 #if defined(__AVR_ATtiny85__)
 #define SIN_CRISTAL
 #endif
#endif
#ifdef SIN_CRISTAL
 #error "No puede usarse sin Cristal"
#endif
#if F_CPU==20000000UL
#define TIPOF " 20MHz"
#elif F_CPU==16000000UL
#define TIPOF " 16MHz"
#elif F_CPU==12000000UL
#define TIPOF " 12MHz"
#elif F_CPU==8000000UL
#define TIPOF "  8MHz"
#else
#error "Frecuencia no soportada"
#endif

// Donde está puesto el sensor DHT/DS18B20
#if defined(__AVR_ATtiny85__)
#define PATA_SENSOR PB2
#define DDR_SENSOR DDRB
#define PIN_SENSOR PINB
#define PORT_SENSOR PORTB
#else
#define PATA_SENSOR PC0
#define DDR_SENSOR DDRC
#define PIN_SENSOR PINC
#define PORT_SENSOR PORTC
#endif 

#if F_CPU==8000000UL
#pragma message "Compilando para 8 MHz"
#endif
#if F_CPU==12000000UL
#pragma message "Compilando para 12 MHz"
#endif
#if F_CPU==16000000UL
#pragma message "Compilando para 16 MHz"
#endif
#if F_CPU==20000000UL
#pragma message "Compilando para 20 MHz"
#endif

// Macros de Compatibilidad
#if defined(__AVR_ATtiny85__)
 #define START_CONTR() TCCR0B=(1<<CS02) | (1<<CS01) | (1<<CS00) // PB2
 #define STOP_CONTR()  TCCR0B=0
 #define CLEAR_FLAGS() TIFR=(1<<TOV0) | (1<<OCF1A)
 #define ENABLE_INTS() TIMSK|=(1<<TOIE0) | (1<<OCIE1A)
 #define LECTURA_HW    TCNT0
 #define DESPLAZAMIENTO 8  // Temporizador 0 es de 8 bits
#else
 // ATMega328P:
 // Uso T1 para contar (pata D5) y T2 para base de tiempo
 #define START_CONTR() TCCR1B=(1<<CS12) | (1<<CS11) | (1<<CS10) // D5
 #define STOP_CONTR()  TCCR1B=0
 #define CLEAR_FLAGS() TIFR1=(1<<TOV1); TIFR2=(1<<OCF2A)
 #define ENABLE_INTS() TIMSK1|=(1<<TOIE1); TIMSK2|=(1<<OCIE2A)
 #define LECTURA_HW    TCNT1
 #define DESPLAZAMIENTO 16 // Temporizador 1 es de 16 bits
#endif
#if F_CPU==16000000L
 #define VALOR_OCR 249
#elif F_CPU==8000000L
 #define VALOR_OCR 124
// Para Attiny85 en 12 y 20 Mhz se suma 1 en un hemiciclo
// Así dan valores exactos.
#elif F_CPU==12000000UL
 // 187,5 -> pongo 187 para que cuente entre 188 y 187
 // Se compensa después
 #define VALOR_OCR 187
#elif F_CPU==20000000UL
 // Hubiera sido 312,5. Cuenta de a cuartos.
 // Se compensa después
 #define VALOR_OCR 155
#endif

// Para iniciar la EEPROM
#define EEPROM_ID1 0xAA
#define EEPROM_ID2 0X55

// No puede ser superior a 254
#define ADC_SAT_ALTO 250

// No puede ser inferior a 1
#define ADC_SAT_BAJO   2

// Valores que dan según el selector usado y los superiores
// a SEL_MAX dará 0. Los que no coincien, dará 4
// La tolerancia de los valores nominales serán de +/-7
//  3,9 kΩ   167  Osc 'Z'
//   12 kΩ   188  Generador
//   33 kΩ   214  Frecuencia
//  100 KΩ   236  Sensor
#define SEL_Z 167
#define SEL_G 188
#define SEL_F 214
#define SEL_S 236
#define SEL_MAX 245

// Según el índice, tendré el valor aproximado del ADC para
// la resistencia correspondiente
const byte selValores[4]={SEL_Z,SEL_G,SEL_F,SEL_S};

// Calibración de 0V
#define RANGO_CAL0 '0'

// Calibración de los divisores de tensión de entrada. Para el
// ATtiny85, los divisores no deberán superar 2,3V y para el
// ATmega328P, 1,1V.
#define RANGO_CALX 'X'
#define RANGO_CALY 'Y'
#define RANGO_CALZ 'Z'

// Para la calibración de las 4 resistencias de selección
#define CAL_SELZ 0
#define CAL_SELG 1
#define CAL_SELF 2
#define CAL_SELS 3

byte EEMEM ee_id1;
byte EEMEM ee_id2;
byte EEMEM ee_band;
byte EEMEM ee_adc_fs0;
byte EEMEM ee_adc_fs[3];
byte EEMEM ee_r[4];

// Tamaño del guardado de las mediciones a mostrar
// Mínimo es el ancho del oled. Más, sirve para precisión.
// Tiene que ser en 8 bits. El valor 255 se reserva para poder
// detectar el fin de captura
#define CAPTURAS_TOTAL 254

// Puntos de la pantalla para que en autoescala muestre una
// onda completa.
#define PUNTOS 100        // Puede ser 128 o menos

// La ESCALA_MINIMA es válida para osciloscopio
// Para el modo generador, debe arrancar en 2
#define ESCALA_MINIMA    0

#if defined(__AVR_ATtiny85__)
 #if F_CPU == 8000000UL
   #define ESCALA_MAXIMA  960  // Máximo 9600 µs
 #elif F_CPU == 12000000UL
   #define ESCALA_MAXIMA  960  // Máximo 9600 µs
 #elif F_CPU == 16000000UL
   #define ESCALA_MAXIMA  960  // Máximo 9600 µs
 #elif F_CPU == 20000000UL
   #define ESCALA_MAXIMA  640  // Máximo 6400 µs
 #else
   #define ESCALA_MAXIMA  960
 #endif
#else
 // Para el ATmega328P también lo limito a 960 para que
 // el menú se comporte igual
 #define ESCALA_MAXIMA    960  
#endif

#define PRESION_PULSADOR       800

// Configuración de las macros de conversión a cadena
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

const char Volver[]    PROGMEM = "<Volver>";

const char Captura[]   PROGMEM = "Captura";
const char AutoEsc[]   PROGMEM = "Autoescala";
const char EscUno[]    PROGMEM = "Sube 1";
const char EscDiez[]   PROGMEM = "Sube 10";
const char EscMax[]    PROGMEM = "Sube toda";
const char Grilla[]    PROGMEM = "Grilla";

const char LibreAuto[] PROGMEM = "Libre/Auto";
const char Flanco[]    PROGMEM = "Flanco";
const char Escuno[]    PROGMEM = "Baja 1";
const char Escdiez[]   PROGMEM = "Baja 10";
const char EscMin[]    PROGMEM = "Baja toda";
const char LinPun[]    PROGMEM = "Lineas o\nPuntos";
const char Escalar[]   PROGMEM = "Estirar";

const char Gen1[]      PROGMEM = "Sube 1";
const char Gen10[]     PROGMEM = "Sube 10";
const char Gen100[]    PROGMEM = "Sube 100";
const char Gen1000[]   PROGMEM = "Sube 1000";
const char Gen1M[]     PROGMEM = "Baja 1";
const char Gen10M[]    PROGMEM = "Baja 10";
const char Gen100M[]   PROGMEM = "Baja 100";
const char Gen1000M[]  PROGMEM = "Baja 1000";
const char GenEn50[]   PROGMEM = "A 50 Hz";
const char GenEn100[]  PROGMEM = "A 100 Hz";
const char GenEn500[]  PROGMEM = "A 500 Hz";
const char GenEn1000[] PROGMEM = "A 1 kHz";
const char GenEn10K[]  PROGMEM = "A 10 kHz";
const char GenEn25K[]  PROGMEM = "A 25 kHz";

const char Frec200MS[] PROGMEM = "Frec 0,2s";
const char Frec1S[]    PROGMEM = "Frec 1s";

const char Anuevo[]    PROGMEM = "A Nuevo";
const char Acerca[]    PROGMEM = "AUTOR";

const char Calibra0[]  PROGMEM = "Cal 0";
const char Calibrax[]  PROGMEM = "Cal X";
const char Calibray[]  PROGMEM = "Cal Y";
const char Calibraz[]  PROGMEM = "Cal Z";
const char Calibrasz[] PROGMEM = "Z 3k9/" STR(SEL_Z);
const char Calibrasg[] PROGMEM = "G 12k/" STR(SEL_G);
const char Calibrasf[] PROGMEM = "F 33k/" STR(SEL_F);
const char Calibrass[] PROGMEM = "S100k/" STR(SEL_S);

enum Menu_Osciloscopio
{
 // Pulsador Más

 MENU_OSC_AUTOESCALA=1,
 MENU_OSC_ESCALA_MAS1,
 MENU_OSC_ESCALA_MAS10,
 MENU_OSC_ESCALA_MAX,
 MENU_OSC_ESCALAR,
 MENU_OSC_GRILLA,
 MENU_OSC_PUL1_VOLVER,

 // Pulsador Menos

 MENU_OSC_CAPTURA,
 MENU_OSC_LIBRE_AUTO,
 MENU_OSC_ESCALA_MENOS1,
 MENU_OSC_ESCALA_MENOS10,
 MENU_OSC_ESCALA_MIN,
 MENU_OSC_LINEAS,
 MENU_OSC_FLANCO,
 MENU_OSC_PUL2_VOLVER,
};

const char* const menuOsc[] PROGMEM =
{
 AutoEsc,EscUno,EscDiez,
 EscMax,Escalar,Grilla,Volver,

 Captura,LibreAuto,Escuno,Escdiez,
 EscMin,LinPun,Flanco,Volver
};

enum Menu_Generador
{
 MENU_GEN_FREC1=1,
 MENU_GEN_FREC10,
 MENU_GEN_FREC100,
 MENU_GEN_FREC1000,
 MENU_GEN_FRECEN50,
 MENU_GEN_FRECEN100,
 MENU_GEN_FRECEN25000,
 MENU_GEN_PUL1_VOLVER,

 MENU_GEN_FREC1M,
 MENU_GEN_FREC10M,
 MENU_GEN_FREC100M,
 MENU_GEN_FREC1000M,
 MENU_GEN_FRECEN500,
 MENU_GEN_FRECEN1000,
 MENU_GEN_FRECEN10000,
 MENU_GEN_PUL2_VOLVER,
};

const char* const menuGen[] PROGMEM =
{
 Gen1, Gen10, Gen100, Gen1000, GenEn50,
 GenEn100,GenEn25K,Volver,
 Gen1M, Gen10M, Gen100M, Gen1000M, GenEn500,
 GenEn1000, GenEn10K, Volver
};

enum Menu_Conf
{
 MENU_CFG_CAL_0=1,
 MENU_CFG_CAL_X,
 MENU_CFG_CAL_Y,
 MENU_CFG_CAL_Z,
 MENU_CFG_PUL1_VOLVER,

 MENU_CFG_CAL_SZ,
 MENU_CFG_CAL_SG,
 MENU_CFG_CAL_SF,
 MENU_CFG_CAL_SS,
 MENU_CFG_ACERCADE,
 MENU_CFG_PUL2_VOLVER,
};

const char* const menuCfg[] PROGMEM =
{
 Calibra0, Calibrax, Calibray, Calibraz, Volver,
 Calibrasz, Calibrasg, Calibrasf, Calibrass, Acerca, Volver
};

enum Menu_Frec
{
 MENU_FRE_1S=1,
 MENU_FRE_PUL1_VOLVER,

 MENU_FRE_200MS,
 MENU_FRE_PUL2_VOLVER
};

const char* const menuFre[] PROGMEM =
{
 Frec1S, Volver,
 Frec200MS, Volver
};

// Para frecuencímetro
volatile unsigned int excesos_contador=0;
volatile byte cuenta_base_tiempo=0;
volatile byte limite_cuentas;

// Para Osciloscopio
volatile byte capturas[CAPTURAS_TOTAL];
volatile byte indice;
int escala;       // Puede tomar valores negativos momentáneos
unsigned int tiempoReal_us;

char rangoActual=RANGO_CALX;

// Bits:
// 0 mostrar grilla
// 1 modo línea     falso = puntos, verdadero = línea
// 2 modo gatillo   Espera cruce, falso: Barrido libre
// 3 auto escala    Arranca con autoescala
// 4 Escalar        Estira por 4 lo visualizado
// 5 Flanco         falso, es flanco asciendente
// 6 Captura        Pone en negativo la imagen del osciloscopio
#define BAND_MOSTRARGRILLA (1<<0)
#define BAND_MODOLINEA     (1<<1)
#define BAND_MODOGATILLO   (1<<2)
#define BAND_AUTOESCALA    (1<<3)
#define BAND_ESCALAR       (1<<4)
#define BAND_FLANCO        (1<<5)
#define BAND_CAPAN         (1<<6)
byte band=0;

// Para leer el pulsador
volatile bool pido=false; // Necesario en modo generador
bool aborta=false;        // Termina de golpe la medición

// Selector anterior
byte lsant=255;          // La primera vez, inicializa

byte adc_fondo_escala;
byte adc_offset;

// Estados de modo
#define MODO_OSCILOSCOPIO 0     // Debe ser 0 OSCILOSCOPIO
#define MODO_GENERADOR    1
#define MODO_CFG          2
#define MODO_FREC         3
#define MODO_SENSOR       4
#define MODO_NOVALIDO     5

volatile byte modoActual=(byte)MODO_OSCILOSCOPIO;

// Variables para el motor del generador
volatile unsigned int contadorFrecuencia;
volatile unsigned int recargaFrecuencia;
int genFrec=100;        // 100 Hz

#if defined(__AVR_ATmega328P__)
bool Sensor=false;
#endif

byte adira(void)
{
 ADCSRA|=(1<<ADSC);
 while(ADCSRA & (1<<ADSC));
 return ADCH;
}

// Habilita interrupciones del temporizador 1 y las generales.
void habilitarT1(void)
{
 aborta=false;
 #if defined(__AVR_ATtiny85__)
 TIMSK|=(1<<OCIE1A);
 #else
 TIMSK1|=(1<<OCIE1A);
 #endif

 // Y hago una primera conversión del ADC para empezar, solo
 // en modo osciloscopio
 if(modoActual==(byte)MODO_OSCILOSCOPIO)
 {
  // Estabilizo el muestreo de onda en ATmega328P
  // Necesita estabilizarse así. No es necesario en ATtiny85
  // pero lo aplico de la misma manera.
  for(int i=0;i<2000;i++)
  {
   adira();
  }
  // Primera captura
  capturas[0]=adira();
  indice=1;
 }
 sei();
}

// Devuelve verdadero si está apagado T1 o por abortar
// Las escalas debajo de 20 no abortan, son rápidas y no nece-
// sitan ser abortadas, y conviene para que pueda calibrarse
bool finCaptura(void)
{
 // Detección de pulsador 1 o 2 para abortar
 // Termina cuando leyó todo, o bien se apretó un pulsador
 #if defined(__AVR_ATtiny85__)

 if(escala>20) aborta=((!(ACSR & (1<<ACO))) | (!(PINB & (1<<PB1))));
 if(aborta) TIMSK&=~(1<<OCIE1A);
 return !(TIMSK&(1<<OCIE1A));

 #else

 // Pata D2 en ATmega328P es pulsador 1, D7 es pulsador 2.
 if(escala>20) aborta=((!(PIND & (1<<PD2))) | (!(PIND & (1<<PD7))));
 if(aborta) TIMSK1&=~(1<<OCIE1A);
 return !(TIMSK1&(1<<OCIE1A));

 #endif
}
/* 
 * -------------------------------------------------------------------------
 * Multiplicador (m) | Tiempo/Punto | Tiempo Pantalla | Frecuencia (1 Ciclo)
 * -------------------------------------------------------------------------
 *       m = 1       |     10 us    |     1.28 ms     |      781,24 Hz
 *       m = 2       |     20 us    |     2.56 ms     |      390.62 Hz
 *       m = 5       |     50 us    |     6.40 ms     |      156.25 Hz
 *       m = 8       |     80 us    |    10.24 ms     |       97.65 Hz
 *       m = 10      |    100 us    |    12.80 ms     |       78.12 Hz
 *       m = 16      |    160 us    |    20.48 ms     |       48.82 Hz
 *       m = 20      |    200 us    |    25.60 ms     |       39.06 Hz
 *       m = 50      |    500 us    |     64.0 ms     |       15.62 Hz
 *       m = 100     |      1 ms    |      128 ms     |        7.81 Hz
 *       m = 200     |      2 ms    |      256 ms     |        3.90 Hz
 *       m = 408     |   4.08 ms    |      522 ms     |        1.91 Hz
 *       m = 816     |   2.04 ms    |     1044 ms     |        0.95 Hz
 * -------------------------------------------------------------------------
 | ---------------------------------------------------- |
 | ADC para cristal de 16 MHz                           |
 | ---------------------------------------------------- |
 | Prescaler | Fadc    | Tiempo real (µs) | Recomendado |
 | ---------------------------------------------------- |
 | /2        | 8 MHz   | **1,625 µs**     | Overclock   |
 | /4        | 4 MHz   | **3,25 µs**      | Overclock   |
 | /8        | 2 MHz   | **6,5 µs**       | Overclock   |
 | /16       | 1 MHz   | **13 µs**        | Límite alto |
 | /32       | 500 kHz | **26 µs**        | Bueno       |
 | /64       | 250 kHz | **52 µs**        | Óptimo      |
 | /128      | 125 kHz | **104 µs**       | Óptimo      |
 | ---------------------------------------------------- |
 | ADC para cristal de 8 MHz                            |
 | ---------------------------------------------------- |
 | Prescaler | Fadc    | Tiempo real (µs) | Recomendado |
 | ---------------------------------------------------- |
 | /2        | 4 MHz   | **3,25 µs**      | Overclock   |
 | /4        | 2 MHz   | **6,5 µs**       | Overclock   |
 | /8        | 1 MHz   | **13 µs**        | Límite alto |
 | /16       | 500 kHz | **26 µs**        | Bueno       |
 | /32       | 250 kHz | **52 µs**        | Óptimo      |
 | /64       | 125 kHz | **104 µs**       | Óptimo      |
 | /128      | 62,5 kHz| **208 µs**       | Conservador |
 | ---------------------------------------------------- |
 | Para ATtiny85/ ATmega328P en el ADC:
 | ADPS2 | ADPS1 | ADPS0 | División |
 | ----- | ----- | ----- | -------- |
 | 0     | 0     | 0     | /2       | Son dos valores iguales
 | 0     | 0     | 1     | /2       |
 | 0     | 1     | 0     | /4       |
 | 0     | 1     | 1     | /8       |
 | 1     | 0     | 0     | /16      |
 | 1     | 0     | 1     | /32      |
 | 1     | 1     | 0     | /64      |
 | 1     | 1     | 1     | /128     |

 Para prescaler de AtMega328P
 CS12,CS11,CS10,Descripción (Prescaler del Timer)
  0    0     1  División por 1
  0    1     0, División por 8
  0    1     1  División por 64
  1    0     0  División por 256
  1    0     1, División por 1024

 Prescaler ATtiny85:
 CS13 CS12 CS11 CS10,Factor de División (Prescaler)
   0    0    0    1     1 (Reloj directo)
   0    0    1    0     2
   0    0    1    1     4
   0    1    0    0     8
   0    1    0    1    16
   0    1    1    0    32
   0    1    1    1    64
   1    0    0    0   128
   1    0    0    1   256
   1    0    1    0   512
   1    0    1    1  1024
   1    1    0    0  2048
   1    1    0    1  4096
   1    1    1    0  8192
   1    1    1    1 16384

 Para cada cristal:
 | ------ | --------------- | -------- | ----------------- |
 | F_CPU  | mínimo práctico | sugerido | Vcc mínima segura |
 | ------ | --------------- | -------- | ----------------- |
 | 8 MHz  | 1 µs            | 2–5 µs   | ~2.7 V            |
 | 12 MHz | 2 µs            | 4 µs     | ~3.3 V            |
 | 16 MHz | 1 µs            | 2–5 µs   | ~4.5 V            |
 | 20 MHz | 2 µs            | 4 µs     | ~5.0 V            |
 | ------ | --------------- | -------- | ----------------- |

 Se deben ajustar el tiempo deseado, según procesador
 y la frecuencia
 FUNCIÓN AUXILIAR DE VALIDACIÓN

   TABLA DE REFERENCIA: LAS 70 ESCALAS REALES VALIDADAS
   CON F_CPU = 12 MHz:

   [RANGO ULTRARRÁPIDO] (Calibraciones Especiales Fijas)
   01. m = 0   --> 6 uS        02. m = 1   --> 8 uS

   [RANGO RÁPIDO] (Prescaler /1 y /2 | Pasos lineales de 10 uS)
   03. m = 2   --> 20 uS       04. m = 3   --> 30 uS       05. m = 4   --> 40 uS
   06. m = 5   --> 50 uS       07. m = 6   --> 60 uS       08. m = 7   --> 70 uS
   09. m = 8   --> 80 uS       10. m = 9   --> 90 uS       11. m = 10  --> 100 uS
   12. m = 11  --> 110 uS      13. m = 12  --> 120 uS      14. m = 13  --> 130 uS
   15. m = 14  --> 140 uS      16. m = 15  --> 150 uS      17. m = 16  --> 160 uS
   18. m = 17  --> 170 uS      19. m = 18  --> 180 uS

   [RANGO INTERMEDIO] (Múltiplos filtrados por módulo del Timer)
   20. m = 20  --> 200 uS      21. m = 22  --> 220 uS      22. m = 24  --> 240 uS
   23. m = 26  --> 260 uS      24. m = 28  --> 280 uS      25. m = 30  --> 300 uS
   26. m = 32  --> 320 uS      27. m = 34  --> 340 uS      28. m = 36  --> 360 uS
   29. m = 40  --> 400 uS      30. m = 44  --> 440 uS      31. m = 48  --> 480 uS
   32. m = 52  --> 520 uS      33. m = 56  --> 560 uS      34. m = 60  --> 600 uS
   35. m = 64  --> 640 uS      36. m = 68  --> 680 uS      37. m = 72  --> 720 uS

   [RANGO LENTO / MILISEGUNDOS] (Saltos espaciados por división de Hardware)
   38. m = 80  --> 800 uS      39. m = 88  --> 880 uS      40. m = 96  --> 960 uS
   41. m = 104 --> 1.04 ms     42. m = 112 --> 1.12 ms     43. m = 120 --> 1.20 ms
   44. m = 128 --> 1.28 ms     45. m = 136 --> 1.36 ms     46. m = 144 --> 1.44 ms
   47. m = 160 --> 1.60 ms     48. m = 176 --> 1.76 ms     49. m = 192 --> 1.92 ms
   50. m = 208 --> 2.08 ms     51. m = 224 --> 2.24 ms     52. m = 240 --> 2.40 ms
   53. m = 256 --> 2.56 ms     54. m = 272 --> 2.72 ms     55. m = 288 --> 2.88 ms
   56. m = 320 --> 3.20 ms     57. m = 352 --> 3.52 ms     58. m = 384 --> 3.84 ms
   59. m = 416 --> 4.16 ms     60. m = 448 --> 4.48 ms     61. m = 480 --> 4.80 ms
   62. m = 512 --> 5.12 ms     63. m = 544 --> 5.44 ms     64. m = 576 --> 5.76 ms
   65. m = 640 --> 6.40 ms     66. m = 704 --> 7.04 ms     67. m = 768 --> 7.68 ms
   68. m = 832 --> 8.32 ms     69. m = 896 --> 8.96 ms     70. m = 960 --> 9.60 ms (MÁXIMA)
================================================================================
   TABLA DE REFERENCIA: LAS 61 ESCALAS REALES VALIDADAS
   CON F_CPU = 8 MHz:

   [RANGO ULTRARRÁPIDO] (Calibraciones Especiales Fijas)
   01. m = 0   --> 10 uS       02. m = 1   --> 12 uS

   [RANGO RÁPIDO] (Prescaler /1 | Pasos lineales exactos de 10 uS)
   03. m = 2   --> 20 uS       04. m = 3   --> 30 uS       05. m = 4   --> 40 uS
   06. m = 5   --> 50 uS       07. m = 6   --> 60 uS       08. m = 7   --> 70 uS
   09. m = 8   --> 80 uS       10. m = 9   --> 90 uS       11. m = 10  --> 100 uS
   12. m = 11  --> 110 uS      13. m = 12  --> 120 uS      14. m = 13  --> 130 uS
   15. m = 14  --> 140 uS      16. m = 15  --> 150 uS      17. m = 16  --> 160 uS
   18. m = 17  --> 170 uS      19. m = 18  --> 180 uS      20. m = 19  --> 190 uS
   21. m = 20  --> 200 uS      22. m = 21  --> 210 uS      23. m = 22  --> 220 uS
   24. m = 23  --> 230 uS      25. m = 24  --> 240 uS      26. m = 25  --> 250 uS

   [RANGO INTERMEDIO] (Múltiplos filtrados por módulo del Timer)
   27. m = 26  --> 260 uS      28. m = 28  --> 280 uS      29. m = 30  --> 300 uS
   30. m = 32  --> 320 uS      31. m = 34  --> 340 uS      32. m = 36  --> 360 uS
   33. m = 38  --> 380 uS      34. m = 40  --> 400 uS      35. m = 44  --> 440 uS
   36. m = 48  --> 480 uS      37. m = 52  --> 520 uS      38. m = 56  --> 560 uS
   39. m = 60  --> 600 uS      40. m = 64  --> 640 uS      41. m = 68  --> 680 uS
   42. m = 72  --> 720 uS      43. m = 76  --> 760 uS      44. m = 80  --> 800 uS

   [RANGO LENTO / MILISEGUNDOS] (Saltos espaciados por división de Hardware)
   45. m = 96  --> 960 uS      46. m = 128 --> 1.28 ms     47. m = 160 --> 1.60 ms
   48. m = 192 --> 1.92 ms     49. m = 224 --> 2.24 ms     50. m = 256 --> 2.56 ms
   51. m = 288 --> 2.88 ms     52. m = 320 --> 3.20 ms     53. m = 352 --> 3.52 ms
   54. m = 384 --> 3.84 ms     55. m = 448 --> 4.48 ms     56. m = 512 --> 5.12 ms
   57. m = 576 --> 5.76 ms     58. m = 640 --> 6.40 ms     59. m = 768 --> 7.68 ms
   60. m = 896 --> 8.96 ms     61. m = 960 --> 9.60 ms (MÁXIMA)
================================================================================
   TABLA DE REFERENCIA: LAS 70 ESCALAS REALES VALIDADAS
   CON F_CPU = 16 MHz:

   [RANGO ULTRARRÁPIDO] (Calibraciones Especiales Fijas)
   01. m = 0   --> 6 uS        02. m = 1   --> 8 uS

   [RANGO RÁPIDO] (Pasos lineales exactos de 10 uS)
   03. m = 2   --> 20 uS       04. m = 3   --> 30 uS       05. m = 4   --> 40 uS
   06. m = 5   --> 50 uS       07. m = 6   --> 60 uS       08. m = 7   --> 70 uS
   09. m = 8   --> 80 uS       10. m = 9   --> 90 uS       11. m = 10  --> 100 uS
   12. m = 11  --> 110 uS      13. m = 12  --> 120 uS      14. m = 13  --> 130 uS
   15. m = 14  --> 140 uS      16. m = 15  --> 150 uS      17. m = 16  --> 160 uS
   18. m = 17  --> 170 uS       

   [RANGO INTERMEDIO] (Múltiplos filtrados por módulo del Timer)
   19. m = 18  --> 180 uS      20. m = 20  --> 200 uS      21. m = 22  --> 220 uS
   22. m = 24  --> 240 uS      23. m = 26  --> 260 uS      24. m = 28  --> 280 uS
   25. m = 30  --> 300 uS      26. m = 32  --> 320 uS      27. m = 34  --> 340 uS
   28. m = 36  --> 360 uS      29. m = 40  --> 400 uS      30. m = 44  --> 440 uS
   31. m = 48  --> 480 uS      32. m = 52  --> 520 uS      33. m = 56  --> 560 uS
   34. m = 60  --> 600 uS      35. m = 64  --> 640 uS      36. m = 68  --> 680 uS
   37. m = 72  --> 720 uS

   [RANGO LENTO / MILISEGUNDOS] (Saltos espaciados por división de Hardware)
   38. m = 80  --> 800 uS      39. m = 88  --> 880 uS      40. m = 96  --> 960 uS
   41. m = 104 --> 1.04 ms     42. m = 112 --> 1.12 ms     43. m = 120 --> 1.20 ms
   44. m = 128 --> 1.28 ms     45. m = 136 --> 1.36 ms     46. m = 144 --> 1.44 ms
   47. m = 160 --> 1.60 ms     48. m = 176 --> 1.76 ms     49. m = 192 --> 1.92 ms
   50. m = 208 --> 2.08 ms     51. m = 224 --> 2.24 ms     52. m = 240 --> 2.40 ms
   53. m = 256 --> 2.56 ms     54. m = 272 --> 2.72 ms     55. m = 288 --> 2.88 ms
   56. m = 320 --> 3.20 ms     57. m = 352 --> 3.52 ms     58. m = 384 --> 3.84 ms
   59. m = 416 --> 4.16 ms     60. m = 448 --> 4.48 ms     61. m = 480 --> 4.80 ms
   62. m = 512 --> 5.12 ms     63. m = 544 --> 5.44 ms     64. m = 576 --> 5.76 ms
   65. m = 640 --> 6.40 ms     66. m = 704 --> 7.04 ms     67. m = 768 --> 7.68 ms
   68. m = 832 --> 8.32 ms     69. m = 896 --> 8.96 ms     70. m = 960 --> 9.60 ms (MÁXIMA)
================================================================================
   TABLA DE REFERENCIA: LAS 61 ESCALAS REALES VALIDADAS
   CON F_CPU = 20 MHz:

   [RANGO ULTRARRÁPIDO] (Calibraciones Especiales Fijas)
   01. m = 0   --> 4 uS        02. m = 1   --> 8 uS

   [RANGO RÁPIDO] (Prescaler /1 | Pasos lineales exactos de 10 uS)
   03. m = 2   --> 20 uS       04. m = 3   --> 30 uS       05. m = 4   --> 40 uS
   06. m = 5   --> 50 uS       07. m = 6   --> 60 uS       08. m = 7   --> 70 uS
   09. m = 8   --> 80 uS       10. m = 9   --> 90 uS       11. m = 10  --> 100 uS
   12. m = 11  --> 110 uS      13. m = 12  --> 120 uS      14. m = 13  --> 130 uS
   15. m = 14  --> 140 uS      16. m = 15  --> 150 uS      17. m = 16  --> 160 uS
   18. m = 17  --> 170 uS      19. m = 18  --> 180 uS      20. m = 19  --> 190 uS
   21. m = 20  --> 200 uS      22. m = 21  --> 210 uS      23. m = 22  --> 220 uS
   24. m = 23  --> 230 uS      25. m = 24  --> 240 uS      26. m = 25  --> 250 uS

   [RANGO INTERMEDIO] (Múltiplos filtrados por módulo del Timer)
   27. m = 26  --> 260 uS      28. m = 28  --> 280 uS      29. m = 30  --> 300 uS
   30. m = 32  --> 320 uS      31. m = 34  --> 340 uS      32. m = 36  --> 360 uS
   33. m = 38  --> 380 uS      34. m = 40  --> 400 uS      35. m = 44  --> 440 uS
   36. m = 48  --> 480 uS      37. m = 52  --> 520 uS      38. m = 56  --> 560 uS
   39. m = 60  --> 600 uS      40. m = 64  --> 640 uS      41. m = 68  --> 680 uS
   42. m = 72  --> 720 uS      43. m = 76  --> 760 uS      44. m = 80  --> 800 uS

   [RANGO LENTO / MILISEGUNDOS] (Saltos espaciados por división de Hardware)
   45. m = 96  --> 960 uS      46. m = 128 --> 1.28 ms     47. m = 160 --> 1.60 ms
   48. m = 192 --> 1.92 ms     49. m = 224 --> 2.24 ms     50. m = 256 --> 2.56 ms
   51. m = 288 --> 2.88 ms     52. m = 320 --> 3.20 ms     53. m = 352 --> 3.52 ms
   54. m = 384 --> 3.84 ms     55. m = 448 --> 4.48 ms     56. m = 512 --> 5.12 ms
   57. m = 576 --> 5.76 ms     58. m = 640 --> 6.40 ms     59. m = 768 --> 7.68 ms
   60. m = 896 --> 8.96 ms     61. m = 960 --> 9.60 ms (MÁXIMA)
================================================================================
*/
bool esEscalaValida(uint16_t m)
{
 if(m<=1) return true; // Casos especiales calibrados fijos

 uint16_t tDeseadoTemp;
 #if F_CPU==12000000UL
 tDeseadoTemp=m*15;
 #elif F_CPU==20000000UL
 tDeseadoTemp=m*25;
 #else
 tDeseadoTemp=m*10;
 #endif

 // Aplico el mismo criterio en ATmega328P para
 // que haga saltos y no tenga tantas escalas, por más que
 // en ese procesador no lo necesite. porque tendría demasiadas
 // escalas intermedias superfluas, y así, más breve, mejora.
 uint8_t ticTemp;
 if(tDeseadoTemp<=255)       ticTemp=1;
 else if(tDeseadoTemp<=510)  ticTemp=2;
 else if(tDeseadoTemp<=1020) ticTemp=4;
 else if(tDeseadoTemp<=2040) ticTemp=8;
 else if(tDeseadoTemp<=4080) ticTemp=16;
 else if(tDeseadoTemp<=8160) ticTemp=32;
 else                        ticTemp=64;

 // Solo los tiempos redondos son los deseados
 return (tDeseadoTemp%ticTemp==0);
}

// Se pasa una escala deseada y se obtiene la real.
// Los valores se ajustan a tiempos exactos, no fraccionarios.
// Pasos puede ir de -10 hasta +10
// m puede ir de 0 a 960
uint16_t configurarEscala(int16_t m, int8_t pasos)
{
 // PROCESAMIENTO Y VALIDACIÓN DE LA ESCALA
 if(pasos!=0) 
 {
  int8_t direccion=(pasos>0)?1:-1;
  byte pasosAbsolutos=(pasos>0)?pasos:-pasos;

  for(byte i=0;i<pasosAbsolutos;i++)
  {
   do
   {
    if(direccion==-1 && m<=ESCALA_MINIMA) {m=ESCALA_MINIMA; break; }
    if(direccion== 1 && m>=ESCALA_MAXIMA) {m=ESCALA_MAXIMA; break; }
    m+=direccion;
   } while(!esEscalaValida(m));
  }
 } 
 else 
 {
  if(!esEscalaValida(m))
  {
   m=ESCALA_MINIMA; 
  }
 }

 // CÁLCULO DEL TIEMPO DESEADO Y CALIBRACIONES ESPECIALES
 #if F_CPU==12000000UL
 unsigned int tiempoDeseado=(unsigned int)m*15;
 #elif F_CPU==20000000UL
 unsigned int tiempoDeseado=(unsigned int)m*25;
 #else
 unsigned int tiempoDeseado=(unsigned int)m*10;
 #endif

 #if F_CPU==20000000UL
  // En 20 MHz, modo osciloscopio
  // 10 son 4 µs. 20 son 8 µs. 25 son 10 µs.
  // No bajar de 4 µs, ya es óptimo/mínimo
  if(m==0) tiempoDeseado=10;            // 4 µs Calibrado
  if(m==1) tiempoDeseado=20;            // 8 µs Calibrado
 #elif F_CPU==16000000UL
  #if defined(__AVR_ATtiny85__)
   if(m==0) tiempoDeseado=6;            // 6 µs Calibrado
   if(m==1) tiempoDeseado=8;            // 8 µs Calibrado
  #else
   // Usado en Arduino UNO, Arduino nano y en Arduino mini
   // pero en ese último no es muy preciso.
   if(m==0) tiempoDeseado=6;            // Supuesto
   if(m==1) tiempoDeseado=8;            // Supuesto
  #endif
 #elif F_CPU==12000000UL
  // En 12 MHz, modo osciloscopio
  // 12 son 8 µs. 15 son 10 µs. 9 son 6 µs
  // No bajar de 6 µs, ya es óptimo/mínimo
  if(m==0) tiempoDeseado=9;             // 6 µs Calibrado
  if(m==1) tiempoDeseado=12;            // 8 µs Calibrado
 #else // 8 MHz
  #if defined(__AVR_ATtiny85__)
   // En 8 MHz, modo osciloscopio, 8 es minimo, 10 óptimo
   if(m==0) tiempoDeseado=10;          // 10 µs Calibrado
   // En 8 MHz, modo generador, 12 mínimo/óptimo
   if(m==1) tiempoDeseado=12;          // 12 µs Calibrado
  #else
   // Para los ATmega328P, un poco más lento
   // En 8 MHz, modo osciloscopio, 12 es minimo
   // Usado en placas Arduino mini de 3,3 V, pero que
   // la frecuencia de 8 MHz no es muy precisa.
   if(m==0) tiempoDeseado=12;         // 12 µs Supuesto
   // En 8 MHz, modo generador, 15 es minimo
   if(m==1) tiempoDeseado=15;         // 15 µs Supuesto
  #endif
 #endif

 // Prescaler del ADC
 if(!modoActual)      
 {
  byte presc;

  #if F_CPU==8000000UL
  if(m<=1) presc=(1<<ADPS1);                   // /4  Crítico
  else if(m==2) presc=(1<<ADPS1)|(1<<ADPS0);   // /8  Límite
  else if(m==3) presc=(1<<ADPS2);              // /16 Bueno
  else presc=(1<<ADPS2)|(1<<ADPS0);            // /32 Óptimo

  #elif F_CPU==12000000UL
  if(m<=1) presc=(1<<ADPS1)|(1<<ADPS0);        // /8  Crítico
  else if(m==2) presc=(1<<ADPS2);              // /16 Límite
  else if(m==3) presc=(1<<ADPS2)|(1<<ADPS0);   // /32 Bueno
  else presc=(1<<ADPS2)|(1<<ADPS1);            // /64 Óptimo

  #elif F_CPU==16000000UL
  if(m<=1) presc=(1<<ADPS1)|(1<<ADPS0);        // /8  Crítico
  else if(m==2) presc=(1<<ADPS2);              // /16 Límite
  else if(m==3) presc=(1<<ADPS2)|(1<<ADPS0);   // /32 Bueno
  else presc=(1<<ADPS2)|(1<<ADPS1);            // /64 Óptimo

  #else     // F_CPU==20000000UL
  if(m==0) presc=(1<<ADPS1)|(1<<ADPS0);        // /8  Crítico
  else if(m==1) presc=(1<<ADPS2);              // /16 Límite
  else if(m==2) presc=(1<<ADPS2)|(1<<ADPS0);   // /32 Bueno
  else presc=(1<<ADPS2)|(1<<ADPS1);            // /64 Óptimo

  #endif

  while(ADCSRA & (1<<ADSC));
  ADCSRA&=~((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));
  ADCSRA|=presc;

  // Conversión de limpieza
  adira();
 }

 // Temporizador 1
 #if defined(__AVR_ATtiny85__)
 TCCR1&=~0x0F; 
 byte tic;

 #if F_CPU==8000000UL || F_CPU==12000000UL || F_CPU==20000000UL
 if(tiempoDeseado<=255)
 {
  TCCR1|=(1<<CS12);
  OCR1C=(byte)(tiempoDeseado-1);
  tic=1;
 } 
 else if(tiempoDeseado<=510)
 {
  TCCR1|=(1<<CS12)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/2)-1);
  tic=2;
 }
 else if(tiempoDeseado<=1020)
 {
  TCCR1|=(1<<CS12)|(1<<CS11);
  OCR1C=(byte)((tiempoDeseado/4)-1);
  tic=4;
 } 
 else if(tiempoDeseado<=2040)
 {
  TCCR1|=(1<<CS12)|(1<<CS11)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/8)-1);
  tic=8;
 }
 else if(tiempoDeseado<=4080)
 {
  TCCR1|=(1<<CS13);
  OCR1C=(byte)((tiempoDeseado/16)-1);
  tic=16;
 }
 else if(tiempoDeseado<=8160)
 {
  TCCR1|=(1<<CS13)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/32)-1);
  tic=32;
 }
 else // Hasta 16320 ticks absolutos
 {
  TCCR1|=(1<<CS13)|(1<<CS11); // Bits correspondientes al prescaler /64 en ATtiny85
  OCR1C=(byte)((tiempoDeseado/64)-1);
  tic=64;
 }
 #else // 16 MHz ATtiny85
 if(tiempoDeseado<=255)
 {
  TCCR1|=(1<<CS12)|(1<<CS10);
  OCR1C=(byte)(tiempoDeseado-1);
  tic=1;
 } 
 else if(tiempoDeseado<=510)
 {
  TCCR1|=(1<<CS12)|(1<<CS11);
  OCR1C=(byte)((tiempoDeseado/2)-1);
  tic=2;
 } 
 else if(tiempoDeseado<=1020)
 {
  TCCR1|=(1<<CS12)|(1<<CS11)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/4)-1);
  tic=4;
 } 
 else if(tiempoDeseado<=2040)
 {
  TCCR1|=(1<<CS13);
  OCR1C=(byte)((tiempoDeseado/8)-1);
  tic=8;
 }
 else if(tiempoDeseado<=4080)
 {
  TCCR1|=(1<<CS13)|(1<<CS10);
  OCR1C=(byte)((tiempoDeseado/16)-1);
  tic=16;
 }
 else if(tiempoDeseado<=8160)
 {
  TCCR1|=(1<<CS13)|(1<<CS11);
  OCR1C=(byte)((tiempoDeseado/32)-1);
  tic=32;
 }
 else
 {
  TCCR1|=(1<<CS13)|(1<<CS11)|(1<<CS10); // Bits para prescaler /64 en modo 16MHz
  OCR1C=(byte)((tiempoDeseado/64)-1);
  tic=64;
 }
 #endif

 OCR1A=OCR1C;

 #if F_CPU==12000000UL
 tiempoReal_us=(((unsigned long)(OCR1C+1)*tic)*2)/3;
 #elif F_CPU==20000000UL
 tiempoReal_us=(((unsigned long)(OCR1C+1)*tic)*2)/5;
 #else
 tiempoReal_us=(unsigned long)(OCR1C+1)*tic;
 #endif

 #else

 // ATMega328P

 #if F_CPU==16000000UL
  #define TICS_POR_US 2
 #else
  #define TICS_POR_US 1
 #endif

 TCCR1A=0;
 TCCR1B=0;

 TCCR1B|=(1<<WGM12); // Modo CTC
 TCCR1B|=(1<<CS11);  // Prescaler /8

 OCR1A=(tiempoDeseado*TICS_POR_US)-1;

 tiempoReal_us=(unsigned long)(OCR1A+1)/TICS_POR_US;

 #endif

 return m; 
}

// Interrupciones
ISR(TIMER1_COMPA_vect)
{
 // El modo osciloscopio es 0. Es más rápido que la comparación
 if(!modoActual)
 {
  if(indice<CAPTURAS_TOTAL)
  {
   capturas[indice++]=ADCH;
   ADCSRA|=(1<<ADSC);          // es ADCSRA|=0x40;
  }
  else
  {
   #if defined(__AVR_ATtiny85__)
   TIMSK&=~(1<<OCIE1A);  // Registro específico del ATtiny85
   #else
   TIMSK1&=~(1<<OCIE1A); // Registro específico del ATmega328P
   #endif
  }
  return;
 }

 // Modo frecuencímetro
 if(modoActual==(byte)MODO_FREC)
 {
  #if defined(__AVR_ATtiny85__)
  #if (F_CPU==12000000UL)

  // TCNT1 es el contador que va desde 0 hasta la dupla OCR1A/C
  // Corrijo las cuentas con decimal en 0,5
  // Esto hace que en 12 MHz sea también preciso
  if(pido=!pido) TCNT1++;

  #endif
  #if (F_CPU==20000000UL)

  // TCNT1 es el contador que va desde 0 hasta la dupla OCR1A/C
  // Corrijo las cuentas con decimal en 0,25.
  // Esto hace que no sea tan preciso. No recomiendo 20 MHz
  // para el modo frecuencímetro. La ventana debería tener
  // el doble de medición para que lo sea, o bien usar en vez
  // de 250 y 50 para medir, valores que sean múltiplos de 4,
  // por ejemplo 252 y 52. Pero no sería exactamente un segundo
  // ni 0,2 segundos pero serían exactas.
  // (Dan ventanas de 1,008 s y 0,208 s).
  static byte ajuste;
  if(++ajuste>=4)
  {
   ajuste=0;
   TCNT1--;
  }

  #endif
  if(++cuenta_base_tiempo>=limite_cuentas)
  {
   STOP_CONTR();      // Termina la lectura
  }
  #else
  excesos_contador++;   // Cambia la función en ATmega328P
  #endif
  return;
 }

 // MODO GENERADOR
 pido=false;
 if(--contadorFrecuencia==0)
 {
  #if defined(__AVR_ATtiny85__)
  PINB=(1<<PB2); 
  #else
  PINC=(1<<PC0);
  #endif
  contadorFrecuencia=recargaFrecuencia;
 }
}

// ISR Base de Tiempo en ATmega328P: Detiene el cronómetro
#if defined(__AVR_ATmega328P__)

ISR(TIMER2_COMPA_vect)
{
 if(++cuenta_base_tiempo>=limite_cuentas)
 {
  STOP_CONTR();      // Termina la lectura
 }
}

#endif

// ISR Contador: Acumula los desbordamientos del Timer 0
#if defined(__AVR_ATtiny85__)

ISR(TIMER0_OVF_vect)
{
 excesos_contador++;    // Cuento en 16 bits + 8 bits = 16777215
}

#endif

/*
| CS13..10 | Divisor |
| -------- | ------- |
| 0100     | /8      |
| 0101     | /16     |
| 0110     | /32     |
| 0111     | /64     |
| 1000     | /128    |
| 1001     | /256    |
| 1010     | /512    |
| 1011     | /1024   |
| 1100     | /2048   |
| 1101     | /4096   |
| 1110     | /8192   |
| 1111     | /16384  |
*/
unsigned long int medirFrecuencia(void)
{
 // Respaldo
 byte guarda_sreg=SREG;
 cli();

 #if defined(__AVR_ATtiny85__)

 //byte guarda_T0A=TCCR0A, guarda_T0B=TCCR0B;
 ////, guarda_TCNT0=TCNT0;
 //byte guarda_T1=TCCR1, guarda_MSK=TIMSK;

 #else
 // En ATmega328P:
 // Timer 1 (Contador) y Timer 2 (Base de tiempo)

 byte guarda_T1A=TCCR1A, guarda_T1B=TCCR1B;
 uint16_t guarda_TCNT1=TCNT1;
 byte guarda_T2A=TCCR2A, guarda_T2B=TCCR2B, guarda_MSK1=TIMSK1, guarda_MSK2=TIMSK2;

 #endif

 // Preparación
 excesos_contador=0;
 cuenta_base_tiempo=0;

 // Configuración del Contador
 #if defined(__AVR_ATtiny85__)

 TCCR0A=0;
 TCNT0=0;
 
 #if F_CPU==20000000

 TCCR1=(1<<CTC1) | (1<<CS13) | (1<<CS11); // /512

 #else

 TCCR1=(1<<CTC1) | (1<<CS13) | (1<<CS10); // /256

 #endif

 // TCNT1 es el contador que va desde 0 hasta la dupla OCR1A/C
 OCR1A=VALOR_OCR;
 OCR1C=VALOR_OCR;

 #else

 TCCR1A=0;
 TCCR1B=0;
 TCNT1=0;
 TCCR2A=(1<<WGM21);
 TCCR2B=(1<<CS22) | (1<<CS21);
 OCR2A=VALOR_OCR;

 #endif

 CLEAR_FLAGS();
 ENABLE_INTS();
    
 sei();
 START_CONTR(); 

 // Espero a que termine
 #if defined(__AVR_ATtiny85__)
 
 while(TCCR0B!=0);

 #else

 while(TCCR1B!=0);

 #endif

 // El resultado tiene 3 bytes. Puedo llegar a multiplicarlo
 // por 125 en caso del cristal de 20 MHz para luego dividirlo
 // por 126, sin perder precisión.
 unsigned long int conteo_final=((unsigned long int)excesos_contador<<DESPLAZAMIENTO)+LECTURA_HW;

 // Restauro
 cli();

 #if defined(__AVR_ATtiny85__)

 //TCCR0A=guarda_T0A;
 //TCCR0B=guarda_T0B;
 ////TCNT0=guarda_TCNT0;
 //TCCR1=guarda_T1;
 //TIMSK=guarda_MSK;

 #else

 TCCR1A=guarda_T1A;
 TCCR1B=guarda_T1B;
 TCNT1=guarda_TCNT1;
 TCCR2A=guarda_T2A;
 TCCR2B=guarda_T2B;
 TIMSK1=guarda_MSK1;
 TIMSK2=guarda_MSK2;

 #endif
    
 SREG=guarda_sreg;

 // Corrijo por si limite_cuentas superan 250 o 50
 // Es el caso del cristal de 20 MHz
 #if F_CPU==20000000UL

 // Debo corregir
 conteo_final*=125;
 return limite_cuentas>=250?(conteo_final/126):(conteo_final/26);

 #else

 return limite_cuentas>=250?conteo_final:(conteo_final*5);

 #endif
}

// Captura según la escala elegida y analiza los datos. Devuel-
// ve la frecuencia x10 y en Gato donde comienza el ciclo para
// hacer la opción gatillo. Si no se quiere y es libre, se ig-
// norará ese resultado y se tomará cero por comienzo.
// Si la frecuencia es cero o p1 es 255, es porque no pudo
// detectarse un ciclo completo.
unsigned long int analiza(unsigned int cap, byte &p1)
{
 #if defined(__AVR_ATtiny85__)

 // Guardo estado del port
 byte ddr=DDRB;
 byte port=PORTB;

 #endif

 cli();
 escala=configurarEscala(cap,0);

 #if defined(__AVR_ATtiny85__)

 // Alta impedancia, para leer el comparador interno.
 // Así leo el pulsador 1 y poder abortar la lectura
 // Y también leo el pulsador 2
 DDRB&=~((1<<PB0) | (1<<PB1));
 PORTB&=~((1<<PB0) | (1<<PB1));

 // Habilito comparador para detectar el pulsador
 ACSR&=~(1<<ACD);

 delai(4);     // Doy tiempo para estabilizar el comparador
 
 #endif

 habilitarT1();         // Lanzo la captura
 while(!finCaptura());  // Espero a que termine

 #if defined(__AVR_ATtiny85__)

 // Devuelvo estado del port
 DDRB=ddr;
 PORTB=port;

 #endif

 if(aborta) return 0xFFFFFFFF;

 // Ya tengo datos. Ahora a analizar.
 // Hago un cálculo de frecuencia
 unsigned long int hz_x10=0;

 // Cálculo de los picos en todo el área de análisis
 byte vMin=255;
 byte vMax=0;
 for(byte i=0;i<CAPTURAS_TOTAL;i++)
 {
  if(capturas[i]>vMax) vMax=capturas[i];
  if(capturas[i]<vMin) vMin=capturas[i];
 }

 byte nivel=(vMin+vMax)>>1;

 int h=(vMax-vMin)>>2;   // 1/4 amplitud
 int nivel_bajo=nivel-h;
 int nivel_alto=nivel+h;

 if(nivel_bajo<0) nivel_bajo=0;
 if(nivel_alto>255) nivel_alto=255;

 p1=255;
 byte ultimo=255;
 byte periodos=0;
 bool armado=false;

 for(byte i=1;i<CAPTURAS_TOTAL;i++)
 {
  byte m_prev=capturas[i-1];
  byte m=capturas[i];
  if(!armado)
  {
   if(((band&BAND_FLANCO)==0))
   {
    if(m<nivel_bajo) armado=true;
   }
   else
   {
    if(m>nivel_alto) armado=true;
   }
  }
  else
  {
   if((band&BAND_FLANCO)==0)
   {
    if(m_prev<nivel && m>=nivel)
    {
     if(p1==255)
     {
      p1=i;
      ultimo=i;
     }
     else
     {
      ultimo=i;
      periodos++;
     }
     armado=false;
    }
   }
   else
   {
    if(m_prev>nivel && m<=nivel)
    {
     if(p1==255)
     {
      p1=i;
      ultimo=i;
     }
     else
     {
      ultimo=i;
      periodos++;
     }
     armado=false;
    }
   }
  }
 }

 if(periodos>0)
 {
  // Calculo sobre el tiempo TOTAL transcurrido entre el
  // primer y último pulso
  unsigned long int deltaTotal_us=(unsigned long)(ultimo-p1)*(unsigned long)tiempoReal_us;
  if(deltaTotal_us>0)
  {
   // Multiplico por periodos ANTES de dividir para ganar
   // resolución
   hz_x10=(10000000UL*periodos)/deltaTotal_us;
  }
 }

 // No se detectó absolutamente nada
 // (Línea plana / Sin flancos)
 if(p1==255) 
 {
  hz_x10=0; // Fuerzo frecuencia a cero
  // Grafica de forma libre desde el inicio de la captura
  p1=0;
 }
 // Hubo señal, pero el ciclo empieza fuera del área visible
 // del OLED
 else if(p1>CAPTURAS_TOTAL-128) 
 {
  // Mantengo hz_x10 calculado arriba para que la
  // autoescala sepa exactamente a qué escala cambiar para
  // centrar la onda.
  // Indico con 255 que el gatillo está fuera de rango
  p1=255;
 }
 
 // Si no entra en los anteriores, p1 conserva su valor
 // original (0 a 127) y hz_x10 conserva su frecuencia calcu-
 // lada con éxito.
 return hz_x10;
}

// Imprime un número entero con espacios de alineación
// a la izquierda
void pantalla_enteros(unsigned long int valor, byte ancho)
{
 unsigned long int temp=valor;
 byte digitos=(valor==0)?1:0;

 // Cuenta cuántos dígitos tiene el número
 while(temp>0)
 {
  temp/=10;
  digitos++;
 }
 // Evita el subdesbordamiento si el número es
 // más ancho que el límite
 if(ancho>digitos)
 {
  byte espacios=ancho-digitos;
  for(byte i=0;i<espacios;i++)
  {
   pantalla_print(' ');
  }
 }
 // Imprime el número final
 pantalla_print(valor);
}

// Imprime una cifra con un decimal y con espacios delante
void pantalla_decimal(unsigned long int val, byte enteros)
{
 pantalla_enteros(val/10,enteros);
 pantalla_print(',');
 pantalla_print(val%10);
}

#if defined(__AVR_ATmega328P__)

void SerialPrintDecimal(unsigned long int val)
{
 Serial.print(val/10);
 Serial.print(',');
 Serial.print(val%10);
}

#endif

void actualizar(int comienzo, unsigned long int hz_x10, unsigned int ms)
{
 // Calculo si es posible hacer lupa.
 // Si es así, multiplico por 2 o 3 lo que se ve
 // Recorro todos los valores. Todos tienen que ser menor a
 // adc_fondo_escala/2 o 3 para que puedan expandirse
 byte lupa=3;
 for(byte i=0;i<128;i++)
 {
  if(capturas[comienzo+i]>=(adc_fondo_escala+adc_offset)/3)
  {
   lupa=2;
   if(capturas[comienzo+i]>=(adc_fondo_escala+adc_offset)/2)
   {
    lupa=1;
    break;
   }
  }
 }

 static byte yAnterior=0;
 static bool primerPunto=true;
 byte vMax=0;

 unsigned int factorY=((unsigned int)(ALTURA_MAX)<<8)/adc_fondo_escala;

 // Líneas de referencia (en coordenadas de pantalla)
 byte y0  =ALTURA_MAX-(((unsigned int)                       factorY)>>8);
 byte y25 =ALTURA_MAX-(((unsigned int)(adc_fondo_escala/4)  *factorY)>>8);
 byte y50 =ALTURA_MAX-(((unsigned int)(adc_fondo_escala/2)  *factorY)>>8);
 byte y75 =ALTURA_MAX-(((unsigned int)(adc_fondo_escala*3/4)*factorY)>>8);
 bool exceso=false;
 for(byte pagina=0;pagina<OLED-1;pagina++)
 { 
  pantalla_cursor(0,pagina);
  pantalla_comienzoDatos();

  primerPunto=true;
  for(byte x=0;x<128;x++)
  {
   // Calculo vpp/%. La onda siempre parte de cero
   // Veo VPP/% a partir de lo que se ve
   // Ponerlo acá ahorra código aunque se procese varias veces
   byte v=capturas[x+comienzo];
   if(v>adc_offset)
   {
    v-=adc_offset;
    if(v>vMax) vMax=v;
   }

   byte byteSalida=0x00;

   // Veo si se produce expansión de la escala
   byte e=((band&BAND_ESCALAR)?x/4:x);

   // Aumento visualización, si corresponde
   byte dato=capturas[comienzo+e]*lupa;

   if(dato>adc_offset) dato-=adc_offset; else dato=0;

   // Encajo la lectura dentro del patrón de puntos
   // de la pantalla, que dará un valor entre 0 y ALTURA_MAX
   // para que las discrepacias de unas pocas cuentas, encajen
   // en una misma hilera de puntos.
   //unsigned int p=((unsigned int)dato*(ALTURA_MAX-1))/(unsigned int)adc_fondo_escala;
   // Si dato supera el fondo de escala, no se muestra
   if(dato<=adc_fondo_escala)
   //if(p<=ALTURA_MAX)
   {
    byte y_pixel=((unsigned int)dato*factorY)>>8;
    if(y_pixel>ALTURA_MAX) y_pixel=ALTURA_MAX;
    byte filaInvertida=ALTURA_MAX-y_pixel;
    if(band&BAND_MODOLINEA)
    {
     if(!primerPunto)
     {
      byte yMin=(filaInvertida<yAnterior)?filaInvertida:yAnterior;
      byte yMax=(filaInvertida>yAnterior)?filaInvertida:yAnterior;
      for(byte y=yMin;y<=yMax;y++)
      {
       if((y/8)==pagina) byteSalida|=(1<<(y%8));
      }
     }
     yAnterior=filaInvertida;
     primerPunto=false;
    }
    else
    {
     if((filaInvertida/8)==pagina) byteSalida|=(1<<(filaInvertida%8));
    }
   }
   else
   {
    exceso=true;
   }
   // GRILLA
   if(band&BAND_MOSTRARGRILLA)
   {
    if((x%20)==0)     // Lineas verticales
    {
     byteSalida|=0x88;
    }
    byte yBase=pagina*8;
    for(byte b=0;b<8;b++)
    {
     int yReal=yBase+b;
     // yReal==0 es y100
     if(yReal==y0||yReal==y50||yReal==y25||yReal==y75||yReal==0)
     {
      if((x%4)==0) byteSalida|=(1<<b); // punteada
     }
    }
   }
   i2c_escribir(byteSalida);
  }
  i2c_parar();
 }

 // Pie de la pantalla

 // Ahora escribo los valores en posiciones fijas
 pantalla_cursor(0,PAGINA_ESTADO);
 pantalla_modoTipo(0);
 pantalla_separacion(1);

 // Pone si está magnificado todo
 pantalla_print(lupa);

 // Autoescala o manual
 pantalla_print((band&BAND_AUTOESCALA)?'A':'M');

 // Si está escalada o normal
 pantalla_print((band&BAND_ESCALAR)?'4':'N');

 pantalla_separacion(3);

 // Verdadero: Espera cruce, falso: Barrido libre
 pantalla_print((band&BAND_MODOGATILLO)?((band&BAND_FLANCO)?'-':'+'):'L');

 pantalla_separacion(0);

 pantalla_enteros(ms,4);

 pantalla_print('u');
 pantalla_separacion(0);
 pantalla_print(' ');

 // Muestra la frecuencia. No muestra debajo de 0,5 Hz
 pantalla_separacion(1);
 if(hz_x10<=5)
 {
  pantalla_print(F("-----"));  // No hay
 }
 else
 {
  // Redondeo a 0,5 Hz
  pantalla_enteros((hz_x10+5)/10,5);
 }

 pantalla_separacion(1);
 pantalla_print('H');

 if(exceso)
 {
  pantalla_print(F("  SAT"));
 }
 else
 {
  // Muestro el porcentaje. No se debe simplificar la fórmula,
  // para poder truncar.
  unsigned int p=(unsigned int)vMax*100/(unsigned int)adc_fondo_escala;
 
  pantalla_enteros(p,4);
  pantalla_print('%');
 }

 // Que tensión seleccionada usa
 pantalla_print(rangoActual);
}

// Obtengo un valor estable del ADC de ADC0/1 para tener una
// lectura firme, usando el prescaler al máximo.
byte leerADCestable(void)
{
 // Guardo configuración actual
 byte Admux=ADMUX;
 byte Adcsra=ADCSRA;

 #if defined(__AVR_ATtiny85__)

 // Leo un selector en pata de reset/ADC0
 // VCC ref y Justificación Izquierda (8 bits)
 ADMUX=(0<<REFS1)|(0<<REFS0)|(1<<ADLAR);

 #else

 // ATmega328P: AVcc como referencia, ajuste a izquierda, ADC1
 ADMUX=(1<<REFS0)|(1<<ADLAR)|1;

 #endif

 // ADC habilitado, prescaler 128
 ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

 // Inicio una conversión de descarte. El micro necesita purgar
 // la carga de la referencia anterior (2,56 V / 1,1 V)
 adira();

 // Lectura válida
 byte valor=adira();

 // Restauro configuración anterior
 ADCSRA=Adcsra;
 ADMUX=Admux;

 return valor;
}

// Leo que selección se hace, al iniciar el equipo
// Se crean selectores en reset
// Las selecciones dan 0, 4, 5, 6, 7 u 8
byte leerSelector(void)
{
 // En los otros modos, no existe abortar
 // Leo un selector en pata de reset/ADC0 en ATtiny85 y en
 // PC1/A1 para el ATmega328P. Y funciona bien.
 // Son leídos cuando termina de medir o cuando es frecuencia.
 // Uso mismos valores y criterio para los dos procesadores,
 // para no hacerlo complicado. Pero en ATmega328P pueden ha-
 // cerse más selecciones, ya que puede recorrer desde 0 hasta
 // 255, cambiando los valores de resistencias, sacando la de
 // 12kΩ que está en el diseño y poniendo en reemplazo, 0 Ω y
 // aumentar la cantidad de posiciones de eeprom para guardar
 // esos valores, y también modificar inicio() para poner los
 // de omisión. En ATmega328P hay más lugar para código nuevo.
 // Ahora inicio la conversión y leo un valor estable
 byte res=leerADCestable();

 // El estado abierto corresponde a Osciloscopio 'X', se dedu-
 // ce al principio.. Por descarte de todos, se obtiene el
 // Osciloscopio 'Y'. Los demás se deben traer desde la cali-
 // bración. Se calibra a través del menú config que se activa
 // manteniendo apretado el pulsador 1 (menos) en el momento de
 // encendido. Se calibran las posiciones 5,6,7 y 8, a causa de
 // la posible dispersión de tolerancias de las resistencias.

 // Son los extremos +/-7 cuentas
 // R_Sel   Extremo_1 Medio Extremo_2 Valor_Sel  Sep. Función
 //    0       144     151    158        4        1   Osc 'Y'

 //  3,9 kΩ    160     167    174        5        6   Osc 'Z'
 //   12 kΩ    181     188    195        6       11   Generador
 //   33 kΩ    207     214    221        7        7   Frecuencia
 //  100 KΩ    229     236    243        8        1   Sensor
 
 // Infinito   245            255        0            Osc 'X'
 
 // Veo en cual encaja la lectura y obtengo el valor de retorno
 if(res>=SEL_MAX) return 0;  // No necesita comparar con eeprom

 // Menores a la calibración de OSC 'Z' será OSC 'Y'
 if(res<SEL_Z-8)  return 4;

 // Fuera del rango +/-7, conviene usar otra resistencia
 // que se adecúe a dicha ventana de dispersión.
 for(byte i=0;i<4;i++)
 {
  byte v=eeprom_read_byte(&ee_r[i]);
  if(abs((int)res-(int)v)<=7)
  {
   return 5+i;
  }
 }
 // Si no son los anteriores, es porque cae entre los 26 huecos
 // de los valores fuera de las tolerancias. Se mostrará como
 // un "no estado válido". Con eso se advertirá el valor inco-
 // recto del selector para corregir la resistencia.
 return 9;
}

// Lectura del pulsador. Verdadero si es presionado. Solo es
// leído cuando termina de medir o cuando es frecuencia o cfg.
// En modo osciloscopio, entre análisis. Los valores son:
// Da 0 si no se apretaron los pulsadores, da 1 si se apretó
// pulsador MENOS, un 2 para el pulsador MAS. Cero también sig-
// nifica falso, para saber si hubo presión de un pulsador.
// Se necesitan unos ms para estabilizar el comparador cuando
// se usa el pulsador 1.
int8_t leerPulsador(void)
{
 // Pido leer pulsadores cuando haya atendido una interrupción
 // cuando es generador porque va a cambiar el port B.
 // No puedo usar a fincaptura(), ya que el port B espera estar
 // alterado antes de ser llamado. Aquí, simplemente si está
 // el modo generador, espero a pido para sincronizar con la
 // interrupción, para hacer todo
 if(modoActual==(byte)MODO_GENERADOR)
 {
  // Debo asegurarme que haya interrupciones
  // para que funcione pido.
  pido=true;
  while(pido);   // Espero a que atienda la interrupción
 }
 // Busco el pulsador diferencial en ATtiny85 que es el pulsa-
 // dor 1 De paso da tiempo a estabilizar la referencia de
 // 2,56 V. Guardo estado del PORT B para no molestar al bus
 // i2C bitbang,  ya que lo uso para leer el pulsador
 #if defined(__AVR_ATtiny85__)
 byte ddr=DDRB;
 byte port=PORTB;

 // Alta impedancia, para leer el comparador interno.
 // Y de paso puedo leer un segundo pulsador en PB1
 DDRB&=~((1<<PB0) | (1<<PB1));
 PORTB&=~((1<<PB0) | (1<<PB1));

 ACSR&=~(1<<ACD);         // Habilito comparador

 // Busco pulsación estable del comparador
 // No puedo usar delay en modo generador
 if(modoActual!=(byte)MODO_GENERADOR) delai(4);   // Estabilizo

 // Leo el pulsador 2
 bool pul2=!(PINB & (1<<PB1));   // 2,2 k a masa

 // Leo pulsador 1
 bool pul=!(ACSR & (1<<ACO));

 #else

 // Leo pulsador 1
 bool pul=!(PIND & (1<<PD2));

 // Leo pulsador 2
 bool pul2=!(PIND & (1<<PD7));

 #endif

 #if defined(__AVR_ATtiny85__)

 // Devuelvo estado del port
 DDRB=ddr;
 PORTB=port;

 #endif

 if(pul) return 1;
 if(pul2) return 2;

 // Nada de lo anterior, es 0
 return 0;
}

// Devuelve la opción elegida
int8_t menu(void)
{
 // Veo si hubo una pulsación para hacer menú. Si no, salgo
 int8_t cual=leerPulsador();

 // Solo si es 1 o 2, hace menú
 if(cual<1) return cual;

 const char* const* ptrMenu;
 byte inicio=0;
 byte finBloque1,finBloque2;

 // Selección de tabla y asignación directa de ambos
 // límites reales
 if(!modoActual)        // MODO_OSCILOSCOPIO
 {
  ptrMenu=menuOsc;
  finBloque1=(byte)MENU_OSC_PUL1_VOLVER;
  finBloque2=(byte)MENU_OSC_PUL2_VOLVER;
 }
 else if(modoActual==(byte)MODO_GENERADOR)
 {
  ptrMenu=menuGen;
  finBloque1=(byte)MENU_GEN_PUL1_VOLVER;
  finBloque2=(byte)MENU_GEN_PUL2_VOLVER;
 
  #if defined(__AVR_ATtiny85__)

  TIMSK&=~(1<<OCIE1A);

  #else

  TIMSK1&=~(1<<OCIE1A);

  #endif

  modoActual=255;   // Para que no actue pido en el menú
 }
 else if(modoActual==(byte)MODO_CFG)
 {
  ptrMenu=menuCfg;
  finBloque1=(byte)MENU_CFG_PUL1_VOLVER;
  finBloque2=(byte)MENU_CFG_PUL2_VOLVER;
 }
 else                   // MODO_FREC
 {
  ptrMenu=menuFre;
  finBloque1=(byte)MENU_FRE_PUL1_VOLVER;
  finBloque2=(byte)MENU_FRE_PUL2_VOLVER;
 }
 // El modo sensor no llama al menú.

 // Ajuste definitivo de ventana según el pulsador detectado
 byte fin=finBloque1;
 if(cual==1)    // Pulsador Menos, debe usar la segunda parte
 {
  inicio=finBloque1;
  fin=finBloque2;
 }

 // Veo si se mantiene apretado
 byte cant=fin-inicio;
 byte i=0;
 while(leerPulsador())
 {
  limpiaX2();
  if(++i>cant) i=1;
  pantalla_println((__FlashStringHelper*)pgm_read_word(ptrMenu+inicio+i-1));
  delai(PRESION_PULSADOR);
 }

 // Devuelvo interrupciones en el modo generador
 if(modoActual==255)
 {
  modoActual=(byte)MODO_GENERADOR;

  #if defined(__AVR_ATtiny85__)

  TIMSK|=(1<<OCIE1A);

  #else

  TIMSK1|=(1<<OCIE1A);

  #endif
 }
 return inicio+i; 
}

// En modo generador, frecuencias se ajustan según la escala
// perfecta.
void activarGenerador(unsigned int fDeseada_Hz)
{
 cli();

 modoActual=(byte)MODO_GENERADOR;

 uint16_t mejorEscala=ESCALA_MINIMA + 2;
 unsigned int mejorRecarga=1;
 unsigned long int mejorFrecuencia_mHz=0;
 unsigned long int mejorError=0xFFFFFFFF;
 bool mejorPerfecta=false;

 // Frecuencia objetivo unificada estrictamente en miliHertz
 unsigned long int objetivo_mHz=(unsigned long int)fDeseada_Hz*1000UL;

 uint16_t esc=configurarEscala(ESCALA_MINIMA+2,0);

 while(esc<=ESCALA_MAXIMA)
 {
  if(tiempoReal_us>0)
  {
   // Frecuencia de la ISR de hardware en miliHertz
   unsigned long int f_isr_mHz=1000000000UL/(unsigned long int)tiempoReal_us;

   // Ambos términos operan en miliHertz
   // de forma homogénea
   // R = F_isr_mHz / (2 * F_objetivo_mHz)
   unsigned long int divisor_r=2UL*objetivo_mHz;
   unsigned long int r_ideal=1;
   
   if(divisor_r>0)
   {
    r_ideal=f_isr_mHz/divisor_r;
   }
   if(r_ideal==0) r_ideal=1;

   // Calculo la frecuencia real final que se va a generar
   // con este r_ideal
   unsigned long int f_calculada_mHz=f_isr_mHz/(2UL*r_ideal);
   
   // Error absoluto en miliHertz
   unsigned long int error=(f_calculada_mHz>objetivo_mHz)?(f_calculada_mHz-objetivo_mHz):(objetivo_mHz-f_calculada_mHz);

   if(error<mejorError)
   {
    mejorError=error;
    mejorEscala=esc;
    mejorRecarga=(unsigned int)r_ideal;
    mejorFrecuencia_mHz=f_calculada_mHz;
    mejorPerfecta=(error==0);
    if(mejorPerfecta) break;
   }
  }

  uint16_t escSiguiente=configurarEscala(esc, 1);
  if(escSiguiente==esc) break; 
  esc=escSiguiente;
 }

 // Consolidación de los registros de la escala ganadora
 configurarEscala(mejorEscala,0);

 // Seteo coordinado de los contadores para la ISR
 recargaFrecuencia=mejorRecarga;
 contadorFrecuencia=mejorRecarga; 

 limpiaX2();

 // Interfaz OLED
 pantalla_println(F("Quiero"));
 pantalla_enteros(fDeseada_Hz, 5);
 pantalla_println(F(",0 Hz"));

 pantalla_print(F("Da "));
 pantalla_println(mejorPerfecta?"=":"#");
 pantalla_decimal(mejorFrecuencia_mHz/100,5); 
 ihz();

 #if defined(__AVR_ATtiny85__)

 DDRB|=(1<<PB2);

 #else

 // Configuro pata de Generador de Funciones. Siempre activa
 // Pongo como salida a D14 (PC0)
 DDRC|=(1<<PC0);

 #endif

 habilitarT1();
}

void restaurarOsc(void)
{
 cli();

 modoActual=(byte)MODO_OSCILOSCOPIO;

 #if defined(__AVR_ATtiny85__)

 DDRB&=~(1<<PB2);          // PB2 vuelve a ser entrada
 PORTB&=~(1<<PB2);         // Sin pullup

 // Configuración ADC (PB2/ADC1)
 // Referencia de 2,56 V
 // (Es como poner ADMUX=0xB1 pero lo detallo)
 ADMUX=(1<<REFS2)|(1<<REFS1)|(0<<REFS0)|(1<<ADLAR)|(1<<MUX0);
 
 delai(20); 

 // Prepara la conversión de limpieza
 ADCSRA=0xC3;  // ADEN, ADSC y Prescaler /8

 #else

 // Caso ATmega328P

 DDRD&=~(1<<PD5);
 PORTD&=~(1<<PD5);   // pull-up desactivado

 DDRC&=~(1<<PC0);    // PC0 vuelve a ser entrada
 PORTC&=~(1<<PC0);   // Sin pullup

 // Configuro ADC para máxima velocidad inicial
 // Mantiene 1.1V y canal ADC0/PC0
 ADMUX=(1<<REFS1) | (1<<REFS0);

 // ADCSRA: Habilito ADC e interrupciones
 // Prescaler inicial de 128 para estabilidad
 ADCSRA=(1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

 // Configuro Temporizador 1 (16 bits)
 TCCR1A=0; 
 TCCR1B=0;
 TCNT1=0;

 // Alinea el resultado para leer solo ADCH
 ADMUX|=(1<<ADLAR);

 #endif

 // Realizar una conversión de "limpieza" para asentar
 // la referencia
 adira();

 // Restaura la escala que tenía el osciloscopio
 escala=configurarEscala(escala,0);
 sei();
}

void limpiaX2(void)
{
 pantalla_limpia();

 #if OLED<8

 pantalla_modoTipo(1);

 #else

 pantalla_modoTipo(2);

 #endif

 pantalla_separacion(1);
 pantalla_cursor(0,0);
}

void limpia(void)
{
 pantalla_limpia();
 pantalla_modoTipo(0);
 pantalla_separacion(1);
}

void hecho(void)
{
 pantalla_print(F("HECHO"));
}

void busco(void)
{
 limpiaX2();
 pantalla_print(F("Busco"));
}

// Verdadero si aborta con pulsador menos. Pulsador más, acepta
// O sea:
// 'Sí' es el más y da falso, 'No' es el menos y da verdadero.
bool veoAborta(void)
{
 pantalla_println(F("<SI/NO>"));
 while(leerPulsador());  // Espero a soltar del menú
 delai(250);
 byte sale;
 do                      // Espero a apretar
 {
  sale=leerPulsador();
 } while(!sale);
 limpiaX2();
 if(sale==2) return false;
 pantalla_print(F("<Aborta>"));
 return true;
}

void anah(byte antes, byte ahora)
{
 pantalla_print(F("Antes:"));
 pantalla_println(antes);
 pantalla_print(F("Ahora:"));
 pantalla_println(ahora);
}
void nono(void)
{
 pantalla_print(F("NO "));
}

// Calibra tensiones del divisor de tensión de entrada
void calibrarRangoGenerico(char rangoActivo)
{
 // Mido a escala 3 para tener promedio
 restaurarOsc();
 byte Gato;

 // Las escalas debajo de 21 no pueden ser abortadas
 analiza(10,Gato);

 unsigned int suma=0;

 adc_offset=eeprom_read_byte(&ee_adc_fs0);

 byte actual=adc_offset;

 // Promediar las muestras (empieza contando desplazado 64)
 for(byte i=64;i<(128+64);i++)
 {
  suma+=capturas[i];
 }

 // Promedio
 suma/=128;

 if(rangoActivo!='0')
 {
  // A lo calibrado le quito el offset, si no es 'CAL E0'
  if(suma<adc_offset)
  {
   suma=0;
  }
  else
  {
   suma-=adc_offset;
  }
  actual=eeprom_read_byte(&ee_adc_fs[rangoActivo-'X']);
 }
 
 // Pongo lo evaluado
 anah(actual,suma);

 if(!veoAborta())
 {
  limpiaX2();

  if(rangoActivo=='0')
  {
   if(suma<=ADC_SAT_BAJO*2)         // En rango
   {
    eeprom_update_byte(&ee_adc_fs0,suma);
   }
   else
   {
    // A pesar de poner sí, no lo valido
    nono();
   }
  }
  else
  {
   // Seguridad mínima. La cantidad de puntos visibles de
   // la onda en la pantalla. No debe saturar
   if(suma>=ALTURA_MAX && suma<=ADC_SAT_ALTO)
   {
    // Guardar en EEPROM según rango
    eeprom_update_byte(&ee_adc_fs[rangoActivo-'X'],suma);
   }
   else
   {
    // A pesar de poner sí, no lo valido
    nono();
   }
  }
  hecho();
 }
 d3000();
}

// Calibra el ADC del selector (Va de 0 a 3). Solo se aprueba
// si el valor de la resistencia está dentro de la tolerancia.
// Valores aproximados esperados:
// SEL_Z = 3,9 kΩ = 167 = Osc 'Z'
// SEL_G =  12 kΩ = 188 = Generador
// SEL_F =  33 kΩ = 214 = Frecuencia
// SEL_S = 100 KΩ = 236 = Sensor
void calibrarSelector(byte r)
{
 byte adcro=leerADCestable();

 // Traigo según el selector usado, para no pasarse
 byte v=selValores[r];    // Valor tentativo
 
 // Pongo valor anterior guardado y el leído
 anah(eeprom_read_byte(&ee_r[r]),adcro);

 // Veo de aprobar. Si no está en rango, no se guarda
 if(!veoAborta())
 {
  limpiaX2();
  if(v-7<=adcro && adcro<=v+7)
  {
   eeprom_update_byte(&ee_r[r],adcro);
  }
  else
  {
   nono();
  }
  hecho();
 }
 d3000();
}

void ihz(void)
{
 pantalla_println(F(" Hz"));
}

void acercade(bool q)
{
 limpiaX2();
 pantalla_println(F(EQUIPO TIPOF));
 pantalla_separacion(0);
 pantalla_println(F(VERSION "\n" FECHA));
 pantalla_separacion(1);
 pantalla_println(F(AUTOR));
 d3000();
}

void modoCFG(void)
{
 restaurarOsc();       // Pone como entrada a PB2
 modoActual=(byte)MODO_CFG;
 limpiaX2();
 pantalla_print(F("CONFIG"));
}

void printFrec(void)
{
 pantalla_println(F("Frecuencia"));
}

void modoFRE(void)
{
 restaurarOsc();       // Pone como entrada a PB2 o PD5/PC0
 modoActual=(byte)MODO_FREC;
 limpiaX2();
 printFrec();
 pantalla_print(F("Cada "));
 if(limite_cuentas>=250) pantalla_println(F("1 s")); else pantalla_println(F("200 ms"));
}

// Pone los valores por omisión en la EEPROM

void inicio(void)
{
 eeprom_update_byte(&ee_id1, EEPROM_ID1);
 eeprom_update_byte(&ee_id2, EEPROM_ID2);
 eeprom_update_byte(&ee_band,(BAND_MODOGATILLO | BAND_AUTOESCALA));
 eeprom_update_byte(&ee_adc_fs0,ADC_SAT_BAJO); // Cal 0V
 eeprom_update_byte(&ee_adc_fs[0],ADC_SAT_ALTO); // Vpp X
 eeprom_update_byte(&ee_adc_fs[1],ADC_SAT_ALTO); // Vpp Y
 eeprom_update_byte(&ee_adc_fs[2],ADC_SAT_ALTO); // Vpp Z
 eeprom_update_byte(&ee_r[0],SEL_Z);   // 3,9 kΩ 167 Osc Z
 eeprom_update_byte(&ee_r[1],SEL_G);   //  12 kΩ 188 Generador
 eeprom_update_byte(&ee_r[2],SEL_F);   //  33 kΩ 214 Frecuencia
 eeprom_update_byte(&ee_r[3],SEL_S);   // 100 KΩ 236 Sensor
}

void delai(int d)
{
 delay(d);
}

void d3000(void)
{
 delai(3000);
}

// Pone un 0
static inline void sensorEn0(void)
{
 DDR_SENSOR|=(1<<PATA_SENSOR);      // Salida
 PORT_SENSOR&=~(1<<PATA_SENSOR);    // 0
}

// Libero línea (pullup). Es como poner un 1
static inline void sensorEn1(void)
{
 PORT_SENSOR|=(1<<PATA_SENSOR);     // 1
 DDR_SENSOR&=~(1<<PATA_SENSOR);     // Entrada con PULLUP
}

#if defined(SENSOR_DHT) || !defined(__AVR_ATtiny85__)
// Pre lectura del sensor
void presensor(void)
{
 sensorEn0();
 delai(18);

 sensorEn1();
 __builtin_avr_delay_cycles((40*F_CPU)/1000000UL);
}
#endif

// Pone el sensor DS18B20 si no se define SENSOR_DHT
#if !defined(SENSOR_DHT) || !defined(__AVR_ATtiny85__)

// Leer estado
static inline byte sensorLeePata(void)
{
 return (PIN_SENSOR & (1<<PATA_SENSOR))!=0;
}

byte sensorInicio(void)
{
 byte r;
 sensorEn0();
 delayMicroseconds(480);
 sensorEn1();
 delayMicroseconds(70);
 r=!sensorLeePata();   // pulso presencia
 delayMicroseconds(410);
 return r;
}

void sensorPoneBit(byte b)
{
 if(b)
 {
  sensorEn0();
  delayMicroseconds(6);
  sensorEn1();
  delayMicroseconds(64);
 }
 else
 {
  sensorEn0();
  delayMicroseconds(60);
  sensorEn1();
  delayMicroseconds(10);
 }
}

byte sensorLeeBit(void)
{
 byte r;
 sensorEn0();
 delayMicroseconds(6);
 sensorEn1();
 delayMicroseconds(9);
 r=sensorLeePata();
 delayMicroseconds(55);
 return r;
}

void sensorPoneByte(byte b)
{
 for(byte i=0;i<8;i++)
 {
  sensorPoneBit(b&1);
  b>>=1;
 }
}

byte sensorLeeByte(void)
{
 byte r=0;
 for(byte i=0;i<8;i++)
 {
  r>>=1;
  if(sensorLeeBit()) r|=0x80;
 }
 return r;
}

int16_t leeDS18B20(void)
{
 byte l,h;
 if(!sensorInicio()) return 32767;
 sensorPoneByte(0xCC); // Saltear ROM
 sensorPoneByte(0x44); // Convertir T

 // Espera conversión
 while(!sensorLeeBit());
 if(!sensorInicio()) return 32767;
 sensorPoneByte(0xCC); // Saltear ROM
 sensorPoneByte(0xBE); // Lee Scratchpad
 l=sensorLeeByte();
 h=sensorLeeByte();
 return ((int16_t)h<<8)|l;
}
#endif

// Según lo conectado, elige. Solo cambia la función, en el
// momento de cambio del conector.
void poneModos(void)
{
 // El modo Config no altera los modos
 if(modoActual==(byte)MODO_CFG) return;

 // Pone un modo. El modo DHT es solo al principio.

 // Selectores:
 // 0) Osciloscopio 'X'
 // 4) Osciloscopio 'Y'
 // 5) Osciloscopio 'Z'
 // 6) Generador
 // 7) Frecuencímetro
 // 8) Sensor de temperatura

 int8_t ls=leerSelector();

 // Si el selector no cambia, no cambio el modo
 // La primera vez lsant tiene 255 para obligar a elegir modo.
 if(ls==lsant) return;

 lsant=ls;
 
 // Indico el modo
 limpiaX2();

 pantalla_println(F("Modo"));

 #if defined(__AVR_ATmega328P__)

 Serial.print(F("Modo "));

 #endif

 if(ls==6)
 {
  #if defined(__AVR_ATmega328P__)

  Serial.println(F("generador"));

  #endif

  pantalla_print(F("Generador"));
  d3000();
  activarGenerador(genFrec);  // Comienzo en 100 Hz
 }
 else if(ls==7)     // Frecuencímetro
 {
  #if defined(__AVR_ATmega328P__)

  Serial.println(F("frecuencímetro"));

  #endif

  printFrec();
  d3000();
  #if F_CPU==20000000UL
  limite_cuentas=252;
  #else
  limite_cuentas=250;
  #endif
  modoFRE();
 }
 else if(ls==8)     // Sensor
 {
  #if defined(__AVR_ATmega328P__)

  Serial.println(F("Sensor"));

  #endif

  pantalla_print(F("Sensor"));
  modoActual=(byte)MODO_SENSOR;

  // Veo cual sensor uso en caso de ATmega328P
  #if defined(__AVR_ATmega328P__)

  Sensor=true;

  // Pre lectura del sensor
  presensor();

  // Respuesta sensor
  if(PIN_SENSOR & (1<<PATA_SENSOR))  // Debería ir a 0
  {
   Sensor=false;              // Error, no está el DHT
  }
  // Puede estar, pero debo verificar
  // Espero de a un µs (5 es el mínimo, 70 por seguridad)
  byte t=70;
  while(PIN_SENSOR & (1<<PATA_SENSOR))      // Espera 1
  {
   __builtin_avr_delay_cycles((F_CPU)/1000000UL);
   if(--t==0) break;
  }
  if(t==0)
  {
   Sensor=false;              // Error, no está el DHT
  }
  else
  {
   // Espero de a un µs (35 es el mínimo, 70 por seguridad)
   t=70;
   while(!(PIN_SENSOR & (1<<PATA_SENSOR)))      // Espera 0
   {
    __builtin_avr_delay_cycles((F_CPU)/1000000UL);
    if(--t==0) break;
   }
   if(t==0)
   {
    Sensor=false;              // Error, no está el DHT
   }
  }

  #endif
 }              // Fin modo sensor
 else if(ls==9)
 {
  modoActual=(byte)MODO_NOVALIDO;
  pantalla_print(F("NO VALIDO"));

  #if defined(__AVR_ATmega328P__)

  Serial.print(F("Selector no válido"));

  #endif

 }
 else
 {
  // Modo osciloscopio.
  #if defined(__AVR_ATmega328P__)
  Serial.print(F("osciloscopio '"));
  #endif
  pantalla_print(F("Oscilosco-\npio "));
  restaurarOsc();

  // Muestreo de prueba para que no se trabe el arranque
  // Traigo valores guardados
  rangoActual=(ls==0?RANGO_CALX:(ls==4?RANGO_CALY:RANGO_CALZ));
  pantalla_print(rangoActual);
  #if defined(__AVR_ATmega328P__)
  Serial.print(rangoActual);
  Serial.println('\'');
  #endif
  d3000();
  adc_fondo_escala=eeprom_read_byte(&ee_adc_fs[rangoActual-'X']);
  adc_offset=eeprom_read_byte(&ee_adc_fs0);
 }
}

// SETUP

void setup(void)
{
 #if defined(__AVR_ATmega328P__)
 
 // Configuarción ATmega328P
 // Para que no queden flotantes, pongo a todas en INPUT_PULLUP
 // y luego excluyo las que necesito. Y de paso favorece a los
 // pulsadores.
 // No toco a A0 y A1 que es por donde se mide.
 // A6 y A7 deben cablearse, ya que no es un port real
 for(byte i=0;i<=13;i++)  pinMode(i,INPUT_PULLUP);
 for(byte i=A2;i<=A5;i++) pinMode(i,INPUT_PULLUP);

 // Apago led
 pinMode(13,OUTPUT);
 digitalWrite(13,LOW);

 // PD5 siempre es entrada sin PULLUP
 // Las patas PD5 y PC0 están unidas. Desacoplo a PD5
 //pinMode(5,INPUT);
 DDRD&=~(1<<PD5);
 PORTD&=~(1<<PD5);   // pull-up desactivado

 // Saco modelo y autor, por port serie.
 Serial.begin(115200);
 Serial.println();
 Serial.println();
 Serial.println(F(EQUIPO));
 Serial.println(F(VERSION "\n" FECHA));
 Serial.println(F(AUTOR));

 #endif

 // Si es la primera vez, inicio la EEPROM
 if(eeprom_read_byte(&ee_id1)!=EEPROM_ID1 || eeprom_read_byte(&ee_id2)!=EEPROM_ID2) inicio();

 // Traigo las opciones
 band=eeprom_read_byte(&ee_band);
 band&=~BAND_CAPAN;               // Quito lo de EEPROM

 pantalla_comienzo();
 pantalla_encendida();

 acercade(false);     // Espera pulsador o selector

 // Por si no selcciono nada, es el de omisión
 modoActual=(byte)MODO_OSCILOSCOPIO;     // Por ahora

 // Pulsador:
 // 0) No hubo pulsación, sigo.
 // 1) Configurar (pulsador menos)
 // 2) A nuevo (pulsador más)
 int8_t lp=leerPulsador();
 while(lp==2)
 {
  // Inicializo EEPROM
  // Espero a reiniciar
  limpiaX2();
  pantalla_println(F("A nuevo"));
  if(!veoAborta())
  {
   limpiaX2();
   inicio();
   hecho();
  }
  d3000();
  lp=1;
 }
 if(lp==1)
 {
  modoActual=(byte)MODO_CFG;
  modoCFG();
  d3000();
 }
}

//  LOOP

void loop(void)
{
 // Según el selector, lo pone en uno u otro modo,
 // excepto el modo CFG, que se mantiene según el inicio.
 poneModos();

 // Pongo en entrada las patas de entrada y salida,
 // para que entren bien cada modo
 #if defined(__AVR_ATmega328P__)

 DDRD&=~(1<<PD5);
 PORTD&=~(1<<PD5);   // pull-up desactivado

 #endif

 // Actúo según el modo elegido
 if(!modoActual) // MODO_OSCILOSCOPIO
 {
  // Mido según la escala actual
  byte Gato;
  byte comienzo=0;

  // También quita el aborto
  unsigned long int hz_x10=analiza(escala,Gato);

  if(!aborta)
  {
   //  Calculo autoescala para que se acomode en la pantalla
   if(band&BAND_AUTOESCALA)
   {
    // Si hubo algo, pero no detecto Hz, usaré la escala máxima
    if(Gato==255)
    {
     escala=configurarEscala(ESCALA_MAXIMA,0);
    }
    else
    {
     // Si se detectó una frecuencia, calculo la escala adecuada
     if(hz_x10>5) // Si hay señal detectable
     {
      // Llevo directo cerca del objetivo
      unsigned int escalaIdeal=1000000UL/hz_x10/PUNTOS;
      // Busco la siguiente dos escalas válidas
      while(escalaIdeal<ESCALA_MAXIMA && !esEscalaValida(escalaIdeal))
      {
       escalaIdeal++;
      }
      escalaIdeal++;
      while(escalaIdeal<ESCALA_MAXIMA && !esEscalaValida(escalaIdeal))
      {
       escalaIdeal++;
      }

      // Ahora busco que las escalas no tiemblen
      bool lejos=false;
      // Busco el siguiente escalón válido hacia arriba
      int t=escala;
      do
      {
       t++;
      } while(t<ESCALA_MAXIMA && !esEscalaValida(t));
      if(escalaIdeal>t) lejos=true;
      // Busco el siguiente escalón válido hacia abajo
      t=escala;
      do
      {
       t--;
      } while(t>ESCALA_MINIMA && !esEscalaValida(t));
      if(escalaIdeal<t) lejos=true;
      if(lejos)
      {
       // Pongo la autoescala
       escala=configurarEscala(escalaIdeal,0);
      }
      else
      {
       // Mantengo la que estaba
       escala=configurarEscala(escala,0);
      }
     }
    }
   }
   // Si tras la búsqueda no hay indicios fiables,
   // asumo muestra limpia desde 0
   if(Gato==255) Gato=0;
   if(band&BAND_MODOGATILLO) comienzo=Gato;

   // Muestro
   actualizar(comienzo,hz_x10,tiempoReal_us);

   if(band&BAND_CAPAN)
   {
    pantalla_invertir(true);
    while(!leerPulsador());
    pantalla_invertir(false);
    delai(PRESION_PULSADOR);
   }
  }
  band&=~BAND_CAPAN;

  // Menú OSCILOSCOPIO
  switch(menu())
  {
   case MENU_OSC_CAPTURA:
    band|=BAND_CAPAN;
    break;
   case MENU_OSC_AUTOESCALA:
    band^=BAND_AUTOESCALA;    // Hace flipflop
    band&=~BAND_ESCALAR;
    // Si hace autoescala, saca al resto
    if(band&BAND_AUTOESCALA)
    {
     band|=BAND_MODOGATILLO;
     //band&=~BAND_FLANCO;
    }
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MAS1:
    escala=configurarEscala(escala,1);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MAS10:
    escala=configurarEscala(escala,10);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MENOS1:
    escala=configurarEscala(escala,-1);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MENOS10:
    escala=configurarEscala(escala,-10);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MAX:
    escala=configurarEscala(ESCALA_MAXIMA,0);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALA_MIN:
    escala=configurarEscala(ESCALA_MINIMA,0);
    band&=~(BAND_AUTOESCALA | BAND_ESCALAR);
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_LIBRE_AUTO:
    band^=BAND_MODOGATILLO;
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_FLANCO:
    band^=BAND_FLANCO;
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_GRILLA:
    band^=BAND_MOSTRARGRILLA;
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_LINEAS:
    band^=BAND_MODOLINEA;
    eeprom_update_byte(&ee_band,band);
    break;
   case MENU_OSC_ESCALAR:
    band^=BAND_ESCALAR;
    eeprom_update_byte(&ee_band,band);
    break;
   default:
    break;
  }
 }
 else if(modoActual==(byte)MODO_GENERADOR)
 {
  // Veo de hacer un menú GENERADOR al apretar un pulsador
  signed int g=genFrec;
  switch(menu())
  {
   case MENU_GEN_FREC1:
    ++genFrec;
    break;
   case MENU_GEN_FREC1M:
    --genFrec;
    break;
   case MENU_GEN_FREC10:
    genFrec+=10;
    break;
   case MENU_GEN_FREC10M:
    genFrec-=10;
    break;
   case MENU_GEN_FREC100:
    genFrec+=100;
    break;
   case MENU_GEN_FREC100M:
    genFrec-=100;
    break;
   case MENU_GEN_FREC1000:
    genFrec+=1000;
    break;
   case MENU_GEN_FREC1000M:
    genFrec-=1000;
    break;
   case MENU_GEN_FRECEN50:
    genFrec=50;               // 50 Hz
    break;
   case MENU_GEN_FRECEN100:
    genFrec=100;              // 100 Hz
    break;
   case MENU_GEN_FRECEN500:
    genFrec=500;              // 500 Hz
    break;
   case MENU_GEN_FRECEN1000:
    genFrec=1000;             // 1000 Hz
    break;
   case MENU_GEN_FRECEN10000:
    genFrec=10000;             // 10000 Hz
    break;
   case MENU_GEN_FRECEN25000:
    genFrec=25000;             // 25000 Hz
    break;
   case MENU_GEN_PUL1_VOLVER:
   case MENU_GEN_PUL2_VOLVER:
    activarGenerador(genFrec);  // Sigue en la que estaba
    break;
   default:
    break;
  }
  // Por los múltiplos, no genera más allá de 25 kHz.
  // Pero el registro es de 16 bits, genero hasta 25 kHz
  if(genFrec<1)
  {
   genFrec=1;
   g=0;
  }
  if(genFrec>25000)
  {
   genFrec=25000;
   g=0;
  }
  if(g!=genFrec) activarGenerador(genFrec);
 }
 else if(modoActual==(byte)MODO_CFG)
 {
  // Modo configurador
  switch(menu())
  {
   case MENU_CFG_CAL_0:
    calibrarRangoGenerico(RANGO_CAL0);
    modoCFG();
    break;
   case MENU_CFG_CAL_X:
    calibrarRangoGenerico(RANGO_CALX);
    modoCFG();
    break;
   case MENU_CFG_CAL_Y:
    calibrarRangoGenerico(RANGO_CALY);
    modoCFG();
    break;
   case MENU_CFG_CAL_Z:
    calibrarRangoGenerico(RANGO_CALZ);
    modoCFG();
    break;
   case MENU_CFG_CAL_SZ:
    calibrarSelector(CAL_SELZ);
    modoCFG();
    break;
   case MENU_CFG_CAL_SG:
    calibrarSelector(CAL_SELG);
    modoCFG();
    break;
   case MENU_CFG_CAL_SF:
    calibrarSelector(CAL_SELF);
    modoCFG();
    break;
   case MENU_CFG_CAL_SS:
    calibrarSelector(CAL_SELS);
    modoCFG();
    break;
   case MENU_CFG_ACERCADE:
    acercade(true);
   case MENU_CFG_PUL1_VOLVER:
   case MENU_CFG_PUL2_VOLVER:
    modoCFG();
    break;
   default:
    break;
  }
 }
 else if(modoActual==(byte)MODO_FREC)
 {
  // Modo frecuencímetro
  // Entra y sale cada un segundo o cada 200 ms,
  // Al terminar de medir, busca el menú
  // Con cristal de 20 MHz, la ventana de medición varía lige-
  // ramente en pos de precisión y no de tiempos.
  pantalla_cursor(0,4);
  pantalla_enteros(medirFrecuencia(),7);
  ihz();
  switch(menu())
  {
   case MENU_FRE_200MS:
    #if F_CPU==20000000UL
    limite_cuentas=52; 
    #else
    limite_cuentas=50; 
    #endif
    modoFRE();
    break;    
   case MENU_FRE_1S:
    #if F_CPU==20000000UL
    limite_cuentas=252;
    #else
    limite_cuentas=250;
    #endif
    modoFRE();
    break;
   case MENU_FRE_PUL1_VOLVER:
   case MENU_FRE_PUL2_VOLVER:
    modoFRE();
    break;
   default:
    break;
  }
 }
 else if(modoActual==(byte)MODO_SENSOR)
 {
  // Modo Sensor

  d3000();
  pantalla_cursor(0,0);

  // Leo el sensor
  #if defined(__AVR_ATmega328P__)

  if(Sensor)
  {

  #endif
   #if defined(SENSOR_DHT) || !defined(__AVR_ATtiny85__)

   byte data[5]={0,0,0,0,0};
   presensor();

   // Respuesta sensor
   if(PIN_SENSOR & (1<<PATA_SENSOR))  // Debería ir a LOW
   {
    busco();
    return;        // Va a loop
   }

   uint16_t m=60000;
   while(!(PIN_SENSOR & (1<<PATA_SENSOR)))     // Espera HIGH
   {
    if(--m==0) return;             // Va a loop
   }
   m=60000;
   while(PIN_SENSOR & (1<<PATA_SENSOR))        // Espera LOW
   {
    if(--m==0) return;             // Va a loop
   }

   // Lectura 40 bits
   for(byte i=0;i<40;i++)
   {
    while(!(PIN_SENSOR & (1<<PATA_SENSOR)));     // espera HIGH
    __builtin_avr_delay_cycles((30*F_CPU)/1000000UL);
    if(PIN_SENSOR & (1<<PATA_SENSOR)) data[i/8]|=(1<<(7-(i%8)));
    while(PIN_SENSOR & (1<<PATA_SENSOR));        // fin del bit
   }
   if(data[4]!=(data[0]+data[1]+data[2]+data[3]))
   {
    busco();
    return;        // Va a loop
   }
   int16_t t,h;

   // Veo cual DHT es
   if(data[0]<=3 && data[2]<=3)
   {
    // DHT22
    h=((uint16_t)data[0]<<8) | data[1];
    t=((uint16_t)data[2]<<8) | data[3];
    if(t & 0x8000)
    {
     t&=0x7FFF;
     t=-t;
    }
   }
   else
   {
    // DHT11
    h=(int16_t)data[0]*10;
    t=(int16_t)data[2]*10;
   }
   pantalla_print(F("T:"));
   pantalla_print(t>0?' ':'-');
   pantalla_decimal(t>0?t:-t,3);
   pantalla_print(F(" C\nH:"));
   pantalla_decimal(h,4);
   pantalla_print(F(" %"));

   #if defined(__AVR_ATmega328P__)

   Serial.print(F("T:"));
   Serial.print(t>0?' ':'-');
   SerialPrintDecimal(t>0?t:-t);
   Serial.print(F(" C\nH:"));
   SerialPrintDecimal(h);
   Serial.print(F(" %"));

   #endif

   #endif
  #if defined(__AVR_ATmega328P__)

  }

  #endif

  #if defined(__AVR_ATmega328P__)

  if(!Sensor)
  {

  #endif
   #if !defined(SENSOR_DHT) || !defined(__AVR_ATtiny85__)
   int16_t t10=leeDS18B20();
   if(t10!=32767)
   {
    bool neg=false;
    if(t10<0)
    {
     t10=-t10;
     neg=true;
    }

    // En el DS18B20 la temperatura viene en pasos de 1/16 °C.
    t10=(t10*10)>>4;
    pantalla_print(F("T:"));
    pantalla_print(neg?'-':' ');
    pantalla_decimal(t10,3);
    pantalla_println(F(" C"));

    #if defined(__AVR_ATmega328P__)

    Serial.print(F("T:"));
    Serial.print(neg?'-':' ');
    SerialPrintDecimal(t10);
    Serial.println(F(" C"));

    #endif

   }
   else
   {
    busco();
   }

  #endif
  #if defined(__AVR_ATmega328P__)

  }

  #endif
 }
 else     //  if(modoActual==(byte)MODO_NOVALIDO)
 {
  // Se podría poner una advertencia, pero ocupa código.  
 }
}
// Fin
