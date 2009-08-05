// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "patternimpl.h"
#include "canvasimpl.h"
#include "contentstream.h"
#include "indirectobjectref.h"
#include "interfaces/indirect_object.h"
#include "objfmt.h"
#include "resource_dictionary.h"
#include "docwriterimpl.h"
#include "colorspace.h"
#include "resourcemanagement.h"
#include <resources/resourcebox/colorspacehelpers.h>
#include <msg_pdflib.h>
#include <core/jstd/optionsparser.h>
#include <core/jstd/transaffine.h>
#include <core/generic/checked_cast.h>
#include <core/generic/smartptrutils.h>
#include <core/generic/floatpointtools.h>
#include <core/generic/refcountedimpl.h>
#include <core/errlib/errlib.h>
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>
#include <boost/bind.hpp>
#include <limits>

using namespace boost;
using namespace jag::jstd;
using namespace jag::resources;

namespace jag {
namespace pdf {


// ---------------------------------------------------------------------------
//                         Tiling Pattern
//


namespace
{
  enum TilingSpecKeywords {
      // shading pattern
      TIP_MATRIX, TIP_TYPE, TIP_BBOX, TIP_STEP
  };

  struct TilingKeywords
      : public spirit::classic::symbols<unsigned>
  {
      TilingKeywords()
      {
          add
              ("matrix", TIP_MATRIX)
              ("type", TIP_TYPE) // not documented
              ("bbox", TIP_BBOX)
              ("step", TIP_STEP) // required
              ;
      }
  } g_tiling_keywords;

} // anonymous namespace


//
//
//
TilingPatternImpl::TilingPatternImpl(DocWriterImpl& doc,
                                     Char const* pattern_str,
                                     ICanvas* canvas)
    : PatternBase(pattern_str)
    , m_doc(doc)
    , m_canvas(checked_static_cast<CanvasImpl*>(canvas))
{
    if (m_canvas->content_stream().is_empty())
        throw exception_invalid_value(msg_pattern_no_canvas()) << JAGLOC;

    IProfileInternal const& profile = m_doc.exec_context().config();
    m_tiling_type = static_cast<PatternTilingType>(
        profile.get_int("patterns.tiling_type"));

    reset_indirect_object_worker(m_canvas);

    // parse the string spec
    try
    {
        ParsedResult const& p =
            parse_options(pattern_str,
                          ParseArgs(&g_tiling_keywords));


        parse_array(p, TIP_MATRIX, m_matrix, true, 6);
        parse_array(p, TIP_STEP, m_step, false, 2);
        if (!parse_array(p, TIP_BBOX, m_bbox, true, 4))
        {
            // if bbox not set, set it to [0, 0, stepx, stepy]
            m_bbox[0] = m_bbox[1] = 0;
            m_bbox[2] = m_step[0];
            m_bbox[3] = m_step[1];
        }
        m_tiling_type = static_cast<PatternTilingType>(
            p.to_<int>(TIP_TYPE, m_tiling_type));
    }
    catch(exception const& exc)
    {
        throw exception_invalid_value(
            msg_invalid_shading_spec(), &exc) << JAGLOC;
    }
}


//
//
//
TilingPatternImpl::~TilingPatternImpl()
{
}


//
// Tells whether the pattern is colored (or uncolored).
//
bool TilingPatternImpl::is_colored() const
{
    return m_canvas->content_stream().object_writer().operator_categories() &
        (OPCAT_COLOR | OPCAT_SHADING_PATTERNS | OPCAT_XOBJECT_IMAGE);
}


//
//
//
void TilingPatternImpl::on_before_output()
{
    ContentStream& content_stream = m_canvas->content_stream();

    content_stream.set_writer_callback(
        bind(&TilingPatternImpl::on_write, this, _1));

    m_res_dict = output_resource_dictionary(
        m_doc, m_canvas->resource_list());
}



//////////////////////////////////////////////////////////////////////////
void TilingPatternImpl::on_write(ObjFmt& fmt)
{
    fmt
        .dict_key("Type").output("Pattern")
        .dict_key("PatternType").space().output(1)
        .dict_key("PaintType").space().output(is_colored() ? 1 : 2)
        .dict_key("TilingType").space().output(m_tiling_type)
        .dict_key("XStep").space().output(m_step[0])
        .dict_key("YStep").space().output(m_step[1])
    ;

    output_array("BBox", fmt, m_bbox, m_bbox+4);
    output_array("Matrix", fmt, m_matrix.begin(), m_matrix.end());
    output_resource_dictionary_ref(m_res_dict, fmt);
}

//
//
// 
ICanvas* TilingPatternImpl::canvas() const
{
    return m_canvas;
}

// ---------------------------------------------------------------------------
//              Symbols common for shadings and shading patterns
//
namespace
{
  enum ShadingSpecKeywords {
      // shading pattern
      SH_MATRIX,
      // shading
      SH_COORDS, SH_EXTEND, SH_DOMAIN, SH_BBOX, SH_BACKGROUND, SH_MATRIX_FUN};

