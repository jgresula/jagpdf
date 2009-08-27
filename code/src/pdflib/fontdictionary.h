// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FONTDICTIONARY_H_JAG_1610__
#define __FONTDICTIONARY_H_JAG_1610__

#include "indirectobjectimpl.h"
#include "pdffontdata.h"
#include <core/jstd/uconverterctrl.h>
#include <core/generic/internal_types.h>
#include <core/errlib/errlib.h>
#include <boost/shared_ptr.hpp>
#include <set>
#include <vector>


namespace jag {
namespace jstd { class UnicodeConverter; }
class UsedGlyphs;

namespace pdf {
class FontDescriptor;
class FontDictionary;

//////////////////////////////////////////////////////////////////////////
class IUsedCharsHandler
    : public noncopyable
{
public:
    /// Retrieves used codepoints.
    virtual void use_char8(Char const* /*start*/, Char const* /*end*/) { JAG_INTERNAL_ERROR; }
    virtual void use_codepoints(Int const* /*start*/, Int const* /*end*/, std::vector<UInt16>& /*gids*/) { JAG_INTERNAL_ERROR; }
    virtual void use_gids(UInt16 const* /*start*/, UInt16 const* /*end*/) { JAG_INTERNAL_ERROR; }
    virtual void output_dictionary(FontDictionary& dict) = 0;
    virtual bool before_output_dictionary(FontDictionary& /*dict*/) { return true; }
    
    virtual UsedGlyphs const& get_used_cids() const {
        JAG_INTERNAL_ERROR; }

    virtual ~IUsedCharsHandler() {}
};




class FontDictionary
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE;
    FontDictionary(DocWriterImpl& doc, int id, PDFFontDictData const& font_data, Char const* encoding);

protected: // IIndirectObject
    bool on_before_output_definition();
    void on_output_definition();

public:
    /// Retrieves used codepoints.
    void set_font_descriptor(boost::shared_ptr<FontDescriptor> const& desc);
    PDFFontDictData const& fdict_data() const { return m_font_data; }
    /**
     * @brief Retrives cids (sorted) used by this dictionary.
     *
     * The actual meaning of retrieved values can be either glyph indices or
     * character codes encoded by some 2-byte encoding (depends on dictionary
     * encoding).
     */
    UsedGlyphs const& get_used_cids() const;
    
public:
    int id() const { return m_id; }
    void use_char8(Char const* start, Char const* end);

    /**
     * @brief Clients reports usage of codepoint through this function.
     *
     * @param start first codepoint
     * @param end end codepoint
     * @param gids filled in with gids corresponding to codepoints
     */
    void use_codepoints(Int const* start, Int const* end, std::vector<UInt16>& gids);

    void use_gids(UInt16 const* start, UInt16 const* end);


    /**
     * @brief Retrives font descriptor associated with this object
     *
     * This method can be called only after set_font_descriptor() has
     * been already called.
     */
    boost::shared_ptr<FontDescriptor> const& font_descriptor() const;


private:
    /**
     * @brief Retrieves converter associated with this font dictionary.
     *
     * Note, that the converter is stateful, so only one sequence
     * of characters can be processed at a time, i.e nested acquires
     * are not allowed. The converter must be released with
     * release_converter() when done.
     *
     * It might return NULL in case of ENC_IDENTITY.
     */
    jstd::UnicodeConverter* acquire_converter() const;

    /// Must be always called in pair with acquire_converter().
    void release_converter() const;

private:
    class UsedCodepoints;
    class UsedChars8;
    class UsedCids;
    class Standard14T1Handler;
    class SimpleFontHandlerBase;
    friend class CIDFontDictionary; //release/acquire_converter()

    PDFFontDictData                       m_font_data;
    jstd::UConverterCtrl               m_conv_ctrl;
    boost::scoped_ptr<IUsedCharsHandler>  m_used_chars_handler;
    boost::shared_ptr<FontDescriptor>     m_font_descriptor;
    const int                             m_id;
};


}} //namespace jag::pdf


#endif //__FONTDICTIONARY_H_JAG_1610__
