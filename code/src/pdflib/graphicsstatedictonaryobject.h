// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __GRAPHICSSTATEDICTONARYOBJECT_H__JAG_1823__
#define __GRAPHICSSTATEDICTONARYOBJECT_H__JAG_1823__

#include "indirectobjectimpl.h"
#include "indirectobjectref.h"

namespace jag { namespace pdf
{
class DocWriterImpl;
class GraphicsStateDictionary;

/// outputs color space as an indirect object
class GraphicsStateDictionaryObject
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE
    GraphicsStateDictionaryObject(DocWriterImpl& doc, GraphicsStateDictionary const& gs_dict);

protected:
    void on_output_definition();
    bool on_before_output_definition();

private:
    GraphicsStateDictionary const& m_gs_dict;
    IndirectObjectRef              m_transfer_fn;
};


}} //namespace jag::pdf

#endif //__GRAPHICSSTATEDICTONARYOBJECT_H__JAG_1823__






















