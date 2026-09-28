# Nanoosciloscopio con ATtiny85

Osciloscopio digital compacto basado en ATtiny85, con pantalla OLED,
generador de funciones, frecuencímetro y modo sensor de temperatura
integrados, diseñado para funcionar con una cantidad mínima de
componentes (solo 5 pines de I/O en el ATtiny85). También compila,
sin cambiar la lógica del programa, para **ATmega328P** (Arduino
Nano / Pro Mini).

👉 English version [README_en.md](README_en.md)

👉 Versão em português [README_br.md](README_br.md)

👉 Versión en castellano, a continuación

> **Nota:** este repositorio reemplaza al prototipo anterior de este
> mismo proyecto. La versión base (**V3.3**) tiene un circuito
> impreso propio (**NOS41**), un esquema de pulsadores distinto al
> del prototipo original, y agrega el modo sensor de temperatura.
> Versiones posteriores, hasta la actual (**V3.5**), suman soporte
> de cristales de 12 y 20 MHz, uso sin cristal (oscilador RC interno
> calibrado contra la frecuencia de línea) y una detección de
> accesorio por "vecino más cercano calibrado", y elimina diodos
> para hacer un circuito impreso nuevo, (**NOS43**) aunque sirve el
> anterior, sin esos diodos.
>
> Ver
> [Funcionamiento_es.md](Funcionamiento_es.md) para el detalle.

Equipo armado mostrando la frecuencia de línea de 50 Hz.
[![NOS41 Mostrando 50 Hz de línea](Fotos/50Hz.jpg)](Fotos/50Hz.jpg)

Detalle de armado:
[![Prototipo NOS41 con batería](Fotos/ATtiny85/IMG_20260527_131120.jpg)](Fotos/ATtiny85/IMG_20260527_131120.jpg)

Prototipo con Arduino Nano y pantalla de 128x32
[![Arduino nano](Fotos/Arduino_NANO_128x32/IMG_20260410_170244.jpg)](Fotos/Arduino_NANO_128x32/IMG_20260410_170244.jpg)

---

## Especificaciones

- **MCU:** ATtiny85 (principal), adaptable a ATmega328P.
- **Display:** OLED SSD1306 (128x64 / 128x32) por I2C bit-banged
  (sin `Wire.h`, sin librería gráfica externa).
- **Escala de tiempo:** ajustable, calibrada específicamente para
  cristales de 8, 12, 16 y 20 MHz (con corrección de fase en los
  que no dan un múltiplo entero de µs), y también para el modo sin
  cristal (RC interno, solo a 8 MHz).
- **Resolución:** 8 bits (ADC interno del AVR).
- **Selección de modo:** un único pin ADC con selector resistivo —
  sin pines dedicados por función.
- **Idioma de menús:** castellano por omisión; también compila en
  inglés o portugués.
- **Funciones integradas:**
  * Osciloscopio (autoescala, disparo por flanco, modo línea o
    puntos, barrido libre o gatillado, "estirar" x4, congelar
    captura, filtro de ruido por amplitud mínima).
  * Generador de funciones (onda cuadrada, 1 Hz a 25 kHz).
  * Frecuencímetro (hasta ~1 MHz en señales cuadradas).
  * Sensor de temperatura (DS18B20 o DHT11/DHT22, autodetecta).
- Menú completo en pantalla para calibración, sin PC conectada,
  incluyendo la calibración del propio reloj interno cuando se
  compila sin cristal.

---

## Hardware necesario

- ATtiny85 (principal), adaptable a ATmega328P. Para mediciones de
  tiempo/frecuencia precisas conviene un **cristal externo**; el
  equipo también puede compilarse sin cristal (oscilador RC interno,
  ver más abajo), a costa de precisión y de calibración manual.
- OLED SSD1306 (128x64 u 128x32, configurable por `#define`).
- Circuito impreso propio: **NOS43** — ver [`hardware/`](hardware/)
  (fuente en `nos43.xcf`).
- 2 pulsadores.
- Opcional: módulo de carga tipo TP4056 + batería de litio 3,7 V,
  interruptor.

### Pines — ATtiny85

