# Template de proyecto — Proyecto Integrador 2 (IoT)

Este es el repositorio base de tu equipo. Acá vive **todo** el proyecto: el
firmware del prototipo, la lógica testeable, la documentación publicada y la
configuración de integración continua. La placa por defecto del curso es la
**ESP32**; el ESP8266 también compila.

---

## Requisitos previos

Leé primero qué es obligatorio y qué no: no todo aplica a todos.

| | Requisito | Para quién |
|---|---|---|
| **Obligatorio** | PlatformIO (paso 2 del arranque) | Todos |
| **Obligatorio** | Módulo de entornos virtuales de Python | Solo Linux, y solo si instalás PlatformIO por línea de comandos |
| *Opcional* | Compilador de C/C++ del sistema | Solo si querés correr los tests en tu máquina |

**Los tests son obligatorios; correrlos localmente, no.** Se ejecutan sí o sí
en cada push mediante GitHub Actions, y ese es el mecanismo que vale para la
entrega. Tenerlos localmente es más cómodo —el ciclo baja de minutos a
segundos— pero es una comodidad, no un requisito del curso.

### 1. El módulo de entornos virtuales de Python (obligatorio en Linux por CLI)

**Solo si estás en Linux y vas a instalar PlatformIO por línea de comandos.**
Si instalás desde la extensión de PlatformIO para VS Code, en Windows o en
macOS, saltealo: no te hace falta.

Las distribuciones basadas en Debian y Ubuntu no traen el módulo que Python
usa para crear entornos virtuales. Instalalo antes de empezar:

```bash
sudo apt install python3-venv
```

Usá el metapaquete `python3-venv` sin número de versión: resuelve solo cuál
corresponde a tu distribución. No pongas `python3.14-venv`, que existe únicamente
en Ubuntu 26.04.

Si te salteás este paso, la compilación para ESP32 falla con un error que no
menciona a ESP32 por ningún lado:

```
The virtual environment was not created successfully because ensurepip is not available.
Error: Failed to create virtual environment
```

Pasa porque la plataforma de ESP32 se arma su propio entorno virtual para el
toolchain de ESP-IDF. Con el paquete instalado, desaparece.

### 2. Un compilador de C/C++ del sistema (opcional, para correr los tests localmente)

Los tests del entorno `native` corren en tu PC, no en la placa, así que usan el
compilador de tu sistema operativo. **PlatformIO no lo instala**: baja solo los
toolchains de ESP32 y ESP8266.

**Esto es opcional.** Sin compilador local, `pio test -e native` falla con
`g++: not found` (Linux/macOS) o `'g++' is not recognized as an internal or
external command` (Windows), pero tus tests **igual corren en cada push** vía
GitHub Actions, y los ves en la pestaña *Actions* de tu repositorio. Eso es lo
que cuenta para la entrega.

