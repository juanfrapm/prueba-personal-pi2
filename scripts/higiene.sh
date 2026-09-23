#!/usr/bin/env bash
#
# Chequeo de higiene del repositorio.
#
# Falla si algún archivo versionado supera el límite por archivo, o si el
# repositorio completo supera el límite total.
#
# Lo corre la integración continua en cada push, pero podés correrlo vos antes
# de commitear:
#
#     bash scripts/higiene.sh
#
# Los límites se pueden cambiar por variable de entorno, sobre todo para
# probar que el chequeo realmente falla cuando tiene que fallar:
#
#     LIMITE_REPO_MB=1 bash scripts/higiene.sh
#
set -euo pipefail

LIMITE_ARCHIVO_KB="${LIMITE_ARCHIVO_KB:-300}"
LIMITE_REPO_MB="${LIMITE_REPO_MB:-20}"

limite_archivo_bytes=$((LIMITE_ARCHIVO_KB * 1024))
limite_repo_bytes=$((LIMITE_REPO_MB * 1024 * 1024))

echo "Chequeo de higiene"
echo "  límite por archivo: ${LIMITE_ARCHIVO_KB} KB"
echo "  límite del repo:    ${LIMITE_REPO_MB} MB"
echo

hubo_error=0

# --- 1. Archivos versionados demasiado grandes -------------------------------
excedidos=()
while IFS= read -r -d '' archivo; do
    [ -f "$archivo" ] || continue
    tamanio=$(stat -c%s "$archivo")
    if [ "$tamanio" -gt "$limite_archivo_bytes" ]; then
        excedidos+=("$archivo ($((tamanio / 1024)) KB)")
    fi
done < <(git ls-files -z)

if [ ${#excedidos[@]} -gt 0 ]; then
    hubo_error=1
    echo "ERROR: hay archivos versionados de más de ${LIMITE_ARCHIVO_KB} KB."
    echo
    for entrada in "${excedidos[@]}"; do
        echo "  - $entrada"
    done
    echo
    echo "Por qué importa:"
    echo
    echo "  Git guarda CADA VERSIÓN de CADA archivo binario para siempre. Una"
    echo "  foto de 3 MB commiteada tres veces deja 9 MB en el repositorio, y"
    echo "  borrarla después NO los recupera: siguen en el historial, y cada"
    echo "  persona que clone se los baja."
    echo
    echo "  Con las imágenes eso se arregla antes de commitear, no después:"
    echo "  exportalas a 1200 px de ancho y calidad 80. Una captura de pantalla"
    echo "  o una foto del prototipo entran cómodas en menos de 300 KB."
    echo
    echo "  Si el archivo grande es un binario generado (un .bin, un .elf, un"
    echo "  PDF que se arma solo), no va al repositorio: va al .gitignore."
    echo
fi

# --- 2. Tamaño total del repositorio -----------------------------------------
# Se mide el tamaño de los objetos de git, que es lo que realmente se baja
# cada persona al clonar. Incluye todo el historial, no solo los archivos
# que están hoy en el árbol de trabajo.
#
# Necesita el historial completo: en CI, el checkout de este job usa
# fetch-depth: 0 justamente por esto. Con el checkout superficial que viene
# por defecto, esta medición no significa nada.
tamanio_repo_kb=$(git count-objects -v | awk '
    /^size-pack:/ { pack = $2 }
    /^size:/      { suelto = $2 }
    END           { print pack + suelto }
')
tamanio_repo_bytes=$((tamanio_repo_kb * 1024))

if [ "$tamanio_repo_kb" -ge 1024 ]; then
    echo "Tamaño del repositorio (objetos de git): $((tamanio_repo_kb / 1024)) MB"
else
    echo "Tamaño del repositorio (objetos de git): ${tamanio_repo_kb} KB"
fi

if [ "$tamanio_repo_bytes" -gt "$limite_repo_bytes" ]; then
    hubo_error=1
    echo
    echo "ERROR: el repositorio supera los ${LIMITE_REPO_MB} MB."
    echo
    echo "  Casi siempre es por binarios en el historial: imágenes sin"
    echo "  comprimir, carpetas de build commiteadas por error, PDFs. Git"
    echo "  conserva cada versión de cada uno para siempre, así que el"
    echo "  repositorio solo crece."
    echo
    echo "  Para ver qué lo está inflando:"
    echo
    echo "      git ls-files | xargs -I{} du -h {} | sort -rh | head -20"
    echo
    echo "  Sacar algo del historial ya escrito requiere reescribirlo, lo que"
    echo "  rompe los clones de tus compañeros. Por eso este chequeo corre en"
    echo "  cada push: para avisar en el commit que lo agregó, cuando"
    echo "  arreglarlo todavía es fácil."
    echo
fi

# --- Resultado ---------------------------------------------------------------
if [ "$hubo_error" -ne 0 ]; then
    echo "Chequeo de higiene: FALLA"
    exit 1
fi

echo "Chequeo de higiene: OK"
