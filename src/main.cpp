/**
 * PI2 2026 — firmware del prototipo.
 *
 * Basado en el ejemplo original de Andrés Ferragut
 * (ferragut@fi365.ort.edu.uy).
 *
 * ---------------------------------------------------------------------------
 * ESTE ES EL ARCHIVO QUE VAS A EDITAR.
 *
 * Tiene solo lo que es de tu proyecto:
 *   leerSensor()                -> TODO: leé tu sensor acá
 *   verificacionesDelProyecto() -> TODO: verificá tu hardware al arrancar
 *   alRecibirMensaje()          -> TODO: agregá tus comandos del dashboard
 *   publicarTelemetria()        -> TODO: armá tu JSON de telemetría
 *   setup() y loop()            -> el esqueleto del programa
 *
 * Lo que NO está acá, porque es igual en todos los proyectos:
 *   lib/conexion/     -> WiFi y MQTT (conectarWifi, conectarMqtt, idCliente)
 *   lib/diagnostico/  -> el autodiagnóstico de arranque (autotest)
 *   lib/control/      -> la histéresis, con sus tests
 *
 * Ningún secreto está escrito acá. El SSID, la contraseña, el host y el token
 * llegan como macros -D desde scripts/inject_secrets.py, que los lee de
 * secrets.ini (ignorado por git).
 * ---------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <ArduinoJson.h>

#include "conexion.h"     /* WiFi y MQTT                     */
#include "config.h"       /* constantes no secretas          */
#include "control.h"      /* la histéresis, con sus tests    */
#include "diagnostico.h"  /* autodiagnóstico de arranque     */

/* ===========================================================================
 * Estado del proyecto
 * ===========================================================================*/

/* Estado del actuador. Lo decide la histéresis, salvo que una RPC lo fuerce. */
static bool actuadorEncendido = false;

/* Modo manual: cuando el dashboard fuerza el actuador por RPC, la histéresis
 * deja de mandar hasta que se vuelva a automático. Sin esto, el control
 * automático pisaría la orden del usuario en el siguiente ciclo y daría la
 * impresión de que el botón del dashboard "no anda". */
static bool modoManual = false;

/* Marca de tiempo de la última publicación. Ver la nota sobre millis() en
 * loop(). */
static unsigned long ultimaPublicacion = 0;

/* ===========================================================================
 * Sensor
 *
 * TODO (equipo): reemplazar por la lectura del sensor real del proyecto.
 * Mientras tanto devuelve una señal simulada, para que el flujo completo
 * (leer -> decidir -> publicar) se pueda probar sin hardware conectado.
 * ===========================================================================*/

static float leerSensor() {
  /* Simulación: una onda lenta entre 24 y 30, que cruza los dos umbrales por
   * defecto (26 y 28) y hace conmutar el actuador cada tanto. */
  const float fase = (millis() % 60000UL) / 60000.0f;  /* 0.0 a 1.0 en 60 s */
  return 27.0f + 3.0f * sinf(fase * 2.0f * PI);
}

/* ===========================================================================
 * Autodiagnóstico: la parte que agrega cada equipo
 *
 * autotest() (lib/diagnostico/) verifica lo genérico —placa, secretos,
 * parámetros— y en el medio llama a esta función. Acá va TU hardware.
 * ===========================================================================*/

void verificacionesDelProyecto() {
  /* --- Sensores ----------------------------------------------------------*/
  /* Criterio: la lectura tiene que ser un número (no NaN) y caer dentro del
   * rango físico plausible del sensor. Un sensor desconectado devuelve NaN o
   * un valor fijo absurdo, y eso conviene verlo al arrancar y no tres días
   * después mirando una línea plana en el dashboard.
   *
   * TODO (equipo): ajustar el rango al sensor real y agregar una verificación
   * por cada sensor que sumen. Si el sensor tiene función de init (por
   * ejemplo dht.begin()), llamarla acá antes de leer. */
  const float lectura = leerSensor();
  char detalleSensor[48];
  if (isnan(lectura)) {
    diagResultado(false, "Sensor", "devolvió NaN: revisá el cableado");
  } else {
    snprintf(detalleSensor, sizeof(detalleSensor), "lectura = %.2f", lectura);
    diagResultado(lectura > -40.0f && lectura < 85.0f, "Sensor", detalleSensor);
  }

  /* --- Actuadores --------------------------------------------------------*/
  /* Se acciona el pin y se lee de vuelta su estado. En ESP8266 y ESP32,
   * digitalRead() sobre un pin configurado como salida devuelve el valor del
   * latch, así que esto confirma que el pin quedó donde lo pusimos.
   *
   * Ojo con lo que esta prueba NO demuestra: que el LED o el relé estén
   * físicamente conectados. Para eso hay que mirar la placa. Lo que sí
   * descarta es un número de pin equivocado o un pin que no se puede usar
   * como salida.
   *
   * TODO (equipo): agregar una verificación por cada actuador del proyecto. */
  pinMode(PIN_ACTUADOR, OUTPUT);
  digitalWrite(PIN_ACTUADOR, HIGH);
  const bool leyoAlto = (digitalRead(PIN_ACTUADOR) == HIGH);
  digitalWrite(PIN_ACTUADOR, LOW);
  const bool leyoBajo = (digitalRead(PIN_ACTUADOR) == LOW);
  diagResultado(leyoAlto && leyoBajo, "Actuador",
                leyoAlto && leyoBajo ? "el pin responde a HIGH y a LOW"
                                     : "el pin no cambió de estado");
}

