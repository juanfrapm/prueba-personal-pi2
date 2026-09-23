/**
 * Tests de la lógica de control (Unity).
 *
 * Se corren en tu PC, sin placa:  pio test -e native
 *
 * Los dos casos de banda muerta son los importantes. Una implementación
 * ingenua (`return valor > umbralAlto;`) pasa los casos de "muy frío" y "muy
 * caliente" y falla únicamente ahí en el medio. Si esos dos tests no
 * existieran, el error llegaría al prototipo y se manifestaría como un relé
 * que castañetea, que es mucho más difícil de diagnosticar que un test rojo.
 *
 * Antes de confiar en un test, vale la pena verlo fallar: rompé a propósito
 * el último `return estadoActual;` de lib/control/control.cpp y corré los
 * tests de nuevo. Después restaurá con `git checkout lib/control/control.cpp`.
 *
 * OJO con un mensaje confuso: cuando algún test falla, PlatformIO agrega una
 * línea del estilo
 *
 *     Program received signal SIGILL (Illegal instruction)
 *
 * y marca la suite como ERRORED. NO es un crash ni un bug de tu código.
 * Unity devuelve la cantidad de tests fallados como código de salida del
 * programa, y PlatformIO interpreta ese número como si fuera una señal del
 * sistema operativo: 4 fallas -> "señal 4" -> SIGILL; 1 falla -> "señal 1"
 * -> SIGHUP. Lo que vale es la línea [FAILED] de cada test y el resumen final.
 */

#include <unity.h>

#include "control.h"

/* Umbrales de los casos de prueba. La banda muerta va de 26,0 a 28,0. */
static const float ALTO = 28.0f;
static const float BAJO = 26.0f;

static const bool ENCENDIDO = true;
static const bool APAGADO = false;

void setUp(void) {}
void tearDown(void) {}

/* --------------------------------------------------------------------------
 * Casos fuera de la banda muerta
 * ------------------------------------------------------------------------*/

/* Por debajo de ambos umbrales y venía apagado: sigue apagado. */
void test_debajo_de_ambos_umbrales_estando_apagado_queda_apagado(void) {
  TEST_ASSERT_FALSE(debeActivar(20.0f, ALTO, BAJO, APAGADO));
}

/* Por debajo de ambos umbrales pero venía encendido: se apaga.
 * Es el caso que cierra el ciclo: enfrió lo suficiente, cortá. */
void test_debajo_de_ambos_umbrales_estando_encendido_se_apaga(void) {
  TEST_ASSERT_FALSE(debeActivar(20.0f, ALTO, BAJO, ENCENDIDO));
}

/* Por encima del umbral alto: enciende. */
void test_por_encima_del_umbral_alto_enciende(void) {
  TEST_ASSERT_TRUE(debeActivar(30.0f, ALTO, BAJO, APAGADO));
}

/* Por encima del umbral alto y ya estaba encendido: sigue encendido. */
void test_por_encima_del_umbral_alto_estando_encendido_sigue_encendido(void) {
  TEST_ASSERT_TRUE(debeActivar(30.0f, ALTO, BAJO, ENCENDIDO));
}

/* --------------------------------------------------------------------------
 * Banda muerta: los dos tests que detectan la implementación ingenua
 * ------------------------------------------------------------------------*/

/* Valor entre los dos umbrales y venía ENCENDIDO: tiene que seguir encendido.
 * Una implementación sin banda muerta devuelve false acá y apaga de más. */
void test_banda_muerta_estando_encendido_sigue_encendido(void) {
  TEST_ASSERT_TRUE(debeActivar(27.0f, ALTO, BAJO, ENCENDIDO));
}

/* Valor entre los dos umbrales y venía APAGADO: tiene que seguir apagado.
 * Una implementación que use >= umbralBajo enciende acá de más. */
void test_banda_muerta_estando_apagado_sigue_apagado(void) {
  TEST_ASSERT_FALSE(debeActivar(27.0f, ALTO, BAJO, APAGADO));
}

