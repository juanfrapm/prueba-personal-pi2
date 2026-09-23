/**
 * config.h — configuración NO secreta del proyecto.
 *
 * Acá va todo lo que se puede leer sin que sea un problema: intervalos,
 * tópicos MQTT, umbrales, tamaños de buffer. Editalo libremente.
 *
 * Lo que NO va acá: SSID, contraseñas ni tokens. Esos llegan como macros -D
 * desde scripts/inject_secrets.py, que los lee de secrets.ini (ignorado por
 * git). Este archivo solo define un valor de respaldo para que compile aunque
 * falte alguno.
 */

#ifndef CONFIG_H
#define CONFIG_H

/* ===========================================================================
 * Secretos: valores de respaldo.
 *
 * Cada #ifndef de abajo solo se aplica si la macro NO llegó por -D. En una
 * compilación normal ninguno de estos valores se usa. Están para que el
 * archivo compile igual si falta una clave, y para que el error se vea al
 * compilar y no como un cuelgue misterioso en la placa.
 * ===========================================================================*/

#ifndef WIFI_SSID
#define WIFI_SSID "SIN_CONFIGURAR"
#warning "WIFI_SSID no fue inyectado: completá wifi_ssid en secrets.ini"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "SIN_CONFIGURAR"
#warning "WIFI_PASSWORD no fue inyectado: completá wifi_password en secrets.ini"
#endif

#ifndef TB_HOST
#define TB_HOST "SIN_CONFIGURAR"
#warning "TB_HOST no fue inyectado: completá tb_host en secrets.ini"
#endif

#ifndef TB_TOKEN
#define TB_TOKEN "SIN_CONFIGURAR"
#warning "TB_TOKEN no fue inyectado: completá tb_token en secrets.ini"
#endif

/* ===========================================================================
 * Conexión a ThingsBoard
 * ===========================================================================*/

/* Puerto MQTT sobre TLS.
 *
 * El servidor del curso NO acepta el 1883, el puerto sin cifrar: está cerrado
 * a propósito. Por ahí el access token del dispositivo viajaría en claro, y
 * cualquiera en la misma red podría leerlo y publicar en nombre de tu placa.
 *
 * Todo el tráfico va por el 8883, cifrado. Ver conexion.cpp para el detalle
 * de qué garantiza ese cifrado y qué no. */
#ifndef TB_PUERTO
#define TB_PUERTO 8883
#endif

/* En ThingsBoard el access token del dispositivo se manda como USERNAME de
 * MQTT, con la contraseña vacía. No es un campo aparte. */
#define TB_MQTT_USER TB_TOKEN
#define TB_MQTT_PASSWORD ""

/* Milisegundos entre reintentos de conexión MQTT. */
#ifndef MQTT_RECONNECT_MS
#define MQTT_RECONNECT_MS 5000
#endif

/* Milisegundos entre reintentos de conexión WiFi cuando se cae la red.
 * El reintento NO espera: solo relanza WiFi.begin() y sigue. */
#ifndef WIFI_RECONNECT_MS
#define WIFI_RECONNECT_MS 10000
#endif

/* Tope de espera para abrir la conexión con el broker.
 *
 * Importa más de lo que parece. PubSubClient abre la conexión de forma
 * sincrónica, así que mientras espera, loop() no avanza. Se fija un valor
 * explícito para que el tope sea el mismo en las dos placas y esté a la
 * vista, en vez de depender de un default que puede cambiar entre versiones.
 *
 * Por qué 15 segundos y no 3: acá no se abre un socket pelado, se negocia
 * TLS. El handshake implica varios intercambios con el servidor y
 * criptografía de clave pública, que en un ESP8266 tarda segundos, no
 * milisegundos. Con un tope corto la conexión se cortaría a mitad del
 * handshake y el síntoma sería "no conecta nunca", sin ningún error claro.
 * 15000 ms es el valor que usa por defecto la propia BearSSL del ESP8266.
 *
 * Subirlo hace que un host inalcanzable congele el loop() más tiempo.
 * Bajarlo rompe el handshake en redes lentas o en la placa más chica. */
#ifndef MQTT_CONNECT_TIMEOUT_MS
#define MQTT_CONNECT_TIMEOUT_MS 15000
#endif

/* ===========================================================================
 * Tópicos MQTT de ThingsBoard
 *
 * Son fijos, los define ThingsBoard, no se inventan. "me" significa
 * literalmente "el dispositivo dueño de este token": no se pone el nombre del
 * dispositivo en ningún lado, el token ya lo identifica.
 *
 * Documentación: https://thingsboard.io/docs/reference/mqtt-api/
 * ===========================================================================*/

/* Publicar telemetría (lo que cambia todo el tiempo: temperatura, humedad). */
#define TB_TOPIC_TELEMETRY "v1/devices/me/telemetry"

/* Publicar atributos del cliente (lo que casi no cambia: versión, IP). */
#define TB_TOPIC_ATTRIBUTES "v1/devices/me/attributes"

/* Suscribirse a las llamadas RPC que manda el servidor. El + es un comodín:
 * el último segmento del tópico es el id de la petición, que cambia en cada
 * llamada. */
#define TB_TOPIC_RPC_REQUEST "v1/devices/me/rpc/request/+"

/* Para responder hay que concatenarle el id de la petición recibida:
 * v1/devices/me/rpc/response/<id> */
#define TB_TOPIC_RPC_RESPONSE_PREFIX "v1/devices/me/rpc/response/"

/* ===========================================================================
 * Buffer de MQTT
 * ===========================================================================*/

/* PubSubClient trae 128 bytes por defecto y descarta en silencio todo mensaje
 * más largo: no da error, simplemente no publica. Un JSON con cuatro campos y
 * nombres descriptivos ya se pasa. Este valor se aplica con setBufferSize()
 * en lib/conexion/; cambiarlo acá no alcanza si no se llama a esa función. */
#ifndef MQTT_BUFFER_SIZE
#define MQTT_BUFFER_SIZE 512
#endif

/* ===========================================================================
 * Comportamiento de la aplicación de ejemplo
 * ===========================================================================*/

/* Cada cuánto se publica telemetría, en milisegundos.
 * Ojo con bajarlo mucho: más mensajes es más consumo y más carga en el
 * servidor, y para un sensor ambiental no aporta nada. */
#ifndef PUBLISH_INTERVAL_MS
#define PUBLISH_INTERVAL_MS 5000
#endif

/* Umbrales de la histéresis de lib/control/.
 * El actuador se enciende al superar el umbral alto y se apaga al bajar del
 * umbral bajo. La franja entre los dos es la banda muerta: ahí no pasa nada,
 * y es lo que evita que el relé castañetee cuando el valor oscila alrededor
 * de un único umbral.
 * Regla: UMBRAL_ALTO tiene que ser mayor que UMBRAL_BAJO. */
#ifndef UMBRAL_ALTO
#define UMBRAL_ALTO 28.0f
#endif

#ifndef UMBRAL_BAJO
#define UMBRAL_BAJO 26.0f
#endif

/* Pin del actuador de ejemplo (LED o relé).
 * En muchas placas ESP8266 el LED integrado es el GPIO2 y está invertido:
 * LOW enciende. En la ESP32 dev habitual no hay LED integrado en un pin fijo,
 * así que se usa el GPIO2 igual pero con lógica normal. */
#ifndef PIN_ACTUADOR
#define PIN_ACTUADOR 2
#endif

#endif /* CONFIG_H */
