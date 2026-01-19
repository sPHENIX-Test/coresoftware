#include "TrainingHits.h"

/**
 * @brief Constructs a TrainingHits object with all members zero-initialized.
 *
 * Initializes numeric members (radius, phi, z, phistep, zstep, layer, ntouch, nedge, cluskey)
 * to zero and fills the ADC vector container with zeros.
 */
TrainingHits::TrainingHits()
  : radius(0.)
  , phi(0.)
  , z(0.)
  , phistep(0.)
  , zstep(0.)
  , layer(0)
  , ntouch(0)
  , nedge(0)
  , cluskey(0)
{
  v_adc.fill(0);
}

/**
 * @brief Resets all members of TrainingHits to their default zero values.
 *
 * Sets floating-point members (radius, phi, z, phistep, zstep) to 0.0, integer members
 * (layer, ntouch, nedge, cluskey) to 0, and fills the v_adc container with zeros.
 */
void TrainingHits::Reset()
{
  v_adc.fill(0);
  radius = 0.;
  phi = 0.;
  z = 0.;
  phistep = 0.;
  zstep = 0.;
  layer = 0;
  ntouch = 0;
  nedge = 0;
  cluskey = 0;
}