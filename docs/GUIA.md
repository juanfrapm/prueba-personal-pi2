# Guía del proyecto

Qué preguntas tiene que responder cada página del sitio. Si una página las
responde, está completa; si no, le falta.

No es un manual de estilo ni hay que responderlas una por una como
cuestionario: son la vara con la que se mira cada página.

## Qué debe responder cada página del sitio

### [Inicio](index.md)

1. ¿Qué problema concreto resuelve el proyecto, y a quién le pasa?
2. ¿Qué construyeron, en una frase que se entienda sin saber de IoT?
3. ¿Quiénes son y de qué se ocupó cada uno?
4. ¿Qué funciona hoy y qué falta?
5. ¿Se puede ver el sistema andando desde acá, sin instalar nada?

### [Plan](plan.md)

1. ¿Cuál es el cronograma, y coincide con el que entregaron en el
   anteproyecto?
2. Para cada sprint: ¿a qué se comprometieron y qué lograron efectivamente?
3. ¿Qué se movió de fecha, y **por qué**?
4. ¿El Gantt refleja el estado de hoy, o quedó congelado en septiembre?
5. Mirando `git log -p docs/plan.md`, ¿se entiende la historia del plan?

!!! note "Por qué el Gantt va versionado y no en una diapositiva"
    Es el mismo diagrama del anteproyecto, pero acá vive en git. Cada ajuste
    queda como un commit con fecha, así que se puede ver qué cambió y cuándo
    sin depender de la memoria de nadie. Que un plan se mueva es normal; lo
    que se evalúa es si el equipo se dio cuenta, lo registró y explicó el
    motivo.

### [Arquitectura](arquitectura.md)

1. ¿Qué componentes hay y cómo se conectan entre sí?
2. ¿Qué recorrido hace un dato desde el sensor hasta la pantalla?
3. ¿Cuál es el formato exacto de la telemetría, campo por campo, con unidades
   y rangos?
4. ¿Qué entidades de ThingsBoard usan y para qué?
5. Si mañana entra alguien nuevo al equipo, ¿puede modificar el firmware sin
   preguntar cómo está armado?

### [Decisiones](decisiones.md)

1. ¿Qué se eligió, entre qué opciones?
2. ¿Por qué esa y no las otras?
3. ¿Cómo se verificó que la decisión fue correcta?
4. Si intervino IA, ¿qué propuso y qué cambió el equipo?
5. Dentro de seis meses, ¿alcanza esta entrada para entender la elección sin
   preguntarle a nadie?

### [Pruebas](pruebas.md)

1. ¿Qué tiene que cumplir el sistema para considerarse terminado?
2. Para cada criterio, ¿cuál es el estado de partida, la acción y el resultado
   esperado?
3. ¿Dónde está la evidencia de cada uno?
4. ¿Qué se probó que **no** funcionó, y qué se hizo al respecto?
5. ¿Otra persona puede reproducir estas pruebas con el prototipo en la mano?

### [Bitácora](bitacora/index.md)

1. ¿Qué se hizo cada día de clase?
2. ¿Qué falló y qué se probó para resolverlo?
3. ¿Qué se decidió, y quedó registrado en Decisiones si era importante?
4. ¿Cuál era el próximo paso al terminar cada clase?
5. ¿Las fechas de los commits acompañan a las fechas de las entradas?

---

!!! tip "La prueba más útil"
    Dale el enlace del sitio a alguien que no curse la materia y pedile que te
    cuente qué hace el proyecto. Si no puede después de leer la portada, la
    portada no está lista. Es más rápido y más honesto que releerla ustedes.

---

## Qué se entrega en este repositorio

| Entregable | Dónde vive |
|---|---|
| Firmware del prototipo (ESP8266 / ESP32) | `src/`, `lib/`, `include/` |
| Tests automáticos de la lógica | `test/` |
| Sitio de documentación del proyecto | `docs/` (se publica con GitHub Pages) |
| Bitácora semanal | `docs/bitacora/` |
| Registro de decisiones | `docs/decisiones.md` |
| Evidencia de pruebas | `docs/pruebas.md` |

