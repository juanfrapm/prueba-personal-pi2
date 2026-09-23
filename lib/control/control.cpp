#include "control.h"

bool debeActivar(float valor, float umbralAlto, float umbralBajo,
                 bool estadoActual) {
  /* Por encima del umbral alto: encender, venga de donde venga. */
  if (valor > umbralAlto) {
    return true;
  }

  /* Por debajo del umbral bajo: apagar. */
  if (valor < umbralBajo) {
    return false;
  }

  /* Banda muerta: ni una cosa ni la otra, se mantiene lo que había.
   * Este return es la histéresis. Sin él la función sigue compilando y sigue
   * pareciendo correcta, pero el actuador conmuta con cada oscilación del
   * sensor. */
  return estadoActual;
}
