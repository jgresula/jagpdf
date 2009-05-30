// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/othermanagers/colorspacemanimpl.h>
#include <resources/othermanagers/colorspacesimpl.h>
#include <resources/resourcebox/binresources.h>
#include <core/generic/refcountedimpl.h>
#include <core/jstd/streamhelpers.h>
#include <core/jstd/optionsparser.h>
#include <boost/intrusive_ptr.hpp>
#include <msg_resources.h>
#include <core/errlib/errlib.h>

// spirit
#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4244)
#endif
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_symbols.hpp>
#include <boost/spirit/include/classic_exceptions.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#if defined(_MSC_VER)
# pragma warning(pop)
#endif

using namespace boost::spirit::classic;
using namespace boost;
using namespace jag::jstd;

namespace jag {
namespace resources {

namespace
{

  /// This operation should be applied before looking up in the resource table
  ColorSpaceHandle& unmask_handle(ColorSpaceHandle csh)
  {
      return csh.rshift(CS_COLOR_SPACE_ID_START_BIT);
  }

  enum cs_name_t {
      CSN_CALGRAY, CSN_CALRGB, CSN_CIELAB, CSN_ICC, CSN_SRGB, CSN_ADOBE_RGB,
      CSN_DEVICE_RGB, CSN_DEVICE_GRAY, CSN_DEVICE_CMYK, CSN_BYID};

  enum cs_kwd_t {
      CSK_PROFILE, CSK_PALETTE, CSK_WHITE, CSK_BLACK, CSK_RANGE,
      CSK_GAMMA, CSK_MATRIX, CSK_COMPONENTS, CSK_ID};


  //
  //
  //
  template <class PT, class T>
  void init_vector(ParsedResult const&p, unsigned kwd, std::vector<T>& vec, int expect_size=-1)
  {
      int nr_values = p.num_values(kwd);

      if (!nr_values)
          return;

      if (expect_size!=-1 && nr_values!=expect_size)
          throw exception_invalid_value(msg_invalid_cs_spec()) << JAGLOC;

      vec.resize(nr_values);
      for(int i=nr_values; i--;)
          vec[i]=static_cast<T>(p.to_<PT>(kwd, 0, i));
  }

  //
  //
  //
  struct cs_str_spec_t
      : noncopyable
  {
      cs_str_spec_t(unsigned csname, ParsedResult const& pres);
      int                 components;
      string_32_cow       icc_profile;
      ColorSpace          id;
      std::vector<double> white;
      std::vector<double> black;
      std::vector<double> range;
      std::vector<double> gamma;
      std::vector<double> matrix;
      std::vector<Byte>   palette;
  };


  //
  //
  //
  cs_str_spec_t::cs_str_spec_t(unsigned csname, ParsedResult const& pres)
      : components(pres.to_<int>(CSK_COMPONENTS, -1))
      , icc_profile(pres.to_<string_32_cow>(CSK_PROFILE))
      , id(static_cast<ColorSpace>(pres.to_<int>(CSK_ID,0)))

  {
      init_vector<double>(pres, CSK_WHITE, white, 2);
      init_vector<double>(pres, CSK_BLACK, black, 3);
      init_vector<double>(pres, CSK_RANGE, range, 4);
      init_vector<double>(pres, CSK_GAMMA, gamma, csname==CSN_CALGRAY ? 1 : 3);
      init_vector<double>(pres, CSK_MATRIX, matrix, 9);
      init_vector<int>(pres, CSK_PALETTE, palette);
  }


  //
  //
  //
  void set_cie_base(cs_str_spec_t const& spec, ICIELabBase& cs)
  {
      if (2 == spec.white.size())
          cs.white_point(spec.white[0], spec.white[1]);

      if (3 == spec.black.size())
          cs.black_point(spec.black[0], spec.black[1], spec.black[2]);
  }


