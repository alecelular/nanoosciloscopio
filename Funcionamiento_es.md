# Detalle del funcionamiento

👉 English version [Funcionamiento_en.md](Funcionamiento_en.md) —
👉 Versão em português [Funcionamiento_br.md](Funcionamiento_br.md)

Este documento complementa al [README](README.md): explica cómo se usa el equipo en la práctica — el selector, el conector de accesorios, los pulsadores y el procedimiento de calibración — a partir de cómo lo implementa el firmware.

> Los nombres de menú de este documento son los que se ven en
> pantalla en castellano (compilación por omisión, `IDIOMA_ES`). El
> firmware también puede compilarse en inglés (`#define IDIOMA_EN`)
> o portugués (`#define IDIOMA_BR`) al principio del `.ino`.

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

### Tabla de selector (valores de fábrica)

La pata de selector se lee con referencia a VCC. Estos son los
valores nominales de fábrica (los que trae `selValores[]` la primera
vez que arranca el equipo, antes de calibrar nada):

| Resistencia | ADC nominal de fábrica | Función |
|---|---|---|
| Corto a masa (0 Ω) | 151 | Osciloscopio **X** |
| 5,6 kΩ | 172 | Osciloscopio **Y** |
| 10 kΩ | 186 | Osciloscopio **Z** |
| 22 kΩ | 203 | Generador |
| 47 kΩ | 221 | Frecuencímetro |
| 150 kΩ | 241 | Sensor de temperatura/humedad |
| Circuito abierto (sin resistencia) | ≥250 | **Sin accesorio conectado** (modo espera) |

### Cómo detecta el firmware qué accesorio está enchufado

El circuito abierto (≥250 cuentas) se descarta primero y siempre
significa "sin accesorio", sin comparar contra nada más.

Para el resto de los casos, el firmware **no** usa ventanas fijas
alrededor de un valor nominal. En su lugar, guarda en EEPROM (en
`ee_r[0..5]`) el valor de ADC que se midió la última vez que se
calibró cada una de las 6 posiciones (de fábrica, esos 6 valores son
los de la tabla de arriba). Al leer el selector, recorre las 6
posiciones calibradas y elige la que esté **más cerca** de la
lectura actual. Si la distancia a la más cercana supera 5 cuentas
(`SEL_TOL`), el valor se considera fuera de cualquier posición
conocida y se informa como "Selector no válido".

En la práctica esto significa que, si dos posiciones calibradas
quedaran demasiado próximas entre sí, gana la que esté objetivamente
más cerca de la lectura — conviene dejar al menos ~10-12 cuentas de
separación entre los valores calibrados de cada posición para que no
haya ambigüedad.

## Comportamiento al encender

Tras la pantalla de presentación (nombre/versión), el firmware lee los pulsadores:

| Al encender | Resultado |
|---|---|
| Ningún pulsador | El modo lo determina el selector (accesorio conectado) |
| Pulsador **"+" (más)** mantenido | Pregunta "A nuevo" con confirmación SI/NO; si se confirma con "+", borra toda la EEPROM y la vuelve a los valores de fábrica, y **a continuación entra directo en modo CONFIG** |
| Pulsador **"-" (menos)** mantenido | Entra directo en modo CONFIG (calibración), sin resetear nada |

## Uso en modo osciloscopio

Igual que en CONFIG, se navega manteniendo presionado un pulsador (cambia de ítem cada ~0,8 s) y soltando en el que se quiere ejecutar:

- Manteniendo **"+"**: `Autoescala` → `<+1>` → `<+10>` → `<MAX>` → `<X4>` → `Grilla` → `<Volver>` (y repite).
- Manteniendo **"-"**: `Captura` → `Libre/Auto` → `<-1>` → `<-10>` → `<MIN>` → `Lineas o Puntos` → `Flanco` → `<Volver>` (y repite).

Qué hace cada uno:

