# Informe de construcción del template

Documento para el equipo docente, no para los estudiantes. Registra los
desvíos, los huecos de la especificación y las decisiones tomadas durante la
construcción del template.

---

# Reanudación

**Leé esta sección primero.** Está escrita para alguien que no participó de la
construcción: con esto alcanza para retomar el trabajo sin más contexto.

## Qué es esto

El repositorio template de la asignatura Proyecto Integrador 2 (IoT), edición
2026. Cada equipo de tres lo usa como base de su proyecto vía GitHub Classroom:
un prototipo con ESP32 o ESP8266 que publica telemetría a ThingsBoard
Community Edition y documenta el proceso en un sitio con GitHub Pages.

Reemplaza al template de 2024, que tenía ocho defectos conocidos (sin
`.gitignore`, credenciales en el `.ino`, Jekyll mal configurado, una foto de
1,47 MB, sin tests ni CI). La especificación completa está en
`ESPEC_template_PI2_2026.md`, que vive **fuera del repositorio**, en el disco
del docente: es documento docente y menciona incidentes de seguridad de
cohortes anteriores. Está excluido vía `.git/info/exclude`, que no se versiona,
así que su nombre tampoco viaja a los equipos.

## Estado exacto

| | |
|---|---|
| Rama | `main` |
| Remoto | `git@github.com:SisCom-PI2-2026-2/template_proyecto.git` |
| Commits | 13 |
| Árbol de trabajo | Limpio, sincronizado con el remoto |
| Tamaño en clon limpio | ~456 KB (límite: 500 KB), 30 archivos versionados |
| Estado | **Publicado.** La rama `feat/template-2026` ya se mergeó; el trabajo posterior va directo a `main` |

> **Sobre el tamaño:** medido recién después de clonar, dio 596 KB, por encima
> del límite. Eran objetos sueltos de los últimos commits: con
> `git gc --prune=now` bajó a 456 KB. Un clon desde GitHub ya viene
> recomprimido, así que el número real es el segundo. Si alguna vez la
> medición da por encima de 500 KB, correr el `gc` antes de preocuparse.

**Las siete fases de la especificación están verificadas**, cada una
ejecutando su criterio y observando la salida real:

| Fase | Entregable | Criterio ejecutado |
|---|---|---|
| 0 | `.gitignore`, `README.md`, estructura | `git status` no lista `secrets.ini` tras crearlo |
| 1 | `platformio.ini`, `inject_secrets.py`, `secrets.ini.example`, `config.h` | `rm -f secrets.ini && pio run -e esp32dev` compila y crea el archivo con aviso |
| 2 | `lib/control/`, `test/` | `pio test -e native` pasa (11/11), y **falla** al romper la banda muerta a propósito (4 rojos) |
| 3 | `src/main.cpp` | `esp32dev` y `nodemcuv2` compilan |
| 4 | `mkdocs.yml`, `requirements.txt`, `docs/` | `mkdocs build --strict` sin advertencias |
| 5 | Los dos workflows | `actionlint` 0 errores, `shellcheck` 0 observaciones, y los tres escenarios de `higiene.sh` ejecutados (1 verde, 2 rojos) |
| 6 | `CLAUDE.md`, limpieza | Sin credenciales reales, ~430 KB, árbol limpio |

Además, los siete criterios de aceptación de la sección 5 de la especificación
fueron verificados **desde un clon limpio**, sin editar nada. El detalle está
en "Estado final", más abajo.

## Las siete decisiones que se apartan de la especificación

1. **Plataforma de ESP32 fijada al tag `55.03.311`**, no al alias `stable` que
   pedía la espec: `stable` es un tag móvil, y dos equipos que clonen en meses
   distintos compilarían con toolchains distintos sin haber tocado una línea.
2. **`board_build.partitions = min_spiffs.csv` en `esp32dev`**: el firmware de
   ejemplo ya ocupaba el 71 % de la partición de aplicación y los equipos
   todavía tienen que agregar lo suyo; pasa a 47,3 % conservando las dos
   particiones OTA (por eso no se usó `huge_app.csv`, que elimina el OTA).
3. **`src/main.cpp` creado en la fase 1**, no en la 3: la fase 1 exige que
   `pio run` compile, y sin ningún archivo en `src/` PlatformIO ni siquiera
   llega a compilar. Quedó como autodiagnóstico de arranque (`autotest()`), que
   la fase 3 **extiende, nunca reemplaza**.
4. **La corrida local de tests es opcional; el CI es obligatorio**: en Windows
   —donde está la mayoría del curso— correr los tests exige instalar MSYS2 y
   editar el `PATH` a mano, y ese es el único paso del arranque que no se pudo
   verificar. Poner el camino crítico sobre el paso no probado era el riesgo
   más grande.
5. **Cuatro archivos que la espec no lista**: `scripts/higiene.sh` (para poder
   verificar el mismo código que corre CI y que los equipos lo corran antes de
   pushear), `docs/plan.md` y `.github/ISSUE_TEMPLATE/tarea.md` (pedidos
   durante el trabajo), y este `INFORME.md`.
6. **La conexión va por MQTT sobre TLS en el 8883**, no por el 1883 sin cifrar
   que suponía la espec: el servidor del curso tiene cerrado el puerto en
   claro, porque por ahí el access token viajaría legible. Se usa
   `setInsecure()`: cifra pero no valida el certificado del servidor.
7. **El firmware está repartido entre `src/` y `lib/`**: la infraestructura
   (WiFi, MQTT, autodiagnóstico genérico) vive en `lib/conexion/` y
   `lib/diagnostico/`, y en `src/main.cpp` queda solo lo que cada equipo
   escribe. La espec pedía un `main.cpp` único; con todo junto no se veía la
   forma del programa.

## El criterio de versionado

