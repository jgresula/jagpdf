// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef PDFCANVAS_H_JG1411__
#define PDFCANVAS_H_JG1411__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/refcounted.h>
#include <interfaces/constants.h>

namespace jag {

class IFont;
class IImage;

/// Canvas is a class which allows describing the appearance of pages and other
/// graphical elements.
///
class ICanvas
    : public INotRefCounted
{
public:
    // ----------------------------------------------------------------------
    //                      GRAPHICS STATE
    //

    /// Saves the current graphics state.
    ///
    /// Can be restored later by calling state_restore(). For each state_save()
    /// there must be a matching state_restore(). The graphics stack can be
    /// saved/restored hierarchically.
    ///
    /// @see [url_pdfref_chapter Graphics | Graphics State | Graphics State
    /// Stack].
    ///
    virtual void state_save() = 0;

    /// Restores the graphics state.
    ///
    /// @pre The graphics state was previously saved with state_save().
    ///
    /// @see [url_pdfref_chapter Graphics | Graphics State | Graphics State
    /// Stack].
    ///
    virtual void state_restore() = 0;





    // ----------------------------------------------------------------------
    //                       COORDINATE SPACE
    //


    /// Transforms the canvas coordinate space.
    ///
    /// This is a most general transformation. Parameters a-f represents a
    /// transformation matrix.
    ///
    /// @see [url_pdfref_chapter Graphics | Coordinate Systems | Common
    /// Transformations]
    ///
    virtual void transform(Double a, Double b, Double c, Double d, Double e, Double f) = 0;

    /// Translates the origin of the canvas coordinate space by (tx, ty).
    ///
    /// This is the same as [code_canvas_transform_t transform(1, 0, 0, 1, tx,
    /// ty)].
    ///
    /// @see [url_pdfref_chapter Graphics | Coordinate Systems | Common
    /// Transformations]
    ///
    virtual void translate(Double tx, Double ty) = 0;

    /// Scales the units of the canvas coordinate space by (sx, sy).
    ///
    /// This is the same as [code_canvas_transform_t transform(sx, 0, 0, sy, 0,
    /// 0)].
    ///
    /// @see [url_pdfref_chapter Graphics | Coordinate Systems | Common
    /// Transformations]
    ///
    virtual void scale(Double sx, Double sy) = 0;

    /// Rotates the canvas coordinate space axes counterclockwise around the origin.
    ///
    /// This is the same as [code_canvas_transform_t transform(cos(a), sin(a),
    /// -sin(a), cons(a), 0, 0)].
    ///
    /// @param alpha angle in radians
    ///
    /// @see [url_pdfref_chapter Graphics | Coordinate Systems | Common
    /// Transformations]
    ///
    virtual void rotate(Double alpha) = 0;

    /// Skews canvas coordinate system axes.
    ///
    /// This is the same as [code_canvas_transform_t transform(1, tan(a),
    /// tan(b), 1, 0, 0)].
    ///
    /// @param alpha skews the x axis
    /// @param beta skews the y axis
    ///
    /// @see [url_pdfref_chapter Graphics | Coordinate Systems | Common
    /// Transformations]
    ///
    virtual void skew(Double alpha, Double beta) = 0;




    // ----------------------------------------------------------------------
    //                       STROKING ATTRIBUTES
    //

    // TBD: provide defaults


    /// Sets the thickness of a line used to stroke a path.
    ///
    /// Initial value: 1.0.
    ///
    /// @see [url_pdfref_chapter Graphics | Graphics State | Details of Graphics
    /// State Parameters | Line Width]
    ///
    /// @param width non-negative number expressed in user space units
    ///
    virtual void line_width(Double width) = 0;

    /// Controls the pattern of dashes and gaps used to stroke paths.
    ///
    /// Initial value: a solid line.
    ///
    /// @see [url_pdfref_chapter Graphics | Graphics State | Details of Graphics
    /// State Parameters | Line Dash Pattern]
    ///
    /// @param array_in dash array
    /// @param length length of the dash array
    /// @param phase dash phase
    ///
    virtual void line_dash(UInt const* array_in, UInt length, UInt phase) = 0;

    /// Sets the mitter for line joins.
    ///
    /// Initial value: 10.0, for a miter cutoff below approximately 11.5
    /// degrees.
    ///
    /// @see [url_pdfref_chapter Graphics | Graphics State | Details of Graphics
    /// State Parameters | Miter Limit]
    ///
    virtual void line_miter_limit(Double limit) = 0;

    /// Specifies the shape to be used at the end of open subpaths.
    ///
    /// Initial value: `LINE_CAP_BUTT`.
    ///
    /// @see [url_pdfref_chapter Graphics | Graphics State | Details of Graphics
    /// State Parameters | Line Cap Style]
    ///
    /// @param style
    ///         - `LINE_CAP_BUTT`
    ///         - `LINE_CAP_ROUND`
    ///         - `LINE_CAP_SQUARE`
    ///
    virtual void line_cap(LineCapStyle style) = 0;

    /// Specifies the shape to be used at corners of the paths that are stroked.
    ///
    /// Initial value: `LINE_JOIN_MITTER`.
    ///
    /// @see [url_pdfref_chapter Graphics | Graphics State | Details of Graphics
    /// State Parameters | Line Join Style]
    ///
    /// @param style
    ///         - `LINE_JOIN_MITER`
    ///         - `LINE_JOIN_ROUND`
    ///         - `LINE_JOIN_BEVEL`
    ///
    virtual void line_join(LineJoinStyle style) = 0;





    // ----------------------------------------------------------------------
    //                        PATH CONSTRUCTION
    //

    /// Begins a new subpath.
    ///
    /// Moves the current point to coordinates (x, y), omits any connecting
    /// line segment. If the previous path construction operator was also
    /// move_to(), the new move_to() overrides it.
    ///
    /// @see [url_pdfref_chapter Graphics | Path Construction and Painting]
    ///
    virtual void move_to(Double x, Double y) = 0;

    /// Appends a straight line segment from the current point to the point (x, y).
    ///
    /// The new current point is (x, y).
    ///
    /// @pre Path construction has already started.
    /// @see [url_pdfref_chapter Graphics | Path Construction and Painting]
    ///
    virtual void line_to(Double x, Double y) = 0;

    /// Appends a rectangle to the current path as a complete subpath.
    ///
    /// Begins a new subpath.
    ///
    /// @param x the x coordinate of lower-left corner
    /// @param y the y coordinate of lower-left corner
    /// @param width rectangle width (user space units)
    /// @param height rectangle height (user space units)
    ///
    /// @see [url_pdfref_chapter Graphics | Path Construction and Painting]
    ///
    virtual void rectangle(Double x, Double y, Double width, Double height) = 0;

    /// Appends a circle to the current path.
    ///
    /// Begins a new subpath.
    ///
    /// @param x the x coordinate of the circle center
    /// @param y the y coordinate of the circle center
    /// @param radius circle radius (user space units)
    ///
    /// @see [url_pdfref_chapter Graphics | Path Construction and Painting]
    ///
    virtual void circle(Double x, Double y, Double radius) = 0;

    /// Appends a cubic Bezier curve to the current path.
    ///
    /// The curve extends from the current point to point (x3, y3), using (x1,
    /// y1) and (x2, y2) as the Bezier control points. The new current point is
    /// (x3, y3).
    ///
    /// @see
    ///  - [url_pdfref_chapter Graphics | Path Construction and Painting]
    ///  - [url_pdfref_chapter Graphics | Path Construction and Painting | Cubic
    ///    Bezier curves]
    ///
    virtual void bezier_to(Double x1, Double y1, Double x2, Double y2, Double x3, Double y3) = 0;

    /// Appends a cubic Bezier curve to the current path.
    ///
    /// The curve extends from the current point to point (x3, y3), using the
    /// current point and (x2, y2) as the Bezier control points. The new current
    /// point is (x3, y3).
    ///
    /// @see
    ///  - [url_pdfref_chapter Graphics | Path Construction and Painting]
    ///  - [url_pdfref_chapter Graphics | Path Construction and Painting | Cubic
    ///    Bezier curves]
    ///
    virtual void bezier_to_2nd_ctrlpt(Double x2, Double y2, Double x3, Double y3) = 0;

    /// Appends a cubic Bezier curve to the current path.
    ///
    /// The curve extends from the current point to point (x3, y3), using (x1,
    /// y1) and (x3, y3) as the Bezier control points. The new current point is
    /// (x3, y3).
    ///
    /// @see
    ///  - [url_pdfref_chapter Graphics | Path Construction and Painting]
    ///  - [url_pdfref_chapter Graphics | Path Construction and Painting | Cubic
    ///    Bezier curves]
    ///
    virtual void bezier_to_1st_ctrlpt(Double x1, Double y1, Double x3, Double y3) = 0;


    /// Appends an elliptical arc to the current path.
    ///
    /// Begins a new subpath.
    ///
    /// @param cx the x center coordinate
    /// @param cy the y center coordinate
    /// @param rx the x radius
    /// @param ry the y radius
    /// @param start_angle start angle
    /// @param sweep_angle sweep angle
    ///
    /// @version 1.1
    ///
    virtual void arc(Double cx, Double cy,
                     Double rx, Double ry,
                     Double start_angle, Double sweep_angle) = 0;


    /// Appends an elliptical arc to the current path.
    ///
    /// The arc extends from the current point to point (x, y).
    ///
    /// @param x the x endpoint coordinate
    /// @param y the y endpoint coordinate
    /// @param rx the x radius
    /// @param ry the y radius
    /// @param angle angle from the x-axis of the current coordinate system to
    ///              the x-axis of the ellipse
    /// @param large_arc_flag 0 - an arc spanning less than or equal to 180
    ///                       degrees is chosen, or 1 - an arc spanning greater
    ///                       than 180 degrees is chosen
    /// @param sweep_flag 0 - the line joining center to arc sweeps through
    ///                   decreasing angles, or 1 - it sweeps through
    ///                   increasing angles
    ///
    /// @version 1.1
    ///
    virtual void arc_to(Double x, Double y, Double rx, Double ry,
                        Double angle, Int large_arc_flag, Int sweep_flag) = 0;


    /// Closes the current subpath by appending a straight line segment.
    ///
    /// The segment starts in the current point and ends in the starting point
    /// of the subpath. If the current subpath is already closed, path_close()
    /// does nothing. This method terminates the current subpath.
    ///
    /// Appending another segment to the current path begins a new subpath,even
    /// if the new segment begins at the endpoint reached by path_close().
    ///
    /// @see [url_pdfref_chapter Graphics | Path Construction and Painting]
    ///
    virtual void path_close() = 0;


    /// Ends the current path.
    ///
    /// The manner in which is the current path ended is specified by a string
    /// comprising of a meaningful combination of the following letters:
    ///
    ///  - `'s'` stroke
    ///  - `'f'` fill - nonzero winding
    ///  - `'F'` fill - even odd
    ///  - `'c'` close
    ///  - `'w'` clip  - nonzero winding
    ///  - `'W'` clip  - even odd
    ///
    /// An empty string discards the current path.
    ///
    /// If a clipping operation is specified along with a painting operation, it
    /// does not alter the clipping path for the painting operation. After the
    /// path has been painted, the clipping path in the graphics state is set to
    /// the intersection of the current clipping path and the newly constructed
    /// path.
    ///
    /// @see [url_pdfref_chapter Graphics | Path Construction and Painting]
    ///
    virtual void path_paint(Char const* cmd) = 0;





    // ----------------------------------------------------------------------
    //                       TRANSPARENCY
    //


    /// Sets the constant opacity.
    ///
    /// This value is stored in the graphics state and used in the transparent
    /// imaging model. Initial value: 1.0.
    ///
    /// @jag_pdfversion 1.4
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    /// @param alpha constant opacity value
    ///
    /// @see [url_pdfref_chapter Transparency | Specifying Transparency in PDF |
    /// Specifying Shape and Opacity | Constant Shape and Opacity]
    ///
    virtual void alpha(Char const* op, Double alpha) = 0;




    // ----------------------------------------------------------------------
    //                          COLORS
    //

    /// Sets the current color space.
    ///
    /// The current color space is maintained in the graphics state.
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    /// @param cs it can be either one of the predefined color spaces:
    ///          - `CS_DEVICE_GRAY`
    ///          - `CS_DEVICE_RGB`
    ///          - `CS_DEVICE_CMYK`
    ///          .
    ///        or a color space registered by
    ///        jag::IDocument::color_space_load()
    ///
    /// @see [url_pdfref_chapter Graphics | Color Spaces]
    ///
    virtual void color_space(Char const* op, ColorSpace cs) = 0;


    /// Sets the current color for painting operations.
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    ///
    /// @param ch1 color value
    ///
    /// @pre The current color space has one color component (e.g. DeviceGray,
    /// CalGray, Indexed).
    ///
    /// @see
    ///  - color_space()
    ///  - [url_pdfref_chapter Graphics | Color Spaces]
    ///
    virtual void color1(Char const* op, Double ch1) = 0;

    /// Sets the current color for painting operations.
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    ///
    /// @param ch1 value of the 1st color component
    /// @param ch2 value of the 2nd color component
    /// @param ch3 value of the 3rd color component
    ///
    /// @pre The current color space has three color components (e.g. DeviceRGB,
    /// CalRGB, Lab).
    ///
    /// @see
    ///  - color_space()
    ///  - [url_pdfref_chapter Graphics | Color Spaces]
    ///
    virtual void color3(Char const* op, Double ch1, Double ch2, Double ch3) = 0;



    /// Sets the current color for painting operations.
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    ///
    /// @param ch1 value of the 1st color component
    /// @param ch2 value of the 2nd color component
    /// @param ch3 value of the 3rd color component
    /// @param ch4 value of the 4th color component
    ///
    /// @pre The current color space has four color components (e.g. DeviceCMYK).
    ///
    /// @see
    ///  - color_space()
    ///  - [url_pdfref_chapter Graphics | Color Spaces]
    ///
    virtual void color4(Char const* op, Double ch1, Double ch2, Double ch3, Double ch4) = 0;




    // ----------------------------------------------------------------------
    //                          IMAGES
    //

    /// Paints an image.
    ///
    /// An image handle can be obtained by registering an image.
    ///
    /// @param img image
    /// @param x the x coordinate of lower-left corner
    /// @param y the y coordinate of lower-left corner
    ///
    /// @see
    ///  - jag::IDocument::image_definition()
    ///  - [code_document_image_load]
    ///  - [code_document_image_load_file]
    ///
    virtual void image(IImage* img, Double x, Double y) = 0;



    // ----------------------------------------------------------------------
    //                           Patterns
    //

    /// Sets the pattern color space as the current color space.
    ///
    /// Can be used for colored tiling patterns or shading patterns.
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    ///
    /// @see
    ///  - jag::ICanvas::color_space_pattern_uncolored()
    ///  - [url_pdfref_chapter Graphics | Patterns]
    ///
    /// @version 1.1
    ///
    virtual void color_space_pattern(Char const* op) = 0;


    /// Sets the pattern color space as the current color space.
    ///
    /// Can be used for uncolored tiling patterns.
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    /// @param cs the underlying color space
    ///
    /// @see
    ///  - jag::ICanvas::color_space_pattern()
    ///  - [url_pdfref_chapter Graphics | Patterns | Tiling Patterns | Uncolored
    ///    Tiling Patterns]
    ///
    /// @version 1.1
    ///
    virtual void color_space_pattern_uncolored(Char const* op,
                                               ColorSpace cs) = 0;


    /// Sets the current pattern for painting operations.
    ///
    /// Either a shading pattern or a colored tiling pattern can be set with
    /// this function.
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    ///
    /// @param patt shading pattern or colored tiling pattern
    ///
    /// @pre The current color space was established by
    ///      jag::ICanvas::color_space_pattern().
    ///
    /// @see
    ///  - jag::ICanvas::color_space_pattern()
    ///  - jag::IDocument::tiling_pattern_load()
    ///  - jag::IDocument::shading_pattern_load()
    ///  - [url_pdfref_chapter Graphics | Patterns]
    ///
    /// @version 1.1
    ///
    virtual void pattern(Char const* op, Pattern patt) = 0;

    /// Sets the current pattern for painting operations.
    ///
    /// Only an uncolored tiling pattern can be set with this function.
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    ///
    /// @param patt uncolored tiling pattern
    /// @param ch1 value of the first color component
    ///
    /// @pre The current color space was established by
    ///      jag::ICanvas::color_space_pattern_uncolored() with the underlying
    ///      color space having one color component.
    ///
    /// @see
    ///  - jag::ICanvas::color_space_pattern_uncolored()
    ///  - jag::IDocument::tiling_pattern_load()
    ///  - [url_pdfref_chapter Graphics | Patterns]
    ///
    /// @version 1.1
    ///
    virtual void pattern1(Char const* op, Pattern patt, Double ch1) = 0;

    /// Sets the current pattern for painting operations.
    ///
    /// Only an uncolored tiling pattern can be set with this function.
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    ///
    /// @param patt uncolored tiling pattern
    /// @param ch1 value of the first color component
    /// @param ch2 value of the second color component
    /// @param ch3 value of the third color component
    ///
    /// @pre The current color space was established by
    ///      jag::ICanvas::color_space_pattern_uncolored() with the underlying
    ///      color space having three color components.
    ///
    /// @see
    ///  - jag::ICanvas::color_space_pattern_uncolored()
    ///  - jag::IDocument::tiling_pattern_load()
    ///  - [url_pdfref_chapter Graphics | Patterns]
    ///
    /// @version 1.1
    ///
    virtual void pattern3(Char const* op, Pattern patt,
                          Double ch1, Double ch2, Double ch3)  = 0;

    /// Sets the current pattern for painting operations.
    ///
    /// Only an uncolored tiling pattern can be set with this function.
    ///
    /// @param op specifies painting operations
    ///         - `'f'` use for filling operations
    ///         - `'s'` use for stroking operations
    ///
    /// @param patt uncolored tiling pattern
    /// @param ch1 value of the first color component
    /// @param ch2 value of the second color component
    /// @param ch3 value of the third color component
    /// @param ch4 value of the fourth color component
    ///
    /// @pre The current color space was established by
    ///      jag::ICanvas::color_space_pattern_uncolored() with the underlying
    ///      color space having four color components.
    ///
    /// @see
    ///  - jag::ICanvas::color_space_pattern_uncolored()
    ///  - jag::IDocument::tiling_pattern_load()
    ///  - [url_pdfref_chapter Graphics | Patterns]
    ///
    /// @version 1.1
    ///
    virtual void pattern4(Char const* op, Pattern patt,
                          Double ch1, Double ch2, Double ch3, Double ch4) = 0;

    /// Applies the shading pattern to the current clipping path.
    ///
    /// Uses the current user space as the pattern space, the pattern matrix is
    /// ignored. It does not alter the current graphics state in any way.
    ///
    /// @param pattern shading pattern
    ///
    /// @see
    ///  - jag::ICanvas::pattern()
    ///  - [url_pdfref_chapter Graphics | Patterns | Shading Patterns | Shading
    ///    Operator]
    ///
    /// @version 1.1
    /// @jag_pdfversion 1.3
    ///
    virtual void shading_apply(Pattern pattern) = 0;



    // ----------------------------------------------------------------------
    //                           TEXT
    //

    ///
    /// @name Text
    ///
    //@{

    ///
    /// @name Text State
    ///
    //@{

    /// Sets the current font used for showing text.
    ///
    /// It is reset to a default value at the beginning of each page.
    ///
    virtual void text_font(IFont* font) = 0;

    /// Sets the character spacing.
    ///
    /// It is a number expressed in unscaled text space units. Initial value: 0.
    ///
    /// @see [url_pdfref_chapter Text | Text State Parameters and Operators |
    /// Character Spacing]
    ///
    virtual void text_character_spacing(Double spacing) = 0;


    /// Sets the text horizontal scaling.
    ///
    /// It is a number specifying the percentage of the normal width of the
    /// glyphs. Initial value: 100 (normal width).
    ///
    /// @see [url_pdfref_chapter Text | Text State Parameters and Operators |
    /// Horizontal Scaling]
    ///
    virtual void text_horizontal_scaling(Double scaling) = 0;

    /// Sets the text rendering mode.
    ///
    ///
    /// @param mode A meaningful combination of the following letters:
    ///   - `'s'` stroke
    ///   - `'f'` fill
    ///   - `'c'` clip
    ///   - `'i'` invisible
    ///   .
    ///   Initial value: `'f'`
    ///
    /// @see [url_pdfref_chapter Text | Text State Parameters and Operators |
    /// Text Rendering Mode]
    ///
    virtual void text_rendering_mode(Char const* mode) = 0;

    /// Sets the text rise.
    ///
    /// It is a number expressed in unscaled text units. Initial value: 0
    ///
    /// @see [url_pdfref_chapter Text | Text State Parameters and Operators |
    /// Text Rise]
    ///
    virtual void text_rise(Double rise) = 0;

    //@}


    ///
    /// @name Text Showing
    ///
    /// The text showing functions can be divided into two basic categories:
    ///  - One line text, function name starts with text_simple*
    ///  - Multiline text, function name starts with text*
    ///
    /// The functions also differ in their arguments:
    ///  - String can be either zero-terminated or specified by a range. Strings
    ///    specified by a range are available only in C/C++ and functions
    ///    accepting these strings have 'r' in the suffix.
    ///  - Offsets allowing individual glyph positioning can be specified.
    ///    Functions allowing glyph offsets have 'o' in the suffix.
    //@{

    /// Starts a text object.
    ///
    /// @pre A text object has not been started.
    /// @see [ref_tuttext_textobject]
    ///
    virtual void text_start(Double x, Double y) = 0;

    /// Ends a text object.
    ///
    /// @pre A text object has been started.
    /// @see [ref_tuttext_textobject]
    ///
    virtual void text_end() = 0;


    /// Shows a text.
    ///
    /// @param txt_u zero terminated string
    ///
    /// @pre A text object has been started.
    ///
    virtual void text(Char const* txt_u) = 0;


    /// Shows a text with custom positioned glyphs.
    ///
    /// @param txt_u zero terminated string
    /// @param offsets glyph offsets expressed in glyph space (i.e. in
    ///        thousandths of a unit of text space).
    /// @param offsets_length number of offsets
    /// @param positions associates glyph offsets with glyph indices in txt_u
    /// @param positions_length number of positions
    ///
    /// @pre A text object has been started.
    ///
    virtual void text_o(Char const* txt_u,
                        Double const* offsets, UInt offsets_length,
                        Int const* positions, UInt positions_length) = 0;


    /// Shows a text.
    ///
    /// Available only in C/C++.
    ///
    /// @param start points to the first byte of the string
    /// @param end points one byte past the last byte of the string
    ///
    /// @pre A text object has been started.
    ///
    virtual void text_r(Char const* start, Char const* end) = 0;

    /// Shows a text.
    ///
    /// Available only in C/C++.
    ///
    /// @param start points to the first byte of the string
    /// @param end points one byte past the last byte of the string
    /// @param offsets glyph offsets expressed in glyph space (i.e. in
    ///        thousandths of a unit of text space).
    /// @param offsets_length number of offsets
    /// @param positions associates glyph offsets with indices in txt_u
    /// @param positions_length number of positions
    ///
    /// @pre A text object has been started.
    ///
    virtual void text_ro(Char const* start, Char const* end,
                         Double const* offsets, UInt offsets_length,
                         Int const* positions, UInt positions_length) = 0;


    /// Shows a text.
    ///
    /// @param x the x baseline coordinate
    /// @param y the y baseline coordinate
    /// @param txt_u zero terminated string
    ///
    /// @pre A text object has not been started.
    ///
    virtual void text_simple(Double x, Double y, Char const* txt_u) = 0;


    /// Shows a text.
    ///
    /// Available only in C/C++.
    ///
    /// @param x the x baseline coordinate
    /// @param y the y baseline coordinate
    /// @param start points to the first byte of the string
    /// @param end points one byte past the last byte of the string
    ///
    /// @pre A text object has not been started.
    ///
    virtual void text_simple_r(Double x, Double y,
                               Char const* start, Char const* end) = 0;


    /// Shows a text with custom positioned glyphs.
    ///
    /// @param x the x baseline coordinate
    /// @param y the y baseline coordinate
    /// @param txt_u zero terminated string
    /// @param offsets glyph offsets expressed in glyph space (i.e. in
    ///        thousandths of a unit of text space).
    /// @param offsets_length number of offsets
    /// @param positions associates glyph offsets with glyph indices in txt_u
    /// @param positions_length number of positions
    ///
    /// @pre A text object has not been started.
    ///
    virtual void text_simple_o(Double x, Double y,
                                Char const* txt_u,
                                Double const* offsets, UInt offsets_length,
                                Int const* positions, UInt positions_length) = 0;

    /// Shows a text.
    ///
    /// Available only in C/C++.
    ///
    /// @param x the x baseline coordinate
    /// @param y the y baseline coordinate
    /// @param start points to the first byte of the string
    /// @param end points one byte past the last byte of the string
    /// @param offsets glyph offsets expressed in glyph space (i.e. in
    ///        thousandths of a unit of text space).
    /// @param offsets_length number of offsets
    /// @param positions associates glyph offsets with indices in txt_u
    /// @param positions_length number of positions
    ///
    /// @pre A text object has not been started.
    ///
    virtual void text_simple_ro(Double x, Double y,
                                 Char const* start, Char const* end,
                                 Double const* offsets, UInt offsets_length,
                                 Int const* positions, UInt positions_length) = 0;


    /// Shows a text specified by glyph indices.
    ///
    /// @param x the x baseline coordinate
    /// @param y the y baseline coordinate
    /// @param array_in array of glyph indices
    /// @param length number of glyphs to show
    ///
    /// @pre A text object has not been started.
    ///
    /// @version 1.4
    /// 
    virtual void text_glyphs(Double x, Double y,
                             UInt16 const* array_in, UInt length) = 0;

    /// Shows a text specified by glyph indices.
    ///
    /// @param x the x baseline coordinate
    /// @param y the y baseline coordinate
    /// @param array_in array of glyph indices
    /// @param length number of glyphs to show
    /// @param offsets glyph offsets expressed in glyph space (i.e. in
    ///        thousandths of a unit of text space).
    /// @param offsets_length number of offsets
    /// @param positions associates glyph offsets with glyphs in array_in
    /// @param positions_length number of positions
    ///
    /// @pre A text object has not been started.
    ///
    /// @version 1.4
    /// 
    virtual void text_glyphs_o(Double x, Double y,
                               UInt16 const* array_in, UInt length,
                               Double const* offsets, UInt offsets_length,
                               Int const* positions, UInt positions_length) = 0;
    //@}



    ///
    /// @name Text Positioning
    ///
    //@{


    /// Moves to the start of the next line.
    ///
    /// Offsets from the start of the current line by (tx, ty). tx and ty are
    /// numbers expressed in scaled text space units. More precisely, this
    /// operation performs the following assignment
    ///
    /// ['text_matrix = text_line_matrix = [1 0 0 1 tx ty] * text_line_matrix]
    ///
    /// @todo clear up discrepancy between specifications (says that
    /// offset is in unscaled text space units) and the actual
    /// behaviour (scaled text units).
    ///
    /// @pre A text object has been started.
    ///
    virtual void text_translate_line(Double tx, Double ty) = 0;

    //@}

    //@}


    ///////////////////////////////////////////////////////////////////////////

    virtual ~ICanvas() {}



public:
    ///////////////////////////////////////////////////////////////////////////
    //                     UNDOCUMENTED METHODS
    //

    virtual void scaled_image(IImage* image, Double x, Double y, Double sx, Double sy) API_ATTR("undocumented") = 0;
    virtual void alpha_is_shape(Int bool_val) API_ATTR("undocumented") = 0;
};

} // namespace jag




#endif //PDFCANVAS_H_JG1411__

