// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef CANVASIMPL_H_JG2056__
#define CANVASIMPL_H_JG2056__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "defines.h"
#include "resourcelist.h"
#include "graphicsstatestack.h"
#include "contentstream.h"
#include "indirectobjectfwd.h"
#include <pdflib/interfaces/canvas.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace jag {

// fwd
namespace jstd {class trans_affine_t;}

namespace pdf {
// fwd
class DocWriterImpl;
class ObjFmtBasic;
class ContentStream;
class PDFFont;

class CanvasImpl
    : public ICanvas
    , public IndirectObjectFwd
{
public:
    DEFINE_VISITABLE;
    CanvasImpl(DocWriterImpl& doc_writer, StreamFilter const* filters=0, int num_filters=0);
    ~CanvasImpl();


public: //ICanvas
    void state_save();
    void state_restore();


    void transform(Double a, Double b, Double c, Double d, Double e, Double f);
    void translate(Double x, Double y);
    void scale(Double x, Double y);
    void rotate(Double angle);
    void skew(Double alpha, Double beta);

    void line_width(Double width);
    void line_dash(UInt const* dash_array, UInt array_size, UInt phase);
    void line_miter_limit(Double limit);
    void line_cap(LineCapStyle style);
    void line_join(LineJoinStyle style);


    void RenderingIntent(RenderingIntentType rendering_intent);
    void FlatnessTolerance(UInt flatness);


    // path construction
    void move_to(Double x, Double y);
    void line_to(Double x, Double y);
    void rectangle(Double x, Double y, Double width, Double height);
    void bezier_to(Double x1, Double y1, Double x2, Double y2, Double x3, Double y3);
    void bezier_to_1st_ctrlpt(Double x1, Double y1, Double x3, Double y3);
    void bezier_to_2nd_ctrlpt(Double x2, Double y2, Double x3, Double y3);
    void path_close();
    void circle(Double x, Double y, Double radius);
    void arc(Double cx, Double cy, Double rx, Double ry,
             Double start_angle, Double sweep_angle);
    void arc_to(Double x, Double y, Double rx, Double ry,
                Double angle, Int large_arc_flag, Int sweep_flag);
    void path_paint(Char const* cmd);


    // colors
    void alpha_is_shape(Int bool_val);
    void alpha(Char const* cmd, Double val);
    void color_space(Char const* cmd, ColorSpace cs);
    void color_space_pattern(Char const* op);
    void color_space_pattern_uncolored(Char const* op, ColorSpace cs);
    void color1(Char const* cmd, Double ch1);
    void color3(Char const* cmd, Double ch1, Double ch2, Double ch3);
    void color4(Char const* cmd, Double ch1, Double ch2, Double ch3, Double ch4);
    void pattern(Char const* cmd, Pattern pid);
    void pattern1(Char const* cmd, Pattern pid, Double ch1);
    void pattern3(Char const* cmd, Pattern pid, Double ch1, Double ch2, Double ch3);
    void pattern4(Char const* cmd, Pattern pid, Double ch1, Double ch2, Double ch3, Double ch4);
    void shading_apply(Pattern pattern);

    void image(IImage* img, Double x, Double y);
    void scaled_image(IImage* img, Double x, Double y, Double sx, Double sy);

    // text
    void text_font(IFont* font);
    void text_simple(Double x, Double y, Char const* text);
    void text_simple_r(Double x, Double y, Char const* start, Char const* end);
    void text_simple_o(Double x, Double y,
                            Char const* txt_u,
                            Double const* offsets, UInt offsets_length,
                            Int const* positions, UInt positions_length);
    void text_simple_ro(Double x, Double y,
                             Char const* start, Char const* end,
                             Double const* offsets, UInt offsets_length,
                             Int const* positions, UInt positions_length);

    void text_start(Double x, Double y);
    void text_end();
    void text(Char const* txt);
    void text_translate_line(Double x, Double y);
    void text_o(Char const* txt_u,
                      Double const* adjustments, UInt adjustments_length,
                      Int const* positions, UInt positions_length );
    void text_r(Char const* start, Char const* end);
    void text_ro(Char const* start, Char const* end,
                       Double const* offsets, UInt offsets_length,
                       Int const* positions, UInt positions_length);

    void text_character_spacing(Double spacing);
    void text_horizontal_scaling(Double scaling);
    void text_rendering_mode(Char const* mode);
    void text_rise(Double rise);
    void text_glyphs(Double x, Double y, UInt16 const* array_in, UInt length);
    void text_glyphs_o(Double x, Double y,
                       UInt16 const* array_in, UInt length,
                       Double const* offsets, UInt offsets_length,
                       Int const* positions, UInt positions_length);



public: //non-interface
    void transform(jstd::trans_affine_t const& mtxt);

    ContentStream const& content_stream() const { return *m_content_stream; }
    ContentStream& content_stream() { return *m_content_stream; }

    boost::shared_ptr<ResourceList> const& resource_list() const;
    void text_font_internal(PDFFont const& font);
    void copy_to(CanvasImpl& other);

private:
    ResourceList& ensure_resource_list();
    void color_space_load(ColorSpaceHandle cs);

    void commit_graphics_state();
    void start_path_construction();
    void end_path_construction();

    void write_graphics_state(GraphicsStateHandle gshandle);

    PDFFont const* current_font();
    template<class PRE_ACTION, class POST_ACTION>
    void text_show_generic(Char const* start, Char const* end,
                            Double const* offsets, UInt offsets_length,
                            Int const* positions, UInt positions_length,
                            PRE_ACTION const& pre_action,
                            POST_ACTION const& post_action);
    void text_show_multienc_font(PDFFont const* font,
                                 Char const* start, Char const* end,
                                 Double const* offsets,
                                 Int const* positions,
                                 UInt pos_length);
private:
    enum StrokeFill_t
    {
        CMD_STROKE = 1U << 0
        , CMD_FILL = 1U << 1
    };
    static unsigned stroke_fill_str_to_val(Char const* cmd);


private:
    DocWriterImpl&                      m_doc_writer;
    boost::scoped_ptr<ContentStream>    m_content_stream;
    boost::shared_ptr<ResourceList>     m_resource_list;
    bool                                m_path_construction_active;

    GraphicsStateStack                    m_graphics_state;

    // start of the current subpath
    Double m_path_start_x;
    Double m_path_start_y;
    // the current point
    Double m_path_end_x;
    Double m_path_end_y;
    ObjFmtBasic& m_fmt;
};

}} //namespace jag::pdf


#endif //CANVASIMPL_H_JG2056__