| Sistema | Qué instalar | Esfuerzo |
|---|---|---|
| Linux (Debian/Ubuntu) | `sudo apt update && sudo apt install build-essential` | Un comando |
| macOS | `xcode-select --install` | Un comando |
| Windows | [MSYS2](https://www.msys2.org/) + configurar el `PATH` a mano (ver abajo) | Varios pasos |

Verificá con `g++ --version`.

En Linux y macOS es un comando y conviene hacerlo: el ciclo de trabajo con
tests locales es de segundos, contra minutos esperando a CI.

#### Windows: opcional de verdad

En Windows hay que instalar [MSYS2](https://www.msys2.org/) y después agregar
estas tres rutas al `PATH` del sistema, a mano:

```
C:\msys64\mingw64\bin
C:\msys64\ucrt64\bin
C:\msys64\usr\bin
```

Después de tocar el `PATH`, **cerrá y reabrí la terminal y VS Code**. Los
procesos que ya estaban abiertos conservan el `PATH` viejo, así que el error no
cambia aunque hayas instalado todo bien. Es la causa número uno de "lo instalé
y sigue sin andar".

Si te parece mucho lío, no lo hagas: **no es requisito del curso**. Escribí tus
tests igual, pusheá, y miralos correr en *Actions*.

> **TODO (docente): probar esto en Windows antes de la primera clase.** Las
> instrucciones salen de la documentación oficial de PlatformIO y de los
> reportes de la comunidad, pero no se verificaron en una máquina Windows real.

---

## Arranque en cinco pasos

Desde cero hasta ver telemetría en ThingsBoard. Los comandos se ejecutan tal
cual, parado en la raíz del repositorio.

### 1. Cloná tu repositorio y entrá

```bash
git clone <URL-de-tu-repo-de-equipo>
cd <nombre-del-repo>
```

### 2. Instalá PlatformIO

Opción recomendada: instalá **VS Code** y dentro la extensión **PlatformIO
IDE**. Trae el toolchain completo y no requiere configurar nada más.

Si preferís la línea de comandos:

```bash
python3 -m venv .venv
source .venv/bin/activate        # Windows: .venv\Scripts\activate
pip install platformio
```

### 3. Creá tu archivo de secretos

```bash
cp secrets.ini.example secrets.ini
```

Editá `secrets.ini` y completá los cuatro valores: el SSID y la contraseña de
tu WiFi, el host de ThingsBoard y el **access token** del dispositivo.

El token se saca de ThingsBoard: entrá a **Entidades → Dispositivos**, abrí tu
dispositivo, botón **Administrar credenciales** (*Manage credentials*), y ahí
está el *Access token*. Si el dispositivo todavía no existe, crealo primero
con **+ Agregar dispositivo**.

El host de ThingsBoard va sin `http://` y sin puerto: es solo el nombre del
servidor. Te lo pasan en clase.

`secrets.ini` está en `.gitignore`. **Nunca lo commitees.**

Si te salteás este paso no pasa nada grave: la primera compilación crea el
archivo sola a partir de la plantilla y te avisa por consola. Compila igual,
pero no se conecta hasta que completes los valores.

### 4. Compilá y subí el firmware

Conectá la placa por USB y corré:

```bash
pio run -t upload                   # ESP32, la placa por defecto del curso
pio run -e nodemcuv2 -t upload      # ESP8266 (NodeMCU v2), alternativa
```

> **Si se te filtró un token, hay que rotarlo.** Borrarlo en el commit
> siguiente no sirve: git guarda el historial para siempre. Cómo se hace, en
> [la guía](docs/GUIA.md#si-se-te-filtró-un-secreto).

### 5. Mirá la telemetría

```bash
pio device monitor
```

Lo primero que sale por el monitor serie es el **autodiagnóstico de arranque**,
antes de cualquier intento de conexión. Después vienen el WiFi, el MQTT y cada
publicación. En ThingsBoard, abrí tu dispositivo y andá a la pestaña **Últimos
datos** (*Latest telemetry*): los campos que publica el firmware aparecen ahí
a los pocos segundos.

---

## Comandos

| Qué querés hacer | Comando |
|---|---|
| Compilar para ESP32 (por defecto) | `pio run` |
| Compilar para ESP8266 | `pio run -e nodemcuv2` |
| Compilar y subir a la placa | `pio run -e <entorno> -t upload` |
| Ver el monitor serie (115200 baud) | `pio device monitor` |
| Correr los tests en la PC | `pio test -e native` |
| Limpiar la carpeta de build | `pio run -t clean` |
| Instalar las dependencias del sitio | `pip install -r requirements.txt` |
| Previsualizar el sitio en el navegador | `mkdocs serve` |
| Construir el sitio como lo hace CI | `mkdocs build --strict` |

`mkdocs serve` levanta el sitio en <http://127.0.0.1:8000> y lo recarga solo
cada vez que guardás un archivo de `docs/`.

---

## Dónde está cada cosa

Este README cubre el primer día: instalar, compilar y ver telemetría. Todo lo
demás vive en la guía del proyecto, para que esta página no te haga leer el
semestre entero antes de encender la placa.

| Lo que buscás | Dónde está |
|---|---|
| Cómo se maneja el tablero del equipo | [docs/GUIA.md](docs/GUIA.md#tablero-de-tareas) |
| Cómo se publica el sitio | [docs/GUIA.md](docs/GUIA.md#cómo-se-publica-el-sitio) |
| Cómo se escribe la bitácora | [docs/bitacora/index.md](docs/bitacora/index.md) |
| Qué se entrega en diciembre | [docs/GUIA.md](docs/GUIA.md#checklist-de-entrega) |

¿Algo no anda? Empezá por el
[autodiagnóstico de arranque](docs/GUIA.md#cuando-algo-no-anda-mirá-el-autodiagnóstico).