---

## Cómo se publica el sitio

El sitio se construye en integración continua y se publica como artefacto de
GitHub Pages. **No hay rama `gh-pages` ni HTML generado versionado:** lo
construido se genera, no se commitea. Vos escribís Markdown en `docs/` y el
workflow hace el resto.

> **TODO (una sola vez, al crear el repositorio): habilitar Pages en modo
> Actions.**
>
> Andá a **Settings → Pages → Build and deployment** y poné
> **Source: "GitHub Actions"**.
>
> Sin ese cambio el workflow corre en verde y no publica nada, o falla en el
> paso de publicación con un error de permisos que no explica la causa. Ningún
> workflow puede hacerlo por sí mismo: es un clic en la configuración del
> repositorio.

---

## Cuando algo no anda: mirá el autodiagnóstico

`lib/diagnostico/` define `autotest()`, que `setup()` ejecuta **antes** de levantar
WiFi y MQTT. Verifica que la configuración haya llegado al firmware y que los
sensores y actuadores respondan. Su salida es lo primero que aparece en el
monitor serie.

Sirve para separar dos problemas que desde afuera se ven idénticos —"no llega
nada a ThingsBoard"— y que se resuelven en lugares opuestos:

| Salida | Qué significa | Dónde buscar |
|---|---|---|
| `AUTODIAGNÓSTICO: PASA` | La configuración y el hardware están bien | El problema es de **red o de plataforma**: WiFi caído, host mal escrito, token vencido, dispositivo mal creado en ThingsBoard |
| `AUTODIAGNÓSTICO: FALLA` | Algo local está mal | Ni mires la red todavía. Faltan valores en `secrets.ini`, o hay un sensor mal conectado |

Ejemplo de salida con el archivo de secretos recién creado y sin completar:

```
===== AUTODIAGNÓSTICO DE ARRANQUE =====
[ OK  ] Placa -> ESP32
[FALLA] WIFI_SSID -> sin completar
[FALLA] TB_TOKEN -> sin completar
...
AUTODIAGNÓSTICO: FALLA (4 verificación/es)
Revisá secrets.ini y el cableado ANTES de mirar la red.
```

Del SSID y del host se imprime el valor, porque no son secretos y verlos ahorra
la mitad de los errores de tipeo. De la contraseña se dice solo si está o no, y
del token únicamente su **longitud**: alcanza para detectar un copiado a medias
sin filtrar el valor por el monitor serie.

**`autotest()` crece con el proyecto.** Cada sensor y cada actuador que agregues
tiene que verificarse en `verificacionesDelProyecto()`, que está en
`src/main.cpp` y que `autotest()` llama en el medio: un sensor desconectado suele devolver `NaN` o un
valor fijo absurdo, y eso conviene detectarlo al arrancar y no tres días después
mirando una línea plana en el dashboard.

### Síntomas frecuentes

| Lo que ves | Causa más probable | Qué mirar primero |
|---|---|---|
| `AUTODIAGNÓSTICO: FALLA` con claves "sin completar" | No completaste `secrets.ini` | El archivo, no la red |
| **La placa se traba de a ratos, y en el monitor serie se ven intentos de conexión MQTT que expiran** | **`tb_host` mal escrito en `secrets.ini`** | **`tb_host`, antes que cualquier otra cosa** |
| `MQTT: falló, rc=5` | Token inválido, vencido, o el dispositivo no existe en ThingsBoard | El *Access token* del dispositivo |
| Conecta, publica un rato y se cae sola | Algo bloquea el `loop()` y rompe el keepalive MQTT | ¿Metiste un `delay()` en `loop()`? |
| Todo dice OK pero ThingsBoard no muestra nada | El payload no entra en el buffer de MQTT | `MQTT: no se pudo publicar` en el monitor; subí `MQTT_BUFFER_SIZE` |