  struct ShadingKeywords
      : public spirit::classic::symbols<unsigned>
  {
      ShadingKeywords()
      {
          add
              ("coords", SH_COORDS)
              ("matrix", SH_MATRIX)
              ("extend", SH_EXTEND)
              ("domain", SH_DOMAIN)
              ("matrix_fun", SH_MATRIX_FUN)
              ("bbox", SH_BBOX)
              ("background", SH_BACKGROUND)
              ;
      }
  } g_shading_keywords;


  enum ShadingSpecValues {
      SHV_FUNCTION = 1, SHV_AXIAL = 2, SHV_RADIAL = 3
  };


  struct ShadingValues
      : public spirit::classic::symbols<unsigned>
  {
      ShadingValues()
      {
          add
              ("axial", SHV_AXIAL)
              ("radial", SHV_RADIAL)
              ("function", SHV_FUNCTION)
              ;
      }
  } g_shading_values;

} // anonymous namespace


// ---------------------------------------------------------------------------
//                         Shading
//

//
//
//
ShadingImpl::ShadingImpl(DocWriterImpl& doc,
                         Char const* pattern,
                         ColorSpaceHandle cs,
                         FunctionHandle const* fns,
                         UInt num_functions)
    : IndirectObjectImpl(doc)
    , m_cs(cs)
{
    // Indexed is not allowed for dictionaries with Function field. Shading
    // types supported now require Function field so indexed color space is
    // forbidden.
    if (is_pattern_color_space(cs) || (CS_INDEXED == color_space_type(cs)))
        throw exception_invalid_value(msg_shading_invalid_space()) << JAGLOC;

    m_function_handles.resize(num_functions);
    std::copy(fns, fns + num_functions, m_function_handles.begin());

    try
    {
        ParsedResult const& p =
            parse_options(pattern, ParseArgs(&g_shading_keywords, &g_shading_values));

        // common options matrix
        parse_array(p, SH_BBOX, m_bbox, true, 4);
        parse_array(p, SH_BACKGROUND, m_background);

        // shading type specific
        m_shading_type = p.explicit_value();
        switch(m_shading_type)
        {
        case SHV_AXIAL:
        case SHV_RADIAL:
            parse_array(p, SH_COORDS, m_coords, false, m_shading_type==SHV_AXIAL ? 4 : 6);
            parse_array(p, SH_DOMAIN, m_domain, true, 2);
            if (parse_array(p, SH_EXTEND, m_extend, true, 2)) m_keys.set(BIT_EXTEND);
            break;

        case SHV_FUNCTION:
            parse_array(p, SH_MATRIX_FUN, m_matrix_fun, true, 6);
            parse_array(p, SH_DOMAIN, m_domain, true, 4);
            break;

        default:
            throw exception_invalid_value(msg_unknown_shading_type()) << JAGLOC;
        }
    }
    catch(exception const& exc)
    {
        throw exception_invalid_value(msg_invalid_shading_spec(), &exc) << JAGLOC;
    }
}



//
//
//
bool ShadingImpl::on_before_output_definition()
{
    // get references to used functions
    m_function_refs.resize(m_function_handles.size());
    ResourceManagement* resmgm = &doc().res_mgm();
    std::transform(m_function_handles.begin(),
                   m_function_handles.end(),
                   m_function_refs.begin(),
                   bind(&ResourceManagement::function_ref, resmgm, _1));

    // get reference to the associated color space
    if (!is_trivial_color_space(m_cs))
        m_cs_ref = doc().res_mgm().color_space_ref(m_cs);

    return true;
}



//
// Writes the shading pattern dictionary
//
void ShadingImpl::on_output_definition()
{
    ObjFmt& fmt(object_writer());
    // shading subdictionary
    fmt
        .dict_start()
        .dict_key("ShadingType").space().output(m_shading_type)
        .dict_key("ColorSpace");

    if (is_valid(m_cs_ref))
    {
        fmt.space().ref(m_cs_ref);
    }
    else
    {
        JAG_ASSERT(is_trivial_color_space(m_cs));
        output_trivial_color_space_name(m_cs, fmt.fmt_basic());
    }

    output_array("BBox", fmt, m_bbox.begin(), m_bbox.end());
    output_array("Background", fmt, m_background.begin(), m_background.end());

    switch(m_shading_type)
    {
    case SHV_AXIAL:
    case SHV_RADIAL:
        output_array("Coords", fmt, m_coords.begin(), m_coords.end());
        output_functions();
        if (m_keys.test(BIT_EXTEND))
        {
            fmt
                .dict_key("Extend")
                .array_start()
                .output_bool(m_extend[0])
                .output_bool(m_extend[1])
                .array_end();
        }
        output_array("Domain", fmt, m_domain.begin(), m_domain.end());

        break;

    case SHV_FUNCTION:
        output_functions();
        output_array("Domain", fmt, m_domain.begin(), m_domain.end());
        output_array("Matrix", fmt, m_matrix_fun.begin(), m_matrix_fun.end());
        break;

    default:
        JAG_INTERNAL_ERROR;
    }
    fmt.dict_end(); // shading
}


//
// Outputs the associated functions
//
void ShadingImpl::output_functions()
{
    JAG_PRECONDITION(!m_function_refs.empty());

    ObjFmt& fmt(object_writer());
    fmt.dict_key("Function");
    if (m_function_refs.size() == 1)
    {
        fmt.space().ref(m_function_refs[0]);
    }
    else
    {
        output_ref_array(fmt, m_function_refs.begin(), m_function_refs.end());
    }
}





// ---------------------------------------------------------------------------
//                         Shading Pattern
//

//
//
//
ShadingPatternImpl::ShadingPatternImpl(DocWriterImpl& doc,
                                       Char const* pattern,
                                       ShadingHandle shading)
    : IndirectObjectImpl(doc)
    , PatternBase(pattern)
    , m_shading(shading)
{
    try
    {
        ParsedResult const& p =
            parse_options(pattern, ParseArgs(&g_shading_keywords, &g_shading_values));

        parse_array(p, SH_MATRIX, m_matrix, true, 6);
    }
    catch(exception const& exc)
    {
        throw exception_invalid_value(msg_invalid_shading_spec(), &exc) << JAGLOC;
    }
}


//
//
//
bool ShadingPatternImpl::on_before_output_definition()
{
    // get reference to used shading dictionary
    m_shading_ref = doc().res_mgm().shading_ref(m_shading);
    return true;
}



//
// Writes the shading pattern dictionary
//
void ShadingPatternImpl::on_output_definition()
{
    ObjFmt& fmt(object_writer());
    fmt
        .dict_start()
        .dict_key("Type").output("Pattern")
        .dict_key("PatternType").space().output(2)
        .dict_key("Shading").space().ref(m_shading_ref);
    output_array("Matrix", fmt, m_matrix.begin(), m_matrix.end());
    fmt.dict_end();
}


// ---------------------------------------------------------------------------
//          class PatternBase

//
//
// 
PatternBase::PatternBase(char const* def_string)
    : m_definition_string(def_string)
{
}


//
//
// 
char const* PatternBase::definition_string() const
{
    JAG_PRECONDITION(!m_definition_string.empty());
    return m_definition_string.c_str();
}

//
//
// 
std::vector<double> const& PatternBase::matrix() const
{
    return m_matrix;
}

//
//
// 
void PatternBase::matrix(trans_affine_t const& mtx)
{
    m_matrix.assign(mtx.data(), mtx.data() + 6);
}




}} //namespace jag::pdf

