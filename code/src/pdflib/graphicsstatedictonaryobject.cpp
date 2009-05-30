// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "graphicsstatedictonaryobject.h"
#include "objfmt.h"
#include "docwriterimpl.h"
#include "resourcemanagement.h"
#include "graphicsstatedictionary.h"
#include <core/generic/assert.h>

namespace jag {
namespace pdf {

// ctor
GraphicsStateDictionaryObject::GraphicsStateDictionaryObject(
      DocWriterImpl& doc
    , GraphicsStateDictionary const& gs_dict
)
    : IndirectObjectImpl(doc)
    , m_gs_dict(gs_dict)
{
}


//////////////////////////////////////////////////////////////////////////
void GraphicsStateDictionaryObject::on_output_definition()
{
    JAG_PRECONDITION(m_gs_dict.m_param_changed.any());

    ObjFmt& fmt(object_writer());
    fmt
        .dict_start()
        .dict_key("Type").output("ExtGState")
    ;

    if (m_gs_dict.m_param_changed[GraphicsStateDictionary::GS_ALPHA_IS_SHAPE])
        fmt.dict_key("AIS").space().output_bool(m_gs_dict.m_alpha_is_shape);

    if (m_gs_dict.m_param_changed[GraphicsStateDictionary::GS_STROKING_ALPHA])
        fmt.dict_key("CA").space().output(m_gs_dict.m_stroking_alpha);

    if (m_gs_dict.m_param_changed[GraphicsStateDictionary::GS_NONSTROKING_ALPHA])
        fmt.dict_key("ca").space().output(m_gs_dict.m_nonstroking_alpha);

    if (is_valid(m_transfer_fn))
        fmt.dict_key("TR").space().ref(m_transfer_fn);

    fmt.dict_end();
}



bool GraphicsStateDictionaryObject::on_before_output_definition()
{
    if (m_gs_dict.m_param_changed[GraphicsStateDictionary::GS_TRANSFER_FUNCTION])
        m_transfer_fn = doc().res_mgm().function_ref(m_gs_dict.m_transfer_fn);

    return true;
}

}} //namespace jag::pdf





