Hay tres formas de fijar versiones en el repositorio y, sin explicación,
parecen contradictorias. La pregunta que las ordena es una sola: **¿esto forma
parte del artefacto que entregamos?**

| Dependencia | Cómo se fija | Por qué |
|---|---|---|
| Plataforma ESP32 (`platformio.ini`) | Tag exacto: `55.03.311` | Es el compilador. Si cambia, cambia el binario, y cambia **en silencio** |
| Librerías de firmware (`platformio.ini`) | Rango menor: `^2.8`, `^7.4.3` | Parte del artefacto, pero solo se aceptan correcciones dentro de la misma versión mayor |
| Dependencias del sitio (`requirements.txt`) | Rango menor: `~=9.7.7` | Ídem |
| Acciones de GitHub (`.github/workflows/`) | Tag mayor: `@v7`, `@v6`, `@v5` | Infraestructura de CI, **no** parte del artefacto. Respetan versionado semántico, reciben correcciones de seguridad, y una rotura se ve en rojo al instante sin haber contaminado ningún binario |

Regla corta: **fijar exacto lo que puede cambiar el resultado sin avisar; dejar
que se actualice lo que, si se rompe, lo dice en la cara.**

Está documentado en el `README.md` (sección propia), en el encabezado de
`.github/workflows/ci.yml`, y en la restricción 4 de `CLAUDE.md` —esta última
para que un agente de código no lo "corrija" por su cuenta—.

## Los seis puntos que no se pudieron verificar

Ninguno es un defecto conocido. Son verificaciones que el entorno de
desarrollo no permite hacer, y **no se dieron por buenas en ningún lado**.

| # | Qué falta verificar | Por qué no se pudo | Cuándo hacerlo |
|---|---|---|---|
| 1 | Pages habilitado en modo "GitHub Actions" | Es un cambio de configuración del repositorio, no de código | Al crear cada repositorio |
| 2 | Que la publicación del sitio funcione de punta a punta | `deploy-pages` necesita el token OIDC y la API de Pages; no se puede simular ni con `act` | Primer push a `main` |
| 3 | La instalación del compilador en Windows (MSYS2 + tres rutas de `PATH`) | No hay máquina Windows disponible | Antes de la primera clase |
| 4 | Que los tres diagramas Mermaid se dibujen | No hay navegador; se verificó el marcado y que el CDN responde, no el render | Al abrir el sitio publicado |
| 5 | `core_dir` compartido en laboratorio con dos cuentas reales | Se probó con una sola cuenta | Al preparar las máquinas |
| 6 | Los tres jobs de CI corriendo de verdad | Se verificaron estáticamente con `actionlint` y ejecutando `higiene.sh` localmente; no se corrió `act` | Primer push |

El punto 1 es el que más probablemente muerda: **sin ese clic, `pages.yml`
corre en verde y no publica nada.**

## Los cinco pasos del docente al crear el repositorio

1. ~~Crear el repositorio en la organización del semestre y pushear.~~
   **HECHO:** `SisCom-PI2-2026-2/template_proyecto`.
2. ~~Abrir el pull request y mergearlo.~~ **HECHO.**
3. **Settings → Pages → Build and deployment → Source: "GitHub Actions".**
   Sin esto el sitio no se publica. **Pendiente de confirmar.**
4. Settings → marcar el repositorio como **Template repository**.
   **Pendiente de confirmar.**
5. Verificar la corrida de CI (tres jobs en verde) y abrir el sitio publicado
   para confirmar que los diagramas se dibujan. **Pendiente.**

Fecha límite: **10/09/2026**, cuando se crea el GitHub Classroom.

---

## Cambios posteriores a la publicación

Lo que se hizo después de que el template ya estaba en GitHub. Dos commits,
los dos directos a `main`.

### `5a7c29e` — reparto del README y separación de la infraestructura

**Por qué.** El problema no era el contenido sino el orden: el README tenía
todo el semestre en la primera pantalla, y `src/main.cpp` mezclaba
autodiagnóstico, WiFi, MQTT, RPC, telemetría y control, así que un estudiante
que lo abría no veía la forma del programa.

**README: de 512 a 209 líneas.** Quedan solo cinco secciones, en este orden:
qué es el repositorio, requisitos previos, arranque en cinco pasos, tabla de
comandos y un índice de "dónde está cada cosa". Todo lo demás se movió a
`docs/GUIA.md` **sin reescribirlo**.

La única excepción deliberada: en el paso 4 del arranque quedó el aviso de
rotar el token si se filtró, con enlace a la guía. Es demasiado importante
para vivir solo en otro archivo.

`docs/GUIA.md` ya existía (las cinco preguntas por página) y ya estaba en el
`nav`. No se pisó: se le anexaron las secciones movidas, se renombró el título
a "Guía del proyecto" y las preguntas por página quedaron un nivel más abajo.

**Firmware: `src/main.cpp` de 617 a 306 líneas.** La infraestructura que los
equipos no tocan se movió a dos librerías nuevas:

| Ruta | Qué contiene |
|---|---|
| `lib/conexion/` | `conectarWifi()`, `mantenerWifi()`, `conectarMqtt()`, `mantenerMqtt()`, `idCliente()`, `configurarMqtt()` y el objeto `mqtt` |
| `lib/diagnostico/` | `autotest()` y las verificaciones genéricas: placa, secretos, parámetros |

En `main.cpp` queda lo que cada equipo sí escribe: `leerSensor()`,
`verificacionesDelProyecto()`, el callback de RPC, `publicarTelemetria()`,
`setup()` y `loop()`.

**Cómo se resolvió la tensión del autodiagnóstico.** Se pidió mover
`autotest()` a `lib/` y a la vez dejar visible el TODO de extenderlo, que son
cosas incompatibles tal cual. Se partió en dos: `lib/diagnostico/` corre lo
genérico y en el medio llama a `verificacionesDelProyecto()`, que se **define
en `src/main.cpp`** y es donde están los TODO del sensor y del actuador. El
orden de las verificaciones y la salida por consola no cambian.

