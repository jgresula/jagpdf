// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __GILEXT_JG00_H__
#define __GILEXT_JG00_H__

#include <core/generic/internal_types.h>
#include <interfaces/streams.h>

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4100)
#endif

   #include <boost/gil/gil_all.hpp>

#if defined(_MSC_VER)
# pragma warning(pop)
#endif

#include <boost/integer/endian.hpp>


// channel traits for ubig16_t
namespace boost {
namespace gil
{
  template <>
  struct channel_traits<boost::integer::ubig16_t> {
      typedef boost::integer::ubig16_t          value_type;
      typedef boost::integer::ubig16_t&         reference;
      typedef boost::integer::ubig16_t*         pointer;
      typedef boost::integer::ubig16_t  const&  const_reference;
      typedef boost::integer::ubig16_t  const*  const_pointer;
      static const bool is_mutable = true;

      static value_type min_value() { return std::numeric_limits<jag::UInt16>::min(); }
      static value_type max_value() { return std::numeric_limits<jag::UInt16>::max(); }
  };
}} //boost::gil




namespace jag {
namespace resources {

/// Sends content of a view to a stream.
template<class View>
void output_view(ISeqStreamOutput& dest, View const& view)
{
    typedef typename View::value_type pixel_t;
    const int buffer_size = 1024/sizeof(pixel_t);
    pixel_t buffer[buffer_size];

    pixel_t* curr = buffer;
    pixel_t const*const end = buffer + buffer_size;

    for (typename View::iterator it = view.begin(); it<view.end(); ++it)
    {
        if (curr == end)
        {
            dest.write(buffer, buffer_size*sizeof(pixel_t));
            curr = buffer;
        }
        *curr++ = *it;
    }

    Long rest = (curr-buffer)*sizeof(pixel_t);
    JAG_ASSERT(rest);
    dest.write(buffer, rest);
}


// big-endian pixels
typedef boost::gil::pixel<
    boost::integer::ubig16_t
  , boost::gil::gray_layout_t>       gray_big16_pixel_t;
typedef gray_big16_pixel_t*          gray_big16_pixel_ptr_t;
typedef gray_big16_pixel_t const*    gray_big16_pixel_cptr_t;


typedef boost::gil::pixel<
    boost::integer::ubig16_t
  , boost::gil::devicen_layout_t<2> > graya_big16_pixel_t;
typedef graya_big16_pixel_t*         graya_big16_pixel_ptr_t;
typedef graya_big16_pixel_t const*   graya_big16_pixel_cptr_t;

typedef boost::gil::pixel<
    boost::integer::ubig16_t
  , boost::gil::rgb_layout_t>        rgb_big16_pixel_t;
typedef rgb_big16_pixel_t*           rgb_big16_pixel_ptr_t;
typedef rgb_big16_pixel_t const*     rgb_big16_pixel_cptr_t;

typedef boost::gil::pixel<
    boost::integer::ubig16_t
  , boost::gil::rgba_layout_t>       rgba_big16_pixel_t;
typedef rgba_big16_pixel_t*          rgba_big16_pixel_ptr_t;
typedef rgba_big16_pixel_t const*    rgba_big16_pixel_cptr_t;


#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4244)
#endif

namespace detail
{
  // converters
  template <typename SrcColorSpace, typename DstColorSpace>
  struct my_color_converter_impl {};


  template<>
  struct my_color_converter_impl<gray_big16_pixel_t,boost::gil::gray8_pixel_t>
  {
      void operator()(gray_big16_pixel_t const& src, boost::gil::gray8_pixel_t& dst) const
      {
          dst[0] = static_cast<UInt16>(src[0])>>8;
      }
  };


  template<>
  struct my_color_converter_impl<rgb_big16_pixel_t,boost::gil::rgb8_pixel_t>
  {
      void operator()(rgb_big16_pixel_t const& src, boost::gil::rgb8_pixel_t& dst) const
      {
          dst[0] = static_cast<UInt16>(src[0])>>8;
          dst[1] = static_cast<UInt16>(src[1])>>8;
          dst[2] = static_cast<UInt16>(src[2])>>8;
      }
  };

  template<>
  struct my_color_converter_impl<rgba_big16_pixel_t,boost::gil::rgba8_pixel_t>
  {
      void operator()(rgba_big16_pixel_t const& src, boost::gil::rgba8_pixel_t& dst) const
      {
          dst[0] = static_cast<UInt16>(src[0])>>8;
          dst[1] = static_cast<UInt16>(src[1])>>8;
          dst[2] = static_cast<UInt16>(src[2])>>8;
          dst[3] = static_cast<UInt16>(src[3])>>8;
      }
  };

