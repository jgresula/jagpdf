// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __COLORSPACE_H__JAG2301__
#define __COLORSPACE_H__JAG2301__

#include "indirectobjectref.h"
#include "indirectobjectfwd.h"
#include "patterncolorspace.h"
#include <resources/othermanagers/colorspacevisitor.h>
#include <resources/interfaces/resourcehandle.h>

namespace jag
{
namespace pdf {
class ObjFmtBasic;
class ObjFmt;
class DocWriterImpl;
class IndirectObjectCallback;


//
//
//
void output_color_space_ref(DocWriterImpl& doc, ColorSpaceHandle csh);

/**
 * @brief Outputs a color space reference.
 *
 * To be used within content streams. Generic funtion for all color space types.
 * It is not complete command, just an operand.
 *
 * @param handle color space to output (cannot be pattern)
 * @param fmt    formatter to be used
 */
void output_color_space_name(ColorSpaceHandle csh, ObjFmtBasic& fmt);


/**
 * @brief Outputs a trivial color space reference.
 *
 * This is not a complete command, just an operand without trailing space.
 *
 * @param handle color space to output
 * @param fmt formatter to be used
 */
void output_trivial_color_space_name(ColorSpaceHandle handle, ObjFmtBasic& fmt);



/**
 * @brief Indirect object representing a color space.
 *
 * Implemented as a IColorSpaceVisitor.
 *
 * Trivial color spaces (i.e. not requiring explicit definition)
 * cannot be outputted through this object.
 *
 * @todo check implementation of indexed color space
 */
class ColorSpaceObj
    : IColorSpaceVisitor
    , public IndirectObjectFwd
{
public:
    DEFINE_VISITABLE;
    ColorSpaceObj(DocWriterImpl& doc, ColorSpaceHandle cs_handle);


private: // IColorSpaceVisitor
    void visit(resources::PaletteImpl& obj);
    void visit(resources::CIELabImpl& obj);
    void visit(pdf::PatternColorSpace& obj);
    void visit(resources::CIECalRGBImpl& obj);
    void visit(resources::CIECalGrayImpl& obj);
    void visit(resources::ICCBasedImpl& obj);

private:
    template<class Impl, class Callback>
    void visit_internal(Impl& impl, Callback callback);

    template<class Impl, class CallbackBefore, class Callback>
    void visit_internal(Impl& impl, CallbackBefore bcallback, Callback callback);


    void output_pattern(PatternColorSpace& obj, ObjFmt& fmt);
    void output_cielab(resources::CIELabImpl& obj, ObjFmt& fmt);
    void output_calrgb(resources::CIECalRGBImpl& obj, ObjFmt& fmt);
    void output_calgray(resources::CIECalGrayImpl& obj, ObjFmt& fmt);
    void output_indexed(resources::PaletteImpl& obj, ObjFmt& fmt);
    bool before_output_indexed(resources::PaletteImpl& obj, ObjFmt& fmt);
    void output_iccbased(resources::ICCBasedImpl& obj, ObjFmt& fmt);
    void output_iccbased_dict(resources::ICCBasedImpl& obj, ObjFmt& fmt);
    bool before_output_iccbased(resources::ICCBasedImpl& obj, ObjFmt& fmt);

private:
    void prepare_inferior_cs(ColorSpaceHandle csh);
    void output_inferior_cs_ref(ColorSpaceHandle csh, ObjFmt& fmt);

private:
    DocWriterImpl&               m_doc;
    boost::shared_ptr<IndirectObjectCallback> m_iobj_callback;

    /// used by indexed cs, if not set then the cs base is a trivial cs
    IndirectObjectRef            m_inferior_cs;
    IndirectObjectRef            m_ref; // icc - data stream
};


}} //namespace jag::pdf

#endif //__COLORSPACE_H__JAG2301__