**Efecto colateral en `platformio.ini`.** Hubo que agregar
`build_flags = -Iinclude`: PlatformIO pasa `include/` a `src/` pero **no** a
las librerías de `lib/`, y sin eso el build falla con
`fatal error: config.h: No such file or directory`.

### `c90509b` — MQTT sobre TLS en el 8883

**Por qué.** El servidor del curso tiene cerrado el 1883. Con el firmware
anterior, que usaba `WiFiClient` en claro, **ningún equipo habría podido
conectarse**. Verificado en hardware por el docente: el 8883 con
`setInsecure()` funciona en ESP8266 y en ESP32 (LoLin NodeMCU v3, 43.520 bytes
de heap libre, primer dato a los 8,43 s del reset).

**Qué cambió.**

- `lib/conexion/` usa `WiFiClientSecure` en lugar de `WiFiClient`.
- `configurarMqtt()` llama a `red.setInsecure()` antes del `setServer`, con el
  comentario que explica que **cifrado y autenticado son cosas distintas**.
- `include/config.h` define `TB_PUERTO = 8883` y explica por qué el 1883 está
  cerrado. La macro `TB_PORT` ya no existe.
- El include de `WiFiClientSecure` está resuelto para las dos placas: en
  ESP8266 es un header aparte de `ESP8266WiFi` (implementado con BearSSL, que
  se expone con `using namespace BearSSL`), en ESP32 viene con el core, en
  `NetworkClientSecure`. `setInsecure()` existe en las dos.

**Cambio no pedido, pero necesario: `MQTT_CONNECT_TIMEOUT_MS` de 3 s a 15 s.**
BearSSL se pone `_timeout = 15000` en sus constructores
(`WiFiClientSecureBearSSL.cpp:73` y `:249`) precisamente porque el handshake
TLS tarda segundos. El `setTimeout(3000)` que había lo hubiera bajado,
cortando el handshake por la mitad; el síntoma habría sido "no conecta nunca"
sin ningún error claro. Está documentado en `config.h`.

**Costo en la placa chica.** El ESP8266 pasa de 28,5 % a 38,3 % de flash
(+100 KB de BearSSL). RAM estática casi igual: 36,9 % → 37,3 %. El consumo
real de TLS es heap en tiempo de ejecución, que es el número que midió el
docente y que no se puede verificar desde el entorno de desarrollo.

**Documentación alineada.** `docs/arquitectura.md`, `docs/GUIA.md` y
`CLAUDE.md` decían que el tráfico iba sin cifrar por el 1883. El aviso "Sobre
el cifrado" de arquitectura ahora explica qué protege `setInsecure()` y qué
no, y que validar la cadena de certificados es el paso que falta para
producción.

### Verificación de los dos commits

Ejecutada en ambos, con la salida a la vista:

```
pio run -e esp32dev     SUCCESS   RAM 14,4 %  Flash 52,0 %
pio run -e nodemcuv2    SUCCESS   RAM 37,3 %  Flash 38,3 %
pio test -e native      11 test cases: 11 succeeded
mkdocs build --strict   exit 0, sin advertencias
```

---

## Estado final

Verificado desde un **clon limpio**, sin editar ningún archivo, contra los
criterios de la sección 5 de la especificación. Los números de tamaño de esta
tabla son los del cierre de la fase 6; el estado actual está más arriba, en
"Estado exacto".

| # | Criterio | Resultado |
|---|---|---|
| 1 | `pio run` compila sin editar nada | `esp32dev SUCCESS` (Flash 47,3 %) y `nodemcuv2 SUCCESS` (Flash 28,5 %) |
| 2 | `pio test -e native` pasa | `11 test cases: 11 succeeded` |
| 3 | `mkdocs build --strict` sin advertencias | `EXIT=0`, 8 páginas |
| 4 | `git status` limpio tras compilar y crear `secrets.ini` | Limpio. Quedan fuera de git `.pio/`, `site/` y `secrets.ini` |
| 5 | Ningún archivo con una credencial real | 76 coincidencias de `ssid\|password\|token`, todas en comentarios, documentación o placeholders. Ver abajo |
| 6 | El repositorio pesa menos de 500 KB | **~430 KB**, de los cuales unos 212 KB son `.git`. 26 archivos versionados. La cifra se mueve unos pocos KB con cada commit a este mismo informe |
| 7 | Se puede publicar telemetría leyendo solo el `README.md` | Cubierto: requisitos previos, arranque en cinco pasos, tabla de comandos y sección de diagnóstico. **No verificado con una persona real** |

### Sobre el criterio 5

El `grep` devuelve 76 líneas, repartidas así: `src/main.cpp` 25,
`README.md` 17, `include/config.h` 15, `secrets.ini.example` 6,
`scripts/inject_secrets.py` 3, `docs/arquitectura.md` 3, `CLAUDE.md` 2,
`.gitignore` 2, `.github/workflows/pages.yml` 2, `INFORME.md` 1.

Filtrando solo las líneas con forma de asignación y descartando los
placeholders conocidos, queda **una sola**:

```
.github/workflows/pages.yml:38:  id-token: write
```

Que es un permiso de GitHub Actions, no una credencial.

Los únicos valores literales del repositorio son los placeholders de
`secrets.ini.example` (`TU_RED_WIFI`, `TU_PASSWORD_WIFI`,
`HOST_DE_THINGSBOARD`, `PEGAR_TOKEN_ACA`) y los cuatro respaldos
`SIN_CONFIGURAR` de `include/config.h`.

### Nota sobre la medición del tamaño

Durante la fase 5, al probar que el chequeo de higiene falla, quedó un objeto
colgado de 350 KB en `.git`. Se limpió con `git gc --prune=now`: los objetos
pasaron de 868 KB sueltos a un pack de 79 KB. Vale como recordatorio de que el
tamaño de un repositorio no baja solo.

