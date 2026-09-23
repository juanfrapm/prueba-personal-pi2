#include "conexion.h"

#include "config.h"

/* Objetos de red. Viven acá para que src/main.cpp no tenga que conocerlos.
 *
 * WiFiClientSecure, no WiFiClient: todo el tráfico con ThingsBoard va cifrado
 * por el puerto 8883. El 1883, sin cifrar, está cerrado en el servidor del
 * curso. */
static WiFiClientSecure red;
PubSubClient mqtt(red);

/* Marcas de tiempo de los reintentos. Ver mantenerWifi() y mantenerMqtt(). */
static unsigned long ultimoIntentoWiFi = 0;
static unsigned long ultimoIntentoMqtt = 0;

/* ===========================================================================
 * WiFi
 * ===========================================================================*/

void conectarWifi() {
  Serial.print(F("WiFi: conectando a "));
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  /* Espera acotada, y solo en el arranque. Si no engancha en 20 segundos, se
   * sigue igual: loop() reintenta sin bloquear. Nunca un while(true) esperando
   * al WiFi, que deja la placa colgada sin decir por qué. */
  const unsigned long limite = millis() + 20000UL;
  while (WiFi.status() != WL_CONNECTED && millis() < limite) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("WiFi: conectado. IP = "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("WiFi: no conectó todavía. Se reintenta sin bloquear."));
  }
}

/**
 * WiFi.begin() es asincrónico: dispara el intento y vuelve enseguida. El
 * resultado se consulta después con WiFi.status(), en la vuelta siguiente del
 * loop. Por eso acá no hay ni un while ni un delay.
 *
 * ¿Por qué reintentar a mano, si el framework ya trae reconexión automática?
 *
 *   Porque esa reconexión automática no es una sola cosa: su comportamiento
 *   por defecto cambia entre versiones del core y entre ESP8266 y ESP32.
 *   Depender de ella significa que el firmware se porta distinto según qué
 *   placa agarró cada equipo y qué versión bajó, sin que eso esté escrito en
 *   ningún lado.
 *
 *   Y sobre todo: si falla, falla en silencio. La placa queda muda, sin
 *   publicar nada y sin decir por qué, y desde afuera parece colgada o
 *   quemada. Con el reintento explícito, cada intento se anuncia por el
 *   monitor serie y el problema se ve.
 *
 * Reintentar a mano cuesta diez líneas y convierte un misterio en un mensaje.
 */
