# Detalle del funcionamiento

Este documento complementa al [README](README.md): explica cómo se usa el equipo en la práctica — el selector, el conector de accesorios, los pulsadores y el procedimiento de calibración — a partir de cómo lo implementa el firmware.

## Selector, conector de entrada y accesorios

El "selector" no es un componente aparte: es la lectura analógica de una sola pata compartida:

- **ATtiny85**: pata 1 (RESET/PB5/ADC0).
- **ATmega328P**: A1 (PC1).

Esa pata se lee entre mediciones (o al terminar una) para saber qué accesorio está enchufado, según la resistencia que el accesorio pone entre esa pata y masa.

La entrada de señal (o salida del generador) usa un conector barato de tipo USB, no por seguir la norma USB sino porque es fácil de conseguir y da las 4 terminales necesarias:

- 2 de alimentación (una es VCC, disponible también para un sensor externo de temperatura/humedad).
- 1 de selector.
- 1 de señal: entrada de osciloscopio, salida del generador, entrada del frecuencímetro o datos del sensor, según el accesorio.

Cada accesorio trae, de forma externa, la resistencia de selector correspondiente a su función y, si es para osciloscopio, el divisor resistivo de esa escala en la línea de señal. Cambiar de rango o de función es simplemente cambiar de accesorio.

### Tabla de selector (valores reales del firmware)

La pata de selector se lee con referencia a VCC; el firmware compara el valor leído (0-255) contra estas ventanas (tolerancia ±7 cuentas):

| Resistencia | Lectura ADC nominal | Función |
|---|---|---|
| Corto a masa (0 Ω) | ~144-158 | Osciloscopio **Y** |
| 3,9 kΩ | ~160-174 (167) | Osciloscopio **Z** |
| 12 kΩ | ~181-195 (188) | Generador |
| 33 kΩ | ~207-221 (214) | Frecuencímetro |
| 100 kΩ | ~229-243 (236) | Sensor de temperatura/humedad |
| Sin resistencia (circuito abierto) | ≥245 | Osciloscopio **X** |
| Cualquier otro valor (huecos entre ventanas) | — | "NO VÁLIDO" (advierte resistencia incorrecta) |

X e Y no necesitan resistencia de selector "calibrable": son los dos extremos naturales (abierto y corto). Z, generador, frecuencímetro y sensor sí usan una resistencia real, y esa resistencia puede recalibrarse (ver más abajo) para compensar su tolerancia de fabricación.

## Comportamiento al encender

Tras la pantalla de presentación (nombre/versión), el firmware lee los pulsadores:

| Al encender | Resultado |
|---|---|
| Ningún pulsador | El modo lo determina el selector (accesorio conectado) |
| Pulsador **"+" (más)** mantenido | Pregunta "A nuevo" con confirmación SI/NO; si se confirma con "+", borra toda la EEPROM y la vuelve a los valores de fábrica, y **a continuación entra directo en modo CONFIG** |
| Pulsador **"-" (menos)** mantenido | Entra directo en modo CONFIG (calibración), sin resetear nada |

## Uso en modo osciloscopio

Igual que en CONFIG, se navega manteniendo presionado un pulsador (cambia de ítem cada ~0,8 s) y soltando en el que se quiere ejecutar:

- Manteniendo **"+"**: Autoescala → Sube 1 → Sube 10 → Sube toda → Estirar → Grilla → `<Volver>` (y repite).
- Manteniendo **"-"**: Captura → Libre/Auto → Baja 1 → Baja 10 → Baja toda → Lineas o Puntos → Flanco → `<Volver>` (y repite).

Qué hace cada uno:

| Opción | Efecto |
|---|---|
| Autoescala | Prende/apaga el ajuste automático de la escala de tiempo según la frecuencia detectada. Al activarla, fuerza modo gatillo. |
| Sube 1 / Sube 10 | Aumenta manualmente la escala de tiempo (más tiempo por división) 1 o 10 pasos válidos. Apaga Autoescala y Estirar. |
| Baja 1 / Baja 10 | Igual, pero disminuye la escala (menos tiempo por división, barrido más rápido). |
| Sube toda / Baja toda | Salta directo a la escala más lenta o más rápida disponible. |
| Estirar | Alterna un acercamiento (zoom ×4) sobre la porción visible de la captura. |
| Grilla | Muestra u oculta las líneas de referencia (verticales cada 20 px y horizontales en 0/25/50/75/100 %). |
| Líneas o Puntos | Alterna entre trazo continuo (líneas) y solo los puntos muestreados. |
| Flanco | Alterna el disparo (trigger) entre flanco ascendente o descendente. |
| Libre/Auto | Alterna entre barrido libre (sin esperar cruce) y modo gatillo (espera un cruce por el nivel medio para sincronizar la onda). |
| Captura | Congela la pantalla (la invierte en video para indicarlo) hasta que se presiona cualquier pulsador; luego continúa normalmente. |
| `<Volver>` | Sale del menú sin cambiar nada. |

Todas estas opciones (salvo Captura) quedan guardadas en la EEPROM apenas se eligen, así que se mantienen tras apagar y encender. La escala de tiempo en sí (a qué paso quedó "Sube/Baja") no se guarda: al reiniciar, vuelve a como haya quedado por defecto/autoescala.

## Uso en modo generador

Misma mecánica de navegación (mantener y soltar):

- Manteniendo **"+"**: Sube 1 → Sube 10 → Sube 100 → Sube 1000 → A 50 Hz → A 100 Hz → A 25 kHz → `<Volver>` (y repite).
- Manteniendo **"-"**: Baja 1 → Baja 10 → Baja 100 → Baja 1000 → A 500 Hz → A 1 kHz → A 10 kHz → `<Volver>` (y repite).

"Sube/Baja N" ajusta la frecuencia de salida en pasos de 1, 10, 100 o 1000 Hz; las opciones "A NN Hz" saltan directo a una frecuencia fija. El rango va de 1 Hz a 25 kHz (se satura en esos extremos). Al confirmar cualquier cambio, la pantalla muestra la frecuencia pedida y la que realmente puede generarse (marcada con "=" si es exacta o "#" si es la aproximación más cercana lograble con el hardware). `<Volver>` no cambia nada, solo vuelve a aplicar la frecuencia actual.

## Uso en modo frecuencímetro

Solo dos opciones, una por bloque:

- Manteniendo **"+"**: "Frec 1s" → `<Volver>`.
- Manteniendo **"-"**: "Frec 0,2s" → `<Volver>`.

Elige la ventana de medición: 1 segundo da más resolución (mejor para señales de baja frecuencia), 0,2 segundos actualiza más rápido pero con menos resolución. El valor medido se muestra en pantalla en Hz, actualizándose automáticamente cada vez que se cumple la ventana elegida.

## Modo CONFIG (calibración)

Hay solo dos pulsadores físicos, usados de dos maneras según el contexto:

- **Navegar el menú**: se mantiene presionado un pulsador; cada ~0,8 s la pantalla avanza al siguiente ítem de una lista; se suelta cuando se ve el ítem deseado, y eso lo ejecuta.
  - Manteniendo **"+"**: Cal 0 → Cal X → Cal Y → Cal Z → `<Volver>` (y repite).
  - Manteniendo **"-"**: "Z 3k9/167" → "G 12k/188" → "F 33k/214" → "S100k/236" → AUTOR → `<Volver>` (y repite).
- **Confirmar/abortar** (pantalla "SI/NO"): "+" confirma (SI), "-" aborta (NO).

### Calibrar una escala de tensión (Cal X / Cal Y / Cal Z)