---

## Pendientes de verificación humana

Cosas que este trabajo dejó listas pero **no pudo comprobar** desde el entorno
de desarrollo. Ninguna es un defecto conocido; son verificaciones faltantes.

| Qué falta verificar | Por qué no se pudo | Cuándo hacerlo |
|---|---|---|
| Habilitar Pages en modo "GitHub Actions" en cada repositorio | Es un cambio de configuración, no de código | Al crear el Classroom, por repositorio |
| Que la publicación funcione de punta a punta | `deploy-pages` necesita el token OIDC y la API de Pages | Primer push a `main` del repositorio template |
| Instalación del compilador en Windows (MSYS2) | No hay máquina Windows en el entorno | Antes de la primera clase |
| Que los tres diagramas Mermaid se dibujen | No hay navegador en el entorno | Al abrir el sitio publicado |
| `core_dir` compartido con dos cuentas reales | Se probó con una sola cuenta | Al preparar las máquinas del laboratorio |
| Los tres jobs de CI corriendo de verdad | Se verificaron estáticamente con `actionlint` y ejecutando la lógica de `higiene.sh` localmente | Primer push |

---

## Archivos que la especificación no pedía

Cuatro archivos fuera de la estructura de la sección 2, con su motivo.

| Archivo | Por qué |
|---|---|
| `scripts/higiene.sh` | El chequeo de tamaños vive en un script y el workflow lo invoca, en vez de estar embebido en el YAML. Permite verificar el mismo código que corre CI en lugar de una transcripción, y que los equipos lo corran antes de pushear |
| `docs/plan.md` | Cronograma en Gantt y seguimiento por sprint, con las fechas reales del curso. Pedido durante el trabajo |
| `.github/ISSUE_TEMPLATE/tarea.md` | Plantilla de issue para el tablero de GitHub Projects. Pedido durante el trabajo |
| `INFORME.md` | Este documento |

---

## Datos para planificar la clase

### Tiempos de compilación de `esp32dev`

| Situación | Tiempo | Qué incluye |
|---|---|---|
| Toolchain frío (primera vez) | **63,9 s** | Descarga e instalación del core Arduino 3.3.11, las libs de IDF 5.5.5, el toolchain xtensa, gdb, esptool y las dos librerías, más la compilación |
| Toolchain cacheado | **2 a 3 s** | Solo compilación |
| ESP8266 cacheado | 1,4 a 3 s | Solo compilación |

El número de toolchain frío es el que reportó PlatformIO en la corrida que
bajó todo salvo el zip del platform (2,1 MB) y `tool-esp_install`, que ya
estaban de un intento anterior. O sea que un arranque realmente desde cero es
algo más, pero del mismo orden. Medido sobre fibra doméstica: **el tiempo lo
domina la descarga, no la CPU**, así que en el laboratorio va a depender del
ancho de banda compartido entre los equipos.

### Espacio en disco

`~/.platformio` quedó en **6,0 GB** después de instalar los tres entornos.
Desglose real, medido con `du -sh`:

| Paquete | Tamaño | Cadena |
|---|---|---|
| `framework-arduinoespressif32-libs` | 2,1 GB | ESP32 |
| `toolchain-xtensa-esp-elf` | 1,3 GB | ESP32 |
| `toolchain-xtensa` | 248 MB | ESP8266 |
| `framework-arduinoespressif8266` | 126 MB | ESP8266 |
| `tool-xtensa-esp-elf-gdb` | 123 MB | ESP32 |
| `framework-arduinoespressif32` | 78 MB | ESP32 |
| `tool-scons`, `contrib-piohome` y otros | ~8 MB | común |

**Está dominado por la cadena de ESP32: unos 3,6 GB de los 6,0 GB**, casi diez
veces lo que ocupa la de ESP8266 (374 MB). El grueso es
`framework-arduinoespressif32-libs`, que trae las bibliotecas precompiladas de
ESP-IDF para todas las variantes del chip. Como la placa por defecto del curso
es la ESP32, es el número que hay que planificar.

Es un dato a avisar antes de la primera clase práctica: en una máquina con poco
disco, o en una cuenta de laboratorio con cuota, esto frena todo.

`~/.platformio` es **por usuario, no por proyecto**: se comparte entre todos
los proyectos del usuario y se descarga una sola vez. En máquinas
multiusuario de laboratorio se puede redirigir a un directorio compartido —
ver la sección siguiente.

### Redirección del `core_dir` en máquinas de laboratorio

Verificado empíricamente, no leído de la documentación. Los dos mecanismos
funcionan:

1. **`core_dir` en la sección `[platformio]` del `platformio.ini`.** Se armó
   un proyecto descartable con `core_dir` apuntando a un directorio temporal y
   se corrió un build: el platform y `tool-scons` se instalaron ahí (4,5 MB),
   y `~/.platformio` quedó intacto en 6,0 GB.
2. **Variable de entorno `PLATFORMIO_CORE_DIR`.** Con el `core_dir` sacado del
   `platformio.ini`, `pio system info` reporta el directorio de la variable
   como `PlatformIO Core Directory`.

**Hallazgo importante: el directorio compartido NO puede ser de solo lectura.**
Con los permisos de escritura quitados, el build falla aun estando todo ya
instalado:

```
PermissionError: [Errno 13] Permission denied: '<core_dir>/platforms.lock'
```

PlatformIO crea un lockfile en la raíz del `core_dir` en cada build. La
configuración viable es un directorio de grupo con `g+w` y el bit setgid, con
la contrapartida de que cualquier alumno puede modificar los paquetes de los
demás.

**Qué NO se verificó:** el escenario multiusuario real con dos cuentas
distintas y permisos de grupo. Se probó con una sola cuenta. Queda como TODO
antes de la clase, anotado también en el `README.md`.