| Pata | Función              | Señal        |
|------|----------------------|--------------|
| 1    | RESET / Selector     | PB5/ADC0     |
| 2    | Cristal A            | PB3/ADC3     |
| 3    | Cristal B            | PB4/ADC2     |
| 4    | MASA                 | 0 V          |
| 5    | SDA' (I2C bitbang)   | PB0/AIN0     |
| 6    | SCL' (I2C bitbang)   | PB1/AIN1     |
| 7    | Sal/Ent/Frec/Sensor  | PB2/ADC1/T0  |
| 8    | Alimentación         | VCC          |

> En una compilación sin cristal, las patas 2 y 3 (Cristal A/B)
> quedan libres para otro uso, ya que no hace falta conectar nada
> en ellas.

### Pines — ATmega328P (Arduino Nano, probado)

| Función           | Pata  | Señal |
|-------------------|-------|-------|
| Sal/Ent/Sensor     | A0    | PC0   |
| Frecuencímetro     | D5    | PD5   |
| SDA' (bitbang)     | D9    | PB1   |
| SCL' (bitbang)     | D10   | PB2   |
| Pulsador 1         | D2    | PD2   |
| Pulsador 2         | D7    | PD7   |
| Selector           | A1    | PC1   |
| Capacitor          | AREF  | VREF  |

> ⚠️ En el ATmega328P, la salida del **generador de funciones usa el
> mismo pin A0/PC0** que la entrada del osciloscopio y del sensor —
> no un pin independiente D8/PB0 como se había descrito en algún
> momento. Ver `activarGenerador()` en el código: para ATmega328P
> alterna `PINC=(1<<PC0)` (configurado como salida con
> `DDRC|=(1<<PC0)`), y solo en ATtiny85 usa un pin propio (PB2), por
> ser el único disponible además del selector y el bus I2C.

Esquemáticos completos en [`hardware/`](hardware/):
- `Esquema_Nano-Osciloscopio_ATtiny85.pdf` / `.json`
- `Esquema_Nano-Osciloscopio_ATmega328P.pdf` / `.json`
- `nos43.xcf` — diseño de circuito impreso NOS43.

### Selector de modo (una resistencia hace todo)

| Resistencia | ADC aprox. | Modo             |
|-------------|-----------:|------------------|
| Corto a masa (0 Ω) | ~151 | Osciloscopio 'X' |
| 5,6 kΩ      | ~172       | Osciloscopio 'Y' |
| 10 kΩ       | ~186       | Osciloscopio 'Z' |
| 22 kΩ       | ~203       | Generador        |
| 47 kΩ       | ~221       | Frecuencímetro   |
| 150 kΩ      | ~241       | Sensor           |
| Abierto (∞) | ~250-255   | Sin accesorio (modo espera) |

Los valores de arriba son los de fábrica. Se guardan en EEPROM y se
recalibran desde el propio menú (todos salvo Osciloscopio X, que es
un corto a masa fijo por diseño); una vez calibrada una posición, el
firmware ya no la compara contra una ventana fija, sino que detecta
el accesorio conectado por cercanía al valor calibrado más próximo
entre las 6 posiciones. Detalle completo, con el algoritmo de
detección y las ventanas de tolerancia, en
[Funcionamiento_es.md](Funcionamiento_es.md).

---

## 📷 Evolución del prototipo

Las fotos en [`Fotos/ATtiny85/`](Fotos/ATtiny85/) documentan tres
etapas del desarrollo:

1. **Placa NOS41 grabada** (14/05) — el circuito impreso recién
   hecho, todavía sin componentes.
2. **Prototipo previo en protoboard** (15/05) — un armado anterior
   sobre placa perforada, usado para probar la lógica antes de
   pasar al impreso definitivo.
3. **NOS41 armada** (27/05 en adelante) — la placa final con todos
   los componentes soldados, OLED y batería.

En [`Fotos/Arduino_NANO_128x32/`](Fotos/Arduino_NANO_128x32/) hay
además fotos del armado de pruebas sobre un Arduino Nano con OLED
de 128x32.

---

## ⚡ Sección Generador de Funciones

Permite generar ondas cuadradas entre **1 Hz y más de 20 kHz**.
* **Funcionamiento:** Reconfigura el Timer y utiliza el pin de
  entrada como salida.
* **Precisión:** Si la frecuencia es exacta, se muestra el símbolo
  `=`. Si es una aproximación, se muestra `#`.
