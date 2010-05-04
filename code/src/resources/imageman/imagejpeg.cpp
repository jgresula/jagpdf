// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "imagejpeg.h"
#include <core/generic/assert.h>
#include <core/generic/checked_cast.h>
#include <core/generic/floatpointtools.h>
#include <core/generic/internal_types.h>
#include <core/jstd/streamhelpers.h>
#include <resources/interfaces/colorspaces.h>
#include <resources/interfaces/resourcectx.h>
#include <resources/interfaces/imagedata.h>
#include <resources/interfaces/colorspaceman.h>
#include <resources/resourcebox/binresources.h>
#include <interfaces/streams.h>
#include <resources/resourcebox/resourcepodarrays.h>
#include <msg_resources.h>
#include <string.h>




using namespace jag::jstd;
using namespace boost::integer;
using namespace boost;


namespace jag {
namespace resources {

namespace
{
  const Byte jpeg_header_c[3] = { 0xff, 0xd8,  0xff };
  const Double cmyk_decode[8] = { 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0 };

  struct jfif_header_t
  {
      Byte      version[2];
      Byte      units;
      ubig16_t  x_density;
      ubig16_t  y_density;
  };

  struct frame_header_t
  {
      ubig16_t  size;
      Byte      bpc;
      ubig16_t  y;
      ubig16_t  x;
      Byte      nr_components;
  };



 /**
  * @brief Checks application marker signature
  *
  * @param stream   stream pointing to the signature
  * @param sig      expected signature
  * @param sig_len  length of the expected signature
  * @return 0 if signatures differ, otherwise offset from the current position
  *         to original stream position
  */
  int check_marker_sig(IStreamInput& stream, char const* sig, int sig_len)
  {
      const int buf_len = 16;
      JAG_PRECONDITION(sig_len <= buf_len);

      Byte buffer[buf_len] = {0};
      stream.read(&buffer, sig_len);
      int seek_offset = -sig_len;  // number of bytes to seek after processing jfif
      if (memcmp(buffer, sig, sig_len))
      {
          stream.seek(seek_offset, OFFSET_FROM_CURRENT);
          return 0;
      }

      return seek_offset;
  }


} // anonymous namespace



////////////////////////////////////////////////////////////
// class jpeg_icc_accu_t
////////////////////////////////////////////////////////////

namespace detail
{
  jpeg_icc_accu_t::jpeg_icc_accu_t(int nr_chunks)
      : m_nr_chunks(nr_chunks)
      , m_next_chunk(1)
      , m_stream(new  MemoryStreamOutput)
  {
      if (m_nr_chunks <= 0)
          throw exception_invalid_input(msg_corrupted_jpeg());
  }


  void jpeg_icc_accu_t::add_chunk(int chunk_nr, shared_array<Byte> chunk, int size)
  {
      JAG_PRECONDITION_MSG(m_stream, "already outputted");
      if (chunk_nr > m_nr_chunks || chunk_nr < m_next_chunk)
          throw exception_invalid_input(msg_corrupted_jpeg());

      if (chunk_nr==m_next_chunk)
      {
          m_stream->write(chunk.get(), size);
          ++m_next_chunk;
      }
      else
      {
          m_free_chunks.push(chunk_rec_t(chunk_nr, chunk, size));
      }
  }



  std::auto_ptr<ISeqStreamInput> jpeg_icc_accu_t::icc_profile()
  {
      JAG_PRECONDITION(m_stream);
      if (static_cast<Long>(m_free_chunks.size()) != m_nr_chunks-(m_next_chunk-1))
          throw exception_invalid_input(msg_corrupted_jpeg());

      while(m_next_chunk <= m_nr_chunks)
      {
          chunk_rec_t const& rec(m_free_chunks.top());
          m_free_chunks.pop();
          if (m_next_chunk != rec.chunk_nr)
              throw exception_invalid_input(msg_corrupted_jpeg());

          m_stream->write(rec.chunk.get(), rec.size);
          ++m_next_chunk;
      }
      JAG_ASSERT(m_free_chunks.empty());

      std::auto_ptr<ISeqStreamInput> result(new MemoryStreamInputFromOutput(m_stream));
      m_stream.reset();
      return result;
  }


