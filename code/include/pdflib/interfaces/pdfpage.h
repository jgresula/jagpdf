// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef PDFPAGE_JG2221_H__
#define PDFPAGE_JG2221_H__

#include <interfaces/refcounted.h>
#include <interfaces/constants.h>
#include <boost/intrusive_ptr.hpp>

namespace jag {

class ICanvas;

/// Represents a page of a PDF document.
class IPage
    : public INotRefCounted
{
public:
    // ----------------------------------------------------------------------
    //                       ANNOTATIONS
    //

    /// Creates a link annotation associated with a URI action.
    ///
    /// The annotation rectangle is specified in the default user space units.
    ///
    /// @param x       lower-left x of the annotation rectangle
    /// @param y       lower-left y of the annotation rectangle
    /// @param width   width of the annotation rectangle
    /// @param height  height of the annotation rectangle
    /// @param uri       URI in 7-bit ASCII
    /// @param style   reserved, must be 0
    virtual void annotation_uri(Double x, Double y,
                                Double width, Double height,
                                Char const* uri, Char const* style=0) = 0;

    /// Creates a link annotation associated with a destination.
    ///
    /// The annotation rectangle is specified in the default user space units.
    ///
    /// @param x       lower-left x of the annotation rectangle
    /// @param y       lower-left y of the annotation rectangle
    /// @param width   width of the annotation rectangle
    /// @param height  height of the annotation rectangle
    /// @param dest    destination string (see [ref_guide_interactive_destref])
    /// @param style   reserved, must be 0
    ///
    virtual void annotation_goto(Double x, Double y,
                                 Double width, Double height,
                                 Char const* dest, Char const* style=0) = 0;

    /// Creates a link annotation associated with a destination.
    ///
    /// The annotation rectangle is specified in the default user space units.
    ///
    /// @param x       lower-left x of the annotation rectangle
    /// @param y       lower-left y of the annotation rectangle
    /// @param width   width of the annotation rectangle
    /// @param height  height of the annotation rectangle
    /// @param dest    destination
    /// @param style   reserved, must be 0
    ///
    /// @see
    ///  - [ref_guide_interactive_destref]
    ///  - jag::IDocument::destination_define()
    ///  - jag::IDocument::destination_reserve()
    ///  - jag::IDocument::destination_define_reserved()
    ///
    virtual void annotation_goto_obj(Double x, Double y,
                                     Double width, Double height,
                                     Destination dest, Char const* style=0) = 0;


    // ----------------------------------------------------------------------
    //                        CANVAS
    //

    /// Retrieves the current page canvas.
    ///
    /// Lifetime of the canvas instance returned from this function is the same
    /// as that of the page instance that issued it.
    ///
    /// @see [code_document_page]
    virtual ICanvas* canvas() = 0;

protected:
    ~IPage() {}
};

} // namespace jag

#endif // PDFPAGE_JG2221_H__
/** EOF @file */