* **ATmega328P:** La salida se genera en el **mismo pin de
  entrada/sensor (A0/PC0)** — no en un pin independiente — para no
  sumar un pin dedicado más allá de los ya usados por osciloscopio
  y sensor.

## 📈 Sección Frecuencímetro

Mide frecuencias de ondas cuadradas (nivel lógico 0 a VCC)
inyectadas en el pin de entrada, alcanzando fácilmente **1 MHz**.
* **ATtiny85:** Utiliza el contador interno **T0**. Es fundamental
  compilar sin `millis()` para evitar conflictos con el contador de
  tiempo.
* **ATmega328P:** Utiliza el contador **T1** en el pin **D5**, lo
  que permite separar la entrada de medición de la de frecuencia.
* *Nota:* Para señales menores a 10 kHz que no sean cuadradas, se
  recomienda usar el **Modo Osciloscopio**.

---

## 🚀 Introducción

La documentación y el código están centrados principalmente en el
**ATtiny85**. Sin embargo, el sistema ha sido adaptado para ser
compatible con el **ATmega328P**.

> **Nota:** Existe una descripción tentativa para el uso de un
> **ATtiny84**, aunque por el momento no se incluye código
> específico adaptado para este modelo.

### Requisitos de Software

Para compilar el código en el **ATtiny85**, se utilizó el **IDE de
Arduino 1.8.19** con las siguientes especificaciones indispensables:
* **Core:** [ATtinyCore 1.5.2](http://drazzy.com/package_drazzy.com_index.json)
* **Opciones de compilación:**
    * `No millis()` (Obligatorio para maximizar Flash y evitar
      conflictos con los contadores usados por frecuencímetro y
      generador).
    * `LTO habilitado`.
    * `Sin bootloader`.

Para **ATmega328P**, seleccioná directamente esa placa (Arduino
Nano / Pro Mini) en el IDE — el mismo `.ino` detecta el micro por
`#if defined(__AVR_ATtiny85__)` y ajusta pines, timers y
periféricos automáticamente.

### Selección de cristal / fuente de reloj

El firmware valida internamente las escalas de tiempo del
osciloscopio para **8, 12, 16 y 20 MHz**, así que en ATtinyCore
podés elegir cualquiera de esos cuatro cristales según lo que
necesites (menú **Tools → Clock**):

| Cristal | Conviene para                                        | Vcc mínima segura |
|---------|-------------------------------------------------------|--------------------|
| 8 MHz   | Alimentación a batería (menor consumo)                | ~2,7 V             |
| 12 MHz  | Punto intermedio                                       | ~3,3 V             |
| 16 MHz  | Alimentación con fuente externa de 5 V (mayor precisión/velocidad) | ~4,5 V |
| 20 MHz  | Máxima velocidad, requiere Vcc casi nominal            | ~5,0 V             |

### Uso sin cristal (oscilador RC interno) — solo ATtiny85

El equipo también puede compilarse sin cristal externo, usando el
oscilador RC interno del ATtiny85 a 8 MHz (menú **Clock Source →
Internal 8 MHz** en ATtinyCore, o cualquier compilación donde el
core defina `CLOCK_SOURCE==0`). Esto libera las dos patas del
cristal (2 y 3) para otro uso, pero el RC interno no es preciso de
fábrica ni estable con la temperatura, así que hay que calibrarlo
contra una frecuencia conocida (línea de 50 o 60 Hz) desde el propio
menú del equipo. El procedimiento paso a paso está en
[Funcionamiento_es.md](Funcionamiento_es.md).

> ⚠️ Esta opción solo compila a 8 MHz: a cualquier otra frecuencia
> con `CLOCK_SOURCE==0` el `.ino` da un error de compilación a
> propósito ("No puede usarse sin Cristal que no sea a 8 MHz y solo
> para pruebas"), porque la calibración solo tiene sentido a esa
> velocidad.

### Habilitar el oscilador RC interno en ATmega328P (MiniCore)

El core nativo de Arduino para ATmega328P no tiene opción de fuente
de reloj: siempre asume cristal externo. Para poder compilar
también un Arduino Nano / Pro Mini (ATmega328P) sin cristal, hace
falta el core [MiniCore](https://github.com/MCUdude/MiniCore) y
agregar una línea a su `boards.txt`, por ejemplo:

```
~/.arduino15/packages/MiniCore/hardware/avr/<versión>/boards.txt
```

Buscando el bloque de la opción "Internal 8 MHz" de esa placa y
agregando, al final de ese bloque, una línea que le pase el flag de
compilación equivalente:

```
328.menu.clock.8MHz_external.build.extra_flags=-DCLOCK_SOURCE=0
```

> El nombre exacto de la clave (`328.menu.clock.<algo>`) puede
> variar según la versión de MiniCore instalada. Conviene abrir el
> `boards.txt` propio, ubicar el bloque real correspondiente a
> "Internal 8 MHz" y agregar la línea ahí, en vez de asumir que la
> clave de arriba coincide letra por letra con tu instalación.

### Idioma de los menús

Por omisión el firmware compila con los textos de menú en
castellano (`IDIOMA_ES`, implícito). Para compilarlo en inglés o
portugués, agregá una de estas líneas cerca del principio del
`.ino`, antes del resto de las definiciones:

```cpp
#define IDIOMA_EN   // Inglés
#define IDIOMA_BR   // Portugués
```

### Ajuste fino de detección de señal

`AMPLITUD_MINIMA_DETECCION` (en el `.ino`, 4 cuentas de ADC por
omisión) es el umbral mínimo de amplitud para que el firmware
considere que hay una señal real y no solo ruido de fondo, tanto
para calcular la frecuencia como para ubicar el punto de disparo.
Si el equipo "inventa" una frecuencia con la entrada sin conectar,
conviene subir este valor; si en cambio ignora señales reales muy
chicas, conviene bajarlo.

### Puesta en marcha

1. Cargá el código de [`src/`](src/) (`NOS_V3.5.ino` + `I2C.ino`).
2. Programá el ATtiny85 vía ISP (ver tabla de pines ISP al inicio
   del `.ino`), o subí directo si usás un Arduino Nano/Pro Mini.
3. Calibrá tensiones de entrada desde el menú de configuración.
4. Conectá la señal a medir y ajustá los parámetros desde el propio
   equipo.

---

## Estructura del repositorio

```
.
├── src/
│   ├── NOS_V3.5.ino     # Programa principal: ADC, timers, menús, modos
│   └── I2C.ino          # Driver I2C por software (bit-banging) para el OLED
├── hardware/
│   ├── Esquema_Nano-Osciloscopio_ATtiny85.pdf/.json
│   ├── Esquema_Nano-Osciloscopio_ATmega328P.pdf/.json
│   └── nos43.xcf         # Fuente gráfica (GIMP) relacionada al diseño
├── Fotos/
│   ├── ATtiny85/              # Prototipo NOS41/NOS43 armado
│   └── Arduino_NANO_128x32/   # Prototipo de pruebas sobre Arduino Nano
├── README.md
├── README_en.md
├── README_br.md
├── Funcionamiento_es.md
├── Funcionamiento_en.md
├── Funcionamiento_br.md
├── LICENSE_es
└── LICENSE_en
```

---

## Limitaciones conocidas

- Sin cristal externo, la precisión de tiempos/frecuencia depende
  de una calibración manual del oscilador RC interno (solo
  ATtiny85, solo a 8 MHz) y puede desviarse con la temperatura.
- Trigger básico, sin memoria de adquisición prolongada.
- Soporte para ATmega328P probado en menor medida que ATtiny85.

## Contribuciones

Se aceptan sugerencias, correcciones y variantes de hardware vía
issues o pull requests. Se agradece informar mejoras o errores.

## Autor

Alejandro F. Fernández (alecelular)
[nanoosciloscopio@gmail.com](mailto:nanoosciloscopio@gmail.com)

## Licencia

Uso no comercial — ver [`LICENSE_es`](LICENSE_es) /
[`LICENSE_en`](LICENSE_en).

Si querés usarlo comercialmente, contactame:
[nanoosciloscopio@gmail.com](mailto:nanoosciloscopio@gmail.com)

## Apoyar el proyecto

Si te resultó útil, podés invitarme un café:
[![Invitame un café](https://cdn.cafecito.app/img/buttons/button_1.svg)](https://cafecito.app/rsp148)

---

*Espero que disfruten este proyecto tanto como yo disfruté su
desarrollo.*