1. Diseñar el divisor resistivo del accesorio para que, a la tensión máxima que se quiere medir, el punto medio del divisor entregue una tensión algo por debajo de la referencia interna del ADC en modo osciloscopio:
   - **ATtiny85**: referencia especial de 2,56 V (no necesita capacitor externo en AREF, lo que libera esa pata); apuntar a no superar ~2,3 V — ese es el mínimo que garantiza el fabricante para esta referencia, pese a la dispersión de fabricación entre unidades.
   - **ATmega328P**: referencia de 1,1 V (con capacitor externo en AREF, ya que dispone de más patas); apuntar a no superar ~1 V (no los 2,3 V de arriba, que son solo para el ATtiny85) — ese 1 V es el mínimo garantizado para esta referencia de 1,1 V.
2. Aplicar esa tensión máxima conocida a la entrada, con el accesorio/selector correspondiente ya conectado.
3. Entrar en modo CONFIG ("-" al encender) y elegir Cal X, Cal Y o Cal Z (mantener "+" hasta verlo, soltar).
4. La pantalla muestra "Antes" (valor guardado) y "Ahora" (valor medido). Confirmar con "+" (SI) para guardarlo como el nuevo 100 % de esa escala, o "-" (NO) para descartarlo.
   - Solo se guarda si la lectura cae dentro de un rango razonable (ni saturada ni demasiado baja); si no, se rechaza aunque se confirme con "SI".
5. Repetir para cada escala que se use. No hace falta que el divisor sea exacto: la calibración absorbe la tolerancia real de las resistencias.

**Ejemplo** para un accesorio de 12 V con ATtiny85 (apuntando a 2,3 V): relación necesaria ≈ 12/2,3 ≈ 5,2:1 — por ejemplo R_top=42 kΩ y R_bottom=10 kΩ da 12 V × 10/52 ≈ 2,31 V. El valor exacto de las resistencias no es crítico, porque el paso 4 calibra contra la tensión real aplicada.

Si se mide hasta la tensión de referencia directamente, no hace falta divisor: se conecta la señal directo a la entrada.

### Calibrar la tolerancia de las resistencias del selector (Cal Sz/Sg/Sf/Ss)

Si el selector de un accesorio no cae dentro de la ventana esperada de la tabla de arriba (por ejemplo, por dispersión de tolerancia de la resistencia real usada), se puede recalibrar:

1. Conectar el accesorio con esa resistencia de selector.
2. Entrar en modo CONFIG y elegir, manteniendo "-", la opción correspondiente ("Z 3k9/167" / "G 12k/188" / "F 33k/214" / "S100k/236").
3. Se muestra "Antes"/"Ahora"; confirmar con "+" solo guarda el nuevo valor si cae dentro de ±7 cuentas del nominal esperado (167/188/214/236) — evita aceptar una resistencia claramente equivocada.

### Volver a valores de fábrica

Mantener "+" al encender y confirmar con "SI" borra toda la EEPROM (todas las calibraciones de tensión y de selector) y la deja en los valores de fábrica, entrando después directamente en modo CONFIG para volver a calibrar.

## Sensor de temperatura / humedad

- **ATtiny85**: se elige un solo modelo (DHT o DS18B20) **en tiempo de compilación** (definir o comentar `SENSOR_DHT`), por límite de tamaño de código.
- **ATmega328P**: al tener más espacio, detecta automáticamente cuál de los dos hay conectado, sin recompilar.

Por eso el conector tipo USB también saca VCC: para poder alimentar ese sensor externo.

## Conector de reprogramación (ATtiny85)

La placa incluye un conector para reprogramar el ATtiny85 sin desoldarlo (actualizaciones de firmware), o para darle otro uso. Sigue el pinout estándar de programación ISP (por ejemplo, con "Arduino as ISP"):

| Señal ISP | Pata ATtiny85 |
|---|---|
| RESET | pin 1 |
| VCC | pin 8 |
| SCK | pin 7 (PB2) |
| MISO | pin 6 (PB1) |
| MOSI | pin 5 (PB0) |
| GND | pin 4 |
