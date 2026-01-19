#include "TrainingHits.h"

/**
 * @brief Default-initializes a TrainingHits instance.
 *
 * Initializes numeric members (radius, phi, z, phistep, zstep) and integer
 * members (layer, ntouch, nedge, cluskey) to 0 and fills the ADC buffer
 * (v_adc) with zeros.
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
 * @brief Resets the hit data to an initial empty state.
 *
 * Clears the ADC sample buffer and sets all positional and state members to zero.
 * Specifically resets: v_adc, radius, phi, z, phistep, zstep, layer, ntouch, nedge, and cluskey.
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