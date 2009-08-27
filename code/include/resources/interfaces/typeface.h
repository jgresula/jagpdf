// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//
#ifndef __TYPEFACE_H_JAG_1227__
#define __TYPEFACE_H_JAG_1227__

#include <interfaces/refcounted.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <string>
#include <set>
#include <map>

#if !defined(__GCCXML__) && !defined(__DOXYGEN__)
#include <core/errlib/errlib.h>
#endif

#if defined(__GCCXML__)
#error "no gccxml" 
#endif 

// freetype fwd
struct FT_FaceRec_;

namespace jag {

class IStreamInput;
class ITypeface;

//
//
// 
class UsedGlyphs
    : public boost::noncopyable
{
public:
    typedef std::set<UInt16> Glyphs;
    typedef Glyphs::const_iterator GlyphsIter;
    
    typedef std::map<Int, UInt16> CodepointToGlyph;

public:
    UsedGlyphs(ITypeface const& face);
    CodepointToGlyph const& codepoint_to_glyph() const;
    
    Glyphs const& glyphs() const;
    GlyphsIter glyphs_begin() const;
    GlyphsIter glyphs_end() const;
    
    UInt16 add_codepoint(Int codepoint);
    void add_glyph(UInt16 glyph);
    bool is_updated() const;
    void set_update_glyphs();
    void set_update_map();
        

public:
    void update();
    void merge(UsedGlyphs const& other);

private:
    void copy_if_not_unique();
    ITypeface const& m_face;
    struct Impl{
        // all glyphs, including those without a codepoint assigned
        Glyphs glyphs;
        // Unicode to glyph index dictionary
        CodepointToGlyph codepoint_to_glyph;
        bool update_glyphs;
        bool update_map;
    };

    boost::shared_ptr<Impl> m_impl;
};


//
//
//
class FaceCharIterator
{
public:
    struct Item {
        Int codepoint;
        UInt glyph;
    };
    
    FaceCharIterator(FT_FaceRec_* face);
    bool done() const;
    Item const& current() const;
    void next();
    
private:
    Item m_item;
    FT_FaceRec_* m_face;
};



/// Face metrics.
struct TypefaceMetrics
{
    /**
     * @brief The number of font units per EM square for this face.
     *
     * This is typically 2048 for TrueType fonts, and 1000 for Type 1 fonts.
     * Relevant only for scalable formats.
     */
    Int units_per_EM;

