# Registro de decisiones

Cada decisión técnica que costó pensar va acá, con el mismo formato. No es
burocracia: es lo que permite, tres meses después, entender por qué el código
es como es sin tener que reconstruir la discusión de memoria.

!!! note "Qué merece una entrada"
    Cualquier elección donde había más de un camino razonable: un sensor sobre
    otro, una librería, un intervalo de muestreo, cómo manejar un error. Si la
    decisión fue obvia y única, no hace falta. Si alguien podría preguntar
    "¿y por qué no hicieron X?", sí.

## Formato de cada entrada

Copiá este bloque y completalo. Las entradas van de la más nueva a la más
vieja, así lo último queda arriba.

```markdown
## AAAA-MM-DD — Título corto de la decisión

**Contexto.** Qué situación nos obligó a decidir.

**Alternativas evaluadas.**

| Opción | A favor | En contra |
|---|---|---|
| A | | |
| B | | |

**Decisión.** Cuál se eligió.

**Motivo.** Por qué esa y no las otras.

**Cómo se verificó.** Qué se midió o probó para confirmar que fue correcta.

**Intervención de IA.** Qué se le pidió, qué propuso, qué cambió el equipo y
por qué. Si no intervino, poner "no intervino".
```

---

## 2026-09-17 — Entrada de ejemplo: intervalo de publicación de telemetría

!!! warning "Esta entrada es un ejemplo"
    Está para mostrar el nivel de detalle esperado. Borrala cuando tengas
    entradas propias.

**Contexto.** Había que elegir cada cuánto publicar la lectura del sensor. El
firmware de ejemplo venía con 5 segundos y no sabíamos si era razonable para
una variable que se mueve lento como la temperatura ambiente.

**Alternativas evaluadas.**

| Opción | A favor | En contra |
|---|---|---|
| 1 s | Gráfico muy fino, se ve cualquier transitorio | 86.400 puntos por día y por dispositivo; satura la base de la instancia compartida |
| 5 s | Valor de fábrica del template | Sigue siendo mucho para una variable que cambia en minutos |
| 30 s | 2.880 puntos por día; suficiente para ver la dinámica térmica | Un pico corto puede pasar desapercibido |

**Decisión.** 30 segundos en operación normal.

**Motivo.** La constante de tiempo térmica del gabinete que medimos es de
varios minutos: entre dos muestras separadas 5 segundos la diferencia es menor
que el ruido del propio sensor, así que las muestras de más no agregan
información, solo puntos. Se dejó configurable en `include/config.h` para poder
bajarlo durante las pruebas.

**Cómo se verificó.** Registramos 20 minutos a 1 s y 20 minutos a 30 s con la
misma perturbación (abrir la puerta del gabinete). Las dos series muestran la
misma curva de subida; la de 1 s solo agrega ruido de ±0,2 °C. Evidencia en
[Pruebas](pruebas.md), fila CA-03.

**Intervención de IA.** Le pedimos a un asistente que sugiriera un intervalo.
Propuso 1 segundo "para buena resolución", sin preguntar qué variable era ni
cuántos dispositivos comparten el servidor. Lo descartamos después de hacer la
cuenta de puntos por día: con 12 equipos publicando a 1 s son más de un millón
de registros diarios en una instancia compartida. La cuenta la hicimos
nosotros; el asistente no la había planteado.

---

TODO: agregá tus decisiones acá arriba, la más reciente primero.
