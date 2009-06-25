// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "graphicsstate.h"
#include "objfmtbasic.h"
#include "resourcemanagement.h"
#include "graphicsstatedictionary.h"
#include "docwriterimpl.h"
#include "pdffont.h"
#include "color.h"
#include "colorspace.h"
#include <resources/interfaces/resourcectx.h>
#include <resources/interfaces/typeman.h>
#include <resources/interfaces/colorspaceman.h>
#include <core/generic/floatpointtools.h>
#include <boost/shared_ptr.hpp>
#include <bitset>

using namespace jag::resources;

namespace jag {
namespace pdf {

namespace
{

  /// contains graphics state parameters which are set through operators
  class GraphicsStateOperators
  {
  public:
      // when adding new members, do not forget to update operators
      Double               m_line_width;
      std::vector<UInt>  m_dash_array;
      UInt               m_dash_phase;
      Double               m_line_miter_limit;
      LineCapStyle         m_line_cap;
      LineJoinStyle        m_line_join;

      // text
      PDFFont const*       m_pdf_font;

      Color                m_stroke_color;
      Color                m_fill_color;
      ColorSpaceHandle     m_stroke_color_space;
      ColorSpaceHandle     m_fill_color_space;

      GraphicsStateOperators();

      // gs param constants used for accessing m_status bits
      enum {
            GS_LINE_WIDTH
          , GS_LINE_DASH
          , GS_MITER_LIMIT
          , GS_LINE_CAP
          , GS_LINE_JOIN
          , GS_FONT
          , GS_STROKE_COLOR
          , GS_FILL_COLOR
          , GS_STROKE_COLOR_SPACE
          , GS_FILL_COLOR_SPACE
          , GS_NUM_OPERATOR_PARAMS
      };
      std::bitset<GS_NUM_OPERATOR_PARAMS>  m_param_changed;
  };

  bool operator==(GraphicsStateOperators const& lhs,
                  GraphicsStateOperators const& rhs)
  {
      return
          lhs.m_pdf_font == rhs.m_pdf_font &&
          lhs.m_stroke_color == rhs.m_stroke_color &&
          lhs.m_fill_color == rhs.m_fill_color &&
          lhs.m_stroke_color_space == rhs.m_stroke_color_space &&
          lhs.m_fill_color_space == rhs.m_fill_color_space &&
          equal_doubles(lhs.m_line_width, rhs.m_line_width) &&
          lhs.m_dash_array == rhs.m_dash_array &&
          lhs.m_dash_phase == rhs.m_dash_phase &&
          equal_doubles(lhs.m_line_miter_limit, rhs.m_line_miter_limit) &&
          lhs.m_line_cap == rhs.m_line_cap &&
          lhs.m_line_join == rhs.m_line_join
          ;
  }



  void output_color1(ObjFmtBasic& fmt, Color const& color)
  {
      fmt.output(color.channel1());
  }



  void output_color3(ObjFmtBasic& fmt, Color const& color)
  {
      fmt
          .output(color.channel1()).space()
          .output(color.channel2()).space()
          .output(color.channel3());
  }



  void output_color4(ObjFmtBasic& fmt, Color const& color)
  {
      fmt
          .output(color.channel1()).space()
          .output(color.channel2()).space()
          .output(color.channel3()).space()
          .output(color.channel4());
  }



  void output_color1i(ObjFmtBasic& fmt, Color const& color)
  {
      fmt.output(color.index()).space();
  }



