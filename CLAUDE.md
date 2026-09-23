# Contexto del proyecto para agentes de código

Este archivo lo lee automáticamente Claude Code (y herramientas equivalentes)
al abrir el repositorio. Le ahorra tener que deducir cómo está armado el
proyecto, y sobre todo le fija las restricciones que no puede romper.

> **Nota para el equipo.** Este archivo cumple doble función: le sirve al
> agente, y es el ejemplo de lo que ustedes tienen que mantener en su propio
> proyecto. Actualizalo cuando cambien el hardware, agreguen dependencias o
> cambien una regla. Un `CLAUDE.md` desactualizado es peor que no tenerlo: el
> agente lo va a creer.

---

## Qué es este proyecto

Prototipo de IoT de la asignatura Proyecto Integrador 2. Un microcontrolador
lee sensores, aplica lógica de control local y publica telemetría a una
instancia de ThingsBoard Community Edition. La documentación se publica como
sitio estático con GitHub Pages.

## Hardware objetivo

| | |
|---|---|
| Placa por defecto | ESP32 (entorno `esp32dev`) |
| Placa alternativa | ESP8266 NodeMCU v2 (entorno `nodemcuv2`) |
| Ambas deben compilar | Sí. Lo específico de cada placa se aísla con `#if defined(ESP8266)` |
| Actuador de ejemplo | Un pin digital (`PIN_ACTUADOR`, GPIO2 por defecto) |

## Plataforma IoT y protocolos

| | |
|---|---|
| Plataforma | ThingsBoard Community Edition |
| Protocolo | **MQTT sobre TLS, puerto 8883.** El 1883 está cerrado en el servidor del curso: por ahí el token viajaría en claro |
| Autenticación | El *access token* del dispositivo va como **username** de MQTT, con contraseña vacía |
| TLS | `WiFiClientSecure` con `setInsecure()`: cifra, pero no valida el certificado del servidor. No cambiar a `WiFiClient` |
| Telemetría | JSON publicado en `v1/devices/me/telemetry` |
| RPC entrante | `v1/devices/me/rpc/request/+` |
| RPC saliente | `v1/devices/me/rpc/response/<id>`, con el mismo `<id>` recibido |

Los tópicos los define ThingsBoard y están verificados contra
<https://thingsboard.io/docs/reference/mqtt-api/>. **No inventar endpoints ni
tópicos:** si hace falta uno que no está acá, verificarlo contra esa
documentación o dejar un `TODO` y preguntar.

## Comandos

```bash
pio run                      # compila para ESP32 (entorno por defecto)
pio run -e nodemcuv2         # compila para ESP8266
pio run -t upload            # compila y sube a la placa
pio device monitor           # monitor serie, 115200 baud
pio test -e native           # tests de lógica en la PC, sin placa
bash scripts/higiene.sh      # el mismo chequeo de tamaños que corre CI
mkdocs serve                 # previsualiza el sitio en localhost:8000
mkdocs build --strict        # construye el sitio como lo hace CI
```

## Organización del código

| Ruta | Qué contiene |
|---|---|
| `src/main.cpp` | Lo propio del proyecto: sensor, verificaciones de hardware, comandos RPC, telemetría, `setup()` y `loop()` |
| `lib/conexion/` | WiFi y MQTT contra ThingsBoard. Infraestructura, no se toca por proyecto |
| `lib/diagnostico/` | Autodiagnóstico de arranque. Se extiende desde `verificacionesDelProyecto()`, en `main.cpp` |
| `lib/control/` | Lógica pura, **sin dependencias de Arduino**, compilable en `native` |
| `include/config.h` | Constantes no secretas: intervalos, tópicos, umbrales, buffers |
| `test/test_control/` | Tests Unity de la lógica de `lib/control/` |
| `scripts/inject_secrets.py` | Script `pre:` de PlatformIO que inyecta los secretos como macros |
| `scripts/higiene.sh` | Chequeo de tamaños del repositorio |
| `docs/` | Contenido del sitio, en Markdown |

**Regla de ubicación:** si algo se puede decidir sin tocar un pin, va en
`lib/control/` y lleva tests. En `src/main.cpp` queda solo el pegamento con el
hardware, la red y MQTT.

## Restricciones duras

Estas no se negocian. Un cambio que rompa cualquiera de ellas está mal, aunque
compile.

### 1. Ningún secreto en el código fuente

No puede haber literales de SSID, contraseña, host ni token en ningún archivo
versionado, **ni siquiera de ejemplo**. Todo llega como macro `-D` desde
`scripts/inject_secrets.py`, que lee `secrets.ini`.