| Opción | Efecto |
|---|---|
| `Autoescala` | Prende/apaga el ajuste automático de la escala de tiempo según la frecuencia detectada. Al activarla, fuerza modo gatillo. |
| `<+1>` / `<+10>` | Aumenta manualmente la escala de tiempo (más tiempo por división) 1 o 10 pasos válidos. Apaga Autoescala y Estirar x4. |
| `<-1>` / `<-10>` | Igual, pero disminuye la escala (menos tiempo por división, barrido más rápido). |
| `<MAX> / <MIN>` | Salta directo a la escala más lenta o más rápida disponible. |
| `<X4>` | Alterna un acercamiento (zoom ×4) sobre la porción visible de la captura. |
| `Grilla` | Muestra u oculta las líneas de referencia (verticales cada 20 px y horizontales en 0/25/50/75/100 %). |
| `Líneas o Puntos` | Alterna entre trazo continuo (líneas) y solo los puntos muestreados. |
| `Flanco` | Alterna el disparo (trigger) entre flanco ascendente o descendente. |
| `Libre/Auto` | Alterna entre barrido libre (sin esperar cruce) y modo gatillo (espera un cruce por el nivel medio para sincronizar la onda). |
| `Captura` | Congela la pantalla (la invierte en video para indicarlo) hasta que se presiona cualquier pulsador; luego continúa normalmente. |
| `<Volver>` | Sale del menú sin cambiar nada. |

Todas estas opciones (salvo Captura) quedan guardadas en la EEPROM apenas se eligen, así que se mantienen tras apagar y encender. La escala de tiempo en sí (a qué paso quedó "<+/-N>") no se guarda: al reiniciar, vuelve a como haya quedado por defecto/autoescala.

### Filtro de ruido en la detección de frecuencia

Antes de calcular una frecuencia o buscar el punto de disparo, el
firmware descarta la captura si la amplitud pico a pico es menor a
`AMPLITUD_MINIMA_DETECCION` cuentas de ADC (4 por omisión). Esto
evita que un poco de ruido de fondo, sin conectar nada a la entrada,
se interprete como una señal real de alta frecuencia. Cuando esto
pasa, la frecuencia mostrada es "-----" y el barrido pasa a ser
libre desde el inicio de la captura, como si no hubiera señal.

### Línea de estado (modo osciloscopio)

En la parte inferior de la pantalla se muestra, de izquierda a
derecha:

- **Lupa (1/2/3)**: factor de aumento automático aplicado a la
  onda cuando su amplitud es chica respecto al fondo de escala.
- **A/M**: Autoescala o Manual.
- **N/4**: Normal o Estirado (bandera `"<X4>"` del menú.).
- **L / + / -**: modo de disparo. 'L' = barrido libre continuo,
  '+' = gatillo por flanco ascendente, '-' = gatillo por flanco
  descendente.
- **Tiempo (núm. + u)**: tiempo real por división en
  microsegundos.
- **Frecuencia (núm. + H)**: frecuencia detectada, con un
  decimal. Se muestra "-----" si es menor a 0,5 Hz o si no se
  detectó un ciclo completo.
- **Amplitud (núm. + %) o SAT**: porcentaje de la amplitud pico
  respecto al fondo de escala. Se muestra "SAT" si la señal
  excede el fondo de escala (satura).
- **Rango (X/Y/Z)**: rango de tensión de entrada seleccionado.

## Uso en modo generador

Misma mecánica de navegación (mantener y soltar):

- Manteniendo **"+"**: `<+1>` → `<+10>` → `<+100>` → `<+1000>` → `A 100 Hz` → `A 25 kHz` → `<Volver>` (y repite).
- Manteniendo **"-"**: `<-1>` → `<-10>` → `<-100>` → `<-1000>` → `A 1 kHz` → `A 10 kHz` → `<Volver>` (y repite).

"`<+/-N>`" ajusta la frecuencia de salida en pasos de 1, 10, 100 o 1000 Hz; las opciones "A NN Hz" saltan directo a una frecuencia fija. El rango va de 1 Hz a 25 kHz (se satura en esos extremos). Al confirmar cualquier cambio, la pantalla muestra la frecuencia pedida y la que realmente puede generarse (marcada con "=" si es exacta o "#" si es la aproximación más cercana lograble con el hardware). `<Volver>` no cambia nada, solo vuelve a aplicar la frecuencia actual.

## Uso en modo frecuencímetro

El valor medido se muestra en pantalla en Hz, actualizándose automáticamente cada vez que se cumple la ventana.

## Modo CONFIG (calibración)

Hay solo dos pulsadores físicos, usados de dos maneras según el contexto:

- **Navegar el menú**: se mantiene presionado un pulsador; cada ~0,8 s la pantalla avanza al siguiente ítem de una lista; se suelta cuando se ve el ítem deseado, y eso lo ejecuta.
  - Manteniendo **"+"**: `Cal 0` → `Cal X` → `Cal Y` → `Cal Z` → *(solo builds sin cristal: `CAL 50Hz`)* → `AUTOR` → `<Volver>` (y repite).
  - Manteniendo **"-"**: `"X 0/151"` → `"Y 5k6/172"` → `"Z 10k/186"` → `"G 22k/203"` → `"F 47k/221"` → `"S 150k/241"` → `<Volver>` (y repite).
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

