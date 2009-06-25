// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __GRAPHICSSTATE_H_JG2056__
#define __GRAPHICSSTATE_H_JG2056__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "colorspace.h"
#include <boost/shared_ptr.hpp>

namespace jag {
namespace pdf {

// fwd
class ObjFmtBasic;
class DocWriterImpl;
class PDFFont;
class Color;

/// copy on write representation of graphics state
class GraphicsState
{
public:
    explicit GraphicsState(DocWriterImpl& doc);
    void line_width(Double width);
    void line_dash(UInt const* dash_array, UInt array_size, UInt phase);
    void line_miter_limit(Double limit);
    void line_cap(LineCapStyle style);
    void line_join(LineJoinStyle style);

    void transfer_fn(FunctionHandle fn);

    // text state
    void font(PDFFont const& fnt);
    PDFFont const* font() const;

    // color
    void fill_color(Color const& color);
    void stroke_color(Color const& color);
    void fill_color_space(ColorSpaceHandle cs);
    void stroke_color_space(ColorSpaceHandle cs);
    ColorSpaceHandle fill_color_space();
    ColorSpaceHandle stroke_color_space();

    void alpha_is_shape(bool val);
    void stroking_alpha(Double val);
    void nonstroking_alpha(Double val);

    /**
     * @brief outputs graphics state incrementally
     *
     * The state is output incrementally against previous state.
     *
     * @param fmt where to output
     * @param res_mgm resource management
     *
     * @return handle to graphics state (if any was outputted)
     */
    GraphicsStateHandle commit(ObjFmtBasic& fmt);


    /// indicates whether the state has been committed
    bool is_committed() const;

    bool is_equal_state(GraphicsState const& other) const;

private:
    void output_colors(ObjFmtBasic& fmt);

private:
    struct GraphicsStatePack;
    boost::shared_ptr<GraphicsStatePack> const& get_unique();

private:
    boost::shared_ptr<GraphicsStatePack> m_state;
    DocWriterImpl*                       m_doc;
};


}} //namespace jag::pdf


#endif //__GRAPHICSSTATE_H_JG2056__