  template<>
  struct my_color_converter_impl<rgba_big16_pixel_t,boost::gil::rgb8_pixel_t>
  {
      void operator()(rgba_big16_pixel_t const& src, boost::gil::rgb8_pixel_t& dst) const
      {
          dst[0] = static_cast<UInt16>(src[0])>>8;
          dst[1] = static_cast<UInt16>(src[1])>>8;
          dst[2] = static_cast<UInt16>(src[2])>>8;
      }
  };

  template<>
  struct my_color_converter_impl<rgba_big16_pixel_t,rgb_big16_pixel_t>
  {
      void operator()(rgba_big16_pixel_t const& src, rgb_big16_pixel_t& dst) const
      {
          dst[0] = src[0];
          dst[1] = src[1];
          dst[2] = src[2];
      }
  };


  template<>
  struct my_color_converter_impl<boost::gil::rgba8_pixel_t,boost::gil::rgb8_pixel_t>
  {
      void operator()(boost::gil::rgba8_pixel_t const& src, boost::gil::rgb8_pixel_t& dst) const
      {
          dst[0] = src[0];
          dst[1] = src[1];
          dst[2] = src[2];
      }
  };


  template<>
  struct my_color_converter_impl<graya_big16_pixel_t,boost::gil::dev2n8_pixel_t>
  {
      void operator()(graya_big16_pixel_t const& src, boost::gil::dev2n8_pixel_t& dst) const
      {
          dst[0] = static_cast<UInt16>(src[0])>>8;
          dst[1] = static_cast<UInt16>(src[1])>>8;
      }
  };

} //detail


#if defined(_MSC_VER)
# pragma warning(pop)
#endif



/**
 * @brief Custom color converter.
 *
 * All cs-with-alpha to cs-without-alpha conversions drop it,
 * no premultiplying is performed
 */
struct my_color_converter {
    template <typename SrcP, typename DstP>  // Model PixelConcept
    void operator()(const SrcP& src,DstP& dst) const {
        detail::my_color_converter_impl<SrcP,DstP>()(src,dst);
    }
};



/// image data info
struct img_data_dict_t
{
    img_data_dict_t(UInt width_, UInt height_, void const* data_, UInt rowbytes_, UInt bpc_)
        : width(width_)
        , height(height_)
        , data(data_)
        , rowbytes(rowbytes_)
        , bpc(bpc_)
    {}

    UInt      width;
    UInt      height;
    void const* data;
    UInt      rowbytes;
    UInt      bpc;
};


/**
 * @brief Outputs a channel from an interleaved view to an output stream.
 *
 * The view can have either 16 or 8 bits per channel.
 *
 * @param Pix16 model of pixel concept for 16-bit depth
 * @param Pix8  model of pixel concept for 8-bit depth
 * @param CC    color converter
 *
 * @param dest         output stream
 * @param idict        image dictionary
 * @param channel      zero-based index of channel to output
 * @param convert16to8 whether to convert 16-bit channel to 8-bits
 */
template<class Pix16, class Pix8,class CC=my_color_converter>
struct output_channel
{
    static void apply(ISeqStreamOutput& dest, img_data_dict_t const& idict, int channel, bool convert16to8)
    {
        if (convert16to8)
        {
            JAG_ASSERT(idict.bpc==16);
            output_view(
                dest, boost::gil::nth_channel_view(
                    boost::gil::color_converted_view<Pix8>(
                        boost::gil::interleaved_view(
                            idict.width
                            , idict.height
                            , static_cast<Pix16 const*>(idict.data)
                            , idict.rowbytes)
                        , CC()
                       )
                    ,channel)
               );
        }
        else
        {
            if (8==idict.bpc)
            {
                //8-bit depth
                output_view(
                    dest, boost::gil::nth_channel_view(
                        boost::gil::interleaved_view(
                            idict.width
                            , idict.height
                            , static_cast<Pix8 const*>(idict.data)
                            , idict.rowbytes)
                        , channel)
                   );
            }
            else
            {
                JAG_ASSERT(16== idict.bpc);
                // 16-bit depth
                output_view(
                    dest
                    , boost::gil::nth_channel_view(
                        boost::gil::interleaved_view(
                            idict.width
                            , idict.height
                            , static_cast<Pix16 const*>(idict.data)
                            , idict.rowbytes)
                        ,channel)
                   );
            }
        }
    }
};




// template<class Pix16, class Pix8,class CC=my_color_converter>
// struct output_converted_view
// {
//     static void apply(ISeqStreamOutput& dest, img_data_dict_t const& idict, int channel, bool convert16to8)
//     {
//     }
// };


}} // namespace jag

#endif // __GILEXT_JG00_H__
/** EOF @file */
