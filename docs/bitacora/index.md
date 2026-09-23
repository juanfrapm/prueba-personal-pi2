# Bitácora

Un registro corto por clase. No es un informe: son cinco viñetas que se
escriben en diez minutos y que valen sobre todo cuando algo sale mal.

## La regla

!!! danger "La entrada se commitea el día de la clase"
    Git registra la fecha de cada commit, y **esa fecha es la evidencia**. Una
    bitácora de doce clases subida toda junta la noche anterior a la entrega se
    nota en el historial, y no sirve para nada: el valor está en escribirla
    cuando todavía te acordás de lo que no funcionó.

    Es preferible una entrada de tres líneas escrita el mismo día, que una
    entrada larga reconstruida de memoria dos meses después.

## Cómo se agrega una entrada

1. Copiá [`plantilla.md`](plantilla.md) a un archivo nuevo con la fecha:
   `2026-09-17.md`.
2. Completá las cinco viñetas.
3. Agregala al índice de abajo y al `nav` de `mkdocs.yml`.
4. Commiteala **ese mismo día**:

```bash
git add docs/bitacora/2026-09-17.md mkdocs.yml
git commit -m "bitacora: entrada del 17/09"
git push
```

!!! tip "Sobre el paso 3"
    La navegación de este sitio es explícita: si no agregás la entrada al `nav`
    de `mkdocs.yml`, la página existe pero no aparece en el menú. Es a
    propósito — evita que archivos sueltos terminen publicados sin querer.

## Para qué sirve

La viñeta que más rinde es **qué no funcionó**. Los problemas se repiten: el
sensor que devuelve valores raros en la clase 4 es el mismo que va a dar
problemas en la demo final, y tener anotado qué se probó ahorra repetir el
diagnóstico entero.

También es la materia prima de [Decisiones](../decisiones.md): cuando una
discusión de la bitácora termina en una elección, esa elección se copia allá
con el formato completo.

## Entradas

TODO: agregá acá el enlace a cada entrada nueva, de la más reciente a la más
vieja.

- TODO: `2026-09-17` — primera clase

<!--
Ejemplo de cómo queda una vez que tengas varias:

- [2026-10-01 — Integración del sensor](2026-10-01.md)
- [2026-09-24 — Primer envío a ThingsBoard](2026-09-24.md)
- [2026-09-17 — Armado del entorno](2026-09-17.md)
-->
