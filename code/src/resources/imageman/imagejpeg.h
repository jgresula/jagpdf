// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEJPEG_JG1326_H__
#define __IMAGEJPEG_JG1326_H__



#include "interfaces/imagefilter.h"
#include <core/jstd/memory_stream.h>
#include <core/errlib/errlib.h>
#include <resources/interfaces/imagemaskdata.h>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_array.hpp>
#include <queue>
#include <boost/integer/endian.hpp>




namespace jag {
// FWD
class IStreamInput;
class ISeqStreamOutput;
class ISeqStreamInput;
class IResourceCtx;
class IImageData;

namespace resources {


namespace detail
{
  /**
   * @brief Accumulator of ICC profile chunks
   *
   * ICC chunks can be shuffled so we need to order
   * and join them and provide a stream with the result.
   */
  class jpeg_icc_accu_t
      : public noncopyable
  {
      const int m_nr_chunks;
      int m_next_chunk;

      struct chunk_rec_t
      {
          int                       chunk_nr;
          boost::shared_array<Byte> chunk;
          int                       size;
          chunk_rec_t(int chunk_nr_, boost::shared_array<Byte> chunk_, int size_);
          bool operator<(chunk_rec_t const& other) const;
      };

      std::priority_queue<chunk_rec_t>                 m_free_chunks;
      boost::shared_ptr<jstd::MemoryStreamOutput>   m_stream;

  public:
      explicit jpeg_icc_accu_t(int nr_chunks);
      int num_chunks() const { return m_nr_chunks; }
      void add_chunk(int chunk_nr, boost::shared_array<Byte> chunk, int size);
      std::auto_ptr<ISeqStreamInput> icc_profile();
  };

} // namespace detail




/**
 * @brief IImageFilter implementation of JPEG format (ISO 10918-1).
 *
 * Figuring out DPI.
 *  - if there is EXIF
 *    - 72dpi is set as a default before processing (EXIF specs)
 *    - EXIF command can explicitly overwrite it
 *  - if there is JFIF, the value is taken there
 *  - otherwise 0 is used
 *
 * Color Management.
 *  - if ICC marker found then the profile is used
 *  - if sRGB or found in EXIF then it is used
 *  - device space according to #components
 *  - if ICC not supported by client (pdf<1.3) - not implemented yet
 *    - check EXIF for sRGB and if found -> convert it to CalRGB
 *    - use device space according to #components
 */
class ImageJPEG
    : public IImageFilter
{
public:
    ImageJPEG(IImageData const* img_spec, boost::shared_ptr<IResourceCtx> res_ctx);
    static bool ping(IStreamInput& in_stream);

public: // IImageFilter
    UInt width() const { return m_info.width; }
    UInt height() const { return m_info.height; }
    UInt bits_per_component() const { return m_info.bits_per_component; }
    Double dpi_x() const { return m_info.dpix; }
    Double dpi_y() const { return m_info.dpiy; }
    bool output_image(ISeqStreamOutput& dest, unsigned max_bits) const;
    ColorSpaceHandle color_space() const;
    DecodeArray const* decode() const;
    Double gamma() const { return 0.0; }

    // do-nothing methods
    ImageMaskType image_mask_type() const { return IMT_NONE; }
    ColorKeyMaskArray const& color_key_mask() const {
        JAG_INTERNAL_ERROR;
//         static ColorKeyMaskArray tmp;
//         return tmp;
    }
    boost::intrusive_ptr<IImageMaskData> create_mask() const {
        JAG_INTERNAL_ERROR;
//         return boost::intrusive_ptr<IImageMaskData>();
    }

    boost::shared_ptr<IStreamInput> data_stream() const { JAG_ASSERT(!"suspicious"); return m_in_stream; }


    struct jpeg_info_t
    {
        int    width;
        int    height;
        double dpix;
        double dpiy;
        int    bits_per_component;
        int    nr_components;
        bool   progressive;
        bool   jfif;
        bool   exif_srgb;
        boost::scoped_ptr<detail::jpeg_icc_accu_t> icc;
        ColorSpaceHandle                           cs_handle;
    };

private:
    void parse_jpeg();
    bool icc_marker(Byte code, int size);
    bool jfif_marker(Byte code, int size);
    bool exif_marker(Byte code, int size);
    template<boost::integer::endianness END> void finish_exif_marker(int offset, int size);
    void prepare_color_space();

    enum { NUM_MARKERS = 3 };
    typedef bool (ImageJPEG::*fn_marker)(Byte, int);
    static const fn_marker s_markers[NUM_MARKERS];

private:
    jpeg_info_t                      m_info;
    boost::shared_ptr<IStreamInput>  m_in_stream;
    boost::weak_ptr<IResourceCtx>    m_res_ctx;
    DecodeArray                      m_decode;
};

}} // namespace jag::resources

#endif // __IMAGEJPEG_JG1326_H__
/** EOF @file */
