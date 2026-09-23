"""
Inyección de secretos en tiempo de compilación.

Se ejecuta ANTES de compilar (está declarado como `pre:` en platformio.ini).
Lee la sección [secrets] de secrets.ini y define cada clave como macro del
compilador (-D). Así el firmware usa WIFI_SSID, TB_TOKEN, etc. sin que ningún
valor esté escrito en el código fuente ni entre nunca al repositorio.

Precedencia: variable de entorno > secrets.ini.
Si existe una variable de entorno con el nombre de la clave en mayúsculas,
esa gana. Eso es lo que permite que la integración continua compile sin tener
el archivo, y que se puedan usar secretos de GitHub Actions si algún día hacen
falta.

Este script NUNCA imprime los valores. Imprime los nombres de las macros y
nada más: la salida de un build de CI es pública.
"""

import configparser
import os
import shutil

Import("env")  # noqa: F821  (lo inyecta SCons, no es un import de Python)

# Claves esperadas. Si agregás una, agregala también en secrets.ini.example
# y usala en el firmware con su nombre en mayúsculas.
CLAVES = ("wifi_ssid", "wifi_password", "tb_host", "tb_token")

RAIZ = env.subst("$PROJECT_DIR")  # noqa: F821
ARCHIVO = os.path.join(RAIZ, "secrets.ini")
PLANTILLA = os.path.join(RAIZ, "secrets.ini.example")


def aviso(*lineas):
    """Recuadro visible en medio de la salida de compilación."""
    print("")
    print("=" * 78)
    for linea in lineas:
        print(linea)
    print("=" * 78)
    print("")


# --- 1. Si falta secrets.ini, se crea a partir de la plantilla ---------------
# La primera compilación de un estudiante NUNCA debe fallar por un archivo que
# todavía no creó. Se crea, se avisa y se sigue compilando.
if not os.path.isfile(ARCHIVO):
    if os.path.isfile(PLANTILLA):
        shutil.copyfile(PLANTILLA, ARCHIVO)
        aviso(
            "AVISO: no existía secrets.ini, así que lo creé copiando",
            "secrets.ini.example.",
            "",
            "Tenés que abrirlo y completar los cuatro valores:",
            "  wifi_ssid, wifi_password, tb_host, tb_token",
            "",
            "Mientras tengan los valores de ejemplo, el firmware compila pero",
            "no se va a poder conectar al WiFi ni a ThingsBoard.",
            "",
            "secrets.ini está en .gitignore: no lo commitees nunca.",
        )
    else:
        aviso(
            "AVISO: no existe secrets.ini ni secrets.ini.example.",
            "Voy a compilar igual usando solo variables de entorno, si las hay.",
            "Si esto no era lo que esperabas, restaurá secrets.ini.example",
            "desde el repositorio.",
        )

# --- 2. Leer la sección [secrets] -------------------------------------------
parser = configparser.ConfigParser()
if os.path.isfile(ARCHIVO):
    parser.read(ARCHIVO, encoding="utf-8")

del_archivo = dict(parser["secrets"]) if parser.has_section("secrets") else {}

if os.path.isfile(ARCHIVO) and not parser.has_section("secrets"):
    aviso(
        "AVISO: secrets.ini existe pero no tiene la sección [secrets].",
        "Compará su formato con el de secrets.ini.example.",
    )

# --- 3. Resolver cada clave: el entorno le gana al archivo -------------------
definidas = []
faltantes = []
desde_entorno = []

for clave in CLAVES:
    nombre_macro = clave.upper()
    valor = os.environ.get(nombre_macro)

    if valor is not None:
        desde_entorno.append(nombre_macro)
    else:
        valor = del_archivo.get(clave)

    if valor is None or valor == "":
        faltantes.append(nombre_macro)
        continue

    # --- 4. Definir la macro -------------------------------------------------
    # StringifyMacro se encarga del entrecomillado y del escapado, para que un
    # valor con espacios o comillas no rompa la línea de compilación.
    env.Append(CPPDEFINES=[(nombre_macro, env.StringifyMacro(valor))])  # noqa: F821
    definidas.append(nombre_macro)

# --- 5. Informar SOLO nombres, jamás valores --------------------------------
if definidas:
    print("inject_secrets: macros definidas -> " + ", ".join(definidas))
if desde_entorno:
    print(
        "inject_secrets: tomadas del entorno (le ganan al archivo) -> "
        + ", ".join(desde_entorno)
    )
if faltantes:
    aviso(
        "AVISO: estas claves quedaron sin valor: " + ", ".join(faltantes),
        "El firmware compila igual (include/config.h tiene un valor de",
        "respaldo), pero esa parte no va a funcionar hasta que las completes",
        "en secrets.ini.",
    )
