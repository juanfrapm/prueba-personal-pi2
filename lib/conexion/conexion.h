/**
 * conexion.h — WiFi y MQTT sobre TLS contra ThingsBoard.
 *
 * Esto es INFRAESTRUCTURA: funciona igual en todos los proyectos del curso y
 * normalmente no hace falta tocarlo. Si querés entender qué hace tu programa,
 * empezá por src/main.cpp; volvé acá cuando quieras saber CÓMO se conecta.
 *
 * Lo específico de cada placa (ESP8266 contra ESP32) se aísla en este archivo
 * y en ningún otro lado. El resto del firmware es igual para las dos.
 */

#ifndef CONEXION_H
#define CONEXION_H

/* WiFiClientSecure es el cliente con TLS, y no está en el mismo lugar en las
 * dos placas: en ESP8266 es un header aparte de la librería ESP8266WiFi
 * (implementado con BearSSL), y en ESP32 viene con el core. Se incluye acá y
 * en ningún otro lado. */
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#else
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

#include <Arduino.h>
#include <PubSubClient.h>

/* El cliente MQTT, compartido con el resto del firmware: src/main.cpp lo usa
 * para publicar telemetría y para responder las llamadas RPC. */
extern PubSubClient mqtt;

/**
 * Identificador único de este dispositivo dentro del broker.
 *
 * El client id tiene que ser único: si dos placas se conectan con el mismo,
 * el broker desconecta a la primera cuando entra la segunda, y el síntoma es
 * un dispositivo que "se cae solo cada pocos segundos" sin razón aparente.
 * Se arma con la MAC, que ya es única por placa.
 */
String idCliente();

/**
 * Primera conexión al WiFi. SE LLAMA SOLO DESDE setup().
 *
 * Es la única función del firmware que espera. Puede hacerlo porque corre una
 * sola vez, en el arranque, cuando todavía no hay ninguna conexión MQTT que
 * mantener viva. NO la llames desde loop(): ahí la reconexión va sin esperas,
 * con millis(), en mantenerWifi().
 */
void conectarWifi();

/**
 * Mantiene el WiFi vivo. SE LLAMA DESDE loop() Y NO ESPERA NUNCA.
 * @return true si hay WiFi en este momento.
 */
bool mantenerWifi(unsigned long ahora);

/**
 * Deja el cliente MQTT listo: servidor, callback de mensajes entrantes, tope
 * de conexión y tamaño de buffer. SE LLAMA UNA VEZ, DESDE setup().
 */
void configurarMqtt(void (*alRecibirMensaje)(char *, uint8_t *, unsigned int));

/**
 * Abre la conexión TLS con el broker. PUEDE DEMORAR hasta
 * MQTT_CONNECT_TIMEOUT_MS: negociar TLS no es instantáneo. El porqué de que
 * eso sea aceptable está explicado en conexion.cpp.
 */
bool conectarMqtt();

/**
 * Mantiene la conexión MQTT, reintentando cada MQTT_RECONNECT_MS.
 * SE LLAMA DESDE loop().
 * @return true si hay conexión en este momento.
 */
bool mantenerMqtt(unsigned long ahora);

#endif /* CONEXION_H */