  void output_color(ObjFmtBasic& fmt, ColorSpaceHandle csh, Color const& color, DocWriterImpl const& doc)
  {
      typedef void (*out_color_fn_t)(ObjFmtBasic&, Color const&);
      static out_color_fn_t output_color_fn[4] = { &output_color1, 0, &output_color3, &output_color4 };

      switch(color_space_type(csh))
      {
      default:
          // was JAG_INTERNAL_ERROR, see #110

      case CS_DEVICE_GRAY:
      case CS_CALGRAY:
          output_color1(fmt, color);
          break;

      case CS_INDEXED:
          output_color1i(fmt, color);
          break;

      case CS_DEVICE_RGB:
      case CS_CIELAB:
      case CS_CALRGB:
          output_color3(fmt, color);
          break;

      case CS_DEVICE_CMYK:
          output_color4(fmt, color);
          break;

      case CS_ICCBASED:
          {
              int i = doc.resource_ctx().color_space_man()->num_components(csh);
              JAG_ASSERT(i==1 || i==3 || i==4);
              (output_color_fn[i-1])(fmt, color);
          }
          break;
      }
  }

} // anonymous namespace


/// encapsulates both dictionary based and operator based parameters
struct GraphicsState::GraphicsStatePack
{
    GraphicsStateDictionary m_dictionary;
    GraphicsStateOperators m_operators;
};




//////////////////////////////////////////////////////////////////////////
GraphicsState::GraphicsState(DocWriterImpl& doc)
    : m_state(new GraphicsStatePack)
    , m_doc(&doc)
{
}

/// retrieves a writable state (performs copy if state is not uniquely held)
boost::shared_ptr<GraphicsState::GraphicsStatePack> const& GraphicsState::get_unique()
{
    if (!m_state.unique())
        m_state.reset(new GraphicsStatePack(*m_state));

    return m_state;
}

//////////////////////////////////////////////////////////////////////////
bool GraphicsState::is_committed() const
{
    return
        m_state->m_operators.m_param_changed.none()
        && m_state->m_dictionary.m_param_changed.none()
    ;
}


void GraphicsState::output_colors(ObjFmtBasic& fmt)
{
    GraphicsStateOperators& ops = m_state->m_operators;
    const ColorSpaceHandle color_space[2] = { ops.m_stroke_color_space, ops.m_fill_color_space };

    const bool color_space_changed[2] = {
        ops.m_param_changed[GraphicsStateOperators::GS_STROKE_COLOR_SPACE],
        ops.m_param_changed[GraphicsStateOperators::GS_FILL_COLOR_SPACE]
    };


    // color spaces
    const GraphicsOperator csset_op[2] = {OP_CS, OP_cs};
    for(int i=0; i<2; ++i)
    {
        if (!color_space_changed[i])
            continue;

        output_color_space_name(color_space[i], fmt);
        fmt.graphics_op(csset_op[i]);
    }


    // colors
    const GraphicsOperator colset11_op[2] = {OP_SC, OP_sc};   // indexed dev_gray/rgb/cmyk, lab_gray/rgb/cie
    const GraphicsOperator colset12_op[2] = {OP_SCN, OP_scn}; // pattern, separation, devicen, iccbase

    const bool color_changed[2] = {
        ops.m_param_changed[GraphicsStateOperators::GS_STROKE_COLOR],
        ops.m_param_changed[GraphicsStateOperators::GS_FILL_COLOR]
    };

    const Color *const colors[2] = { &ops.m_stroke_color, &ops.m_fill_color };

    for(int i=0; i<2; ++i)
    {
        if (!color_changed[i])
            continue;

        GraphicsOperator const* color_set_op = colset11_op + i;
        if (is_pattern_color_space(color_space[i]))
        {
            if (!is_trivial_pattern_color_space(color_space[i])) //? colored pattern
                output_color(fmt, unmask_pattern(color_space[i]), *colors[i], *m_doc);

            fmt.output_resource(colors[i]->pattern());
            color_set_op = colset12_op + i;
        }
        else
        {
            output_color(fmt, color_space[i], *colors[i], *m_doc);
        }
        fmt.graphics_op(*color_set_op);
    }
}



//////////////////////////////////////////////////////////////////////////
GraphicsStateHandle GraphicsState::commit(ObjFmtBasic& fmt)
{
    GraphicsStateHandle result;

    // check operators first
    if (m_state->m_operators.m_param_changed.any())
    {
        GraphicsStateOperators& ops = m_state->m_operators;

        if (ops.m_param_changed[GraphicsStateOperators::GS_LINE_WIDTH])
            fmt.output(ops.m_line_width).graphics_op(OP_w);

        if (ops.m_param_changed[GraphicsStateOperators::GS_LINE_DASH])
        {
            fmt.array_start();
            if (!ops.m_dash_array.empty())
            {
                size_t dash_size_minus_1 = ops.m_dash_array.size()-1;
                size_t i=0;
                for(; i<dash_size_minus_1; ++i)
                    fmt.output(ops.m_dash_array[i]).space();

                fmt.output(ops.m_dash_array[i]); //the last element
            }

            fmt
                .array_end()
                .output(ops.m_dash_phase)
                .graphics_op(OP_d)
            ;
        }

        if (ops.m_param_changed[GraphicsStateOperators::GS_MITER_LIMIT])
            fmt.output(ops.m_line_miter_limit).graphics_op(OP_M);

        if (ops.m_param_changed[GraphicsStateOperators::GS_LINE_CAP])
            fmt.output(static_cast<int>(ops.m_line_cap)).graphics_op(OP_J);

        if (ops.m_param_changed[GraphicsStateOperators::GS_LINE_JOIN])
            fmt.output(static_cast<int>(ops.m_line_join)).graphics_op(OP_j);

        // do not commit a font with multiple encodings, this is handled during
        // text output
        if (ops.m_param_changed[GraphicsStateOperators::GS_FONT] &&
            !ops.m_pdf_font->has_multiple_encondings())
        {
            fmt
                .output_resource(ops.m_pdf_font->font_dict())
                .space()
                .output(ops.m_pdf_font->font()->size())
                .graphics_op(OP_Tf)
            ;
        }

        output_colors(fmt);
        ops.m_param_changed.reset();
    }

    if (m_state->m_dictionary.m_param_changed.any())
    {
        result.reset(
            m_doc->res_mgm().register_graphics_state(m_state->m_dictionary)
       );
        m_state->m_dictionary.m_param_changed.reset();
    }

    return result;
}


//////////////////////////////////////////////////////////////////////////
void GraphicsState::font(PDFFont const& fnt)
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    if (!ops.m_pdf_font || &fnt != ops.m_pdf_font)
    {
        ops.m_pdf_font = &fnt;
        ops.m_param_changed.set(GraphicsStateOperators::GS_FONT);
    }
}

//////////////////////////////////////////////////////////////////////////
PDFFont const* GraphicsState::font() const
{
    return m_state->m_operators.m_pdf_font;
}



void GraphicsState::fill_color_space(ColorSpaceHandle cs)
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    if (cs != ops.m_fill_color_space)
    {
        ops.m_fill_color_space = cs;
        ops.m_param_changed.set(GraphicsStateOperators::GS_FILL_COLOR_SPACE);
    }
}



