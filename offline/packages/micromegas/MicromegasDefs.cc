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
   * @brief Compose a Micromegas hitset key encoding layer, segmentation type, and tile id.
   *
   * The returned key encodes the Micromegas tracker identifier and the specified layer,
   * and embeds the segmentation type and tile identifier into their respective bit fields.
   *
   * @param layer Detector layer index.
   * @param type Segmentation type to encode.
   * @param tile Tile identifier (0–255).
   * @return TrkrDefs::hitsetkey A hitset key containing the Micromegas ID, layer, segmentation type, and tile id.
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
   * @brief Extracts the segmentation type encoded in a hitset key.
   *
   * @param key Hitset key from which to retrieve the segmentation type.
   * @return SegmentationType The segmentation type stored in the given hitset key.
   */
  SegmentationType getSegmentationType(TrkrDefs::hitsetkey key)
  {
    TrkrDefs::hitsetkey tmp = (key >> kBitShiftSegmentation);
    return static_cast<SegmentationType>(tmp&0x1U);
  }

  /**
   * @brief Extracts the tile ID from a Micromegas hitset key.
   *
   * @param key Hitset key that encodes the tile ID.
   * @return uint8_t Tile ID in the range 0–255.
   */
  uint8_t getTileId(TrkrDefs::hitsetkey key)
  {
    TrkrDefs::hitsetkey tmp = (key >> kBitShiftTileId);
    return tmp&0xFFU;
  }

  /**
   * @brief Constructs a hit key that encodes a strip index and a sample value.
   *
   * @param strip 8-bit strip identifier; only the lowest 8 bits are used.
   * @param sample 16-bit sample value; only the lowest 16 bits are used.
   * @return TrkrDefs::hitkey Combined hit key with the strip in bits [7:0] and the sample in bits [23:8].
   */
  TrkrDefs::hitkey genHitKey(uint16_t strip, uint16_t sample)
  {
    const TrkrDefs::hitkey key = (strip&0xFFU) << kBitShiftStrip;
    const TrkrDefs::hitkey tmp = (sample&0xFFFFU) << kBitShiftSample;
    return key|tmp;
  }

  /**
   * @brief Extracts the strip index from a hit key.
   *
   * @param key Packed hit key containing the strip and sample fields.
   * @return uint8_t Strip index extracted from the hit key (0–255).
   */
  uint8_t getStrip( TrkrDefs::hitkey key )
  {
    TrkrDefs::hitkey tmp = (key >> kBitShiftStrip);
    return tmp & 0xFFU;
  }

  /**
   * @brief Extracts the sample field from a hit key.
   *
   * @param key Hit key containing encoded strip and sample fields.
   * @return uint16_t The 16-bit sample value encoded in the key (sample bits starting at kBitShiftSample).
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