  jpeg_icc_accu_t::chunk_rec_t::chunk_rec_t(int chunk_nr_, shared_array<Byte> chunk_, int size_)
      : chunk_nr(chunk_nr_)
      , chunk(chunk_)
      , size(size_)
  {
  }


  bool jpeg_icc_accu_t::chunk_rec_t::operator<(jpeg_icc_accu_t::chunk_rec_t const& other) const
  {
      return chunk_nr < other.chunk_nr;
  }



} // namespace detail





////////////////////////////////////////////////////////////
// class ImageJPEG
////////////////////////////////////////////////////////////


const ImageJPEG::fn_marker ImageJPEG::s_markers[NUM_MARKERS] =
{
      &ImageJPEG::icc_marker
    , &ImageJPEG::exif_marker  // must go before jfif marker
    , &ImageJPEG::jfif_marker
};



template<boost::integer::endianness END>
struct exif_interop_t
{
    endian <END,uint_least16_t,16>  tag;
    endian <END,uint_least16_t,16>  type;
    Byte  count[4];
    Byte  val_offset[4];
};


/**
 * @brief Finishes exif processing with specified endianess.
 *
 * @param seek_offset   relative offset to the begining of the marker
 *                      (right after the size field)
 * @param size          size of the marker (without size field itself)
 */
template<boost::integer::endianness END>
void ImageJPEG::finish_exif_marker(int seek_offset, int size)
{
    typedef endian <END,int_least32_t, 32>  my_int32_t;
    typedef endian <END,uint_least32_t, 32>  my_uint32_t;
    typedef endian <END,int_least16_t, 16> my_int16_t;
    typedef endian <END,uint_least16_t, 16> my_uint16_t;

    // seek to the 0th IFD
    my_int32_t ifd_offset_raw;
    m_in_stream->read(&ifd_offset_raw, 4);
    seek_offset-=4;

    int ifd_offset = static_cast<Int>(ifd_offset_raw) - 8;
    if (ifd_offset)
    {
        m_in_stream->seek(ifd_offset, OFFSET_FROM_CURRENT);
        seek_offset -= ifd_offset;
    }

    // read the number of fields and iterate over them
    my_uint16_t nr_fields_raw;
    m_in_stream->read(&nr_fields_raw, 2);
    seek_offset-=2;
    int nr_fields = static_cast<UInt16>(nr_fields_raw);

    if (nr_fields)
    {
        enum {
              EXIF_DPIX = 1u
            , EXIF_DPIY = 1u<<1
            , EXIF_UNITS = 1u<<2
            , EXIF_COLOR_SPACE = 1u<<3
            // once all these are done then exit the loop
            , EXIF_ALL = EXIF_DPIX | EXIF_DPIY | EXIF_UNITS | EXIF_COLOR_SPACE
        };
        unsigned status = 0;

        // load all records at once
        typedef exif_interop_t<END> vec_item_t;
        std::vector<vec_item_t> interops(nr_fields);
        const int to_read = nr_fields*sizeof(vec_item_t);
        m_in_stream->read(&interops[0], to_read);
        seek_offset-=to_read;

        // relative offset to the tiff header start
        // compensation for 6 bytes following the marker size field ('Exif\x0\x0')
        const int tiff_start_offset = seek_offset + 5 + 1;

        // set default values
        int res_unit = 2;
        m_info.dpix  = 72.0;
        m_info.dpiy  = 72.0;

        // traverse over the records
        for (size_t i=0; i<interops.size() && status!=EXIF_ALL; ++i)
        {
            switch(interops[i].tag)
            {
            case 0x11a: // xres
            case 0x11b: // yres
            {
                int offset = tiff_start_offset + static_cast<Int>(*jag_reinterpret_cast<my_int32_t*>(&interops[i].val_offset));
                m_in_stream->seek(offset, OFFSET_FROM_CURRENT);
                my_uint32_t val[2]; // 0..numerator, 1..denominator
                m_in_stream->read(val, 2*sizeof(my_uint32_t));
                m_in_stream->seek(-static_cast<int>((offset+2*sizeof(my_uint32_t))), OFFSET_FROM_CURRENT);
                double dpi = double(static_cast<UInt>(val[0]))/static_cast<UInt>(val[1]);

                if (static_cast<UInt16>(interops[i].tag) == 0x11a)
                {
                    m_info.dpix = dpi;
                    status |= EXIF_DPIX;
                }
                else
                {
                    m_info.dpiy = dpi;
                    status |= EXIF_DPIY;
                }
                break;
            }

            case 0x128: // res unit
                res_unit = *jag_reinterpret_cast<my_int16_t*>(&interops[i].val_offset);
                status |= EXIF_UNITS;
                break;


            // -- EXIF specific IFD

            case 0x8769: // set of tags for recording Exif-specific attr. information
            {
                int offset = tiff_start_offset + static_cast<Int>(*jag_reinterpret_cast<my_int32_t*>(&interops[i].val_offset));
                m_in_stream->seek(offset, OFFSET_FROM_CURRENT); // + offset
                my_uint16_t nr_fields_raw = 0;
                m_in_stream->read(&nr_fields_raw, 2);  // +2
                int nr_fields = static_cast<UInt16>(nr_fields_raw);
                const int to_read = nr_fields*sizeof(vec_item_t);
                if (nr_fields)
                {
                    // append them to the existing interop array
                    const size_t orig_size = interops.size();
                    interops.resize(orig_size + nr_fields);
                    m_in_stream->read(&interops[orig_size], to_read); // +to_read
                }
                m_in_stream->seek(-offset-2-to_read, OFFSET_FROM_CURRENT);
                break;
            }


            case 0xa001: // color space
                m_info.exif_srgb = (static_cast<my_int16_t>(1)==*jag_reinterpret_cast<my_int16_t*>(&interops[i].val_offset));
                status |= EXIF_COLOR_SPACE;
                break;
            }
        }

        if (res_unit == 3)
        {
            // centimeters
            m_info.dpix *= 2.54;
            m_info.dpiy *= 2.54;
        }
    }


    m_in_stream->seek(seek_offset+size, OFFSET_FROM_CURRENT);
}



/// Processes marker containing EXIF.
bool ImageJPEG::exif_marker(Byte code, int size)
{
    if (code != 0xe1)
        return false;

    int seek_offset = check_marker_sig(*m_in_stream, "Exif", 5);
    if (!seek_offset)
        return false;

    Byte ignore;
    m_in_stream->read(&ignore, 1);
    seek_offset-=1;


    // select appropriate endian handler
    Byte header[4] = {0}; // first two byte order, the other can be ignored
    m_in_stream->read(&header, 4);
    seek_offset-=4;

    if (header[0] == 0x49 && header[1] == 0x49)
    {
        finish_exif_marker<boost::integer::little>(seek_offset, size);
    }
    else if (header[0] == 0x4d && header[1] == 0x4d)
    {
        finish_exif_marker<boost::integer::big>(seek_offset, size);
    }
    else
    {
        throw exception_invalid_input(msg_corrupted_jpeg());
    }

    return true;
}



/// Processes marker containing ICC profile.
bool ImageJPEG::icc_marker(Byte code, int size)
{
    if (code != 0xe2)
        return false;

    int seek_offset = check_marker_sig(*m_in_stream, "ICC_PROFILE", 12);
    if (!seek_offset)
        return false;

    Byte chunk_tag[2] = {0}; // 0..number of this chunk, 1..#chunks
    m_in_stream->read(chunk_tag, 2);
    seek_offset-=2;

    // read the chunk and send it to accu
    int chunk_size = size+seek_offset;
    shared_array<Byte> chunk_data(new Byte[chunk_size]);
    ULong read=0;
    m_in_stream->read(chunk_data.get(), chunk_size, &read);
    if (static_cast<int>(read) != chunk_size)
        throw exception_invalid_input(msg_corrupted_jpeg());

    if (!m_info.icc)
        m_info.icc.reset(new detail::jpeg_icc_accu_t(chunk_tag[1]));

    m_info.icc->add_chunk(chunk_tag[0], chunk_data, chunk_size);

    return true;
}



/// Processes marker containing JFIF information.
bool ImageJPEG::jfif_marker(Byte code, int size)
{
    if (code != 0xe0)
        return false;

    int seek_offset = check_marker_sig(*m_in_stream, "JFIF", 5);
    if (!seek_offset)
        return false;

    jfif_header_t header;
    m_in_stream->read(&header, sizeof(jfif_header_t));
    seek_offset -= sizeof(jfif_header_t);
    m_info.jfif = true;

    // resolution - set only if it has not been set yet
    if (equal_to_zero(m_info.dpix) && equal_to_zero(m_info.dpiy))
    {
        switch(header.units)
        {
        case 0:  // pixel aspect ratio
            // e.g. irfan interprets it as DPI
            break;

        case 1:  // dots per inch
            m_info.dpix = header.x_density;
            m_info.dpiy = header.y_density;
            break;

        case 2:  // dots per cm
            m_info.dpix = static_cast<UInt>(header.x_density) * 2.54;
            m_info.dpiy = static_cast<UInt>(header.y_density) * 2.54;
            break;

        default:
            JAG_ASSERT("unknown");
        }
    }

    seek_offset += size;
    JAG_ASSERT(seek_offset);
    m_in_stream->seek(seek_offset, OFFSET_FROM_CURRENT);

    return true;
}



/**
 * @brief Parses the jpeg stream and fills in some header info
 *
 * @param stream stream to parse
 * @param info   struct to fill in
 *
 * Current limitations:
 *   - exif not parsed (might contain dpi and color space)
 *   - line count in frame header is allowed to contain zero; in this case
 *     this value should be retrieved elsewhere - this is not handled
 */
void ImageJPEG::parse_jpeg()
{
    memset(&m_info, 0, sizeof(ImageJPEG::jpeg_info_t));

    Byte marker[2] = {0};

    m_in_stream->read(marker, 2);
    if (marker[0] != 0xff || marker[1] != 0xd8)
        throw exception_invalid_input(msg_corrupted_jpeg());

    while(1)
    {
        ULong read=0;
        marker[0] = marker[1] = 0;
        m_in_stream->read(marker, 2);
        if (marker[0]!=0xff || marker[1]==0)
            throw exception_invalid_input(msg_corrupted_jpeg());

        if (marker[1] == 0xd9) // EOI - end of image
            break;

        // is it start of frame?
        if (marker[1] != 0xc4 && // denotes DHT (Huffman table specification)
            marker[1]>=0xc0 && marker[1]<=0xcf)
        {
            switch(marker[1])  // is it a progressive frame?
            {
            case 0xc2:
            case 0xc6:
            case 0xca:
            case 0xce:
                m_info.progressive = true;
            }

            // read frame header
            frame_header_t fheader;
            ULong read = 0;
            m_in_stream->read(&fheader, sizeof(frame_header_t), &read);
            if (read!=sizeof(frame_header_t))
                throw exception_invalid_input(msg_corrupted_jpeg());

            m_info.width = fheader.x;
            m_info.height = fheader.y;
            m_info.bits_per_component = fheader.bpc;
            m_info.nr_components = fheader.nr_components;
            break;
        }

        // test whether the marker isn't standalone
        if (!((marker[1]>=0xd0 && marker[1]<=0xd7)   // RSTm
               || marker[1]==0x01))                    // TEM
        {
            ubig16_t size_w=0;
            m_in_stream->read(&size_w, 2, &read);
            if (read != 2)
                throw exception_invalid_input(msg_corrupted_jpeg());

            int size = static_cast<UInt16>(size_w) - 2;

            // ?application specific
            if (marker[1]>=0xe0 && marker[1]<=0xef)
            {
                int i=0;
                for(;i<NUM_MARKERS; ++i)
                {
                    if ((this->*s_markers[i])(marker[1], size))
                    {
                        size = 0;
                        break;
                    }
                }
            }

            if (size)
                m_in_stream->seek(size, OFFSET_FROM_CURRENT);
        }
    }
}




bool ImageJPEG::ping(IStreamInput& in_stream)
{
    in_stream.seek(0, OFFSET_FROM_BEGINNING);
    Byte header[3]={0};
    in_stream.read(header, 3);
    return !memcmp(jpeg_header_c, header, 3);
}



ImageJPEG::ImageJPEG(
      IImageData const* img_spec
    , shared_ptr<IResourceCtx> res_ctx
)
    : m_in_stream(img_spec->data_stream())
    , m_res_ctx(res_ctx)

{
    JAG_PRECONDITION(img_spec->format() == IMAGE_FORMAT_JPEG);

    if (!ImageJPEG::ping(*m_in_stream))
        throw exception_invalid_input(msg_not_jpeg_format()) << JAGLOC;

    m_in_stream->seek(0, OFFSET_FROM_BEGINNING);
    parse_jpeg();

    prepare_color_space();

    // temporarily - JPEG file is allowed to have this set to 0
    // and the height can be retrived when the file is parsed
    // more thoroughly than we do now
    if (!m_info.height)
    {
        JAG_TBD;
        throw exception_invalid_input(msg_not_jpeg_format()) << JAGLOC;
    }
}



/// Prepares the color space according to information retrieved from JPEG.
void ImageJPEG::prepare_color_space()
{
    boost::shared_ptr<IResourceCtx> res_ctx(m_res_ctx);

    // register icc profile (if any was found)

    // Do not output icc profile when the jpeg has only one color component (#137)
    if (m_info.icc && m_info.nr_components != 1)
    {
        intrusive_ptr<IICCBased> spec(
            res_ctx->color_space_man()->define_iccbased());

        spec->num_components(m_info.nr_components);
        spec->icc_profile_stream(m_info.icc->icc_profile());
        m_info.cs_handle = res_ctx->color_space_man()->color_space_load(spec);
    }
    else if (m_info.exif_srgb)
    {
        // Exif specifies sRGB
        intrusive_ptr<IICCBased> spec(
            res_ctx->color_space_man()->define_iccbased());

        spec->num_components(3);

        std::auto_ptr<ISeqStreamInput> mem_stream(
            create_stream_from_memory(binres::icc_srgb, binres::icc_srgb_size));

        spec->icc_profile_stream(mem_stream);
        m_info.cs_handle = res_ctx->color_space_man()->color_space_load(spec);
    }

    // CMYK needs a decode array to invert values
    if (m_info.nr_components == 4)
        m_decode.reset(DecodeArray(cmyk_decode, 8));
}




bool ImageJPEG::output_image(ISeqStreamOutput& dest, unsigned max_bits) const
{
    // sending the whole jpeg stream
    if (max_bits < (unsigned)m_info.bits_per_component)
        return false;

    m_in_stream->seek(0, OFFSET_FROM_BEGINNING);
    copy_stream(*m_in_stream, dest);
    return true;
}



DecodeArray const* ImageJPEG::decode() const
{
    return m_decode.size() ? &m_decode : 0;
}



ColorSpaceHandle ImageJPEG::color_space() const
{
    if (is_valid(m_info.cs_handle))
        return m_info.cs_handle;

    switch(m_info.nr_components)
    {
    case 1:
        return ColorSpaceHandle(CS_DEVICE_GRAY);

    case 3:
        return ColorSpaceHandle(CS_DEVICE_RGB);

    case 4:
        return ColorSpaceHandle(CS_DEVICE_CMYK);

    default:
        return ColorSpaceHandle();
    }
}



}} // namespace jag::resources

/** EOF @file */
