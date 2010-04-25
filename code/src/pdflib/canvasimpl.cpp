// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


//
// Portions of this file are
//   Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
#include "precompiled.h"
#include "canvasimpl.h"
#include "docwriterimpl.h"
#include "resourcenames.h"
#include "resourcemanagement.h"
#include "defines.h"
#include <resources/interfaces/imageman.h>
#include <resources/interfaces/imagedata.h>
#include <resources/interfaces/resourcectx.h>
#include <core/jstd/crt.h>
#include <core/jstd/transaffine.h>
#include <core/generic/floatpointtools.h>
#include <core/generic/checked_cast.h>
#include <core/jstd/tracer.h>
#include <core/generic/math.h>

#include <core/errlib/msg_writer.h>
#include <pdflib/interfaces/canvas.h>
#include <msg_pdflib.h>

#include <interfaces/execcontext.h>
#include <interfaces/configinternal.h>

#include <cmath>
#include <iostream>
#include <memory>

using namespace boost;
using namespace jag::jstd;

namespace jag {
namespace pdf {

//////////////////////////////////////////////////////////////////////////
CanvasImpl::CanvasImpl(
    DocWriterImpl& doc_writer
    , StreamFilter const* filter
    , int num_filters
)
    : m_doc_writer(doc_writer)
    , m_content_stream(new ContentStream(doc_writer, filter, num_filters))
    , m_path_construction_active(false)
    , m_graphics_state(m_doc_writer, m_content_stream->object_writer())
    , m_path_start_x(0.0)
    , m_path_start_y(0.0)
    , m_path_end_x(0.0)
    , m_path_end_y(0.0)
    , m_fmt(m_content_stream->object_writer())
{
    reset_indirect_object_worker(m_content_stream.get());
    TRACE_DETAIL << "Content stream writer created (" << this << ")." ;
}



//////////////////////////////////////////////////////////////////////////
CanvasImpl::~CanvasImpl()
{
    TRACE_DETAIL << "Content stream writer released (" << this << ")." ;
}

/**
 * @brief begins a new subpath by moving the current point to given coordinates
 *
 * only this method and rectangle start a path object
 */
void CanvasImpl::move_to(jag::Double x, jag::Double y)
{
    start_path_construction();
    m_fmt.output(x).space().output(y).graphics_op(OP_m);
    m_path_start_x = m_path_end_x = x;
    m_path_start_y = m_path_end_y = y;
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::line_to(jag::Double x, jag::Double y)
{
    m_fmt.output(x).space().output(y).graphics_op(OP_l);
    m_path_end_x = x;
    m_path_end_y = y;
}


//////////////////////////////////////////////////////////////////////////
void CanvasImpl::state_save()
{
    write_graphics_state(m_graphics_state.save());
    m_fmt.graphics_op(OP_q);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::state_restore()
{
    m_graphics_state.restore();
    m_fmt.graphics_op(OP_Q);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::line_width(Double width)
{
    m_graphics_state.top().line_width(width);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::line_cap(LineCapStyle style)
{
    m_graphics_state.top().line_cap(style);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::line_join(LineJoinStyle style)
{
    m_graphics_state.top().line_join(style);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::line_miter_limit(Double limit)
{
    m_graphics_state.top().line_miter_limit(limit);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::line_dash(UInt const* dash_array, UInt array_size, UInt phase)
{
    m_graphics_state.top().line_dash(dash_array, array_size, phase);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::alpha_is_shape(Int bool_val)
{
    m_doc_writer.ensure_version(4, "alpha is shape");
    m_graphics_state.top().alpha_is_shape(bool_val ? true : false);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::alpha(Char const* cmd, Double val)
{
    unsigned cmd_val = CanvasImpl::stroke_fill_str_to_val(cmd);
    if (cmd_val & CMD_STROKE)
    {
        m_doc_writer.ensure_version(4, "current stroking alpha");
        m_graphics_state.top().stroking_alpha(val);
    }

    if (cmd_val & CMD_FILL)
    {
    m_doc_writer.ensure_version(4, "current filling alpha");
    m_graphics_state.top().nonstroking_alpha(val);
    }

}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::RenderingIntent(RenderingIntentType rendering_intent)
{
    if (rendering_intent == RI_UNDEFINED)
        return;

    m_fmt
        .output(rendering_intent_string(rendering_intent))
        .graphics_op(OP_ri)
    ;
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::FlatnessTolerance(UInt flatness)
{
    flatness = (std::min)(100U, flatness);

    m_fmt
        .output(flatness)
        .graphics_op(OP_i)
    ;
}


//////////////////////////////////////////////////////////////////////////
void CanvasImpl::transform(Double a, Double b, Double c, Double d, Double e, Double f)
{
    m_fmt
        .output(a).space()
        .output(b).space()
        .output(c).space()
        .output(d).space()
        .output(e).space()
        .output(f).graphics_op(OP_cm);
}

//
//
// 
void CanvasImpl::transform(trans_affine_t const& mtx)
{
    Double const* mtx_data = mtx.data();
    m_fmt
        .output(mtx_data[0]).space()
        .output(mtx_data[1]).space()
        .output(mtx_data[2]).space()
        .output(mtx_data[3]).space()
        .output(mtx_data[4]).space()
        .output(mtx_data[5]).graphics_op(OP_cm);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::translate(Double x, Double y)
{
    transform(1, 0, 0, 1, x, y);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::scale(Double x, Double y)
{
    transform(x, 0, 0, y, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::rotate(Double angle)
{
    Double sin_ = sin(angle);
    Double cos_ = cos(angle);
    transform(cos_, sin_, -sin_, cos_, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::skew(Double alpha, Double beta)
{
    transform(1, tan(alpha), tan(beta), 1, 0, 0);
}

/**
 * @brief Append a rectangle to the current path a complete subpath
 *
 * only this method and move_to() start a path object
 */
void CanvasImpl::rectangle(Double x, Double y, Double width, Double height)
{
    start_path_construction();
    m_fmt
        .output(x).space()
        .output(y).space()
        .output(width).space()
        .output(height).graphics_op(OP_re);

    m_path_start_x = m_path_end_x = x;
    m_path_start_y = m_path_end_y = y;
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::bezier_to(Double x1, Double y1, Double x2, Double y2, Double x3, Double y3)
{
    m_fmt
        .output(x1).space()
        .output(y1).space()
        .output(x2).space()
        .output(y2).space()
        .output(x3).space()
        .output(y3).graphics_op(OP_c)
        ;

    m_path_end_x = x3;
    m_path_end_y = y3;
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::bezier_to_1st_ctrlpt(Double x1, Double y1, Double x3, Double y3)
{
    m_fmt
        .output(x1).space()
        .output(y1).space()
        .output(x3).space()
        .output(y3).graphics_op(OP_y)
    ;

    m_path_end_x = x3;
    m_path_end_y = y3;
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::bezier_to_2nd_ctrlpt(Double x2, Double y2, Double x3, Double y3)
{
    m_fmt
        .output(x2).space()
        .output(y2).space()
        .output(x3).space()
        .output(y3).graphics_op(OP_v)
    ;

    m_path_end_x = x3;
    m_path_end_y = y3;
}


//////////////////////////////////////////////////////////////////////////
void CanvasImpl::circle(Double x, Double y, Double radius)
{
    // credits: http://www.whizkidtech.redprince.net/bezier/circle/
    const Double kappa = 0.5522847498;
    Double length = radius * kappa;

    move_to(x-radius, y);
    bezier_to(x-radius, y+length, x-length, y+radius, x, y+radius);
    bezier_to(x+length, radius+y, x+radius, y+length, x+radius, y);
    bezier_to(x+radius, y-length, x+length, y-radius, x, y-radius);
    bezier_to(x-length, y-radius, x-radius, y-length, x-radius, y);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::path_close()
{
    m_fmt.graphics_op(OP_h);

    m_path_end_x = m_path_start_x;
    m_path_end_y = m_path_start_y;
}



void CanvasImpl::path_paint(Char const* cmd)
{
    enum {
        STROKE         = 1 << 1,
        FILL_NONZERO   = 1 << 2,
        FILL_EVEN      = 1 << 3,
        CLOSE          = 1 << 4,
        CLIP_NONZERO   = 1 << 5,
        CLIP_EVEN      = 1 << 6
    };

    end_path_construction();

    unsigned cmd_i = 0;
    if (cmd)
    {
        for(;*cmd; ++cmd)
        {
            switch(*cmd)
            {
            case 's': cmd_i |= STROKE; break;
            case 'f': cmd_i |= FILL_NONZERO; break;
            case 'F': cmd_i |= FILL_EVEN; break;
            case 'c': cmd_i |= CLOSE; break;

            // 4.4.3 [.....] A clipping path operator (W or W*, shown in Table
            // 4.11) may appear after the last path construction operator and
            // before the path-painting operator that terminates a path
            // object. Although the clipping path operator appears before the
            // painting operator, it does not alter the clipping path at the
            // point where it appears. Rather, it modifies the effect of the
            // succeeding painting operator. After the path has been painted,
            // the clipping path in the graphics state is set to the
            // intersection of the current clipping path and the newly
            // constructed path.
            case 'w':
                m_fmt.graphics_op(OP_W);
                break;

            case 'W':
                m_fmt.graphics_op(OP_W_star);
                break;

            default:
                throw exception_invalid_value(msg_invalid_paint_cmd()) << JAGLOC;
            }
        }
    }

    GraphicsOperator op;
    switch(cmd_i)
    {
    case STROKE:                        op = OP_S; break;
    case CLOSE | STROKE:                op = OP_s; break;
    case FILL_NONZERO:                  op = OP_f; break;
    case FILL_NONZERO | STROKE:         op = OP_B; break;
    case CLOSE | FILL_NONZERO | STROKE: op = OP_b; break;
    case FILL_EVEN:                     op = OP_f_star; break;
    case STROKE | FILL_EVEN:            op = OP_B_star; break;
    case STROKE | FILL_EVEN | CLOSE:    op = OP_b_star; break;
    default:                            op = OP_n; // end path
    }
    m_fmt.graphics_op(op);
}



//////////////////////////////////////////////////////////////////////////
void CanvasImpl::start_path_construction()
{
    if (!m_path_construction_active)
    {
        m_path_construction_active = true;
        commit_graphics_state();
    }
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::end_path_construction()
{
    if (!m_path_construction_active)
        write_message(WRN_PAINTING_NOTHING);

    if (! m_graphics_state.top().is_committed())
        write_message(WRN_GR_STATE_MODIFIED_WITHIN_PATH_CONSTRUCTION);

    m_path_construction_active = false;
}


//////////////////////////////////////////////////////////////////////////
void CanvasImpl::image(IImage* image, Double x, Double y)
{
    scaled_image(image, x, y, 1.0, 1.0);
}

//////////////////////////////////////////////////////////////////////////
void CanvasImpl::scaled_image(IImage* image, Double x, Double y, Double sx, Double sy)
{
    IImageData const& img_data = *checked_static_cast<IImageData*>(image);
    ImageHandle image_handle(img_data.handle());
    ensure_resource_list().add_image(image_handle);

    state_save();

    // resolution
    double swidth = img_data.width() * 72.0/ img_data.dpi_x();
    double sheight = img_data.height() * 72.0/ img_data.dpi_y();

    if (m_doc_writer.exec_context().config().get_int("doc.topdown"))
    {
        transform(swidth * sx, 0, 0, -sheight * sy, x, y + sheight);
    }
    else
    {
        transform(swidth * sx, 0, 0, sheight * sy, x, y);
    }

    

    // gamma correction
    if (img_data.has_gamma() && !equal_doubles(img_data.gamma(), 1.0) && m_doc_writer.version()>=3)
    {
        char func_str[PDF_DOUBLE_MAX_SIZE];
        snprintf_pdf_double(func_str, PDF_DOUBLE_MAX_SIZE, 1.0/img_data.gamma());

        const int gamma_str_len = 128 + PDF_DOUBLE_MAX_SIZE;
        char gamma_str[gamma_str_len];
        jstd::snprintf(gamma_str, gamma_str_len, "domain=0.0, 1.0; range=0.0, 1.0; func={%s exp}", func_str);
        m_graphics_state.top().transfer_fn(
            m_doc_writer.res_mgm().function_4_load(gamma_str));
    }

    // the image
    commit_graphics_state();
    m_fmt
        .output_resource(image_handle)
        .graphics_op(OP_Do_image)
    ;

    state_restore();
}



//////////////////////////////////////////////////////////////////////////
shared_ptr<ResourceList> const& CanvasImpl::resource_list() const
{
    return m_resource_list;
}

//////////////////////////////////////////////////////////////////////////
ResourceList& CanvasImpl::ensure_resource_list()
{
    if (!m_resource_list)
        m_resource_list.reset(new ResourceList);

    return *m_resource_list;
}


/**
 * @brief commits graphics state to the output
 *
 * should be invoked before any operation like stroke,fill, show text,do xobject etc.
 */
void CanvasImpl::commit_graphics_state()
{
    write_graphics_state(m_graphics_state.commit());
}

void CanvasImpl::write_graphics_state(GraphicsStateHandle handle)
{
    if (is_valid(handle))
    {
        ensure_resource_list().add_graphics_state(handle);

        m_fmt
            .output_resource(handle)
            .graphics_op(OP_gs)
        ;
    }
}


/// converts string commad to STROKE, FILL mask
unsigned CanvasImpl::stroke_fill_str_to_val(Char const* cmd)
{
    unsigned result(0);
    Char const*const end = cmd + 2;
    for(; cmd!=end && *cmd; ++cmd)
    {
        switch (*cmd)
        {
        case 'f':
        case 'F':
            result |= CMD_FILL;
            break;

        case 's':
        case 'S':
            result |= CMD_STROKE;
            break;

        default:
            throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;
        }
    }
    if (!result)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    return result;
}


namespace
{
  // This epsilon is used to prevent us from adding degenerate curves
  // (converging to a single point).
  // The value isn't very critical. Function arc_to_bezier() has a limit
  // of the sweep_angle. If fabs(sweep_angle) exceeds pi/2 the curve
  // becomes inaccurate. But slight exceeding is quite appropriate.
  const double bezier_arc_angle_epsilon = 0.01;

  //
  //
  //
  void arc_to_bezier(Double cx, Double cy, Double rx, Double ry,
                     Double start_angle, Double sweep_angle,
                     Double* curve)
  {
      Double x0 = cos(sweep_angle / 2.0);
      Double y0 = sin(sweep_angle / 2.0);
      Double tx = (1.0 - x0) * 4.0 / 3.0;
      Double ty = y0 - tx * x0 / y0;
      Double px[4];
      Double py[4];
      px[0] =  x0;
      py[0] = -y0;
      px[1] =  x0 + tx;
      py[1] = -ty;
      px[2] =  x0 + tx;
      py[2] =  ty;
      px[3] =  x0;
      py[3] =  y0;

      Double sn = sin(start_angle + sweep_angle / 2.0);
      Double cs = cos(start_angle + sweep_angle / 2.0);

      unsigned i;
      for(i = 0; i < 4; i++)
      {
          curve[i * 2]     = cx + rx * (px[i] * cs - py[i] * sn);
          curve[i * 2 + 1] = cy + ry * (px[i] * sn + py[i] * cs);
      }
  }

  //
  //
  //
  struct arc_approx_t
  {
      enum { LINE_TO, BEZIER_TO } cmd;
      int num_vertices;
      Double vertices[26];
  };

  //
  //
  //
  void bezier_arc_centre(Double x,  Double y,
                         Double rx, Double ry,
                         Double start_angle,
                         Double sweep_angle,
                         arc_approx_t* approx)
  {
      JAG_PRECONDITION(approx);

      start_angle = fmod(start_angle, 2.0 * jag::PI);
      if(sweep_angle >=  2.0 * jag::PI) sweep_angle =  2.0 * jag::PI;
      if(sweep_angle <= -2.0 * jag::PI) sweep_angle = -2.0 * jag::PI;

      if(fabs(sweep_angle) < 1e-10)
      {
          approx->num_vertices = 4;
          approx->cmd = arc_approx_t::LINE_TO;
          approx->vertices[0] = x + rx * cos(start_angle);
          approx->vertices[1] = y + ry * sin(start_angle);
          approx->vertices[2] = x + rx * cos(start_angle + sweep_angle);
          approx->vertices[3] = y + ry * sin(start_angle + sweep_angle);
          return;
      }

      Double total_sweep = 0.0;
      Double local_sweep = 0.0;
      Double prev_sweep;
      approx->num_vertices = 2;
      approx->cmd = arc_approx_t::BEZIER_TO;
      bool done = false;
      do
      {
          if(sweep_angle < 0.0)
          {
              prev_sweep  = total_sweep;
              local_sweep = -jag::PI * 0.5;
              total_sweep -= jag::PI * 0.5;
              if(total_sweep <= sweep_angle + bezier_arc_angle_epsilon)
              {
                  local_sweep = sweep_angle - prev_sweep;
                  done = true;
              }
          }
          else
          {
              prev_sweep  = total_sweep;
              local_sweep =  jag::PI * 0.5;
              total_sweep += jag::PI * 0.5;
              if(total_sweep >= sweep_angle - bezier_arc_angle_epsilon)
              {
                  local_sweep = sweep_angle - prev_sweep;
                  done = true;
              }
          }

          arc_to_bezier(x, y, rx, ry,
                        start_angle,
                        local_sweep,
                        approx->vertices + approx->num_vertices - 2);

          approx->num_vertices += 6;
          start_angle += local_sweep;
      }
      while(!done && approx->num_vertices < 26);
  }


  //
  //
  //
  void bezier_arc_endpoints(Double x0, Double y0,
                            Double rx, Double ry,
                            Double angle,
                            bool large_arc_flag,
                            bool sweep_flag,
                            Double x2, Double y2,
                            arc_approx_t* approx)
  {
      bool radii_ok = true;

      if(rx < 0.0) rx = -rx;
      if(ry < 0.0) ry = -rx;

      // Calculate the middle point between
      // the current and the final points
      //------------------------
      Double dx2 = (x0 - x2) / 2.0;
      Double dy2 = (y0 - y2) / 2.0;

      Double cos_a = cos(angle);
      Double sin_a = sin(angle);

      // Calculate (x1, y1)
      //------------------------
      Double x1 =  cos_a * dx2 + sin_a * dy2;
      Double y1 = -sin_a * dx2 + cos_a * dy2;

      // Ensure radii are large enough
      //------------------------
      Double prx = rx * rx;
      Double pry = ry * ry;
      Double px1 = x1 * x1;
      Double py1 = y1 * y1;

      // Check that radii are large enough
      //------------------------
      Double radii_check = px1/prx + py1/pry;
      if(radii_check > 1.0)
      {
          rx = sqrt(radii_check) * rx;
          ry = sqrt(radii_check) * ry;
          prx = rx * rx;
          pry = ry * ry;
          if(radii_check > 10.0) radii_ok = false;
      }

      // Calculate (cx1, cy1)
      //------------------------
      Double sign = (large_arc_flag == sweep_flag) ? -1.0 : 1.0;
      Double sq   = (prx*pry - prx*py1 - pry*px1) / (prx*py1 + pry*px1);
      Double coef = sign * sqrt((sq < 0) ? 0 : sq);
      Double cx1  = coef *  ((rx * y1) / ry);
      Double cy1  = coef * -((ry * x1) / rx);

      //
      // Calculate (cx, cy) from (cx1, cy1)
      //------------------------
      Double sx2 = (x0 + x2) / 2.0;
      Double sy2 = (y0 + y2) / 2.0;
      Double cx = sx2 + (cos_a * cx1 - sin_a * cy1);
      Double cy = sy2 + (sin_a * cx1 + cos_a * cy1);

      // Calculate the start_angle (angle1) and the sweep_angle (dangle)
      //------------------------
      Double ux =  (x1 - cx1) / rx;
      Double uy =  (y1 - cy1) / ry;
      Double vx = (-x1 - cx1) / rx;
      Double vy = (-y1 - cy1) / ry;
      Double p, n;

      // Calculate the angle start
      //------------------------
      n = sqrt(ux*ux + uy*uy);
      p = ux; // (1 * ux) + (0 * uy)
      sign = (uy < 0) ? -1.0 : 1.0;
      Double v = p / n;
      if(v < -1.0) v = -1.0;
      if(v >  1.0) v =  1.0;
      Double start_angle = sign * acos(v);

      // Calculate the sweep angle
      //------------------------
      n = sqrt((ux*ux + uy*uy) * (vx*vx + vy*vy));
      p = ux * vx + uy * vy;
      sign = (ux * vy - uy * vx < 0) ? -1.0 : 1.0;
      v = p / n;
      if(v < -1.0) v = -1.0;
      if(v >  1.0) v =  1.0;
      Double sweep_angle = sign * acos(v);
      if(!sweep_flag && sweep_angle > 0)
      {
          sweep_angle -= jag::PI * 2.0;
      }
      else if (sweep_flag && sweep_angle < 0)
      {
          sweep_angle += jag::PI * 2.0;
      }

      // We can now build and transform the resulting arc
      //------------------------
      bezier_arc_centre(0.0, 0.0, rx, ry, start_angle, sweep_angle, approx);

      trans_affine_t mtx = trans_affine_translation(cx, cy);
      mtx *= trans_affine_rotation(angle);


      for(int i = 2; i < approx->num_vertices - 2; i += 2)
          mtx.transform(approx->vertices + i, approx->vertices + i + 1);

      // We must make sure that the starting and ending points
      // exactly coincide with the initial (x0,y0) and (x2,y2)
      approx->vertices[0] = x0;
      approx->vertices[1] = y0;
      if(approx->num_vertices > 2)
      {
          approx->vertices[approx->num_vertices - 2] = x2;
          approx->vertices[approx->num_vertices - 1] = y2;
      }
  }


  //
  //
  //
  void draw_arc_approximation(CanvasImpl& canvas, arc_approx_t& approx)
  {
      if (approx.cmd == arc_approx_t::LINE_TO)
      {
          canvas.line_to(approx.vertices[2], approx.vertices[3]);
      }
      else
      {
          JAG_ASSERT(approx.cmd == arc_approx_t::BEZIER_TO);
          for(int i=2; i<approx.num_vertices; i+=6)
          {
              canvas.bezier_to(approx.vertices[i], approx.vertices[i+1],
                               approx.vertices[i+2], approx.vertices[i+3],
                               approx.vertices[i+4], approx.vertices[i+5]);
          }
      }
  }

} //anonymous namespace

//
//
//
void CanvasImpl::arc(Double cx, Double cy,
                     Double rx, Double ry,
                     Double start_angle, Double sweep_angle)
{
    arc_approx_t approx;
    approx.num_vertices = 0;

    bezier_arc_centre(cx, cy, rx, ry, start_angle, sweep_angle, &approx);
    JAG_INTERNAL_ERROR_EXPR(approx.num_vertices > 3);
    //JAG_INTERNAL_ERROR_EXPR(!((approx.num_vertices - 2) % 6));

    move_to(approx.vertices[0], approx.vertices[1]);
    draw_arc_approximation(*this, approx);
}


//
//
//
void CanvasImpl::arc_to(Double x, Double y, Double rx, Double ry,
                        Double angle, Int large_arc_flag, Int sweep_flag)
{
    arc_approx_t approx;
    approx.num_vertices = 0;

    bezier_arc_endpoints(m_path_end_x, m_path_end_y,
                         rx, ry, angle, large_arc_flag,
                         sweep_flag, x, y, &approx);
    JAG_INTERNAL_ERROR_EXPR(approx.num_vertices > 3);
    JAG_INTERNAL_ERROR_EXPR(!((approx.num_vertices - 2) % 6));

    draw_arc_approximation(*this, approx);
}

//
// Copies deeply the content stream. The resource list is shared between this
// one and the other. The graphics state is not transferred.
//
// This is intended for taking a *read-only* snapshot of the canvas.
// 
void CanvasImpl::copy_to(CanvasImpl& other)
{
    m_content_stream->copy_to(other.content_stream());
    other.m_resource_list = m_resource_list;
    
}



}} //namespace jag::pdf