void GraphicsState::stroke_color_space(ColorSpaceHandle cs)
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    if (cs != ops.m_stroke_color_space)
    {
        ops.m_stroke_color_space = cs;
        ops.m_param_changed.set(GraphicsStateOperators::GS_STROKE_COLOR_SPACE);
    }
}


//
//
// 
ColorSpaceHandle GraphicsState::fill_color_space()
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    return ops.m_fill_color_space;

}

//
//
// 
ColorSpaceHandle GraphicsState::stroke_color_space()
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    return ops.m_stroke_color_space;
}




void GraphicsState::fill_color(Color const& color)
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    if (color != ops.m_fill_color)
    {
        ops.m_fill_color = color;
        ops.m_param_changed.set(GraphicsStateOperators::GS_FILL_COLOR);
    }
    else
    {
        // force the color operator as the color space has changed
        if (ops.m_param_changed[GraphicsStateOperators::GS_FILL_COLOR_SPACE])
            ops.m_param_changed.set(GraphicsStateOperators::GS_FILL_COLOR);
    }
}




void GraphicsState::stroke_color(Color const& color)
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    if (color != ops.m_stroke_color)
    {
        ops.m_stroke_color = color;
        ops.m_param_changed.set(GraphicsStateOperators::GS_STROKE_COLOR);
    }
    else
    {
        // force the color operator as the color space has changed
        if (ops.m_param_changed[GraphicsStateOperators::GS_STROKE_COLOR_SPACE])
            ops.m_param_changed.set(GraphicsStateOperators::GS_STROKE_COLOR);
    }
}



//////////////////////////////////////////////////////////////////////////
void GraphicsState::line_width(Double width)
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    if (!equal_doubles(ops.m_line_width, width))
    {
        ops.m_line_width = width;
        ops.m_param_changed.set(GraphicsStateOperators::GS_LINE_WIDTH);
    }
}



