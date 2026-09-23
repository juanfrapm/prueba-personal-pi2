# TODO: nombre del proyecto

!!! note "Cómo se usa esta página"
    Es la portada del sitio y lo primero que ve quien corrige. Tiene que
    responder, en menos de un minuto de lectura: qué problema resuelven, para
    quién, y quiénes son. Borrá los bloques como este a medida que completes.

## El problema

TODO: dos o tres párrafos. Qué situación concreta existe hoy, a quién le pasa,
y por qué es un problema que valga la pena resolver. Sin hablar todavía de la
solución ni de la tecnología.

> Ejemplo del tono esperado (de un proyecto de otro semestre):
>
> *En la sala de servidores del edificio no hay medición de temperatura. El
> aire acondicionado falló dos veces en el último año y en ambos casos el
> problema se descubrió recién cuando un equipo se apagó por sobrecalentamiento,
> varias horas después. No hay registro histórico, así que tampoco se puede
> saber si el equipo de refrigeración está trabajando al límite antes de que
> falle.*

## Nuestra solución

TODO: un párrafo. Qué construyeron y cómo ataca el problema de arriba.

TODO: una foto del prototipo armado.

!!! warning "Antes de subir una imagen"
    Comprimila. El límite del repositorio es 300 KB por archivo, y el job
    `higiene` de la integración continua falla si te pasás. Una foto de celular
    sin comprimir pesa entre 3 y 8 MB. Con exportarla a 1200 px de ancho y
    calidad 80 alcanza y sobra.

## El equipo

| Integrante | Rol / de qué se ocupó |
|---|---|
| TODO: Nombre Apellido | TODO: firmware, sensores |
| TODO: Nombre Apellido | TODO: ThingsBoard, dashboard |
| TODO: Nombre Apellido | TODO: documentación, pruebas |

## Estado actual

TODO: una lista corta de qué funciona hoy y qué falta. Actualizala en cada
entrega; es lo que permite ver el avance sin leer todo el sitio.

- [x] TODO: ejemplo de algo terminado
- [ ] TODO: ejemplo de algo pendiente

## Dashboard en vivo

<!--
  CÓMO EMBEBER EL DASHBOARD DE THINGSBOARD

  1. En ThingsBoard, abrí tu dashboard.
  2. Entrá al menú de los tres puntos (arriba a la derecha) y elegí
     "Compartir dashboard" / "Make dashboard public".
  3. Copiá la URL pública que te da.
  4. Descomentá el bloque de abajo y pegá esa URL en el atributo src.

  Ojo con dos cosas:
    - La URL pública deja el dashboard accesible para cualquiera que la tenga.
      No pongas datos sensibles ahí.
    - Si tu instancia de ThingsBoard usa http:// y el sitio de GitHub Pages es
      https://, el navegador puede bloquear el iframe por contenido mixto. En
      ese caso el iframe aparece vacío: no es un error de tu HTML.

<iframe
  src="TODO-PEGAR-URL-PUBLICA-DEL-DASHBOARD"
  width="100%"
  height="600"
  style="border: 1px solid #ccc; border-radius: 4px;"
  title="Dashboard de telemetría en ThingsBoard">
</iframe>
-->

TODO: descomentá el bloque de arriba cuando tengas el dashboard público.

## Cómo está organizado este sitio

| Sección | Qué vas a encontrar |
|---|---|
| [Plan](plan.md) | El cronograma y el seguimiento sprint por sprint |
| [Arquitectura](arquitectura.md) | Cómo está armado el sistema y el contrato de datos |
| [Decisiones](decisiones.md) | Por qué está armado así y qué alternativas se descartaron |
| [Pruebas](pruebas.md) | Qué se probó y con qué evidencia |
| [Bitácora](bitacora/index.md) | El registro del proceso, clase por clase |
