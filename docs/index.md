# NexoCampo

**Proyecto de prueba de monitoreo ambiental con placas ESP32 y módulos LoRa.**

NexoCampo propone conectar puntos de medición de una huerta o un invernadero con una estación receptora. El objetivo es experimentar con el envío de lecturas de temperatura y humedad desde lugares sin cobertura Wi-Fi y reunirlas en un panel de consulta.

!!! note "Propuesta de prueba"
    Esta página describe el sistema que se propone desarrollar. La integración LoRa, los sensores y las mediciones de alcance están pendientes de implementar y validar.

## El problema

En el escenario propuesto, los puntos de medición están separados y revisar cada sensor presencialmente lleva tiempo. Llevar conectividad Wi-Fi a todos esos puntos puede resultar poco práctico. Se busca transportar lecturas pequeñas y periódicas hacia un único punto de consulta.

## Nuestra solución

La prueba utilizará dos placas ESP32, cada una conectada a un módulo LoRa compatible. Una funcionará como nodo de medición y la otra como estación receptora. Primero se enviarán datos de prueba; después se incorporarán sensores reales y se evaluará la publicación en ThingsBoard mediante Wi-Fi.

1. El nodo ESP32 obtiene una lectura y prepara un mensaje con su identificador.
2. El módulo LoRa transmite el mensaje a la estación receptora.
3. La segunda ESP32 procesa el mensaje recibido.
4. La estación publica los datos en ThingsBoard mediante MQTT cuando dispone de Wi-Fi e internet.

## Las tecnologías

### ESP32: lectura y procesamiento

El ESP32 es un microcontrolador de Espressif. El ESP32 clásico dispone de Wi-Fi de 2,4 GHz, Bluetooth e interfaces como GPIO, I2C, SPI y UART para conectar periféricos. En NexoCampo se propone utilizarlo para leer sensores, preparar mensajes y gestionar la comunicación de la estación con el panel.

**El ESP32 por sí solo no incorpora radio LoRa.** Se necesita un módulo externo o una placa que integre ambos componentes. La conexión y los pines se definirán según el módulo elegido.

Referencia: [documentación oficial del ESP32, de Espressif](https://documentation.espressif.com/esp32_datasheet_en.html).

### LoRa: comunicación entre nodos

LoRa es una tecnología de modulación de radio orientada a comunicaciones de largo alcance y bajo consumo. Es adecuada para transmitir mensajes pequeños, como lecturas de sensores. El alcance y el consumo reales deberán medirse en el prototipo: dependen de las antenas, el entorno y la configuración del enlace.

**LoRa y LoRaWAN no son lo mismo.** LoRa proporciona el enlace de radio; LoRaWAN define un protocolo de red que utiliza esa tecnología. La primera prueba de NexoCampo propone un enlace LoRa punto a punto entre dos módulos, sin una red LoRaWAN.

Referencia: [introducción oficial a LoRa, de Semtech](https://www.semtech.com/lora/what-is-lora).

## Componentes propuestos

| Componente | Función en la prueba |
|---|---|
| Dos placas de desarrollo ESP32 | Ejecutar el programa del nodo y de la estación receptora |
| Dos módulos LoRa compatibles entre sí | Transmitir y recibir las lecturas |
| Antenas adecuadas para los módulos | Establecer el enlace de radio |
| Sensor de temperatura y humedad, por seleccionar | Obtener mediciones ambientales reales |
| Alimentación adecuada para cada placa y módulo | Sostener el funcionamiento del prototipo |
| ThingsBoard | Visualizar la telemetría cuando se complete la integración |

Antes del montaje se definirán el modelo de radio, la banda permitida para el lugar de uso, las antenas, la alimentación y los niveles lógicos compatibles.

## Estado y próximas pruebas

- [x] Definir el nombre y el escenario de prueba del proyecto.
- [x] Describir la función de ESP32 y LoRa en la propuesta.
- [ ] Seleccionar placas, módulos, sensores y conexiones.
- [ ] Implementar el envío y la recepción de mensajes de prueba por LoRa.
- [ ] Incorporar lecturas reales de temperatura y humedad.
- [ ] Medir mensajes recibidos y perdidos a distintas distancias.
- [ ] Evaluar el consumo y la recuperación ante interrupciones del enlace.
- [ ] Integrar la recepción con ThingsBoard y documentar los resultados.

## Equipo y panel

Los integrantes y sus responsabilidades están pendientes de completar. El enlace al panel se agregará cuando esté configurado y se hayan verificado las primeras lecturas.

## Documentación del proyecto

| Sección | Qué vas a encontrar |
|---|---|
| [Plan](plan.md) | Cronograma y seguimiento de tareas |
| [Arquitectura](arquitectura.md) | Componentes, conexiones y formato de los datos |
| [Decisiones](decisiones.md) | Elecciones técnicas y alternativas consideradas |
| [Pruebas](pruebas.md) | Procedimientos, resultados y evidencias |
| [Bitácora](bitacora/index.md) | Registro del trabajo y los avances |