/* ===========================================================================
 * Comandos desde el dashboard (RPC)
 *
 * ThingsBoard manda las llamadas RPC al tópico
 *   v1/devices/me/rpc/request/<id>
 * con un cuerpo {"method": "...", "params": ...}, y espera la respuesta en
 *   v1/devices/me/rpc/response/<id>
 * con el mismo <id>. Documentación:
 *   https://thingsboard.io/docs/reference/mqtt-api/
 *
 * Métodos que atiende este firmware (contrato del proyecto, documentado en
 * docs/arquitectura.md):
 *   setEstado(bool)  -> fuerza el actuador y pasa a modo manual
 *   getEstado()      -> devuelve el estado actual del actuador
 *   setAuto()        -> vuelve al control automático por histéresis
 * ===========================================================================*/

static void alRecibirMensaje(char *topico, uint8_t *carga, unsigned int largo) {
  Serial.print(F("MQTT: mensaje en "));
  Serial.println(topico);

  /* ArduinoJson v7: JsonDocument crece solo. DynamicJsonDocument y
   * StaticJsonDocument quedaron obsoletos en la v7 y no se usan más. */
  JsonDocument peticion;
  const DeserializationError error = deserializeJson(peticion, carga, largo);
  if (error) {
    Serial.print(F("MQTT: JSON inválido -> "));
    Serial.println(error.c_str());
    return;
  }

  const char *metodo = peticion["method"];
  if (metodo == nullptr) {
    Serial.println(F("MQTT: la petición no trae \"method\""));
    return;
  }

  /* El id de la petición es el último segmento del tópico. Hay que
   * devolvérselo tal cual a ThingsBoard o la respuesta se pierde. */
  const char *idPeticion = strrchr(topico, '/');
  idPeticion = (idPeticion != nullptr) ? idPeticion + 1 : "0";

  JsonDocument respuesta;

  if (strcmp(metodo, "setEstado") == 0) {
    actuadorEncendido = peticion["params"].as<bool>();
    modoManual = true;
    digitalWrite(PIN_ACTUADOR, actuadorEncendido ? HIGH : LOW);
    respuesta["estado"] = actuadorEncendido;
    respuesta["modo"] = "manual";
    Serial.print(F("RPC setEstado -> "));
    Serial.println(actuadorEncendido ? F("encendido") : F("apagado"));

  } else if (strcmp(metodo, "getEstado") == 0) {
    respuesta["estado"] = actuadorEncendido;
    respuesta["modo"] = modoManual ? "manual" : "automatico";

  } else if (strcmp(metodo, "setAuto") == 0) {
    modoManual = false;
    respuesta["modo"] = "automatico";
    Serial.println(F("RPC setAuto -> vuelve el control por histéresis"));

  /* TODO (equipo): agregá acá los comandos de tu proyecto. Uno más es otro
   * "else if" con su método, lo que tenga que hacer, y qué contesta. Cada
   * método nuevo va documentado en la tabla de docs/arquitectura.md. */

  } else {
    respuesta["error"] = "metodo desconocido";
    Serial.print(F("RPC: método desconocido -> "));
    Serial.println(metodo);
  }

  char topicoRespuesta[96];
  snprintf(topicoRespuesta, sizeof(topicoRespuesta), "%s%s",
           TB_TOPIC_RPC_RESPONSE_PREFIX, idPeticion);

  char cuerpo[MQTT_BUFFER_SIZE];
  serializeJson(respuesta, cuerpo, sizeof(cuerpo));
  mqtt.publish(topicoRespuesta, cuerpo);
}

/* ===========================================================================
 * Telemetría
 * ===========================================================================*/

