// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//



#include "precompiled.h"
#include "colorspace.h"
#include "indirectobjectcallback.h"
#include "objfmt.h"
#include "docwriterimpl.h"
#include "contentstream.h"
#include "resourcemanagement.h"
#include "genericcontentstream.h"
#include <core/generic/smartptrutils.h>
#include <core/generic/checked_cast.h>
#include <core/generic/refcountedimpl.h>
#include <core/jstd/streamhelpers.h>
#include <resources/resourcebox/colorspacehelpers.h>
#include <resources/othermanagers/colorspacesimpl.h>
#include <resources/interfaces/resourcectx.h>
#include <resources/interfaces/colorspaces.h>
#include <resources/interfaces/colorspaceman.h>
#include <boost/intrusive_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace boost;
using namespace jag::resources;
using namespace jag::jstd;

namespace jag {
namespace pdf {


namespace
{
  /**
   * @brief Outputs common base of CIE based color spaces
   *
   * @param obj cie color space base
   * @param fmt formatter to use
   */
  void output_cie_base(CIEBase const& obj, ObjFmt& fmt)
  {
      CIEBase::white_point_ref_t white = obj.white_point();
      fmt
          .dict_key("WhitePoint")
          .array_start()
          .output(white[0])
          .raw_bytes(" 1.0 ", 5)
          .output(white[1])
          .array_end()
      ;

      if (obj.is_black_point_valid())
      {
          CIEBase::black_point_ref_t black = obj.black_point();
          fmt
              .dict_key("BlackPoint")
              .array_start()
              .output(black[0]).space()
              .output(black[1]).space()
              .output(black[2])
              .array_end()
              ;
      }
  }

} // anonymous namespace


//
//
//
void output_trivial_color_space_name(ColorSpaceHandle handle, ObjFmtBasic& fmt)
{
    JAG_PRECONDITION(is_trivial_color_space(handle));

    char const* cs = 0;
    switch(color_space_type(handle))
    {
    case CS_DEVICE_CMYK:
        cs = "DeviceCMYK";
        break;

    case CS_DEVICE_RGB:
          cs = "DeviceRGB";
          break;

    case CS_DEVICE_GRAY:
        cs = "DeviceGray";
        break;

    case CS_PATTERN:
        cs = "Pattern";
        break;

    default:
        JAG_INTERNAL_ERROR;
    }

    fmt.output(cs);
}



//
//
//
void output_color_space_name(ColorSpaceHandle handle, ObjFmtBasic& fmt)
{
    JAG_PRECONDITION(is_valid(handle));

    if (is_trivial_color_space(handle))
    {
        output_trivial_color_space_name(handle, fmt);
    }
    else
    {
        fmt.output_resource(handle);
    }
}

//
//
//
void output_color_space_ref(DocWriterImpl& doc, ColorSpaceHandle csh)
{
    if (is_trivial_color_space(csh))
    {
        output_trivial_color_space_name(csh,
                                        doc.object_writer().fmt_basic());
    }
    else
    {
        doc.object_writer().ref(doc.res_mgm().color_space_ref(csh));
    }
}


///////////////////////////////////////////////////
// class ColorSpaceObj
///////////////////////////////////////////////////

ColorSpaceObj::ColorSpaceObj(DocWriterImpl& doc, ColorSpaceHandle cs_handle)
    : m_doc(doc)
{
    JAG_PRECONDITION_MSG(!is_trivial_color_space(cs_handle)
                          , "trivial color spaces are never output as indirect objects");

    intrusive_ptr<IColorSpace> color_space(
        m_doc.resource_ctx().color_space_man()->color_space(cs_handle));
    color_space->accept(*this);
}



/// Generic visit() implementation.
template<class Impl, class Callback>
void ColorSpaceObj::visit_internal(Impl& impl, Callback callback_)
{
    IndirectObjectCallback::output_callback_t outfn(
        bind(callback_, this, ref(impl), _1));

    m_iobj_callback.reset(
        new RefCountImpl<IndirectObjectCallback>(m_doc, outfn));

    reset_indirect_object_worker(m_iobj_callback);
}



/// Generic visit() implementation with on_before callback.
template<class Impl, class CallbackBefore, class Callback>
void ColorSpaceObj::visit_internal(Impl& impl, CallbackBefore bcallback, Callback callback)
{
    IndirectObjectCallback::output_callback_t outfn(
        bind(callback, this, ref(impl), _1));
    IndirectObjectCallback::before_callback_t beforefn(
        bind(bcallback, this, ref(impl), _1));

    m_iobj_callback.reset(
        new RefCountImpl<IndirectObjectCallback>(
            m_doc, outfn, beforefn));

    reset_indirect_object_worker(m_iobj_callback);
}




/**
 * @brief Outputs a non-trivial color space definition
 *
 * @param csh color space (no restrictions)
 */
void ColorSpaceObj::prepare_inferior_cs(ColorSpaceHandle csh)
{
    // already done?
    if (is_valid(m_inferior_cs))
        return;

    if (!is_trivial_color_space(csh))
        m_inferior_cs = m_doc.res_mgm().color_space_ref(csh);
}



/**
 * @brief Outputs a reference to an inferior color space
 *
 * @param csh   inferior color space
 * @param fmt   formatter to use
 */
void ColorSpaceObj::output_inferior_cs_ref(ColorSpaceHandle csh, ObjFmt& fmt)
{
    JAG_ASSERT_MSG(
           is_trivial_color_space(csh)
        || is_valid(m_inferior_cs)
        ,  "non-trivial color spaces has not been ouputted"
   );
    output_color_space_name(csh, fmt.fmt_basic());
}



////////////////////////////////////////////
// Patterns

void ColorSpaceObj::visit(PatternColorSpace& obj)
{
    visit_internal(obj, &ColorSpaceObj::output_pattern);
}



void ColorSpaceObj::output_pattern(PatternColorSpace& obj, ObjFmt& fmt)
{
    fmt
        .array_start()
        .output("Pattern")
    ;
    output_color_space_name(obj.cs_handle(), fmt.fmt_basic());
    fmt.array_end();
}



////////////////////////////////////////////
// CIE-based


void ColorSpaceObj::visit(CIELabImpl& obj)
{
    visit_internal(obj, &ColorSpaceObj::output_cielab);
}



void ColorSpaceObj::visit(CIECalRGBImpl& obj)
{
    visit_internal(obj, &ColorSpaceObj::output_calrgb);
}



void ColorSpaceObj::visit(CIECalGrayImpl& obj)
{
    visit_internal(obj, &ColorSpaceObj::output_calgray);
}



void ColorSpaceObj::output_calrgb(resources::CIECalRGBImpl& obj, ObjFmt& fmt)
{
    fmt.array_start().output("CalRGB").dict_start();
    output_cie_base(obj.cie_base(), fmt);

    if (obj.is_gamma_valid())
    {
        CIECalRGBImpl::gamma_ref_t gamma = obj.gamma();
        fmt.dict_key("Gamma");
        output_array(&gamma[0], gamma.size(), fmt.fmt_basic());
    }

    if (obj.is_matrix_valid())
    {
        CIECalRGBImpl::matrix_ref_t matrix = obj.matrix();
        fmt.dict_key("Matrix");
        output_array(&matrix[0], matrix.size(), fmt.fmt_basic());
    }

    fmt.dict_end().array_end();

}



void ColorSpaceObj::output_calgray(resources::CIECalGrayImpl& obj, ObjFmt& fmt)
{
    fmt.array_start().output("CalGray").dict_start();
    output_cie_base(obj.cie_base(), fmt);

    if (obj.is_gamma_valid())
        fmt.dict_key("Gamma").space().output(obj.gamma());

    fmt.dict_end().array_end();
}



void ColorSpaceObj::output_cielab(CIELabImpl& obj, ObjFmt& fmt)
{
    fmt.array_start().output("Lab").dict_start();
    output_cie_base(obj.cie_base(), fmt);

    if (obj.is_range_valid())
    {
        CIELabImpl::range_ref_t range =  obj.range();
        fmt.dict_key("Range");
        output_array(&range[0], range.size(), fmt.fmt_basic());
    }

    fmt.dict_end().array_end();
}



////////////////////////////////////////////
// ICC-based

void ColorSpaceObj::visit(resources::ICCBasedImpl& obj)
{
    visit_internal(
        obj
        , &ColorSpaceObj::before_output_iccbased
        , &ColorSpaceObj::output_iccbased);
}



void ColorSpaceObj::output_iccbased(resources::ICCBasedImpl& /*obj*/, ObjFmt& fmt)
{
    JAG_ASSERT(is_valid(m_ref));
    fmt
        .array_start()
        .output("ICCBased")
        .space()
        .ref(m_ref)
        .array_end()
    ;
}



bool ColorSpaceObj::before_output_iccbased(resources::ICCBasedImpl& obj,
                                           ObjFmt& /*fmt*/)
{
    // this method might be called multiple times, so check if we have not been
    // here
    if (is_valid(m_ref))
        return true;

    // output the stream with icc profile
    GenericContentStream cstream(
        m_doc
        , bind(&ColorSpaceObj::output_iccbased_dict, this, ref(obj), _1)
   );

    copy_stream(*obj.icc_profile(), cstream.out_stream());

    m_ref = IndirectObjectRef(cstream);
    cstream.output_definition();

    // output alternate color space (if any)
    ColorSpaceHandle alternate(obj.alternate());
    if (is_valid(alternate))
        prepare_inferior_cs(alternate);

    return true;
}



void ColorSpaceObj::output_iccbased_dict(resources::ICCBasedImpl& obj, ObjFmt& fmt)
{
    // callback - enriches stream's dict by iccbased color space specifics
    fmt.dict_key("N").space().output(obj.num_components());
    ColorSpaceHandle alternate(obj.alternate());
    if (is_valid(alternate))
    {
        fmt.dict_key("Alternate");
        output_inferior_cs_ref(alternate, fmt);
    }
}




////////////////////////////////////////////
// Indexed


void ColorSpaceObj::visit(PaletteImpl& obj)
{
    visit_internal(obj, &ColorSpaceObj::before_output_indexed, &ColorSpaceObj::output_indexed);
}



void ColorSpaceObj::output_indexed(PaletteImpl& obj, ObjFmt& fmt)
{
    fmt
        .array_start()
        .raw_bytes("/Indexed", 8)
    ;



    ColorSpaceHandle cs_handle(obj.color_space());
    if (is_trivial_color_space(cs_handle))
    {
        JAG_ASSERT(!is_valid(m_inferior_cs));
        output_trivial_color_space_name(cs_handle, fmt.fmt_basic());
    }
    else
    {
        JAG_ASSERT(is_valid(m_inferior_cs));
        fmt.space().ref(m_inferior_cs);
    }

    int bytes_per_color = num_components(cs_handle, m_doc.resource_ctx().color_space_man().get());

    Byte const* data = obj.data();
    size_t data_len = obj.length();

    fmt
        .space()
        .output(static_cast<UInt>((data_len-1)/bytes_per_color))
        .space()
        .text_string_hex(jag_reinterpret_cast<Char const*>(data), data_len)
        .array_end()
    ;

    obj.drop_data();
}



bool ColorSpaceObj::before_output_indexed(PaletteImpl& obj, ObjFmt& /*fmt*/)
{
    prepare_inferior_cs(obj.color_space());
    return true;
}


}} //namespace jag::pdf



