    Int bbox_xmin;
    Int bbox_ymin;
    Int bbox_xmax;
    Int bbox_ymax;
    Int baseline_distance;
    Int ascent;
    Int descent;
    Int avg_width;
    Int max_width;
    Int missing_width;
    Int cap_height;
    /// Might be zero if not provided by face.
    Int xheight;
};


/**
 * @brief Supported face types
 */
enum FaceType
{
      FACE_UNINITIALIZED = -1
    , FACE_TRUE_TYPE           ///< covers OpenType with truetype outlines
    , FACE_OPEN_TYPE_CFF       ///< OpenType with postcript outlines (since 1.6)
                               ///- can be passed as TrueType
    , FACE_TYPE_1
    , FACE_TYPE_1_ADOBE_STANDARD
};

typedef Byte Panose[10];
typedef Byte Hash16[16];


// There is a problem under MSVC with boost::ptr_set<T> when T is an abstract
// class. MSVC reports: use of undefined type 'boost::result_of<F>'
//
// This is similar to this report: https://svn.boost.org/trac/boost/ticket/2472
//
// To work around this issue, ITypeface is non-abstract under MSVC.
#ifdef _MSC_VER
# define PURE_FUNCTION { JAG_INTERNAL_ERROR; }
# pragma warning(disable: 4100) //'' : unreferenced formal parameter
#else
# define PURE_FUNCTION = 0
#endif 

/// Concept representing a face.
class ITypeface
    : public boost::noncopyable
{
public:
    virtual FaceType type() const PURE_FUNCTION;

    /**
     * @brief Metrics.
     *
     * @return Metrics.
     */
    virtual TypefaceMetrics const& metrics() const PURE_FUNCTION;

    /**
     * @brief Panose classification.
     *
     * See http://www.monotypeimaging.com/ProductsServices/pan1.aspx.
     *
     * Implementations are allowed not to implement this method.
     *
     * @return Panose classification.
     */
    virtual Panose const& panose() const PURE_FUNCTION;

    /**
     * @brief values: 100, 200, 300, 400, 500, 600, 700, 800, 900
     *
     * 400 is a normal weight, 700 indicates bold
     */
    virtual Int weight_class() const PURE_FUNCTION;

    /**
     * @brief values: 0-9
     *
     * 0 in case face does not provide that information, 5 is normal
     */
    virtual Int width_class() const PURE_FUNCTION;

    virtual Double italic_angle() const PURE_FUNCTION;
    virtual Int fixed_width() const PURE_FUNCTION;
    virtual Int bold() const PURE_FUNCTION;
    virtual Int italic() const PURE_FUNCTION;

    /// Never returns empty string.
    virtual Char const* family_name() const PURE_FUNCTION;
    virtual Char const* style_name() const PURE_FUNCTION;

    /**
     * @brief Retrieves full name.
     *
     * @return full name.
     */
    virtual std::string full_name() const PURE_FUNCTION;
    /// Might not be present for some face types.
    virtual Char const* postscript_name() const PURE_FUNCTION;

public:
    virtual Int char_horizontal_advance(Int codepoint) const PURE_FUNCTION;
    /**
     * @brief Retrieves width of a glyph.
     *
     * Implementations are allowed not to implement this method.
     *
     * @param gid   glyph index
     * @return      glyph width
     */
    virtual Int gid_horizontal_advance(UInt gid) const PURE_FUNCTION;
    virtual Int kerning_for_gids(UInt left, UInt right) const PURE_FUNCTION;
    virtual Int kerning_for_chars(Int left, Int right) const PURE_FUNCTION;

public:
    virtual Int can_embed() const PURE_FUNCTION;
    virtual Int can_subset() const PURE_FUNCTION;

    /**
     * @brief Retrieves glyph index for given codepoint.
     * @param codepoint codepoint to look up
     * @return corresponding gid or 0 if no gid is found (standard convention)
     * Implementations are allowed not to implement this method.
     */
    virtual UInt16 codepoint_to_gid(Int codepoint) const PURE_FUNCTION;
    virtual FaceCharIterator char_iterator() const PURE_FUNCTION;



public: // not going to be exported

    /**
     * @brief Retrieves character encoding scheme.
     *
     * Should be one of the following values
     *  - encoding       .. public encoding       (Type1)
     *  - "FontSpecific" .. not a public encoding (Type1)
     *  - ""             .. no encoding           (TrueType)
     *
     * @return face encoding scheme
     */
    virtual Char const* encoding_scheme() const PURE_FUNCTION;


    /**
     * @brief Number of streams the font consists of.
     *
     * @return Number of streams the font consists of.
     */
    virtual Int num_streams() const PURE_FUNCTION;


     /**
      * @brief Options for retrieving font data.
      */
    enum FontProgramOptions {
        EXTRACT_CFF         = 1 << 0,      ///< extracts CFF from open type with cff
        DONT_INCLUDE_CMAP   = 1 << 1       ///< does not include cmap for truetype
    };

    /**
     * @brief Retrives stream containing font data.
     * @param index of font stream
     * @param options combination of FontProgramOptions values
     * Stream lifetime is tied to ITypeface.
     *
     */
    virtual std::auto_ptr<IStreamInput> font_program(
        int index, unsigned options) const PURE_FUNCTION;


    /**
     * @brief Subsets a font program.
     * @param glyphs glyphs to be included in the subset
     * @param options combination of FontProgramOptions values
     *
     * @pre true==can_subset()
     *
     * @return an input stream with the subset font
     */
    virtual std::auto_ptr<IStreamInput> subset_font_program(
        UsedGlyphs const& glyphs,
        unsigned options) const PURE_FUNCTION;


    /**
     * @brief Hash array uniquely identifing the face.
     *
     * @return Hash array.
     */
    virtual Hash16 const& hash() const PURE_FUNCTION;

    virtual ~ITypeface() {}
};

#undef PURE_FUNCTION

} // namespace jag

#endif //__TYPEFACE_H_JAG_1227__