static void publicarTelemetria(float valor) {
  /* Contrato JSON de la telemetría. Cada campo está documentado en
   * docs/arquitectura.md; si agregan o renombran uno, actualicen ese archivo:
   * los widgets del dashboard se rompen con un cambio de nombre.
   *
   * TODO (equipo): agregá acá los campos de tus sensores. */
  JsonDocument telemetria;
  telemetria["temperatura"] = valor;
  telemetria["actuador"] = actuadorEncendido;
  telemetria["modo"] = modoManual ? "manual" : "automatico";
  telemetria["rssi"] = WiFi.RSSI();

  char cuerpo[MQTT_BUFFER_SIZE];
  const size_t largo = serializeJson(telemetria, cuerpo, sizeof(cuerpo));

  if (mqtt.publish(TB_TOPIC_TELEMETRY, cuerpo)) {
    Serial.print(F("TX "));
    Serial.print(largo);
    Serial.print(F(" bytes -> "));
    Serial.println(cuerpo);
  } else {
    /* publish() devuelve false si el payload no entra en el buffer. Si ven
     * esto, subí MQTT_BUFFER_SIZE en include/config.h. */
    Serial.println(F("MQTT: no se pudo publicar (¿payload más grande que el buffer?)"));
  }
}

/* ===========================================================================
 * setup() / loop()
 * ===========================================================================*/

void setup() {
  Serial.begin(115200);

  /* Un respiro para que el monitor serie enganche la salida del arranque.
   * Es la única espera bloqueante aceptable: pasa una sola vez y todavía no
   * hay ninguna conexión MQTT que mantener viva. */
  delay(500);

  /* Primero el autodiagnóstico, después la red. Si algo de esto está mal, no
   * tiene sentido intentar conectarse. Incluye verificacionesDelProyecto(),
   * que está más arriba en este archivo. */
  autotest();

  /* Espera acotada a 20 s, y solo acá. La reconexión en loop() no espera. */
  conectarWifi();

  /* Servidor, callback de mensajes entrantes, tope de conexión y tamaño de
   * buffer de MQTT. El detalle de por qué cada uno está en lib/conexion/. */
  configurarMqtt(alRecibirMensaje);

  pinMode(PIN_ACTUADOR, OUTPUT);
  digitalWrite(PIN_ACTUADOR, actuadorEncendido ? HIGH : LOW);
}

void loop() {
  /* ---------------------------------------------------------------------
   * NADA DE delay() ACÁ. Todo el trabajo periódico se hace comparando
   * millis() con la marca de tiempo de la última vez.
   *
   * Por qué: delay() congela el procesador entero. Durante ese rato no se
   * llama a mqtt.loop(), que es quien mantiene vivo el keepalive de MQTT y
   * quien procesa los mensajes entrantes. Con un delay(5000) el broker deja
   * de recibir señales de vida, corta la conexión, y encima las RPC del
   * dashboard llegan tarde o no llegan. El síntoma es "se desconecta solo
   * cada tanto" y la causa está acá.
   * -------------------------------------------------------------------*/

  const unsigned long ahora = millis();

  /* --- Mantener la conexión ---------------------------------------------*/
  /* Sin WiFi no hay nada que hacer. mantenerWifi() relanza la conexión cada
   * WIFI_RECONNECT_MS y vuelve enseguida: no espera. */
  if (!mantenerWifi(ahora)) {
    return;
  }

  /* Sin MQTT tampoco hay nada que publicar. mantenerMqtt() reintenta cada
   * MQTT_RECONNECT_MS; ese reintento es el único punto del loop que puede
   * demorar, acotado a MQTT_CONNECT_TIMEOUT_MS y solo estando ya
   * desconectados. El porqué, en lib/conexion/conexion.cpp. */
  if (!mantenerMqtt(ahora)) {
    return;
  }

  /* Procesa los mensajes entrantes y manda el keepalive. Tiene que llamarse
   * seguido, en cada vuelta. */
  mqtt.loop();

  /* --- Trabajo periódico -------------------------------------------------*/
  if (ahora - ultimaPublicacion >= PUBLISH_INTERVAL_MS) {
    ultimaPublicacion = ahora;

    const float valor = leerSensor();

    /* Control automático con histéresis. La lógica está en lib/control/ y
     * tiene tests: acá solo se la usa.
     * En modo manual no se toca: mandó el dashboard. */
    if (!modoManual) {
      actuadorEncendido =
          debeActivar(valor, UMBRAL_ALTO, UMBRAL_BAJO, actuadorEncendido);
      digitalWrite(PIN_ACTUADOR, actuadorEncendido ? HIGH : LOW);
    }

    publicarTelemetria(valor);
  }
}