/* --------------------------------------------------------------------------
 * Bordes exactos
 *
 * Criterio fijado en control.h: las dos comparaciones son estrictas, así que
 * un valor exactamente igual a cualquiera de los dos umbrales cae DENTRO de
 * la banda muerta y mantiene el estado anterior.
 * ------------------------------------------------------------------------*/

/* valor == umbralAlto, apagado: NO enciende. Igualar no es superar. */
void test_valor_igual_al_umbral_alto_estando_apagado_no_enciende(void) {
  TEST_ASSERT_FALSE(debeActivar(ALTO, ALTO, BAJO, APAGADO));
}

/* valor == umbralAlto, encendido: sigue encendido. */
void test_valor_igual_al_umbral_alto_estando_encendido_sigue_encendido(void) {
  TEST_ASSERT_TRUE(debeActivar(ALTO, ALTO, BAJO, ENCENDIDO));
}

/* valor == umbralBajo, encendido: NO se apaga. Igualar no es bajar de. */
void test_valor_igual_al_umbral_bajo_estando_encendido_sigue_encendido(void) {
  TEST_ASSERT_TRUE(debeActivar(BAJO, ALTO, BAJO, ENCENDIDO));
}

/* valor == umbralBajo, apagado: sigue apagado. */
void test_valor_igual_al_umbral_bajo_estando_apagado_sigue_apagado(void) {
  TEST_ASSERT_FALSE(debeActivar(BAJO, ALTO, BAJO, APAGADO));
}

/* --------------------------------------------------------------------------
 * Secuencia completa: así se comporta a lo largo del tiempo
 *
 * Simula una lectura que sube, cruza la banda, baja y vuelve a cruzarla.
 * Es el test que muestra para qué sirve la histéresis: entre 26,5 y 27,5 el
 * estado NO cambia, sube o baje.
 * ------------------------------------------------------------------------*/
void test_secuencia_de_calentamiento_y_enfriamiento(void) {
  bool estado = APAGADO;

  estado = debeActivar(25.0f, ALTO, BAJO, estado);  /* frío        */
  TEST_ASSERT_FALSE(estado);
  estado = debeActivar(27.0f, ALTO, BAJO, estado);  /* subiendo    */
  TEST_ASSERT_FALSE(estado);                        /* aún apagado */
  estado = debeActivar(28.5f, ALTO, BAJO, estado);  /* pasó ALTO   */
  TEST_ASSERT_TRUE(estado);                         /* encendió    */
  estado = debeActivar(27.5f, ALTO, BAJO, estado);  /* bajando     */
  TEST_ASSERT_TRUE(estado);                         /* sigue on    */
  estado = debeActivar(26.5f, ALTO, BAJO, estado);  /* banda       */
  TEST_ASSERT_TRUE(estado);                         /* sigue on    */
  estado = debeActivar(25.5f, ALTO, BAJO, estado);  /* pasó BAJO   */
  TEST_ASSERT_FALSE(estado);                        /* apagó       */
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();

  RUN_TEST(test_debajo_de_ambos_umbrales_estando_apagado_queda_apagado);
  RUN_TEST(test_debajo_de_ambos_umbrales_estando_encendido_se_apaga);
  RUN_TEST(test_por_encima_del_umbral_alto_enciende);
  RUN_TEST(test_por_encima_del_umbral_alto_estando_encendido_sigue_encendido);

  RUN_TEST(test_banda_muerta_estando_encendido_sigue_encendido);
  RUN_TEST(test_banda_muerta_estando_apagado_sigue_apagado);

  RUN_TEST(test_valor_igual_al_umbral_alto_estando_apagado_no_enciende);
  RUN_TEST(test_valor_igual_al_umbral_alto_estando_encendido_sigue_encendido);
  RUN_TEST(test_valor_igual_al_umbral_bajo_estando_encendido_sigue_encendido);
  RUN_TEST(test_valor_igual_al_umbral_bajo_estando_apagado_sigue_apagado);

  RUN_TEST(test_secuencia_de_calentamiento_y_enfriamiento);

  return UNITY_END();
}