**Dónde se documentó.** En un anexo del `README.md` para administradores de
laboratorio, explícitamente separado de las instrucciones para estudiantes, y
por variable de entorno. **No** se puso `core_dir` en el `platformio.ini` del
template: es configuración de máquina, no de proyecto, y si viajara en el
repositorio cada equipo se llevaría una ruta que solo existe en el laboratorio.

**Corrección de un error de este informe.** Una versión anterior de este
documento afirmaba que `~/.platformio` "no se puede redirigir". Es falso: se
puede, por las dos vías de arriba. La afirmación se emitió sin verificarla.

### Verificación de la regla crítica del `.gitignore`

La espec pedía que `secrets.ini` fuera la primera regla del archivo.
Confirmado: está en la **línea 9**, y las líneas 1 a 8 son un único bloque de
comentario que explica por qué no se commitea y qué hacer si se filtró. No hay
ninguna regla anterior — verificado con `awk` sobre las ocho líneas previas,
que devolvió vacío.

### Pages hay que habilitarlo a mano, y ningún workflow puede hacerlo

**Fase:** 5

**TODO (docente, y para cada repositorio de equipo): Settings → Pages → Build
and deployment → Source: "GitHub Actions".**

El workflow `pages.yml` publica con `actions/deploy-pages`, que requiere que
el repositorio tenga Pages configurado en modo "GitHub Actions". Si quedó en
el modo por defecto ("Deploy from a branch"), pasa una de dos cosas, y ninguna
es un mensaje claro: o el workflow termina en verde sin publicar nada, o falla
en el paso de publicación con un error de permisos que no menciona la
configuración.

No hay forma de resolverlo desde el repositorio: es un cambio en la
configuración, no en el código. Conviene incluirlo en la checklist de
preparación del GitHub Classroom, porque se multiplica por cada equipo.

Anotado en tres lugares: la cabecera de `.github/workflows/pages.yml`, la
sección de arranque del `README.md`, y acá.

**No verificado:** que la publicación funcione de punta a punta. Requiere un
repositorio real en GitHub con Pages habilitado; no se puede probar en local
ni con `act`, porque `deploy-pages` necesita el token OIDC y la API de Pages.

### Los diagramas Mermaid dependen de un CDN externo

**Fase:** 4

`mkdocs build --strict` termina sin advertencias y los bloques ```mermaid
quedan correctamente marcados como `class="mermaid"` en el HTML. Pero el
diagrama **no se dibuja al construir el sitio**: el HTML solo lleva el texto
del diagrama, y el dibujo lo hace el navegador del visitante con una librería
que el tema Material descarga en ese momento desde
`https://unpkg.com/mermaid@11/dist/mermaid.min.js` (verificado dentro del
bundle JavaScript del tema; la URL responde HTTP 200 y hoy redirige a la
versión 11.16.1).

Tres consecuencias a tener presentes:

1. **Sin internet no hay diagramas.** En una red del laboratorio sin salida, o
   con el CDN bloqueado, los diagramas se ven como texto plano. No es un error
   de configuración y el mensaje de error no existe: simplemente no se dibuja.
2. **La versión del renderizador no está fijada.** El `@11` de la URL es un
   rango mayor: cambia sola. Es el mismo problema que se corrigió con el
   platform de ESP32, pero acá la URL está adentro del bundle del tema, así
   que no se puede fijar sin sobrescribir el JavaScript del tema. Se dejó como
   está: el costo de mantener ese override supera al riesgo de que un diagrama
   se vea distinto.
3. **No se pudo verificar el render visual.** No hay navegador en este entorno.
   Lo verificado es que el marcado sale correcto y que la URL del CDN responde.
   Falta abrir el sitio publicado y confirmar que los tres diagramas de
   `arquitectura.md` (bloques, secuencia y Gantt) se dibujan.

Documentado en un comentario de `mkdocs.yml`, donde el estudiante lo va a leer
si le pasa.

**La integración continua NO depende del CDN.** Verificado empíricamente: se
construyó el sitio dentro de un contenedor con `--network none`, sin ninguna
salida de red (comprobado antes con una resolución DNS fallida). El resultado
fue `mkdocs build --strict` con `EXIT=0`, las 8 páginas generadas y los 3
bloques Mermaid presentes en el HTML. O sea que un laboratorio con unpkg
bloqueado no rompe el build ni la publicación: lo único que se pierde es el
dibujo del diagrama en el navegador de quien tenga el CDN bloqueado.

### El aviso de MkDocs 2.0 no nos afecta

Al construir, Material imprime un recuadro rojo advirtiendo que MkDocs 2.0
romperá los plugins y los temas. No es una advertencia del build de este
proyecto. Verificado: `mkdocs-material` 9.7.7 declara `mkdocs<2,>=1.6`, así que
la dependencia ya está acotada y una instalación limpia no puede traerse
MkDocs 2.0. Con `mkdocs-material~=9.7.7` en `requirements.txt`, el sitio se
construye igual en cualquier máquina.

### El falso SIGILL cuando un test falla

Al ejecutar el paso de la fase 2 que exige ver los tests fallar, apareció algo
que va a confundir a los equipos. Con la banda muerta rota, la salida termina
así:

```
test_banda_muerta_estando_encendido_sigue_encendido: Expected TRUE Was FALSE	[FAILED]
...
Program received signal SIGILL (Illegal instruction)
--------------- native:test_control [ERRORED] Took 0.19 seconds ---------------
============= 12 test cases: 4 failed, 7 succeeded in 00:00:00.191 =============
```

No hay ningún crash. Unity devuelve la cantidad de tests fallados como código
de salida del programa, y el runner de PlatformIO interpreta ese número como
si fuera una señal del sistema operativo.

