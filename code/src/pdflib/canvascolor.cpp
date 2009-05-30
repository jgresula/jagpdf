// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "canvasimpl.h"
#include "docwriterimpl.h"
#include "resourcenames.h"
#include "resourcemanagement.h"
#include "contentstream.h"
#include "colorspace.h"
#include "color.h"
#include "visitornoop.h"
#include "patterncolorspace.h"
#include <msg_pdflib.h>
#include <resources/interfaces/colorspaceman.h>
#include <resources/interfaces/resourcectx.h>
#include <resources/interfaces/imageman.h>
#include <resources/interfaces/imagedata.h>
#include <resources/resourcebox/colorspacehelpers.h>

#include <core/errlib/msg_writer.h>
#include <core/generic/refcountedimpl.h>
#include <pdflib/interfaces/canvas.h>

#include <cmath>
#include <iostream>

using namespace boost;
using namespace jag::resources;

namespace jag {
namespace pdf {


namespace
{
  // ---------------------------------------------------------------------------
  //                   Pattern analysis
  //

  enum { PT_UNKNOWN,
         PT_SHADING,
         PT_COLORED,
         PT_UNCOLORED };
  //
  // Determines type of the pattern (shading/colored/uncolored)
  //
  class PatternVisitor
      : public VisitorNoOp
  {
      unsigned m_pattern_type;

  public:
      PatternVisitor() : m_pattern_type(PT_UNKNOWN) {}

      unsigned pattern_type() const {
          JAG_ASSERT(m_pattern_type != PT_UNKNOWN);
          return m_pattern_type;
      }

      bool visit(TilingPatternImpl &pattern) {
          m_pattern_type = pattern.is_colored()
              ? PT_COLORED
              : PT_UNCOLORED;

          return true;
      }

      bool visit(ShadingPatternImpl &) {
          m_pattern_type = PT_SHADING;
          return true;
      }
  };

