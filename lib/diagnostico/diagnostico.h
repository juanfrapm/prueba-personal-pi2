/**
 * diagnostico.h — autodiagnóstico de arranque.
 *
 * setup() llama a autotest() ANTES de levantar WiFi y MQTT. La idea es
 * separar dos clases de problema que desde afuera se ven igual ("no llega
 * nada a ThingsBoard"):
 *
 *   - autotest PASA  -> la configuración y el hardware están bien.
 *                       El problema es de red o de plataforma: WiFi, host,
 *                       token vencido, dispositivo mal creado en ThingsBoard.
 *   - autotest FALLA -> ni intentes mirar la red. Falta completar secrets.ini,
 *                       o hay un sensor mal conectado.
 *
 * Es el primer diagnóstico que hay que mirar cuando algo no anda, y es lo
 * primero que aparece en el monitor serie.
 *
 * Esto es INFRAESTRUCTURA: las verificaciones genéricas (placa, secretos,
 * parámetros) están acá y no hace falta tocarlas. Las de TU proyecto —cada
 * sensor y cada actuador que agregues— van en verificacionesDelProyecto(),
 * que está en src/main.cpp.
 */

#ifndef DIAGNOSTICO_H
#define DIAGNOSTICO_H

#include <Arduino.h>

/**
 * Corre el autodiagnóstico completo e imprime el resultado por el monitor
 * serie. Verifica lo genérico y, en el medio, llama a
 * verificacionesDelProyecto().
 */
void autotest();

/**
 * Imprime una línea de resultado y, si falló, la cuenta para el resumen final.
 * Es lo que usás para sumar tus propias verificaciones.
 *
 *   diagResultado(temp > -40 && temp < 85, "Sensor DHT22", "lectura en rango");
 *
 * @param ok       true si la verificación pasó.
 * @param que      Qué se verificó, en pocas palabras.
 * @param detalle  Dato o pista extra. Puede ser nullptr.
 */
void diagResultado(bool ok, const char *que, const char *detalle);

/**
 * Verificaciones propias del proyecto. LA DEFINÍS VOS, en src/main.cpp.
 *
 * autotest() la llama después de las verificaciones genéricas y antes del
 * resumen. Cada sensor y cada actuador que agregues al proyecto tiene que
 * verificarse ahí, usando diagResultado().
 */
void verificacionesDelProyecto();

#endif /* DIAGNOSTICO_H */