Verificado con dos experimentos: con 4 tests fallando, el binario sale con
código 4 y PlatformIO reporta "señal 4" (SIGILL); con una implementación
preparada para que falle exactamente 1, sale con código 1 y reporta "señal 1"
(SIGHUP). La correlación es directa.

Efectos colaterales del mismo malentendido: la suite se marca `ERRORED` en vez
de `FAILED`, y el total de casos se infla en uno (12 en vez de 11).

**Cómo se resolvió.** Nota explicativa en la cabecera de
`test/test_control/test_control.cpp`, donde el estudiante la va a leer justo
cuando le pase. Lo que vale es la línea `[FAILED]` de cada test y el resumen
final, no el mensaje de señal.

---

## Desvíos de la especificación

Puntos donde el template se aparta de lo que la espec dice literalmente, con
el motivo.

### Plataforma de ESP32 fijada a un release concreto, no a `stable`

**Qué dice la espec.** La sección 3.2 pide el release `stable` de
`pioarduino/platform-espressif32`.

**Qué se hizo.** Se fijó la URL al tag `55.03.311`:

```
platform = https://github.com/pioarduino/platform-espressif32/releases/download/55.03.311/platform-espressif32.zip
```

**Motivo.** `stable` es un tag móvil: apunta siempre al último release
estable, así que cambia solo. Un equipo que clonara en septiembre y otro en
noviembre podrían compilar con toolchains distintos sin haber tocado una
línea del repositorio, que es exactamente el "en mi máquina anda" que el
template viene a eliminar. Además contradice lo que el curso enseña sobre
fijar versiones, y lo que el propio `platformio.ini` hace con `lib_deps`.

**Cómo se eligió el tag.** No se adivinó: se leyó el
`version: 55.03.311` del `platform.json` del platform ya instalado y se
confirmó contra la API de releases de GitHub, donde el tag `55.03.311`
(publicado el 2026-07-24) corresponde a "Arduino Release v3.3.11 based on
ESP-IDF v5.5.5". Se verificó además que las dos URLs devuelven exactamente el
mismo archivo: HTTP 200 y 2.116.056 bytes tanto con `stable` como con el tag
fijo.

**Verificación.** `pio run -e esp32dev` compila igual con la URL fijada:
mismo `Espressif 32 (55.3.311)`, mismo `framework-arduinoespressif32 @ 3.3.11`
y los mismos 276.808 bytes de flash que con `stable`.

**Mantenimiento.** Actualizar el tag es una decisión consciente, una vez por
semestre: se mira la lista de releases, se cambia el tag, se verifica que
compila y se anota en `docs/decisiones.md`. Queda comentado en
`platformio.ini`.

**Corrección sugerida para la espec.** Cambiar en 3.2 "release `stable`" por
"un release concreto, fijado por tag", y agregar la actualización del tag a la
checklist de preparación del semestre.

### El criterio de fijación de versiones, explicitado

El repositorio usa tres formas de fijar versiones y, sin explicación, parecen
contradictorias — sobre todo para un alumno al que se le insiste con fijar
versiones y después ve `actions/checkout@v7`.

| Dependencia | Criterio | Motivo |
|---|---|---|
| Plataforma ESP32 | Tag exacto (`55.03.311`) | Es el compilador: parte del artefacto. Si cambia, cambia el binario, y cambia en silencio |
| Librerías de firmware y del sitio | Rango menor (`^`, `~=`) | Parte del artefacto, pero solo se aceptan correcciones dentro de la misma versión mayor |
| Acciones de GitHub | Tag mayor (`@v7`) | Infraestructura de CI, no parte del artefacto. Sus autores respetan versionado semántico, y una rotura se ve en rojo al instante sin haber contaminado ningún binario |

La regla que ordena las tres: **fijar exacto lo que puede cambiar el resultado
sin avisar; dejar que se actualice lo que, si se rompe, lo dice en la cara.**

Documentado en tres lugares, para que sea criterio y no inconsistencia
aparente: sección propia en el `README.md`, encabezado de
`.github/workflows/ci.yml`, y la restricción 4 de `CLAUDE.md` —esta última
para que un agente de código no lo "corrija" por su cuenta—.

---

## Huecos de la especificación

Cosas que la espec no contemplaba y que aparecieron al ejecutar los criterios
de verificación. Están acá para corregir la espec de la próxima edición.

### 1. La fase 1 no se puede verificar sin fuente en `src/`

**Fase:** 1

**Qué dice la espec.** La sección 4 pide, como criterio de la fase 1, que
`pio run` compile sin que exista `secrets.ini`. Pero `src/main.cpp` es
entregable de la fase 3, dos fases después.

**Qué pasa.** Con `src/` vacío, PlatformIO ni siquiera llega a compilar:

```
Error: Nothing to build. Please put your source code files to the '.../src' folder
```

El script de inyección de secretos ya había corrido y hecho su trabajo
(el archivo se creó, el aviso se imprimió, las cuatro macros se definieron),
así que el criterio *conceptual* de la fase se cumplía. Lo que fallaba era el
comando con el que se lo verifica.

**Cómo se resolvió.** Se creó `src/main.cpp` en la fase 1. No como relleno
provisorio: quedó como **autodiagnóstico de arranque** (`autotest()`), que en
la fase 1 verifica lo único que existe a esa altura —que las cuatro macros
llegaron— y en la fase 3 se extiende con la verificación de sensores y
actuadores. Esto además cubre un requisito que ya estaba en la rúbrica del
curso: el código debe validar el funcionamiento individual de sensores y
actuadores en la etapa de set-up o en un modo test.

**Corrección sugerida para la espec.** Mover a la fase 1 la creación de
`src/main.cpp` con el autodiagnóstico, y dejar en la fase 3 la conexión WiFi,
MQTT, telemetría y RPC. La fase 3 extiende `autotest()`, no lo reemplaza.

### 2. Requisitos previos del sistema operativo no documentados

**Fases:** 1 y 2

