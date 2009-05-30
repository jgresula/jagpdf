// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef DOCOUTLINE_JG1722_H__
#define DOCOUTLINE_JG1722_H__

#include <interfaces/constants.h>
#include <interfaces/refcounted.h>

namespace jag {

/// Represents a [ref_guide_interactive_docoutline_t document outline].
class IDocumentOutline
    : public INotRefCounted
{
public:
    /// Creates an outline item associated with the current page.
    ///
    /// Clicking the item displays the upper-left corner of the current page
    /// positioned at the upper-left corner of the window.
    ///
    /// @param title UTF-8 text to be displayed
    ///
    /// @pre Can be used only between jag::IDocument::page_start() and
    ///      jag::IDocument::page_end()
    ///
    virtual void item(Char const* title) = 0;


    /// Creates an outline entry associated with a destination.
    ///
    /// Clicking the item changes the view of a document according to the
    /// destination.
    ///
    /// The `page` option can be omitted in the destination specification if
    /// this method is used between jag::IDocument::page_start() and
    /// jag::IDocument::page_end(). The current page is used in such case.
    ///
    /// @param title UTF-8 text to be displayed
    /// @param dest destination specification (see [ref_guide_interactive_destref])
    ///
    virtual void item_destination(Char const* title, Char const* dest) = 0;


    /// Creates an outline entry associated with a destination.
    ///
    /// Clicking the item changes the view of a document according to the
    /// destination.
    ///
    /// @param title UTF-8 text to be displayed
    /// @param dest destination id
    ///
    /// @see
    ///  - [ref_guide_interactive_destref]
    ///  - jag::IDocument::destination_define()
    ///  - jag::IDocument::destination_reserve()
    ///  - jag::IDocument::destination_define_reserved()
    ///
    virtual void item_destination_obj(Char const* title, Destination dest) = 0;



    /// Goes one level down in the outline hierarchy.
    ///
    /// @pre There already must be at least one outline item.
    ///
    virtual void level_down() = 0;


    /// Goes one level up in the outline hierarchy.
    ///
    /// @pre Corresponding level_down() was called previously.
    ///
    virtual void level_up() = 0;


    /// Specifies DeviceRGB color of the outline item's text.
    ///
    /// The specified color is used for the following outline items. The default
    /// value is 0.0, 0.0, 0.0.
    ///
    /// @jag_pdfversion 1.4
    ///
    /// @param red    red component
    /// @param green  green component
    /// @param blue   blue component
    ///
    virtual void color(Double red, Double green, Double blue) = 0;


    /// @brief Specifies the style of the outline entry's text.
    ///
    /// The specified style is used for the following outline items. The default
    /// style is 0.
    ///
    /// @jag_pdfversion 1.4
    ///
    /// @param val combination of flags
    ///              - 1 .. italic
    ///              - 2 .. bold
    ///
    virtual void style(Int val) = 0;


    /// Saves the current state (outline items' color and style).
    ///
    /// Can be restored later by calling state_restore().
    ///
    virtual void state_save() = 0;

    /// Restores the state (outline items' color and style).
    ///
    /// @pre The state was previously saved with state_save().
    ///
    virtual void state_restore() = 0;

protected:
    ~IDocumentOutline() {}
};

} // namespace jag


#endif // DOCOUTLINE_JG1722_H__
/** EOF @file */