  //
  // Tests wheter the given patter meets expectations.
  //
  void check_pattern(PatternHandle ph, DocWriterImpl& doc, bool need_colored=true)
  {
    IIndirectObject& obj(doc.res_mgm().pattern(ph));
    PatternVisitor visitor;
    obj.accept(visitor);
    unsigned pt = visitor.pattern_type();

    if (need_colored)
    {
        if ((pt != PT_COLORED) && (pt != PT_SHADING))
        {
            throw exception_invalid_value(
                msg_expected_colored_pattern()) << JAGLOC;
        }
    }
    else
    {
        if (pt != PT_UNCOLORED)
        {
            throw exception_invalid_value(
                msg_expected_uncolored_pattern()) << JAGLOC;
        }
    }
  }

} // namespace



// ---------------------------------------------------------------------------
//                       class CanvasImpl
//


/**
 * @brief Registers color space to the resource list.
 *
 * @param cs color space to be registered
 */
void CanvasImpl::color_space_load(ColorSpaceHandle csh)
{
    if (!is_trivial_color_space(csh))
        ensure_resource_list().add_color_space(csh);
}



void CanvasImpl::color_space(Char const* cmd, ColorSpace cs)
{
    ColorSpaceHandle csh = handle_from_id<RESOURCE_COLOR_SPACE>(cs);
    if (!is_valid(csh))
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    unsigned cmd_val = CanvasImpl::stroke_fill_str_to_val(cmd);
    color_space_load(csh);

    if (cmd_val & CMD_STROKE)
        m_graphics_state.top().stroke_color_space(csh);

    if (cmd_val & CMD_FILL)
        m_graphics_state.top().fill_color_space(csh);
}



void CanvasImpl::color_space_pattern(Char const* op)
{
    color_space(op, CS_PATTERN);
}


void CanvasImpl::color_space_pattern_uncolored(Char const* cmd,
                                                ColorSpace cs)
{
    ColorSpaceHandle csh = handle_from_id<RESOURCE_COLOR_SPACE>(cs);

    if (!is_valid(csh) || is_pattern_color_space(csh))
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    intrusive_ptr<PatternColorSpace> pcs(
        new RefCountImpl<PatternColorSpace>(csh));

    ColorSpaceHandle phandle(
        m_doc_writer.resource_ctx().color_space_man()->color_space_load(pcs));

    JAG_ASSERT(!is_trivial_pattern_color_space(phandle));
    color_space_load(phandle);

    unsigned cmd_val = CanvasImpl::stroke_fill_str_to_val(cmd);
    if (cmd_val & CMD_STROKE)
        m_graphics_state.top().stroke_color_space(phandle);

    if (cmd_val & CMD_FILL)
        m_graphics_state.top().fill_color_space(phandle);
}



void CanvasImpl::color1(Char const* cmd, Double ch1)
{
    // cs or cs2 - gray, calgray or indexed
    unsigned cmd_val = CanvasImpl::stroke_fill_str_to_val(cmd);
    if (cmd_val & CMD_STROKE)
        m_graphics_state.top().stroke_color(Color(ch1));

    if (cmd_val & CMD_FILL)
        m_graphics_state.top().fill_color(Color(ch1));
}



void CanvasImpl::color3(Char const* cmd, Double ch1, Double ch2, Double ch3)
{
    // cs or cs2 - rgb, cielab, calrgb
    unsigned cmd_val = CanvasImpl::stroke_fill_str_to_val(cmd);
    if (cmd_val & CMD_STROKE)
        m_graphics_state.top().stroke_color(Color(ch1,ch2,ch3));

    if (cmd_val & CMD_FILL)
        m_graphics_state.top().fill_color(Color(ch1,ch2,ch3));
}



void CanvasImpl::color4(Char const* cmd, Double ch1, Double ch2, Double ch3, Double ch4)
{
    // cs or cs2 - cmyk
    unsigned cmd_val = CanvasImpl::stroke_fill_str_to_val(cmd);
    if (cmd_val & CMD_STROKE)
        m_graphics_state.top().stroke_color(Color(ch1,ch2,ch3,ch4));

    if (cmd_val & CMD_FILL)
        m_graphics_state.top().fill_color(Color(ch1,ch2,ch3,ch4));
}

namespace
{
  void check_is_pattern_space(ColorSpaceHandle cs)
  {
      if (!is_pattern_color_space(cs))
          throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;
  }
}

//
//
//
void CanvasImpl::pattern(Char const* cmd, Pattern pid)
{
    // colored or shading
    PatternHandle ph(handle_from_id<RESOURCE_PATTERN>(pid));
    check_pattern(ph, m_doc_writer, true);
    unsigned cmd_val = CanvasImpl::stroke_fill_str_to_val(cmd);
    ensure_resource_list().add_pattern(ph);

    if (cmd_val & CMD_STROKE)
    {
        check_is_pattern_space(m_graphics_state.top().stroke_color_space());
        m_graphics_state.top().stroke_color(Color(ph));
    }

    if (cmd_val & CMD_FILL)
    {
        check_is_pattern_space(m_graphics_state.top().fill_color_space());
        m_graphics_state.top().fill_color(Color(ph));
    }
}

//
//
//
void CanvasImpl::pattern1(Char const* cmd, Pattern pid, Double ch1)
{
    // uncolored
    PatternHandle ph(handle_from_id<RESOURCE_PATTERN>(pid));
    check_pattern(ph, m_doc_writer, false);
    unsigned cmd_val = CanvasImpl::stroke_fill_str_to_val(cmd);
    ensure_resource_list().add_pattern(ph);

    if (cmd_val & CMD_STROKE)
    {
        check_is_pattern_space(m_graphics_state.top().stroke_color_space());
        m_graphics_state.top().stroke_color(Color(ph, ch1));
    }

    if (cmd_val & CMD_FILL)
    {
        check_is_pattern_space(m_graphics_state.top().fill_color_space());
        m_graphics_state.top().fill_color(Color(ph, ch1));
    }
}


//
//
//
void CanvasImpl::pattern3(Char const* cmd, Pattern pid, Double ch1, Double ch2, Double ch3)
{
    // uncolored
    PatternHandle ph(handle_from_id<RESOURCE_PATTERN>(pid));
    check_pattern(ph, m_doc_writer, false);
    unsigned cmd_val = CanvasImpl::stroke_fill_str_to_val(cmd);
    ensure_resource_list().add_pattern(ph);

    if (cmd_val & CMD_STROKE)
    {
        check_is_pattern_space(m_graphics_state.top().stroke_color_space());
        m_graphics_state.top().stroke_color(Color(ph, ch1, ch2, ch3));
    }

    if (cmd_val & CMD_FILL)
    {
        check_is_pattern_space(m_graphics_state.top().fill_color_space());
        m_graphics_state.top().fill_color(Color(ph, ch1, ch2, ch3));
    }
}


//
//
//
void CanvasImpl::pattern4(Char const* cmd, Pattern pid, Double ch1, Double ch2, Double ch3, Double ch4)
{
    // uncolored
    PatternHandle ph(handle_from_id<RESOURCE_PATTERN>(pid));
    check_pattern(ph, m_doc_writer, false);
    unsigned cmd_val = CanvasImpl::stroke_fill_str_to_val(cmd);
    ensure_resource_list().add_pattern(ph);

    if (cmd_val & CMD_STROKE)
    {
        check_is_pattern_space(m_graphics_state.top().stroke_color_space());
        m_graphics_state.top().stroke_color(Color(ph, ch1, ch2, ch3, ch4));
    }

    if (cmd_val & CMD_FILL)
    {
        check_is_pattern_space(m_graphics_state.top().fill_color_space());
        m_graphics_state.top().fill_color(Color(ph, ch1, ch2, ch3, ch4));
    }
}

//
//
//
void CanvasImpl::shading_apply(Pattern pattern)
{
    PatternHandle ph(handle_from_id<RESOURCE_PATTERN>(pattern));
    ShadingHandle sh(m_doc_writer.res_mgm().shading_from_pattern(ph));
    ensure_resource_list().add_shading(sh);

    m_fmt.output_resource(sh).graphics_op(OP_sh);
}



}} //namespace jag::pdf