Son dos dependencias distintas del sistema operativo, cada una descubierta al
ejecutar el criterio de verificación de una fase distinta. Ninguna de las dos
está en la espec.

#### 2.a — Compilador de C/C++ para el entorno `native` (fase 2)

**Qué pasa.** El entorno `native` compila con el toolchain del sistema
operativo, que PlatformIO no instala. En una máquina de escritorio sin
herramientas de desarrollo (el caso de una instalación limpia de Ubuntu
26.04: no venían ni `gcc`, ni `g++`, ni `clang`, ni `make`),
`pio test -e native` falla con:

```
sh: 1: gcc: not found
*** [.pio/build/native/unity_config_build/unity_config.o] Error 127
```

La documentación oficial de PlatformIO lo confirma y da instrucciones por
sistema operativo: `build-essential` en Debian/Ubuntu, Xcode Command Line
Tools en macOS, MSYS2 con tres rutas agregadas al `PATH` en Windows.
Referencia: https://docs.platformio.org/en/latest/platforms/native.html

Esto afecta a los tres sistemas operativos, no solo a Linux, y es la
dependencia con más probabilidad de frenar a un equipo en la primera semana:
el mensaje de error no dice en ningún lado que falte instalar algo del
sistema.

**Riesgo específico en Windows, donde está la mayoría del curso.** Windows no
trae compilador de C++ y PlatformIO tampoco lo instala, así que el alumno
tiene que instalar MSYS2 y agregar tres rutas al `PATH` del sistema a mano. Es
el paso más frágil de todo el arranque: además de instalar, hay que reabrir la
terminal y VS Code, porque los procesos ya abiertos conservan el `PATH` viejo y
el error no cambia. Está documentado en el foro oficial de PlatformIO como una
consulta recurrente desde Windows 7 hasta Windows 11.

Referencias:
- https://docs.platformio.org/en/latest/platforms/native.html
- https://github.com/platformio/platform-native/issues/2
- https://community.platformio.org/t/g-is-not-recognized/11256

**No verificado.** No hay máquina Windows disponible en este entorno. Las
instrucciones documentadas salen de la documentación oficial y de los reportes
de la comunidad, pero **hay que probarlas en Windows antes de la primera
clase**. Queda como TODO explícito en el `README.md`, sin darlo por resuelto.

**Decisión tomada: la corrida local de tests es OPCIONAL; el CI es el
mecanismo obligatorio.** El job `test` corre `pio test -e native` sobre Ubuntu
en cada push, y ese es el resultado que vale para la entrega. Quien esté en
Linux o macOS instala el compilador con un comando y trabaja con el ciclo
rápido; quien esté en Windows puede instalar MSYS2 si quiere, pero no es
requisito del curso.

El motivo es de riesgo: la mayoría del curso está en Windows, es donde la
instalación tiene más pasos manuales (tres rutas de `PATH` agregadas a mano),
y es lo único del arranque que no se pudo verificar. Hacerlo obligatorio sería
poner el camino crítico del curso sobre el único paso no probado.

**Cómo quedó en el README.** La sección de requisitos previos abre con una
tabla de obligatorio contra opcional. El compilador quedó como punto 2,
marcado opcional, con Windows en su propia subsección y la aclaración de que
sin él los tests igual corren en cada push.

**Cómo se resolvió.** Sección de requisitos previos del `README.md`, con una
tabla por sistema operativo, la verificación `g++ --version`, la advertencia
de reabrir la terminal en Windows, el TODO de verificación y la nota sobre CI.

#### 2.b — Módulo de entornos virtuales de Python (fase 1)

**Qué pasa.** En Debian y Ubuntu, Python no trae el módulo de entornos
virtuales por defecto. La plataforma de ESP32 (pioarduino) se crea su propio
entorno virtual en `~/.platformio/penv` para el toolchain de ESP-IDF, y falla
con un mensaje que no menciona a ESP32 por ningún lado:

```
The virtual environment was not created successfully because ensurepip is not available.
Error: Failed to create virtual environment
```

Verificado en Ubuntu 26.04 con Python 3.14. El entorno `nodemcuv2` no está
afectado: compila sin el paquete, porque el toolchain de ESP8266 no arma
ningún entorno virtual. Es decir, el problema aparece **solo** en la placa por
defecto del curso.

**Cómo se resolvió.** Se agregó al `README.md` una sección de requisitos
previos, antes del arranque en cinco pasos, con `sudo apt install python3-venv`
(metapaquete sin número de versión, que resuelve la versión según la
distribución). Aclara que aplica solo a Linux con instalación por línea de
comandos: quien use la extensión de VS Code en Windows o macOS no lo necesita.

**Corrección sugerida para la espec.** Agregar una sección de requisitos
previos por sistema operativo a la estructura objetivo del `README.md`
(sección 3.14).

---

## Qué archivo tocar para cada tipo de cambio

Mapa de mantenimiento del template. Cada fila dice qué archivos se tocan y qué
hay que verificar después.

### Cambios de curso

| Si hay que… | Tocar | Verificar con |
|---|---|---|
| Cambiar las fechas de los sprints | `docs/plan.md` (hitos del Gantt y tabla de seguimiento) | `mkdocs build --strict` |
| Cambiar la placa por defecto | `platformio.ini` (`default_envs`) y el `README.md` (paso 4 del arranque) | `pio run` sin `-e` |
| Cambiar el host de ThingsBoard | Nada: es configuración, va en `secrets.ini` de cada equipo. Solo el comentario de `secrets.ini.example` si cambia el formato | — |
| Cambiar el puerto de MQTT | `TB_PUERTO` en `include/config.h`. Si se volviera al 1883 sin cifrar habría que cambiar `WiFiClientSecure` por `WiFiClient` y sacar el `setInsecure()` en `lib/conexion/` | `pio run -e esp32dev -e nodemcuv2` |
| Validar el certificado del servidor en vez de `setInsecure()` | `configurarMqtt()` en `lib/conexion/conexion.cpp`, más el aviso "Sobre el cifrado" de `docs/arquitectura.md`. Ojo con el heap del ESP8266 | Prueba en hardware: no se puede verificar compilando |
| Agregar una clave secreta nueva | `secrets.ini.example`, la tupla `CLAVES` de `scripts/inject_secrets.py`, el respaldo `#ifndef` en `include/config.h`, y la verificación en `autotest()` de `src/main.cpp` | `rm -f secrets.ini && pio run -e esp32dev` |
| Cambiar los umbrales o el intervalo de publicación | `include/config.h` | `pio run -e esp32dev` |