El segundo caso merece una explicación, porque es el que más tiempo hace
perder. Si el host no existe, el firmware intenta abrir una conexión contra una
dirección que no contesta y **espera hasta 15 segundos** en cada intento (el
tope tiene que dar lugar al handshake TLS, que tarda segundos).
Durante esa espera la placa no hace nada más: no publica, no responde. Se ve
como tirones o como un cuelgue intermitente, y es fácil salir a buscar el
problema en el sensor, en la alimentación o en el cable.

La pista está en el monitor serie: si aparece `MQTT: conectando (TLS) a ...`
seguido de `MQTT: falló, rc=-2`, no es el hardware. Es el host, o el 8883
bloqueado en la red donde estás.
Revisá que `tb_host` no tenga `http://` adelante, ni el puerto pegado, ni un
espacio de más al copiar y pegar.

---

## Qué va en cada carpeta

| Ruta | Qué va acá |
|---|---|
| `src/` | El firmware. `main.cpp` es el punto de entrada: `setup()` y `loop()` |
| `lib/control/` | Lógica del proyecto **sin dependencias de Arduino**, para poder testearla en la PC |
| `lib/conexion/` | WiFi y MQTT. Infraestructura: normalmente no se toca |
| `lib/diagnostico/` | El autodiagnóstico de arranque. Infraestructura: se extiende desde `main.cpp` |
| `include/` | `config.h`: constantes no secretas (intervalos, tópicos, umbrales) |
| `test/` | Tests Unity. Un subdirectorio por suite (`test_control/`) |
| `scripts/` | `inject_secrets.py`, que inyecta los secretos como macros al compilar, y `higiene.sh`, el chequeo de tamaños que corre CI |
| `docs/` | Todas las páginas del sitio, en Markdown |
| `docs/bitacora/` | Una entrada por clase, más la plantilla |
| `.github/workflows/` | CI (compila y testea) y publicación del sitio |

Regla práctica: si el código se puede probar sin la placa, va en
`lib/control/` y se testea. En `src/main.cpp` queda solo el pegamento con el
hardware, la red y MQTT.

---

## Cómo se fijan las versiones, y por qué no siempre igual

Si mirás los archivos de configuración vas a ver dos criterios distintos, y a
primera vista parece una inconsistencia. No lo es.

| Dónde | Cómo se fija | Ejemplo |
|---|---|---|
| Plataforma de ESP32 (`platformio.ini`) | Tag **exacto** | `.../releases/download/55.03.311/...` |
| Librerías de firmware (`platformio.ini`) | Rango menor `^` | `bblanchon/ArduinoJson@^7.4.3` |
| Dependencias del sitio (`requirements.txt`) | Rango menor `~=` | `mkdocs-material~=9.7.7` |
| Acciones de GitHub (`.github/workflows/`) | Tag **mayor** | `actions/checkout@v7` |

La regla que ordena todo eso es una sola: **¿esto forma parte del artefacto que
entregamos?**

**El compilador sí.** La plataforma de ESP32 trae el toolchain que produce el
`.bin` que corre en la placa. Si cambia, cambia el binario: distinto tamaño,
distinto comportamiento en los bordes, a veces un bug que aparece en una
versión y no en otra. Y cambia **en silencio**: nada te avisa. Por eso va con
el tag exacto y no con el alias `stable`, que se mueve solo. Dos equipos que
clonan en meses distintos tienen que obtener el mismo compilador.

**Las acciones de GitHub no.** `actions/checkout` copia archivos, `actions/cache`
guarda una carpeta. Nada de eso entra en el firmware ni en el sitio: son
herramientas del proceso, no ingredientes del producto. Sus autores respetan
versionado semántico, así que dentro de `@v7` no hay cambios que rompan, y en
cambio sí llegan las correcciones de seguridad. Y si algo se rompiera, se ve
al instante y en rojo en la pestaña *Actions*, sin haber contaminado ningún
binario.