### Calibrar la tolerancia de las resistencias del selector

Si el selector de un accesorio no cae dentro de la posición
esperada (por ejemplo, por dispersión de tolerancia de la
resistencia real usada, o porque cambiaste el valor de una
resistencia), se puede recalibrar:

1. Conectar el accesorio con esa resistencia de selector.
2. Entrar en modo CONFIG y elegir, manteniendo "-", la opción correspondiente ("X 0/151" / "Y 5k6/172" / "Z 10k/186" / "G 22k/203" / "F 47k/221" / "S 150k/241").
3. Se muestra "Antes"/"Ahora"; confirmar con "+" solo guarda el
   nuevo valor si cae **estrictamente entre 128 y 250 cuentas de
   ADC** (evita aceptar un valor claramente inválido, por ejemplo
   casi en corto o casi a circuito abierto). No hay una ventana fija
   alrededor del nominal esperado: una vez guardado, ese valor pasa
   a ser el nuevo punto de referencia para esa posición, y el
   firmware la detecta después por cercanía (ver "Cómo detecta el
   firmware qué accesorio está enchufado" más arriba).

Osciloscopio X no aparece en esta lista: al usar un corto a masa fijo por diseño, no tiene tolerancia de fabricación que calibrar.

### Calibrar el oscilador interno (solo compilaciones sin cristal)

El ATtiny85 puede compilarse sin cristal externo, usando su
oscilador RC interno a 8 MHz (ver [README.md](README.md), sección
"Uso sin cristal"). El RC interno no es preciso de fábrica ni
estable con la temperatura, así que el firmware ofrece una
calibración contra una frecuencia de referencia conocida: la línea
eléctrica, a 50 Hz o 60 Hz según cómo se haya compilado (constante
`CAL50` en el `.ino`: definida calibra contra 50 Hz, comentada
calibra contra 60 Hz).

Esta opción de menú (`CAL 50Hz` o `CAL 60Hz`, según la compilación)
**solo aparece** en compilaciones sin cristal; en una compilación
con cristal externo no está disponible ni hace falta.

**Antes de empezar:**
- El equipo debe estar en modo osciloscopio con una señal de 50 o
  60 Hz conocida ya conectada a la entrada (por ejemplo, la
  frecuencia de línea, tomada con el accesorio y el divisor
  resistivo de alguna de las escalas de osciloscopio).

**Procedimiento:**
1. Entrar en modo CONFIG ("-" al encender).
2. Mantener "+" hasta ver "CAL 50Hz" (o "CAL 60Hz") y soltar.
3. Confirmar con "+" (o abortar con "-", que cancela sin tocar nada).
4. El equipo mide en ventanas de 600 ms (múltiplo tanto de 50 Hz
   como de 60 Hz) y recorre automáticamente todos los valores
   posibles de `OSCCAL` (de 2 a 253), buscando el punto donde la
   frecuencia medida cruza el valor esperado (500 o 600, en décimas
   de Hz, según `CAL50`).
5. Si encuentra un cruce válido, guarda el nuevo `OSCCAL` en EEPROM
   y muestra "HECHO". Si no lo encuentra (por ejemplo, si no hay
   señal real conectada), restaura el `OSCCAL` que tenía antes de
   empezar y muestra "Aborta", sin guardar nada.

El valor calibrado queda en EEPROM y se recupera solo al encender el
equipo, así que sobrevive a un apagado/encendido. Conviene rehacer
la calibración si cambia mucho la temperatura ambiente, o si se
reprograma el ATtiny85 (lo que puede alterar el valor de fábrica de
`OSCCAL`).

> Esta calibración es independiente de las escalas de tiempo del
> osciloscopio: `configurarEscala()` ya tiene tabulada la relación
> entre pasos de escala y microsegundos reales para 8 MHz (con o sin
> cristal). Calibrar `OSCCAL` no cambia esa tabla, solo corrige qué
> tan rápido corre realmente el reloj interno del ATtiny85.

### Volver a valores de fábrica

Mantener "+" al encender y confirmar con "SI" borra toda la EEPROM (todas las calibraciones de tensión y de selector) y la deja en los valores de fábrica, entrando después directamente en modo CONFIG para volver a calibrar.

## Sensor de temperatura / humedad

- Detecta automáticamente DHT11/22/12/DS18B20.

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
