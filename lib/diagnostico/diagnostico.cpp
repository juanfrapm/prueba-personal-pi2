#include "diagnostico.h"

#include "config.h"

/* Verificaciones falladas en la corrida actual. */
static int fallas = 0;

void diagResultado(bool ok, const char *que, const char *detalle) {
  Serial.print(ok ? F("[ OK  ] ") : F("[FALLA] "));
  Serial.print(que);
  if (detalle != nullptr && detalle[0] != '\0') {
    Serial.print(F(" -> "));
    Serial.print(detalle);
  }
  Serial.println();
  if (!ok) {
    fallas++;
  }
}

/**
 * Verifica que una macro de secrets.ini haya llegado con un valor propio.
 *
 * Hay dos formas de que esté mal, y conviene distinguirlas:
 *   - "SIN_CONFIGURAR": el valor de respaldo de config.h. La macro no llegó
 *     por -D, o sea que el problema es del build, no del contenido.
 *   - El placeholder de la plantilla: el archivo se creó pero nadie lo
 *     completó.
 *
 * Nunca imprime el valor: solo dice si está o no.
 */
static bool valorConfigurado(const char *valor, const char *placeholder) {
  if (valor == nullptr || valor[0] == '\0') {
    return false;
  }
  if (strcmp(valor, "SIN_CONFIGURAR") == 0) {
    return false;
  }
  if (strcmp(valor, placeholder) == 0) {
    return false;
  }
  return true;
}

void autotest() {
  fallas = 0;

  Serial.println();
  Serial.println(F("===== AUTODIAGNÓSTICO DE ARRANQUE ====="));

  /* --- Placa -------------------------------------------------------------*/
#if defined(ESP8266)
  diagResultado(true, "Placa", "ESP8266");
#elif defined(ESP32)
  diagResultado(true, "Placa", "ESP32");
#else
  diagResultado(false, "Placa", "no reconocida");
#endif

  /* --- Configuración inyectada desde secrets.ini -------------------------*/
  /* Del SSID y del host se puede imprimir el valor: no son secretos, y
   * verlos ahorra la mitad de los problemas de tipeo.
   * De la contraseña y del token NO se imprime nada, ni siquiera parcial. */

  diagResultado(valorConfigurado(WIFI_SSID, "TU_RED_WIFI"), "WIFI_SSID",
                valorConfigurado(WIFI_SSID, "TU_RED_WIFI") ? WIFI_SSID
                                                           : "sin completar");

  diagResultado(valorConfigurado(WIFI_PASSWORD, "TU_PASSWORD_WIFI"),
                "WIFI_PASSWORD",
                valorConfigurado(WIFI_PASSWORD, "TU_PASSWORD_WIFI")
                    ? "presente (no se imprime)"
                    : "sin completar");

  diagResultado(valorConfigurado(TB_HOST, "HOST_DE_THINGSBOARD"), "TB_HOST",
                valorConfigurado(TB_HOST, "HOST_DE_THINGSBOARD")
                    ? TB_HOST
                    : "sin completar");

  /* Del token se informa la LONGITUD, nunca el contenido. Alcanza para saber
   * si llegó entero: los access token de ThingsBoard son de 20 caracteres,
   * así que un largo raro delata un copiado a medias. */
  const bool tokenOk = valorConfigurado(TB_TOKEN, "PEGAR_TOKEN_ACA");
  Serial.print(tokenOk ? F("[ OK  ] ") : F("[FALLA] "));
  Serial.print(F("TB_TOKEN -> "));
  if (tokenOk) {
    Serial.print(F("presente, longitud "));
    Serial.println(strlen(TB_TOKEN));
  } else {
    Serial.println(F("sin completar"));
    fallas++;
  }

  /* --- Parámetros de operación -------------------------------------------*/
  Serial.print(F("[ OK  ] Puerto MQTT (TLS) -> "));
  Serial.println(TB_PUERTO);

  Serial.print(F("[ OK  ] Intervalo de publicación (ms) -> "));
  Serial.println(PUBLISH_INTERVAL_MS);

  Serial.print(F("[ OK  ] Buffer MQTT (bytes) -> "));
  Serial.println(MQTT_BUFFER_SIZE);

  /* La histéresis con los umbrales al revés no falla al compilar, pero deja
   * el actuador pegado. Se verifica acá porque es un error de configuración,
   * no de código. */
  diagResultado(UMBRAL_ALTO > UMBRAL_BAJO, "Umbrales de histéresis",
                UMBRAL_ALTO > UMBRAL_BAJO
                    ? "alto > bajo"
                    : "UMBRAL_ALTO debe ser mayor que UMBRAL_BAJO");

  /* --- Lo que agrega cada equipo ------------------------------------------*/
  /* Sensores y actuadores del proyecto. Está en src/main.cpp. */
  verificacionesDelProyecto();

  /* --- Resumen ------------------------------------------------------------*/
  Serial.println(F("---------------------------------------"));
  if (fallas == 0) {
    Serial.println(F("AUTODIAGNÓSTICO: PASA"));
    Serial.println(F("Configuración y hardware OK. Si igual no llega nada a"));
    Serial.println(F("ThingsBoard, el problema es de red o de plataforma."));
  } else {
    Serial.print(F("AUTODIAGNÓSTICO: FALLA ("));
    Serial.print(fallas);
    Serial.println(F(" verificación/es)"));
    Serial.println(F("Revisá secrets.ini y el cableado ANTES de mirar la red."));
  }
  Serial.println(F("======================================="));
  Serial.println();
}
