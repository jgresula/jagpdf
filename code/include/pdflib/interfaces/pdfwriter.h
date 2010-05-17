// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PDFWRITER_H_JG1057__
#define __PDFWRITER_H_JG1057__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/refcounted.h>
#include <interfaces/constants.h>
#include <boost/intrusive_ptr.hpp>

namespace jag {

// fwd
class ICanvas;
class IImageDef;
class IImageMask;
class IImage;
class IFontSpec;
class IFont;
class IDocumentOutline;
class IPage;


/// Represents a PDF document.
///
/// Instances of this class are [ref_langspecific_t reference counted].
///
class IDocument
    : public IRefCounted
{
public:
    // ----------------------------------------------------------------------
    //                          COLORS
    //
    /// Loads a color space.
    //
    /// Upon loading the color space can be set by jag::ICanvas::color_space().
    ///
    /// @param spec follows the [ref_options_string] format. Specifies the color
    ///   space type and its parameters. Supported color space types are:
    ///     - `'rgb'` - DeviceRGB
    ///     - `'gray'` - DeviceGray
    ///     - `'cmyk'` - DeviceCMYK
    ///     - `'calgray'` - CalGray
    ///     - `'calrgb'` - CalRGB
    ///     - `'cielab'` - CIELab
    ///     - `'icc'` - ICC based
    ///     - `'srgb'` - sRGB color space
    ///     - `'adobe-rgb'` -  AdobeRGB color space
    ///     - `'by-id'` - already registered color space
    ///     .
    ///   Based on the color space type the following options are recognized:
    ///     - `'white'`
    ///       - diffuse white point, values X and Z
    ///       - example: `'calrgb; white=0.9505, 1.0890'`
    ///       - required for: `'calgray'`, `'calrgb'`, `'cielab'`
    ///     - `'black'`
    ///       - diffuse black point, values X, Y and Z
    ///       - default: 0, 0, 0
    ///       - example: `'calrgb; black=0.4084, 1.0208, 0.7113; ...'`
    ///       - applicable to: `'calgray'`, `'calrgb'`, `'cielab'`
    ///     - `'range'`
    ///       - four numbers Amin, Amax, Bmin, Bmax specifying the range of
    ///         valid values for the a* and b* components of the color space
    ///       - default: -100, 100, -100, 100
    ///       - example: `'cielab; range=-75, 75, -75, 75; ...'`
    ///       - applicable to: `'cielab'`
    ///     - `'gamma'`
    ///       - one (`'calgray'`) or three (`'calrgb'`) numbers specifying gamma
    ///       - default: 1.0 for all components of the color
    ///       - example: `'calrgb; gamma=2.2, 2.2, 2.2; ...'`
    ///     - `'matrix'`
    ///       - nine numbers specifying the linear interpolation of the decoded
    ///         A, B and C components of the color space with respect to the
    ///         final XYZ representation
    ///       - default: the identity matrix
    ///       - example: `'calrgb; matrix=0.4497, 0.2446, 0.0252, 0.3163,
    ///         0.6720, 0.1412, 0.1845, 0.0833, 0.9227; ...'`
    ///       - applicable to: `'calrgb'`
    ///     - `'profile'`
    ///       - path to a file with an ICC profile
    ///       - example: `'icc; profile=/path/to/icc; ...'`
    ///       - required for: `'icc'`
    ///     - `'components'`
    ///       - number of color components of the color space defined by an ICC
    ///         profile
    ///       - example: `'icc; components=3; ...'`
    ///       - required for: `'icc'`
    ///     - `'palette'`
    ///       - defines Indexed color space
    ///         - up to ['m] x 256 values, where ['m] is the number of color
    ///           components in the base color space
    ///         - each value is an unsigned integer in range 0 to 255 that is
    ///           scaled to the range of the corresponding color component in
    ///           the base color space
    ///       - example: `'gray; palette=0, 127, 255;'`
    ///       - applicable for: any color space type
    ///     - `'id'`
    ///       - valid color space id, useful for Indexed color space
    ///       - example: `'by-id; id=34; ...'`
    ///       - required for: `'by-id'`
    ///
    /// @see
    ///  - jag::ICanvas::color_space()
    ///  - jag::ICanvas::color1()
    ///  - jag::ICanvas::color3)
    ///  - jag::ICanvas::color4()
    ///  - [url_pdfref_chapter Graphics | Color Spaces]
    ///
    virtual ColorSpace color_space_load(Char const* spec) = 0;


    // ----------------------------------------------------------------------
    //                   DOCUMENT STRUCTURE
    //

    /// Starts a new page.
    ///
    /// Use page_end() to close the page.
    ///
    /// @pre There is no opened page at the moment.
    ///
    /// @param width page width (in default user space units)
    /// @param height height (in default user space units)
    ///
    virtual void page_start(Double width, Double height) = 0;

    /// Ends the current page.
    ///
    /// @pre A page has been started with page_start()
    ///
    virtual void page_end() = 0;

    /// Finalizes the document.
    ///
    /// Upon returning from this function:
    ///  - The PDF document is fully written to the destination (file, stream).
    ///  - All resources associated with the document are released (including this object).
    ///  - No further operations are allowed on the document.
    ///
    virtual void finalize() = 0;

    /// Retrieves the current page.
    ///
    /// Lifetime of the returned page instance ends when page_end() is
    /// called. Using the instance after page_end() has been called results in
    /// undefined behavior.
    ///
    /// @pre There is an active page, i.e between page_start() and page_end().
    ///
    virtual IPage* page() = 0;

    /// Retrieves the current page number (zero-based).
    ///
    ///  - between page_start(), page_end() returns the current page number.
    ///  - outside page_start(), page_end() returns the number of the next page.
    ///
    virtual Int page_number() const = 0;

    /// Retrieves document outline.
    virtual IDocumentOutline* outline() = 0;



    // ----------------------------------------------------------------------
    //                        FONTS
    //

    /// Loads a font.
    ///
    /// Upon loading the font can be set by jag::ICanvas::text_font().
    ///
    /// @param fspec follows the [ref_options_string] format. Specifies the
    ///   font to load. Recognizes the following options:
    ///     - `'standard'`
    ///       - specifies the Standard Type 1 Font
    ///       - example: `'standard; name=Courier; ...'`
    ///     - `'file'`
    ///       - specifies path to a font
    ///       - mutually exclusive with `'name'`
    ///       - example: `'file=/path/to/my/font; ...'`
    ///     - `'name'`
    ///       - specifies facename
    ///       - mutually exclusive with `'file'`
    ///       - for Standard Type 1 Font it can be one of
    ///         - `'Courier-Bold'`
    ///         - `'Courier-BoldOblique'`
    ///         - `'Courier-Oblique'`
    ///         - `'Courier'`
    ///         - `'Helvetica-Bold'`
    ///         - `'Helvetica-BoldOblique'`
    ///         - `'Helvetica-Oblique'`
    ///         - `'Helvetica'`
    ///         - `'Symbol'`
    ///         - `'Times-Bold'`
    ///         - `'Times-BoldItalic'`
    ///         - `'Times-Italic'`
    ///         - `'Times-Roman'`
    ///         - `'ZapfDingbats'`
    ///       - required when using Windows font matching
    ///       - example: `'standard; name=Times-Bold; ...'`
    ///       - example: `'name=verdana; ...'`
    ///     - `'size'`
    ///       - font size in 1/72 inch
    ///       - required
    ///       - example: `'size=12; ...'`
    ///     - `'enc'`
    ///       - changes a font's built-in encoding, the value is a IANA name:
    ///         - [^[url_UTF_8]]
    ///         - [^[url_windows_1250]]
    ///         - [^[url_windows_1251]]
    ///         - [^[url_windows_1252]]
    ///         - [^[url_windows_1253]]
    ///         - [^[url_windows_1254]]
    ///         - [^[url_windows_1255]]
    ///         - [^[url_windows_1256]]
    ///         - [^[url_windows_1257]]
    ///         - [^[url_windows_1258]]
    ///         - [^[url_macintosh]]
    ///         - [^[url_Adobe_Standard_Encoding]]
    ///         - [^[url_ISO_8859_1]]
    ///         - [^[url_ISO_8859_2]]
    ///         - [^[url_ISO_8859_3]]
    ///         - [^[url_ISO_8859_4]]
    ///         - [^[url_ISO_8859_5]]
    ///         - [^[url_ISO_8859_6]]
    ///         - [^[url_ISO_8859_7]]
    ///         - [^[url_ISO_8859_8]]
    ///         - [^[url_ISO_8859_9]]
    ///         - [^[url_ISO_8859_10]]
    ///         - [^[url_ISO_8859_13]]
    ///         - [^[url_ISO_8859_14]]
    ///         - [^[url_ISO_8859_15]]
    ///       - example: `'enc=iso-8859-2; ...'`
    ///     - `'italic'`
    ///       - specifies an italic font
    ///       - only when using Windows font matching
    ///       - example: `'name=verdana; italic; ...'`
    ///     - `'bold'`
    ///       - specifies a bold font
    ///       - only when using Windows font matching
    ///       - example: `'name=verdana; bold; ...'`
    ///
    virtual IFont* font_load(Char const* fspec) = 0;


    // ----------------------------------------------------------------------
    //                      DESTINATIONS
    //

    /// Reserves a destination that can be used immediately, but defined later.
    ///
    /// The reserved destination must be defined by destination_define_reserved()
    /// before invoking finalize().
    ///
    virtual Destination destination_reserve() = 0;

    /// Defines a previously reserved destination.
    ///
    /// @param id    destination ID retrieved from destination_reserve()
    /// @param dest  destination (see [ref_guide_interactive_destref])
    ///
    virtual void destination_define_reserved(Destination id, Char const* dest) = 0;

    /// Defines a destination.
    ///
    /// @param dest  destination (see [ref_guide_interactive_destref])
    ///
    virtual Destination destination_define(Char const* dest) = 0;



    // ----------------------------------------------------------------------
    //                      IMAGES
    //

    /// Provides an empty image definition object.
    ///
    /// After the image has been defined it can be loaded with image_load() and
    /// after that the image can be painted with jag::ICanvas::image().
    ///
    /// The definition object can be used only for definition of a single
    /// image. It cannot be reused after the image has been loaded with
    /// image_load().
    ///
    virtual IImageDef* image_definition() const = 0;

    /// Loads an image.
    ///
    /// Upon loading the image can be painted with jag::ICanvas::image().
    ///
    /// @param image image definition; its lifetime ends here and cannot be used
    ///              further
    ///
    /// @see image_definition()
    ///
    virtual IImage* image_load(IImageDef* image) = 0;

    /// Loads an image from a file.
    ///
    /// Upon registering, the registered image can be painted with
    /// jag::ICanvas::image().
    ///
    /// @param image_file_path path to an image file
    /// @param image_format
    ///         - `IMAGE_FORMAT_AUTO`, automatically recognizes the image format
    ///         - `IMAGE_FORMAT_JPEG`
    ///         - `IMAGE_FORMAT_PNG`
    ///         .
    ///
    virtual IImage* image_load_file(Char const* image_file_path,
                                    ImageFormat image_format = IMAGE_FORMAT_AUTO) = 0;


    // ----------------------------------------------------------------------
    //                      FUNCTIONS
    //

    /// Loads a Type 2 (Exponential Interpolation) function.
    ///
    /// Type 2 functions are 1-in, n-out interpolation functions.
    ///
    /// @param fun Specifies the function. Follows the [ref_options_string]
    ///            format. Recognizes the following options (all optional):
    ///   - `'domain'` - the function domain; an array of 2 numbers, default
    ///                  value: 0.0, 1.0
    ///   - `'range'` - the function range; an array of 2 * ['n] numbers, ['n]
    ///                 is the number of output values
    ///   - `'c0'` - an array of ['n] numbers; the function result when x = 0.0,
    ///              default value: 0.0
    ///   - `'c1'` - an array of ['n] numbers; the function result when x = 1.0,
    ///              default value: 1.0
    ///   - `'exponent'` - the interpolation exponent, default value: 1
    ///
    /// @version 1.1
    /// @jag_pdfversion 1.3
    ///
    /// @see
    ///  - [url_pdfref_chapter Syntax | Functions | Type 2 (Exponential
    ///    Interpolation) Functions]
    ///  - section [ref_guide_functions]
    ///
    virtual Function function_2_load(Char const* fun) = 0;


    /// Loads a Type 3 (Stitching) function.
    ///
    /// Type 3 functions combine ['k] 1-in functions into a single 1-in
    /// function.
    ///
    /// @param fun Specifies the function. Follows the [ref_options_string]
    ///            format. Recognizes the following options:
    ///   - `'domain'` - ['(optional)] the function domain; an array of 2
    ///                  numbers, default value: 0.0, 1.0
    ///   - `'range'` - ['(optional)] the function range; an array of 2 * ['n]
    ///                 numbers, ['n] is the number of output values
    ///   - `'bounds'` - ['(required)] an array of ['k] - 1 numbers; defines
    ///                  the intervals to which each function applies
    ///   - `'encode'` - ['(required)] an array of 2 * ['k] numbers, the
    ///                  individual pairs map each domain subset defined by
    ///                  ['domain] and ['range] to the domain of the
    ///                  corresponding function
    /// @param array_in an array of 1-in functions to combine
    /// @param length number of functions
    ///
    /// @version 1.1
    /// @jag_pdfversion 1.3
    ///
    /// @see
    ///  - [url_pdfref_chapter Syntax | Functions | Type 3 (Stitching)
    ///    Functions]
    ///  - section [ref_guide_functions]
    ///
    virtual Function function_3_load(Char const* fun,
                                     Function const* array_in,
                                     UInt length) = 0;


    /// Loads a Type 4 (PostScript Calculator) function.
    ///
    /// @param fun Specifies the function. Follows the [ref_options_string]
    ///            format. Recognizes the following options (all required):
    ///   - `'domain'` - the function domain; an array of 2 * ['m] numbers, ['m]
    ///                  is the number of input values
    ///   - `'range'` - the function range; an array of 2 * ['n'] numbers, ['n]
    ///                 is the number of output values
    ///   - `'func'` - code written in a subset of the PostScript language
    ///
    /// @version 1.1
    /// @jag_pdfversion 1.3
    ///
    /// @see
    ///  - [url_pdfref_chapter Syntax | Functions | Type 4 (PostScript
    ///    Calculator) Functions]
    ///  - allowed operators in [url_pdfref_chapter Syntax | Functions | Type 4
    ///    (PostScript Calculator) Functions], table ['Operators in type 4
    ///    functions]
    ///  - see section [ref_guide_functions]
    ///
    virtual Function function_4_load(Char const* fun) = 0;



    // ----------------------------------------------------------------------
    //                      PATTERNS
    //

    /// Loads a tiling pattern.
    ///
    /// @param pattern Specifies the pattern properties. Follows the
    ///                [ref_options_string] format. Recognizes the following
    ///                options:
    ///  - `'step'` - ['(required)] an array of two numbers, defines spacing
    ///               between pattern cells in horizontal and vertical
    ///               directions
    ///  - `'matrix'` - ['(optional)] the pattern matrix, determines the pattern
    ///                 coordinate space, default value: the identity matrix
    ///  - `'bbox'` - ['(optional)] the pattern cell's bounding box in the
    ///               pattern coordinate space; expressed as four values: left,
    ///               bottom, right, top
    ///
    /// @param canvas the pattern cell; upon finishing this function the canvas
    ///               can't be used for further operations
    ///
    /// @version 1.1
    ///
    /// @see
    ///  - [url_pdfref_chapter Graphics | Patterns | Tiling Patterns]
    ///  - section [ref_patterns_tiling]
    ///
    virtual Pattern tiling_pattern_load(Char const* pattern,
                                        ICanvas* canvas) = 0;

    /// Loads a shading pattern.
    ///
    /// @param pattern Specifies the shading pattern. Follows the
    ///                [ref_options_string] format. The following shading
    ///                pattern types are recognized:
    ///  - `'function'` - Type 1 (function-based) shadings
    ///  - `'axial'` - Type 2 (axial) shadings
    ///  - `'radial'` - Type 3 (radial) shadings
    ///  .
    /// The following options (all optional) are common for all shading types:
    ///  - `'matrix'` - the pattern matrix, determines the pattern coordinate
    ///                 space, default value: the identity matrix
    ///  - `'bbox'` - the shading pattern's bounding box in the pattern
    ///               coordinate space; expressed as four values: left, bottom,
    ///               right, top
    ///  - `'background'` - an array of color components defining a single
    ///                     background color value
    ///  .
    ///  Based on the shading type these options can be specified:
    ///   - function-based
    ///    - `'domain'` - ['(optional)] an array of 4 numbers specifying the
    ///                   function domain
    ///    - `'matrix_fun'` - ['(optional)] an array of 6 numbers, maps `domain`
    ///                       into the shading's target coordinate space;
    ///                       default value: the identity matrix
    ///   - axial
    ///    - `'coords'` - ['(required)] an array of four numbers [x0, y0, x1,
    ///                   y1], the axis extends from (x0, y0) to (x1, y1),
    ///                   expressed in the shading's target coordinate space
    ///    - `'domain'` - ['(optional)] an array of two limiting values of a
    ///                   parametric variable, default value: 0.0, 1.0
    ///    - `'extend'` - ['(optional)] an array of two values specifying
    ///                   whether to extend the shading beyond the starting and
    ///                   ending points of the axis, respectively; default
    ///                   value: 0, 0
    ///   - radial
    ///    - `'coords'` - ['(required)] an array of six numbers [x0, y0, r0, x1,
    ///                   y1, r1] specifying the centers and radii of the
    ///                   starting and ending circles, expressed in the
    ///                   shading's target coordinate space.
    ///    - `'domain'` - ['(optional)] an array of two limiting values of a
    ///                   parametric variable, default value: 0.0, 1.0
    ///    - `'extend'` - ['(optional)] an array of two values specifying
    ///                   whether to extend the shading beyond the starting and
    ///                   ending circles, respectively; default value: 0, 0
    ///
    /// @param color_space color space in which color values are expressed
    /// @param func
    ///  - function-based
    ///   - 2-in, ['n]-out function
    ///  - axial, radial
    ///   - 1-in, ['n]-out function
    ///  .
    ///  in both cases, ['n] is the number of color components of `color_space`
    ///
    /// @version 1.1
    /// @jag_pdfversion 1.3
    ///
    /// @see
    ///  - [url_pdfref_chapter Graphics | Patterns | Shading Patterns]
    ///  - section [ref_patterns_shading]
    ///  - jag::IDocument::shading_pattern_load_n()
    ///
    virtual Pattern shading_pattern_load(Char const* pattern,
                                         ColorSpace color_space,
                                         Function func) = 0;

    /// Loads a shading pattern.
    ///
    /// @param pattern see description of `pattern` argument
    ///                in jag::IDocument::shading_pattern_load()
    /// @param color_space color space in which color values are expressed
    /// @param array_in
    ///  - function-based
    ///   - ['n] 2-in, 1-out functions
    ///  - axial, radial
    ///   - ['n] 1-in, 1-out functions
    ///  .
    ///  in both cases, ['n] is the number of color components of `color_space`
    /// @param length
    ///
    /// @version 1.1
    /// @jag_pdfversion 1.3
    ///
    /// @see
    ///  - [url_pdfref_chapter Graphics | Patterns | Shading Patterns]
    ///  - section [ref_patterns_shading]
    ///  - jag::IDocument::shading_pattern_load()
    ///
    virtual Pattern shading_pattern_load_n(Char const* pattern,
                                           ColorSpace cs,
                                           Function const* array_in,
                                           UInt length) = 0;


    // ----------------------------------------------------------------------
    //                      MISCELLANEOUS
    //

    /// Retrieves the PDF version.
    virtual Int version() const = 0;

    /// Creates a blank canvas.
    ///
    /// @version 1.1
    virtual ICanvas* canvas_create() const = 0;

    /// Sets a document title.
    ///
    /// It is an alternative to the info.title jag::IProfile option. Use this
    /// function if the document title is unknown prior to document creation.
    ///
    /// @version 1.5
    virtual void title(Char const* title) = 0;

    /// Sets a document author.
    ///
    /// It is an alternative to the info.author jag::IProfile option. Use this
    /// function if the author is unknown prior to document creation.
    ///
    /// @version 1.5
    virtual void author(Char const* author) = 0;
    
    


protected:
    ~IDocument() {}


    // ----------------------------------------------------------------------
    //                          UNDOCUMENTED
    //

public: // masks
    virtual boost::intrusive_ptr<IImageMask> define_image_mask() const API_ATTR("undocumented") = 0;
    virtual ImageMaskID register_image_mask(boost::intrusive_ptr<IImageMask> image_mask) API_ATTR("undocumented") = 0;

    virtual void add_output_intent(Char const* output_condition_id,
                                   Char const* iccpath,
                                   Char const* info,
                                   Int num_components,
                                   Char const* output_condition) API_ATTR("undocumented") = 0;
};


//////////////////////////////////////////////////////////////////////////

} // namespace jag



#endif //__PDFWRITER_H_JG1057__

