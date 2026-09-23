/**
 * control.h — lógica de control del proyecto.
 *
 * Esta librería NO incluye Arduino.h ni depende del hardware. Es a propósito:
 * así se compila y se testea en tu PC con `pio test -e native`, en segundos y
 * sin la placa conectada.
 *
 * Regla del repositorio: si algo se puede decidir sin tocar un pin, va acá y
 * tiene tests. En src/main.cpp queda solo el pegamento con el hardware, la
 * red y MQTT, que es lo que no se puede testear en la PC.
 */

#ifndef CONTROL_H
#define CONTROL_H

/**
 * Decide si el actuador debe quedar encendido, con histéresis.
 *
 * La histéresis usa DOS umbrales en vez de uno. Con un solo umbral, un valor
 * que oscila apenas alrededor de él hace que el actuador conmute sin parar
 * (un relé castañetea, una bomba arranca y para cada dos segundos, y el
 * dashboard se llena de eventos). Con dos umbrales y una banda muerta entre
 * ellos, eso no pasa.
 *
 * Semántica:
 *   - valor > umbralAlto            -> enciende
 *   - valor < umbralBajo            -> apaga
 *   - umbralBajo <= valor <= umbralAlto  -> mantiene el estado anterior
 *
 * Esa tercera regla es la banda muerta, y es la parte que se olvida siempre.
 * Una implementación sin ella (por ejemplo, `return valor > umbralAlto;`)
 * pasa los casos obvios y falla justo en el medio, que es donde el sensor
 * pasa la mayor parte del tiempo.
 *
 * Criterio en los bordes: ambas comparaciones son ESTRICTAS.
 *   - "se enciende al superar umbralAlto": superar es ser mayor, no igual.
 *     Con valor == umbralAlto se mantiene el estado anterior.
 *   - "se apaga al bajar de umbralBajo": bajar es ser menor, no igual.
 *     Con valor == umbralBajo se mantiene el estado anterior.
 *   Los dos umbrales pertenecen entonces a la banda muerta. La alternativa
 *   (usar >= y <=) también es defendible; lo que no es defendible es no haber
 *   elegido, porque ahí el comportamiento en el borde queda librado a la
 *   suerte del redondeo en punto flotante. Los tests fijan este criterio.
 *
 * @param valor         Lectura actual del sensor.
 * @param umbralAlto    Umbral de encendido. Debe ser mayor que umbralBajo.
 * @param umbralBajo    Umbral de apagado.
 * @param estadoActual  Estado del actuador ahora (true = encendido).
 * @return              Estado que debe tener el actuador.
 */
bool debeActivar(float valor, float umbralAlto, float umbralBajo,
                 bool estadoActual);

#endif /* CONTROL_H */