En el medio quedan las librerías, con rango menor: son parte del artefacto,
pero el `^` acepta solo correcciones dentro de la misma versión mayor, y esas
sí se quieren.

Resumido: **fijá exacto lo que puede cambiar el resultado sin avisarte; dejá
que se actualice lo que, si se rompe, te lo dice en la cara.**

---

## Tablero de tareas

El seguimiento del trabajo va en un **GitHub Project** del repositorio, y cada
tarea es un issue.

### Crear el tablero, una vez

1. En el repositorio, pestaña **Projects → New project**.
2. Elegí la plantilla **Board** y ponele el nombre del equipo.
3. **Add item → agregá los issues existentes**, o creá los nuevos desde el
   propio tablero.

### Crear una tarea

**Issues → New issue** y elegí la plantilla **Tarea del proyecto**. Pide tres
cosas: qué se hace, el criterio de aceptación en formato *Dado / Cuando /
Entonces*, y qué riesgo reduce.

El criterio de aceptación no es burocracia: cuando la tarea se cierra, esa
misma redacción se copia a [`docs/pruebas.md`](pruebas.md) con su
evidencia. Se escribe una vez y sirve dos veces.

### Cerrar issues desde los commits

Si escribís `Closes #12` en el mensaje del commit o en la descripción del pull
request, GitHub cierra el issue **solo** al mergear a `main`, y lo mueve a
*Done* en el tablero.

```bash
git commit -m "Integra el DHT22 y publica humedad

Closes #12"
```

Sirven también `Fixes #12` y `Resolves #12`. Ojo con dos detalles: el cierre
ocurre al mergear a la rama principal, no al pushear a una rama de trabajo; y
si ponés solo `#12` sin la palabra clave, queda el enlace pero el issue no se
cierra.

Vale la pena hacerlo así: el historial de git queda contando qué tarea resolvió
cada cambio, sin tener que mantener el tablero a mano.

---

## Secretos

**Ningún secreto se escribe en el código fuente.** El firmware no tiene ni un
literal con el SSID, la contraseña ni el token.

Cómo funciona:

1. Los valores viven en `secrets.ini`, que está en `.gitignore`.
2. Al compilar, `scripts/inject_secrets.py` corre antes que el compilador,
   lee ese archivo y define cada valor como macro (`-D`).
3. El código usa las macros. Si falta el archivo, el script lo crea copiando
   `secrets.ini.example` y avisa por consola: **la primera compilación nunca
   falla por eso**.
4. Si existe una variable de entorno con el mismo nombre en mayúsculas
   (`WIFI_SSID`, `TB_TOKEN`, …), esa gana sobre el archivo. Así corre CI sin
   necesitar secretos de repositorio.

### Si se te filtró un secreto

Pasa. Lo que importa es qué hacés después.

1. **Rotá el token en ThingsBoard.** Entidades → Dispositivos → tu dispositivo
   → Administrar credenciales → generá un *Access token* nuevo y guardá. El
   token viejo deja de servir en el acto.
2. Si lo que se filtró fue la contraseña del WiFi, cambiala en el router.
3. Actualizá tu `secrets.ini` local con el valor nuevo.
4. Avisale al equipo docente en la bitácora del día.

**Borrarlo en el commit siguiente no sirve.** Git guarda cada versión de cada
archivo para siempre: el valor sigue estando en el historial, y si el
repositorio es público hay bots que lo levantan en minutos. Reescribir el
historial tampoco alcanza si alguien ya clonó o si GitHub cacheó el commit.
Lo único que corta el problema de verdad es **invalidar la credencial**.

---

## Checklist de entrega

Antes de cada entrega, verificá que se cumpla todo esto:

- [ ] `pio run -e <tu entorno>` compila en un clon limpio, sin editar nada.
- [ ] `pio test -e native` pasa.
- [ ] `mkdocs build --strict` no tira advertencias.
- [ ] Los tres jobs de CI están en verde en el último commit de la rama.
- [ ] `git status` está limpio: no hay `.pio/`, ni `site/`, ni `secrets.ini`.
- [ ] Ningún archivo del repositorio contiene un token, una contraseña ni un
      SSID real.