**No leer, no escribir y no proponer cambios a `secrets.ini`.** Está en
`.gitignore` y contiene credenciales reales. Si falta una clave, se agrega a
`secrets.ini.example` con un valor obviamente falso.

### 2. El `loop()` no se bloquea

Nada de `delay()` en `loop()`. El trabajo periódico se hace comparando
`millis()` con la marca de la última vez. Un `loop()` bloqueado no llama a
`mqtt.loop()`, se rompe el keepalive y el broker corta la conexión.

Las dos únicas esperas admitidas, ambas documentadas en el código:

- `conectarWifi()` (`lib/conexion/`), espera acotada a 20 s, **llamada solo
  desde `setup()`**.
- `conectarMqtt()` (`lib/conexion/`), hasta `MQTT_CONNECT_TIMEOUT_MS` (15 s)
  por la apertura de la conexión, que es sincrónica en PubSubClient e incluye
  el handshake TLS. Solo ocurre estando ya desconectados, donde no hay
  keepalive que mantener.

Antes de agregar cualquier función que espere, verificar desde dónde se la
llama.

### 3. `autotest()` se extiende, nunca se reemplaza

`setup()` llama a `autotest()` (en `lib/diagnostico/`) **antes** de levantar
WiFi y MQTT. Verifica la configuración, los sensores y los actuadores, y su
salida separa un problema local de uno de red.

Las verificaciones genéricas están en `lib/diagnostico/`; las del proyecto van
en `verificacionesDelProyecto()`, en `src/main.cpp`, usando `diagResultado()`.
Cada sensor o actuador que se agregue tiene que sumar su verificación ahí.

### 4. Dependencias con versión fijada

- Librerías, con rango menor: `knolleary/PubSubClient@^2.8`,
  `bblanchon/ArduinoJson@^7.4.3`.
- Plataforma ESP32, **con el tag exacto** `55.03.311`, no con el alias
  `stable`, que es un tag móvil y rompería la reproducibilidad.
- Sitio: `mkdocs-material~=9.7.7`.

Nada sin versión.

**Excepción deliberada:** las acciones de `.github/workflows/` van con el tag
mayor (`actions/checkout@v7`). No es un descuido y no hay que "corregirlo". El
criterio es si la dependencia forma parte del artefacto: el compilador de
ESP32 sí —si cambia, cambia el binario, en silencio—, y las acciones de CI no
—copian archivos y guardan cachés, y si se rompen se ve en rojo al instante—.
Está explicado en el encabezado de `ci.yml` y en el `README.md`.

### 5. ArduinoJson v7

Usar `JsonDocument`. `DynamicJsonDocument` y `StaticJsonDocument` están
obsoletos en la v7 y no se usan.

### 6. `setBufferSize()` explícito en MQTT

PubSubClient trae 128 bytes por defecto y **descarta en silencio** los mensajes
más largos: `publish()` devuelve `false` y no se publica nada. El tamaño está
en `MQTT_BUFFER_SIZE` y se aplica en `configurarMqtt()`, en `lib/conexion/`.

### 7. Nada pesado en el repositorio

Ningún archivo versionado de más de 300 KB, repositorio total por debajo de
20 MB. Lo verifica el job `higiene` de CI. Las imágenes se comprimen **antes**
de commitear: git guarda cada versión de cada binario para siempre.

### 8. Atribución

La cabecera de `src/main.cpp` conserva la atribución a Andrés Ferragut
(ferragut@fi365.ort.edu.uy), autor del ejemplo original. No sacarla.

## Convenciones

- **Idioma:** todo el contenido visible para estudiantes va en español —
  comentarios del código, mensajes por el monitor serie, documentación y
  mensajes de commit.
- **Comentarios:** el código de este repositorio se lee en clase. Los
  comentarios explican **por qué**, no qué hace la línea de al lado.
- **Navegación del sitio:** explícita en `mkdocs.yml`. Una página nueva que no
  se agrega al `nav` no aparece en el menú, y eso es a propósito.
- **Commits:** en español, chicos, uno por unidad de trabajo. `Closes #N`
  cierra el issue al mergear a `main`.
- **Ramas:** se trabaja en rama y se abre pull request. No se pushea a `main`.

## Antes de dar algo por terminado

```bash
pio run -e esp32dev && pio run -e nodemcuv2   # las dos placas compilan
pio test -e native                            # los tests pasan
mkdocs build --strict                         # el sitio construye sin advertencias
bash scripts/higiene.sh                       # no entró nada pesado
git status                                    # el árbol quedó limpio
```