  struct color_space_names_t
      : public symbols<unsigned>
  {
      color_space_names_t()
      {
          add
              ("calgray", CSN_CALGRAY)
              ("calrgb", CSN_CALRGB)
              ("cielab", CSN_CIELAB)
              ("icc", CSN_ICC)
              ("srgb", CSN_SRGB)
              ("adobe-rgb", CSN_ADOBE_RGB)
              ("rgb", CSN_DEVICE_RGB)
              ("gray", CSN_DEVICE_GRAY)
              ("cmyk", CSN_DEVICE_CMYK)
              ("by-id", CSN_BYID)
              ;
      }
  } g_cs_names;

  struct color_space_kwds_t
      : public symbols<unsigned>
  {
      color_space_kwds_t()
      {
          add
              ("profile", CSK_PROFILE)
              ("palette", CSK_PALETTE)
              ("white", CSK_WHITE)
              ("black", CSK_BLACK)
              ("range", CSK_RANGE)
              ("gamma", CSK_GAMMA)
              ("matrix", CSK_MATRIX)
              ("components", CSK_COMPONENTS)
              ("id", CSK_ID)
              ;
      }
  } g_cs_kwds;

} // anonymous namespace



ColorSpaceManImpl::ColorSpaceManImpl()
{
}



ColorSpaceHandle ColorSpaceManImpl::register_palette(
      ColorSpaceHandle cs
    , Byte const* palette
    , UInt byte_size
)
{
    intrusive_ptr<IPalette> indexed_cs(new RefCountImpl<PaletteImpl>);
    indexed_cs->set(id_from_handle<ColorSpace>(cs), palette, byte_size);
    return color_space_load(indexed_cs);
}


//
//
//
ColorSpaceHandle ColorSpaceManImpl::register_icc_from_memory(Byte const* mem,
                                                             size_t size,
                                                             int nr_components)
{
    JAG_PRECONDITION(mem && nr_components && size);
    intrusive_ptr<IICCBased> spec(define_iccbased());
    spec->num_components(nr_components);
    std::auto_ptr<ISeqStreamInput> mem_stream(
        create_stream_from_memory(mem, size));

    spec->icc_profile_stream(mem_stream);
    return color_space_load(spec);
}





//
//      *  - type ... calgray, calrgb, cielab, icc, [palette]
//      *  - white   = x, z
//      *  - black   = x, y, z
//      *  - range   = amin, amax, bmin, bmax (cielab)
//      *  - gamma   = val [,val, val]        (calgray [,calrgb])
//      *  - matrix  = 9 vals                 (calrgb)
//      *  - profile = file-path              (icc based)
//      *  - name    = sRGB  .. some predefined name
IColorSpaceMan::cs_handle_pair_t
ColorSpaceManImpl::color_space_load(Char const* spec_str)
{
    ParseArgs pargs(&g_cs_kwds, &g_cs_names);
    ParsedResult pres(parse_options(spec_str, pargs));

    unsigned name = pres.explicit_value();
    if (name == ParsedResult::NO_EXPL_VALUE)
        throw exception_invalid_value(msg_invalid_cs_spec()) << JAGLOC;

    cs_str_spec_t spec(name, pres);
    cs_handle_pair_t result;

    switch (name)
    {
    case CSN_SRGB:
        result.first = register_icc_from_memory(binres::icc_srgb,
                                                binres::icc_srgb_size,
                                                3);
        break;

    case CSN_ADOBE_RGB:
        result.first = register_icc_from_memory(binres::icc_adobe_rgb,
                                                binres::icc_adobe_rgb_size,
                                                3);
        break;

    case CSN_BYID:
        if (!spec.id)
            throw exception_invalid_value(msg_invalid_cs_spec()) << JAGLOC;

        result.first = ColorSpaceHandle(
            handle_from_id<RESOURCE_COLOR_SPACE>(spec.id));
        break;

    case CSN_DEVICE_RGB:
        result.first = ColorSpaceHandle(CS_DEVICE_RGB);
        break;

    case CSN_DEVICE_GRAY:
        result.first = ColorSpaceHandle(CS_DEVICE_GRAY);
        break;

    case CSN_DEVICE_CMYK:
        result.first = ColorSpaceHandle(CS_DEVICE_CMYK);
        break;

    case CSN_CALGRAY:
    {
        intrusive_ptr<ICIECalGray> gray(define_calgray());
        set_cie_base(spec, *gray);
        if (!spec.gamma.empty())
            gray->gamma(spec.gamma[0]);

        result.first = color_space_load(gray);
        break;
    }

    case CSN_CALRGB:
    {
        intrusive_ptr<ICIECalRGB> rgb(define_calrgb());
        set_cie_base(spec, *rgb);
        if (!spec.gamma.empty())
        {
            JAG_ASSERT(spec.gamma.size()==3);
            rgb->gamma(spec.gamma[0], spec.gamma[1], spec.gamma[2]);
        }
        if (!spec.matrix.empty())
        {
            JAG_ASSERT(spec.matrix.size()==9);
            rgb->matrix(spec.matrix[0], spec.matrix[1], spec.matrix[2],
                        spec.matrix[3], spec.matrix[4], spec.matrix[5],
                        spec.matrix[6], spec.matrix[7], spec.matrix[8]);
        }
        result.first = color_space_load(rgb);
        break;
    }

    case CSN_CIELAB:
    {
        intrusive_ptr<ICIELab> lab(define_cielab());
        set_cie_base(spec, *lab);

        if (!spec.range.empty())
        {
            JAG_ASSERT(4 == spec.range.size());
            lab->range(spec.range[0], spec.range[1], spec.range[2], spec.range[3]);
        }
        result.first = color_space_load(lab);
        break;
    }

    case CSN_ICC:
    {
        if (-1==spec.components || spec.icc_profile.empty())
            throw exception_invalid_value(msg_invalid_cs_spec()) << JAGLOC;

        intrusive_ptr<IICCBased> icc(define_iccbased());
        icc->num_components(spec.components);
        icc->icc_profile(spec.icc_profile.c_str());
        result.first = color_space_load(icc);
        break;
    }


    default:
        ;
    } // switch


    JAG_ASSERT(is_valid(result.first));

    // is it palette?
    if (!spec.palette.empty())
    {
        intrusive_ptr<IPalette> indexed(define_indexed());
        indexed->set(
            id_from_handle<ColorSpace>(result.first),
            &spec.palette[0],
            static_cast<UInt>(spec.palette.size()));
        result.second = result.first;
        result.first = color_space_load(indexed);
    }

    return result;
}



intrusive_ptr<IPalette> ColorSpaceManImpl::define_indexed() const
{
    return intrusive_ptr<IPalette>(new RefCountImpl<PaletteImpl>);
}



intrusive_ptr<ICIELab> ColorSpaceManImpl::define_cielab() const
{
    return intrusive_ptr<ICIELab>(new RefCountImpl<CIELabImpl>);
}



intrusive_ptr<ICIECalGray> ColorSpaceManImpl::define_calgray() const
{
    return intrusive_ptr<ICIECalGray>(new RefCountImpl<CIECalGrayImpl>);
}



intrusive_ptr<ICIECalRGB> ColorSpaceManImpl::define_calrgb() const
{
    return intrusive_ptr<ICIECalRGB>(new RefCountImpl<CIECalRGBImpl>);
}


intrusive_ptr<IICCBased> ColorSpaceManImpl::define_iccbased() const
{
    return intrusive_ptr<IICCBased>(new RefCountImpl<ICCBasedImpl>);
}



intrusive_ptr<IColorSpace> const&
ColorSpaceManImpl::color_space(ColorSpaceHandle cs) const
{
    return m_cs_table.lookup(unmask_handle(cs));
}



ColorSpaceHandle
ColorSpaceManImpl::color_space_load(intrusive_ptr<IColorSpace> cs)
{
    cs->check_validity();

    ColorSpaceHandle handle(m_cs_table.add(cs->clone()));
    return handle
        .lshift(CS_COLOR_SPACE_ID_START_BIT)
        .bitwise_or(cs->color_space_type())
    ;
}


int ColorSpaceManImpl::num_components(ColorSpaceHandle csh) const
{
    return m_cs_table.lookup(unmask_handle(csh))->num_components();
}



}} // namespace jag::resources