- [ ] Ningún archivo versionado pesa más de 300 KB, y el repo entero pesa
      menos de 20 MB (el job `higiene` lo verifica solo).
- [ ] `docs/index.md` dice qué problema resuelve el proyecto y quiénes son los
      integrantes del equipo.
- [ ] `docs/arquitectura.md` tiene el diagrama y el contrato JSON de la
      telemetría, con cada campo documentado.
- [ ] `docs/decisiones.md` tiene una entrada por cada decisión técnica tomada,
      incluidas las que involucraron IA.
- [ ] `docs/pruebas.md` tiene los criterios de aceptación con su evidencia.
- [ ] La bitácora tiene una entrada por clase, **commiteada el día de la
      clase**.
- [ ] El sitio publicado en GitHub Pages abre y no tiene enlaces rotos.

---

## Anexo para administradores de laboratorio

**Esto no es para estudiantes.** Si estás cursando, ignorá esta sección: en tu
máquina personal la configuración por defecto es la correcta.

### Una sola copia del toolchain por máquina, no una por alumno

PlatformIO guarda plataformas y toolchains en `~/.platformio`, o sea **por
usuario**. En una máquina multiusuario de laboratorio eso significa que cada
alumno que compile se baja su propia copia. Medido en este proyecto con los
tres entornos instalados: **6,0 GB por usuario**, de los cuales unos 3,6 GB son
solo la cadena de ESP32.

Se puede redirigir a un directorio compartido con la variable de entorno
`PLATFORMIO_CORE_DIR`. Por ejemplo, en `/etc/profile.d/platformio.sh`:

```bash
export PLATFORMIO_CORE_DIR=/opt/platformio
```

Verificalo con `pio system info`, que imprime `PlatformIO Core Directory`.

### El directorio compartido tiene que ser escribible por los alumnos

Esto es lo que hay que tener en cuenta: **no alcanza con montarlo de solo
lectura.** PlatformIO crea un lockfile en la raíz del `core_dir` en cada
build, incluso cuando ya está todo instalado y no hay nada que descargar. Con
el directorio sin permiso de escritura, el build muere así:

```
PermissionError: [Errno 13] Permission denied: '/opt/platformio/platforms.lock'
```

La configuración que funciona es un directorio de grupo, escribible por el
grupo del curso:

```bash
sudo mkdir -p /opt/platformio
sudo chgrp -R alumnos /opt/platformio
sudo chmod -R g+w /opt/platformio
sudo find /opt/platformio -type d -exec chmod g+s {} +   # los archivos nuevos heredan el grupo
```

Conviene precargar los toolchains una vez, con una compilación de prueba, antes
de la primera clase: son varios GB de descarga y no es algo para hacer con
treinta alumnos compilando a la vez.

Contrapartida a tener presente: con el directorio compartido y escribible,
cualquier alumno puede modificar los paquetes que usan los demás. Es el precio
de no multiplicar 6 GB por cada cuenta.

> **TODO (docente): verificar con dos cuentas reales antes de la clase.** La
> redirección del `core_dir` y el fallo con permisos de solo lectura están
> verificados; el escenario multiusuario con permisos de grupo no se pudo
> probar, porque se hizo con una sola cuenta.

### No poner esto en el `platformio.ini` del template

`core_dir` también se puede fijar en la sección `[platformio]` del
`platformio.ini`, y funciona igual de bien. Pero **no va en el repositorio**:
es configuración de la máquina, no del proyecto. Si viajara en el template,
cada equipo se llevaría una ruta que solo existe en el laboratorio y les
rompería el build en su casa. Por eso va por variable de entorno, del lado del
sistema.

---

## Créditos

El firmware de ejemplo se basa en el trabajo de **Andrés Ferragut**
(ferragut@fi365.ort.edu.uy).
