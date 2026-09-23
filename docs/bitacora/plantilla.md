# Plantilla de entrada

Copiá el bloque de abajo a un archivo nuevo con la fecha del día
(`docs/bitacora/2026-09-17.md`), completalo y commiteálo **ese mismo día**.

Son cinco viñetas fijas. No cambian, no se agregan, no se sacan: la gracia es
que todas las entradas se puedan leer en diez segundos.

```markdown
# 2026-MM-DD

- **Qué hicimos hoy:**
- **Qué no funcionó:**
- **Qué decidimos y por qué:**
- **Qué sigue:**
- **Evidencia:**
```

## Qué va en cada viñeta

**Qué hicimos hoy.** Lo concreto. "Conectamos el DHT22 al GPIO4 y leímos
temperatura por serie", no "avanzamos con el sensor".

**Qué no funcionó.** La más importante de las cinco. El error tal como
apareció, y qué probaron. Si no falló nada, escribí "nada" — pero es raro.

**Qué decidimos y por qué.** Las decisiones chicas del día. Si alguna es
grande, va también a [Decisiones](../decisiones.md) con el formato completo.

**Qué sigue.** El próximo paso concreto, no un objetivo general. Sirve para
arrancar la clase siguiente sin perder veinte minutos recordando dónde
quedaron.

**Evidencia.** Un hash de commit, una captura, una línea de salida del monitor
serie. Algo que se pueda mirar.

## Ejemplo completo

!!! example "Entrada real del tipo que esperamos"

    # 2026-09-24

    - **Qué hicimos hoy:** conectamos el DHT22 al GPIO4 y lo integramos a
      `leerSensor()`. Primera telemetría real llegando a ThingsBoard.
    - **Qué no funcionó:** las primeras lecturas daban `nan`. Probamos otro pin
      y otro cable sin éxito. Era la resistencia de pull-up de 10k que no
      habíamos puesto entre datos y 3V3; con ella, anduvo a la primera.
    - **Qué decidimos y por qué:** dejar el pull-up en la protoboard y no
      depender del interno del ESP32, porque el datasheet del DHT22 pide 10k y
      el interno ronda los 45k. Anotado en Decisiones.
    - **Qué sigue:** agregar humedad al contrato JSON y actualizar la tabla de
      Arquitectura.
    - **Evidencia:** commit `a3f9c21`, captura `docs/evidencia/2026-09-24-dht22.png`

Fijate que la viñeta de lo que no funcionó es la más larga. Casi siempre es
así, y es la que más se agradece releer dos meses después.