bool mantenerWifi(unsigned long ahora) {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  if (ahora - ultimoIntentoWiFi >= WIFI_RECONNECT_MS) {
    ultimoIntentoWiFi = ahora;
    Serial.println(F("WiFi: caído, relanzando conexión (sin esperar)"));
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
  return false;
}

/* ===========================================================================
 * MQTT
 * ===========================================================================*/

String idCliente() { return "pi2-" + WiFi.macAddress(); }

void configurarMqtt(void (*alRecibirMensaje)(char *, uint8_t *, unsigned int)) {
  // Cifra la conexión pero NO verifica la identidad del servidor.
  // El token viaja protegido, que es lo que importa acá. Verificar
  // la cadena completa consume memoria que al ESP8266 le sobra poca.
  // Cifrado y autenticado son cosas distintas.
  red.setInsecure();

  mqtt.setServer(TB_HOST, TB_PUERTO);
  mqtt.setCallback(alRecibirMensaje);

  /* Tope explícito para abrir la conexión. Es lo que acota cuánto puede
   * demorar el reintento que corre dentro de loop().
   *
   * Cada placa lo expone con un nombre distinto, y los defaults tampoco
   * coinciden; acá se igualan y quedan a la vista. El valor tiene que dar
   * lugar al handshake TLS, que tarda segundos: ver MQTT_CONNECT_TIMEOUT_MS
   * en include/config.h. */
#if defined(ESP8266)
  red.setTimeout(MQTT_CONNECT_TIMEOUT_MS);
#else
  red.setConnectionTimeout(MQTT_CONNECT_TIMEOUT_MS);
#endif

  /* setBufferSize() EXPLÍCITO. Esto no es opcional.
   *
   * PubSubClient trae un buffer de 128 bytes por defecto, y cuando un mensaje
   * no entra NO da error: publish() devuelve false y el mensaje simplemente no
   * sale. Del lado de ThingsBoard no llega nada, y en el monitor serie no hay
   * ninguna excepción ni ningún aviso. Es de los errores más difíciles de
   * encontrar del curso, porque todo "parece" estar bien.
   *
   * Un JSON con cuatro campos y nombres descriptivos ya se pasa de 128 bytes.
   * El tamaño se define en include/config.h. */
  mqtt.setBufferSize(MQTT_BUFFER_SIZE);
}

/**
 * Es el único punto del loop que espera: PubSubClient abre la conexión de
 * forma sincrónica (en su código, "_client->connect(this->domain, this->port)"),
 * así que mientras el broker no conteste, loop() no avanza. Con TLS esa espera
 * incluye el handshake completo. El tope es MQTT_CONNECT_TIMEOUT_MS, que se
 * aplica en configurarMqtt().
 *
 * ¿Por qué es aceptable esperar todo eso ACÁ, si en todo el resto del
 * firmware evitamos bloquear?
 *
 *   Porque esto solo corre cuando YA estamos desconectados. En ese estado no
 *   hay keepalive de MQTT que mantener —la conexión no existe— ni mensajes
 *   entrantes que atender. No se está postergando ningún trabajo: no hay
 *   trabajo. Es exactamente lo contrario de un delay() en medio del loop
 *   estando conectados, que sí rompe el keepalive y hace que el broker corte.
 *
 * ¿Y por qué no alcanzaba con dejar el valor por defecto?
 *
 *   Porque sin un tope explícito, un tb_host mal escrito en secrets.ini deja
 *   la placa trabada de a ratos largos, esperando a un servidor que no existe.
 *   Desde afuera eso es indistinguible de un cuelgue: el LED no responde, la
 *   telemetría no sale, y nada en el monitor serie dice "estoy esperando".
 *   Con el tope, el síntoma cambia: se ven intentos que expiran cada pocos
 *   segundos, que es una pista que lleva derecho a la causa.
 *
 * Además los defaults son distintos según la placa, así que fijarlo también
 * sirve para que las dos se comporten igual.
 */
bool conectarMqtt() {
  Serial.print(F("MQTT: conectando (TLS) a "));
  Serial.print(TB_HOST);
  Serial.print(':');
  Serial.println(TB_PUERTO);

  /* En ThingsBoard, el access token del dispositivo va como USERNAME de MQTT
   * y la contraseña queda vacía. No hay un campo "token" aparte. */
  if (mqtt.connect(idCliente().c_str(), TB_MQTT_USER, TB_MQTT_PASSWORD)) {
    Serial.println(F("MQTT: conectado"));
    mqtt.subscribe(TB_TOPIC_RPC_REQUEST);
    Serial.print(F("MQTT: suscrito a "));
    Serial.println(TB_TOPIC_RPC_REQUEST);
    return true;
  }

  /* Los dos códigos que vas a ver en la práctica:
   *   rc=5  "not authorized": token equivocado, vencido, o el dispositivo no
   *         existe en ThingsBoard. Llegaste al servidor, te rechazó.
   *   rc=-2 no se pudo abrir la conexión: no llegaste al servidor, o el
   *         handshake TLS no terminó. Casi siempre es tb_host mal escrito en
   *         secrets.ini, o la red bloquea el 8883. En ESP8266 también puede
   *         ser falta de memoria: TLS necesita varios KB de heap libre. */
  Serial.print(F("MQTT: falló, rc="));
  Serial.print(mqtt.state());
  Serial.println(F("  (rc=5: token inválido | rc=-2: no se llegó al host)"));
  return false;
}

bool mantenerMqtt(unsigned long ahora) {
  if (mqtt.connected()) {
    return true;
  }

  /* Se reintenta cada MQTT_RECONNECT_MS, no en cada vuelta del loop: con el
   * broker caído, insistir sin pausa sería quedarse esperando el timeout una
   * y otra vez sin hacer nada más. */
  if (ahora - ultimoIntentoMqtt >= MQTT_RECONNECT_MS) {
    ultimoIntentoMqtt = ahora;
    conectarMqtt();
  }
  return false;
}