//////////////////////////////////////////////////////////////////////////
void GraphicsState::line_dash(UInt const* dash_array, UInt array_size, UInt phase)
{
    GraphicsStateOperators& ops = get_unique()->m_operators;
    if (  ops.m_dash_array.size() != array_size
           || !std::equal(dash_array, dash_array+array_size, ops.m_dash_array.begin())
   )
    {
        ops.m_dash_array.resize(array_size);
        std::copy(dash_array, dash_array+array_size, ops.m_dash_array.begin());
        ops.m_dash_phase = phase;
        ops.m_param_changed.set(GraphicsStateOperators::GS_LINE_DASH);
    }
}


//////////////////////////////////////////////////////////////////////////
void GraphicsState::line_miter_limit(Double limit)
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    if (!equal_doubles(ops.m_line_miter_limit, limit))
    {
        ops.m_line_miter_limit = limit;
        ops.m_param_changed.set(GraphicsStateOperators::GS_MITER_LIMIT);
    }
}


//////////////////////////////////////////////////////////////////////////
void GraphicsState::line_cap(LineCapStyle style)
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    if (ops.m_line_cap != style)
    {
        ops.m_line_cap = style;
        ops.m_param_changed.set(GraphicsStateOperators::GS_LINE_CAP);
    }
}


//////////////////////////////////////////////////////////////////////////
void GraphicsState::line_join(LineJoinStyle style)
{
    GraphicsStateOperators& ops(get_unique()->m_operators);
    if (ops.m_line_join != style)
    {
        ops.m_line_join = style;
        ops.m_param_changed.set(GraphicsStateOperators::GS_LINE_JOIN);
    }
}


//////////////////////////////////////////////////////////////////////////
void GraphicsState::alpha_is_shape(bool val)
{
    GraphicsStateDictionary& dict(get_unique()->m_dictionary);
    if (dict.m_alpha_is_shape != val)
    {
        dict.m_alpha_is_shape = val;
        dict.m_param_changed.set(GraphicsStateDictionary::GS_ALPHA_IS_SHAPE);
    }
}

//////////////////////////////////////////////////////////////////////////
void GraphicsState::stroking_alpha(Double val)
{
    GraphicsStateDictionary& dict(get_unique()->m_dictionary);
    if (!equal_doubles(val, dict.m_stroking_alpha))
    {
        dict.m_stroking_alpha = val;
        dict.m_param_changed.set(GraphicsStateDictionary::GS_STROKING_ALPHA);
    }
}

//////////////////////////////////////////////////////////////////////////
void GraphicsState::nonstroking_alpha(Double val)
{
    GraphicsStateDictionary& dict(get_unique()->m_dictionary);
    if (!equal_doubles(val, dict.m_nonstroking_alpha))
    {
        dict.m_nonstroking_alpha = val;
        dict.m_param_changed.set(GraphicsStateDictionary::GS_NONSTROKING_ALPHA);
    }
}



void GraphicsState::transfer_fn(FunctionHandle fn)
{
    GraphicsStateDictionary& dict(get_unique()->m_dictionary);
    if (fn != dict.m_transfer_fn)
    {
        dict.m_transfer_fn = fn;
        dict.m_param_changed.set(GraphicsStateDictionary::GS_TRANSFER_FUNCTION);
    }
}

bool GraphicsState::is_equal_state(GraphicsState const& other) const
{
    if (m_state == other.m_state)
        return true;

    return (m_state->m_operators == other.m_state->m_operators) &&
        (m_state->m_dictionary == other.m_state->m_dictionary);
}




namespace {


//////////////////////////////////////////////////////////////////////////
// GraphicsStateOperators
//////////////////////////////////////////////////////////////////////////

/// ctor
GraphicsStateOperators::GraphicsStateOperators()
    : m_line_width(1.0)
    , m_dash_phase(0)
    , m_line_miter_limit(10.0)
    , m_line_cap(LINE_CAP_BUTT)
    , m_line_join(LINE_JOIN_MITER)
    , m_pdf_font(0)
    , m_stroke_color(0.0)
    , m_fill_color(0.0)
      // changed from device gray to undefined -> see #110
    , m_stroke_color_space(ColorSpaceHandle())
    , m_fill_color_space(ColorSpaceHandle())
{
}

} // anonymous namespace



}} //jag::pdf