### Cambios de contenido del sitio

| Si hay que… | Tocar | Verificar con |
|---|---|---|
| Agregar una página nueva | El archivo en `docs/` **y** el `nav` de `mkdocs.yml` (la navegación es explícita a propósito: sin esa línea, la página no aparece) | `mkdocs build --strict` |
| Agregar algo al README | Pensarlo dos veces: el README tiene solo lo del primer día, y todo lo demás vive en `docs/GUIA.md`. Si es material de consulta, va en la guía y se enlaza desde "Dónde está cada cosa" | Que el README siga entrando en pocas pantallas |
| Cambiar qué se le pide a una página | `docs/GUIA.md` (las cinco preguntas de esa sección) | `mkdocs build --strict` |
| Cambiar el contrato JSON de la telemetría | La tabla de `docs/arquitectura.md` **y** `publicarTelemetria()` en `src/main.cpp`, en el mismo commit. Renombrar un campo rompe los widgets del dashboard sin avisar | `pio run -e esp32dev` |
| Cambiar el aspecto del sitio | El bloque `theme` de `mkdocs.yml` | `mkdocs serve` |

### Cambios de versiones

| Si hay que… | Tocar | Verificar con |
|---|---|---|
| Actualizar la plataforma de ESP32 | El tag de la URL en `[env:esp32dev]` de `platformio.ini`. Mirar los releases de `pioarduino/platform-espressif32`, y anotarlo en `docs/decisiones.md` | `pio run -e esp32dev`, comparando el uso de flash antes y después |
| Actualizar una librería de firmware | `lib_deps` en `platformio.ini` | `pio run -e esp32dev -e nodemcuv2` y `pio test -e native` |
| Actualizar MkDocs Material | `requirements.txt` | `mkdocs build --strict` |
| Actualizar las acciones de CI | Los tags `@vN` en los dos workflows. Verificar las versiones contra la API de releases, no de memoria | `actionlint .github/workflows/*.yml` |

Antes de cambiar cualquier criterio de fijación, releer "El criterio de
versionado" más arriba: los tres criterios distintos son deliberados.

### Cambios de integración continua

| Si hay que… | Tocar | Verificar con |
|---|---|---|
| Cambiar los límites de tamaño | Los valores por defecto de `LIMITE_ARCHIVO_KB` y `LIMITE_REPO_MB` en `scripts/higiene.sh` | Correrlo en verde y en rojo: `bash scripts/higiene.sh` y `LIMITE_REPO_MB=0 bash scripts/higiene.sh` |
| Agregar un job a CI | `.github/workflows/ci.yml` | `actionlint` |
| Cambiar de dónde se publica el sitio | `on.push.branches` en `.github/workflows/pages.yml` | `actionlint` |
| Cambiar el directorio de salida del sitio | `site_dir` en `mkdocs.yml` **y** el `path:` de `upload-pages-artifact` en `pages.yml`, juntos | `mkdocs build --strict` y comparar la ruta con el `path:` |

### Cambios en el firmware

| Si hay que… | Tocar | Verificar con |
|---|---|---|
| Leer un sensor real | `leerSensor()` en `src/main.cpp`, **y** su verificación en `verificacionesDelProyecto()`, en el mismo archivo | `pio run -e esp32dev` y mirar el autodiagnóstico por el monitor serie |
| Agregar un comando del dashboard | La cadena de `else if` en `alRecibirMensaje()`, en `src/main.cpp`, **y** la tabla de RPC de `docs/arquitectura.md` | `pio run -e esp32dev` |
| Agregar un campo a la telemetría | `publicarTelemetria()` en `src/main.cpp` **y** el contrato JSON de `docs/arquitectura.md`, en el mismo commit | `pio run -e esp32dev` |
| Tocar WiFi o MQTT | `lib/conexion/`. Es infraestructura: si cambia, cambia para todos los equipos | `pio run -e esp32dev -e nodemcuv2` |
| Agregar una verificación genérica al autodiagnóstico | `lib/diagnostico/`. Si es de un sensor o actuador concreto, **no va acá**: va en `verificacionesDelProyecto()`, en `main.cpp` | `pio run -e esp32dev` |
| Agregar lógica testeable | `lib/control/` y sus tests en `test/test_control/` | `pio test -e native` |

### Cambios de reglas del proyecto

| Si hay que… | Tocar | Verificar con |
|---|---|---|
| Cambiar una restricción dura (secretos, `delay()`, versiones fijadas) | `CLAUDE.md`, sección "Restricciones duras", **y** el lugar del código donde se aplica | Revisión a mano |
| Cambiar los campos de las tareas | `.github/ISSUE_TEMPLATE/tarea.md` | Crear un issue de prueba |
| Cambiar el formato de la bitácora | `docs/bitacora/plantilla.md` y la explicación en `docs/bitacora/index.md` | `mkdocs build --strict` |

### Antes de dar por terminado cualquier cambio

```bash
pio run -e esp32dev && pio run -e nodemcuv2
pio test -e native
mkdocs build --strict
bash scripts/higiene.sh
actionlint .github/workflows/*.yml     # si tocaste los workflows
git status
```
