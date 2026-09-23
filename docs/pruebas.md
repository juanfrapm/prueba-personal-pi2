# Pruebas

Criterios de aceptación en formato **Dado / Cuando / Entonces**, cada uno con
su evidencia. Un criterio sin evidencia es una intención, no una prueba.

!!! note "Qué cuenta como evidencia"
    Algo que otra persona pueda mirar y verificar por su cuenta: una captura
    del dashboard, un pedazo de salida del monitor serie, un enlace a la
    corrida de CI, una foto del banco de pruebas. "Lo probamos y anduvo" no es
    evidencia.

## Cómo escribir un criterio

- **Dado**: el estado de partida, todo lo que hace falta para reproducirlo.
- **Cuando**: la acción concreta, una sola.
- **Entonces**: el resultado observable, con números cuando se pueda.

Lo importante es que sea **reproducible**: otra persona con el prototipo
enfrente tiene que poder seguir los tres pasos y obtener lo mismo.

## Criterios de aceptación

| ID | Dado | Cuando | Entonces | Evidencia | Estado |
|---|---|---|---|---|---|
| CA-01 | El prototipo está alimentado y con `secrets.ini` completo, y el WiFi del laboratorio está disponible | Se energiza la placa y se espera 30 segundos | El monitor serie muestra `AUTODIAGNÓSTICO: PASA`, la IP obtenida y `MQTT: conectado`, y en ThingsBoard aparece el dispositivo como *Activo* con telemetría de menos de un minuto | Captura del monitor serie y captura de la pestaña *Últimos datos*: `evidencia/CA-01-arranque.png` | TODO |
| CA-02 | TODO | TODO | TODO | TODO | TODO |

TODO: agregá una fila por criterio. La primera está completa como ejemplo del
nivel de detalle esperado; fijate que el "Entonces" dice exactamente qué se
tiene que ver, no "funciona".

!!! tip "Criterios que casi siempre hacen falta"
    - Qué pasa cuando se cae el WiFi y vuelve.
    - Qué pasa con un sensor desconectado.
    - Qué pasa si el actuador se acciona desde el dashboard mientras el control
      automático está activo.
    - Cuánto tarda el sistema en reaccionar a un cambio real del sensor.

## Tests automáticos

La lógica de control tiene tests que corren sin la placa, en cada push:

```bash
pio test -e native
```

Cubren la histéresis de `lib/control/`: los casos por encima y por debajo de
los umbrales, los dos casos de banda muerta y los bordes exactos.

TODO: cada vez que agreguen lógica testeable, sumen tests acá. Regla práctica:
si se puede decidir sin tocar un pin, va en `lib/` y tiene test.

**Estado de la última corrida en CI:** TODO: pegá el enlace a la corrida en la
pestaña *Actions* de tu repositorio.

## Evidencia

Las capturas van en `docs/evidencia/`. Nombralas con el ID del criterio
(`CA-01-arranque.png`) para que se pueda ir de la tabla al archivo sin buscar.

!!! warning "Comprimí las imágenes antes de commitear"
    Límite de 300 KB por archivo, verificado por el job `higiene` de CI. Una
    captura de pantalla en PNG puede pasarse; exportala como JPG de calidad 80,
    o reducile el ancho. Git guarda **cada versión** de cada binario para
    siempre: una imagen pesada commiteada tres veces queda pesando tres veces
    en el repositorio, aunque después la borres.
