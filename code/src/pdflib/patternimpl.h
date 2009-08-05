// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PATTERNIMPL_H_JG2027__
#define __PATTERNIMPL_H_JG2027__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "indirectobjectref.h"
#include "indirectobjectfwd.h"
#include "indirectobjectimpl.h"
#include <resources/interfaces/resourcehandle.h>
#include <interfaces/constants.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive/list.hpp>
#include <bitset>

namespace jag {
// fwd
class ICanvas;

namespace jstd
{
  class trans_affine_t;
}
namespace pdf {

// fwd
class DocWriterImpl;
class CanvasImpl;
class IIndirectObject;
class IndirectObjectRef;
class ObjFmt;

//
//
// 
class PatternBase
{
protected:    
    std::vector<double> m_matrix;
    std::string m_definition_string;
    
public:
    PatternBase(char const* def_string);
    char const* definition_string() const;
    std::vector<double> const& matrix() const;
    void matrix(jstd::trans_affine_t const& mtx);
};

// ---------------------------------------------------------------------------
//                         Tiling Pattern
//

//
//
//
class TilingPatternImpl
    : public IndirectObjectFwd
      // hook for an intrusive list in image manager
    , public boost::intrusive::list_base_hook<>
    , public PatternBase
{
public:
    TilingPatternImpl(DocWriterImpl& doc, Char const* pattern_str, ICanvas* canvas);
    ~TilingPatternImpl();
    bool is_colored() const;
    ICanvas* canvas() const;

public: // IIndirectObject
    DEFINE_VISITABLE;

public: // IndirectObjectFwd
    void on_before_output();

private:
    void on_write(ObjFmt& fmt);

private:
    enum PatternTilingType
    {
        TILING_CONSTANT_SPACING                    = 1,
        TILING_NO_DISTORTION                    = 2,
        TILING_CONSTANT_SPACING_FASTER_TILING    = 3
    };

    DocWriterImpl&        m_doc;
    CanvasImpl*         m_canvas;
    IndirectObjectRef    m_res_dict;

    PatternTilingType   m_tiling_type;
    Double                m_bbox[4];
    Double              m_step[2];
};



// ---------------------------------------------------------------------------
//                         Shading
//

//
//
//
class ShadingImpl
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE;
    ShadingImpl(DocWriterImpl& doc,
                Char const* pattern,
                ColorSpaceHandle cs,
                FunctionHandle const* fns,
                UInt num_functions);

protected:
    void on_output_definition();
    bool on_before_output_definition();

private:
    void output_functions();

private:
    enum {BIT_EXTEND, NUM_BITS};

    std::vector<FunctionHandle>     m_function_handles;
    std::vector<IndirectObjectRef>  m_function_refs;
    ColorSpaceHandle                m_cs;
    IndirectObjectRef               m_cs_ref;
    unsigned                        m_shading_type;
    std::vector<double>     m_bbox;
    std::vector<double>     m_background;
    std::vector<double>     m_domain; // size 2 or 4
    // axial radial
    std::vector<double>     m_coords;
    int                     m_extend[2];
    // function
    std::vector<double>     m_matrix_fun;
    std::bitset<NUM_BITS>   m_keys;
};


// ---------------------------------------------------------------------------
//                         Shading Pattern
//

//
//
//
class ShadingPatternImpl
    : public IndirectObjectImpl
    , public PatternBase
{
public:
    DEFINE_VISITABLE;
    ShadingPatternImpl(DocWriterImpl& doc,
                       Char const* pattern,
                       ShadingHandle handle);
    ShadingHandle shading_handle() const { return m_shading; }

protected:
    void on_output_definition();
    bool on_before_output_definition();

private:
    ShadingHandle           m_shading;
    IndirectObjectRef       m_shading_ref;
};


}} //namespace jag::pdf


#endif //__PATTERNIMPL_H_JG2027__


