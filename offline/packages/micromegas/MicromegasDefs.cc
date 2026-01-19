/*!
 * \file MicromegasDefs.cc
 * \author Hugo Pereira Da Costa <hugo.pereira-da-costa@cea.fr>
 */

#include "MicromegasDefs.h"

namespace
{
   //* converninece trait for underlying type
  template<class T>
    using underlying_type_t = typename std::underlying_type<T>::type;

  //* convert an strong type enum to integral type
  template<class T>
    constexpr underlying_type_t<T>
    to_underlying_type(T value) noexcept
  { return static_cast<underlying_type_t<T>>(value);}

  /*!
   * hitsetkey layout:
   * Micromegas specific lower 16 bits
   * 24 - 32  tracker id
   * 16 - 24  layer
   * 8 - 16 segmentation type
   * 0 - 8 tile id
   */
  constexpr unsigned int kBitShiftSegmentation = 8;
  constexpr unsigned int kBitShiftTileId = 0;

  //! bit shift for hit key
  constexpr unsigned int kBitShiftStrip = 0;
  constexpr unsigned int kBitShiftSample = 8;

}

namespace MicromegasDefs
{

  /**
   * @brief Construct a Micromegas hitset key encoding layer, segmentation, and tile.
   *
   * Builds a TrkrDefs::hitsetkey for a Micromegas detector layer with the given
   * segmentation type and tile identifier encoded into the key.
   *
   * @param layer Micromegas layer number.
   * @param type Segmentation type to embed in the hitset key.
   * @param tile Tile identifier (0–255) to embed in the hitset key.
   * @return TrkrDefs::hitsetkey Hitset key with the layer, segmentation type, and tile id encoded.
   */
  TrkrDefs::hitsetkey genHitSetKey(uint8_t layer, SegmentationType type, uint8_t tile )
  {
    TrkrDefs::hitsetkey key = TrkrDefs::genHitSetKey(TrkrDefs::TrkrId::micromegasId, layer);

    TrkrDefs::hitsetkey tmp = to_underlying_type(type)&0x1U;
    key |= (tmp << kBitShiftSegmentation);

    tmp = tile&0xFFU;
    key |= (tmp << kBitShiftTileId);

    return key;
  }

  /**
   * @brief Retrieve the segmentation type encoded in a Micromegas hitset key.
   *
   * @param key Hitset key from which the segmentation type is extracted.
   * @return SegmentationType The segmentation type stored at the `kBitShiftSegmentation` bit field of the key.
   */
  SegmentationType getSegmentationType(TrkrDefs::hitsetkey key)
  {
    TrkrDefs::hitsetkey tmp = (key >> kBitShiftSegmentation);
    return static_cast<SegmentationType>(tmp&0x1U);
  }

  /**
   * @brief Extracts the tile identifier encoded in a Micromegas hitset key.
   *
   * @param key Hitset key containing the encoded tile id.
   * @return uint8_t Tile identifier (0–255) extracted from the key.
   */
  uint8_t getTileId(TrkrDefs::hitsetkey key)
  {
    TrkrDefs::hitsetkey tmp = (key >> kBitShiftTileId);
    return tmp&0xFFU;
  }

  /**
   * @brief Compose a hit key encoding a strip index and a sample identifier.
   *
   * Encodes the lower 8 bits of |strip| and the lower 16 bits of |sample| into a single TrkrDefs::hitkey.
   *
   * @param strip Lower 8-bit strip index to embed in the key.
   * @param sample 16-bit sample identifier to embed in the key.
   * @return TrkrDefs::hitkey Hit key with strip in the low 8 bits and sample placed at the sample bit field.
   */
  TrkrDefs::hitkey genHitKey(uint16_t strip, uint16_t sample)
  {
    const TrkrDefs::hitkey key = (strip&0xFFU) << kBitShiftStrip;
    const TrkrDefs::hitkey tmp = (sample&0xFFFFU) << kBitShiftSample;
    return key|tmp;
  }

  /**
   * @brief Retrieves the strip index encoded in a hit key.
   *
   * @param key Hit key containing the encoded strip and sample fields.
   * @return uint8_t Strip index (0–255).
   */
  uint8_t getStrip( TrkrDefs::hitkey key )
  {
    TrkrDefs::hitkey tmp = (key >> kBitShiftStrip);
    return tmp & 0xFFU;
  }

  /**
   * @brief Extracts the sample index encoded in a hit key.
   *
   * @param key Hit key containing an encoded sample field.
   * @return uint16_t Sample index extracted from the hit key (16-bit value).
   */
  uint16_t getSample( TrkrDefs::hitkey key )
  {
    TrkrDefs::hitkey tmp = (key >> kBitShiftSample);
    return tmp & 0xFFFFU;
  }

  //________________________________________________________________
  SegmentationType getSegmentationType(TrkrDefs::cluskey key)
  {
    const TrkrDefs::hitsetkey tmp = TrkrDefs::getHitSetKeyFromClusKey(key);
    return getSegmentationType( tmp );
  }

  //________________________________________________________________
  uint8_t getTileId(TrkrDefs::cluskey key)
  {
    const TrkrDefs::hitsetkey tmp = TrkrDefs::getHitSetKeyFromClusKey(key);
    return getTileId( tmp );
  }

}